# -*- coding: utf-8 -*-
"""
Qt-App 上传工具库

职责：
    - MD5 计算
    - update.json 生成
    - SFTP / HTTP PUT 上传（含进度显示）

入口：
    由 packaging/upload.py 调用。本文件只提供函数，不提供 CLI；
    需要单独上传单个文件时，请扩展 upload.py，而非在本文件加 CLI。
"""

import base64
import hashlib
import json
import logging
import os
import shutil
import time
from pathlib import Path
from typing import Callable, Iterable, Optional

import paramiko
import requests
from requests.adapters import HTTPAdapter
from urllib3.util.retry import Retry

# ---------------- 日志 ----------------
logging.basicConfig(
    level=logging.INFO,
    format="[%(asctime)s] %(levelname)s | %(message)s",
    datefmt="%H:%M:%S",
)
log = logging.getLogger("uploader")

MB = 1024 * 1024
DEFAULT_CHUNK = 8 * MB


# ---------------- 文件工具 ----------------
def remove(path: str) -> None:
    """安全删除文件或目录（不存在时静默）"""
    if os.path.isfile(path):
        os.remove(path)
    elif os.path.isdir(path):
        shutil.rmtree(path)


def calc_md5(path: str, chunk_size: int = DEFAULT_CHUNK) -> str:
    h = hashlib.md5()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(chunk_size), b""):
            h.update(chunk)
    return h.hexdigest()


def write_json_describe(
    version: str, md5_file_path: str, file_name: str, json_path: str
) -> None:
    """生成 update.json：version / buildVersion / name / md5"""
    md5 = calc_md5(md5_file_path)
    build = int(time.strftime("%Y%m%d"))
    with open(json_path, "w", encoding="utf-8") as f:
        json.dump(
            {
                "version": version,
                "buildVersion": build,
                "name": file_name,
                "md5": md5,
            },
            f,
            indent=4,
            ensure_ascii=False,
        )
    log.info(f"生成描述文件 {json_path}")


# ---------------- 进度显示 ----------------
def _format_bytes(n: int, precision: int = 2) -> str:
    value = float(n)
    for unit in ["B", "KB", "MB", "GB", "TB"]:
        if value < 1024:
            return f"{value:.{precision}f} {unit}"
        value /= 1024
    return f"{value:.2f} PB"


def _progress(total: int) -> Callable[[int], None]:
    """返回增量回调；每次上传新文件时新建一个，累加清零"""
    transferred = 0

    def callback(chunk_size: int) -> None:
        nonlocal transferred
        transferred += chunk_size
        percent = transferred / total * 100
        bar_len = 30
        bar = "#" * int(percent / 100 * bar_len)
        sys_stdout_write(
            f"\r[{bar:<{bar_len}}] {percent:6.2f}%  "
            f"{_format_bytes(transferred)}/{_format_bytes(total)}"
        )
        if transferred >= total:
            sys_stdout_write("\n")

    return callback


def sys_stdout_write(s: str) -> None:
    # 延迟 import，保持本文件顶层依赖最小
    import sys

    sys.stdout.write(s)
    sys.stdout.flush()


def _chunked_reader(
    path: Path, chunk_size: int, on_progress: Optional[Callable[[int], None]] = None
) -> Iterable[bytes]:
    """HTTP PUT 使用的分块生成器，内部自动累加进度"""
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(chunk_size), b""):
            yield chunk
            if on_progress:
                on_progress(len(chunk))


# ---------------- 上传函数 ----------------
def upload_http_put(
    host: str,
    port: int,
    username: str,
    password: str,
    remote_path: str,
    local_file: str,
    chunk_size: int = DEFAULT_CHUNK,
    timeout: int = 30,
) -> bool:
    filepath = Path(local_file)
    url = f"http://{host}:{port}{remote_path}"
    auth = base64.b64encode(f"{username}:{password}".encode()).decode()
    headers = {
        "User-Agent": "Uploader/1.0",
        "Content-Type": "application/octet-stream",
        "Overwrite": "T",
        "translate": "f",
        "Authorization": f"Basic {auth}",
    }

    session = requests.Session()
    retry = Retry(total=3, backoff_factor=1, status_forcelist=[502, 503, 504])
    session.mount("http://", HTTPAdapter(max_retries=retry))
    session.mount("https://", HTTPAdapter(max_retries=retry))

    try:
        total = filepath.stat().st_size
        callback = _progress(total)
        resp = session.put(
            url,
            data=_chunked_reader(filepath, chunk_size, callback),
            headers=headers,
            timeout=timeout,
            verify=False,
        )
        if resp.status_code == 201:
            log.info(f"[HTTP PUT] 上传完成 -> {url}")
            return True
        log.error(f"上传失败 {resp.status_code} {resp.reason}")
        return False
    except Exception as e:
        log.error(f"上传异常 {e}")
        return False


def upload_sftp(
    host: str,
    port: int,
    username: str,
    password: str,
    remote_path: str,
    local_file: str,
    chunk_size: int = DEFAULT_CHUNK,
    timeout: int = 30,
) -> bool:
    """单文件 SFTP 上传（每次新建连接）"""
    filepath = Path(local_file)
    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    try:
        ssh.connect(
            hostname=host,
            port=port,
            username=username,
            password=password,
            timeout=timeout,
        )
        sftp = ssh.open_sftp()

        total = filepath.stat().st_size
        callback = _progress(total)

        with filepath.open("rb") as fl, sftp.file(remote_path, "wb") as fr:
            for chunk in iter(lambda: fl.read(chunk_size), b""):
                fr.write(chunk)
                callback(len(chunk))

        log.info(f"[SFTP] 上传完成 -> {remote_path}")
        return True
    except Exception as e:
        log.error(f"上传异常 {e}")
        return False
    finally:
        ssh.close()


def upload_sftp_many(
    host: str,
    port: int,
    username: str,
    password: str,
    jobs: list[tuple[str, str]],
    chunk_size: int = DEFAULT_CHUNK,
    timeout: int = 30,
) -> bool:
    """
    一次 SSH 连接上传多个文件，减少握手开销。
    jobs: [(remote_path, local_file), ...]
    """
    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    try:
        ssh.connect(
            hostname=host,
            port=port,
            username=username,
            password=password,
            timeout=timeout,
        )
        sftp = ssh.open_sftp()

        for remote_path, local_file in jobs:
            filepath = Path(local_file)
            if not filepath.exists():
                log.error(f"[SFTP] 本地文件不存在: {local_file}")
                return False

            total = filepath.stat().st_size
            callback = _progress(total)
            log.info(f"[SFTP] 上传 -> {remote_path}")

            with filepath.open("rb") as fl, sftp.file(remote_path, "wb") as fr:
                for chunk in iter(lambda: fl.read(chunk_size), b""):
                    fr.write(chunk)
                    callback(len(chunk))

        log.info(f"[SFTP] 全部上传完成，共 {len(jobs)} 个文件")
        return True
    except Exception as e:
        log.error(f"上传异常 {e}")
        return False
    finally:
        ssh.close()
