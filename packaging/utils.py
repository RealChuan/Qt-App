# -*- coding: utf-8 -*-
"""打包发布的基础能力：清单生成、文件摘要、目录处理与 SFTP 分块上传。

产品名、版本、平台表、服务器地址等具体值由调用方传入。
"""

import hashlib
import json
import logging
import os
import posixpath
import re
import shutil
import sys
import time
from pathlib import Path
from typing import Callable

import paramiko

logging.basicConfig(
    level=logging.INFO,
    format="[%(asctime)s] %(levelname)s | %(message)s",
    datefmt="%H:%M:%S",
)
log = logging.getLogger("uploader")

MB = 1024 * 1024
DEFAULT_CHUNK = 8 * MB


def remove(path: str) -> None:
    """安全删除文件或目录（不存在时静默）"""
    if os.path.isfile(path):
        os.remove(path)
    elif os.path.isdir(path):
        shutil.rmtree(path)


def _format_bytes(n: int, precision: int = 2) -> str:
    value = float(n)  # 用浮点副本做除法
    for unit in ["B", "KB", "MB", "GB", "TB"]:
        if value < 1024:
            return f"{value:.{precision}f} {unit}"
        value /= 1024
    return f"{value:.2f} PB"


def calc_md5(path: str, chunk_size: int = DEFAULT_CHUNK) -> str:
    h = hashlib.md5()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(chunk_size), b""):
            h.update(chunk)
    return h.hexdigest()


def calc_sha256(path: str, chunk_size: int = DEFAULT_CHUNK) -> str:
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(chunk_size), b""):
            h.update(chunk)
    return h.hexdigest()


def validate_version(version: str) -> None:
    """版本号必须形如 3.0.9；带后缀会让客户端把新包误判为旧包。"""
    if not re.fullmatch(r"\d+\.\d+\.\d+", version):
        raise ValueError(f"版本号必须形如 3.0.9，当前为 {version!r}")


SCHEMA_VERSION = 2


def write_json_describe(
    version: str,
    package_path: str,
    platform: str,
    arch: str,
    json_path: str,
    product: str,
    channel: str,
) -> None:
    """生成与安装包同目录的升级描述文件，字段随包文件现算。"""
    validate_version(version)
    package = Path(package_path)
    describe = {
        "schemaVersion": SCHEMA_VERSION,
        "product": product,
        "platform": platform,
        "arch": arch,
        "channel": channel,
        "version": version,
        "releaseDate": time.strftime("%Y-%m-%d"),
        "fileName": package.name,
        "size": package.stat().st_size,
        "sha256": calc_sha256(str(package)),
        "md5": calc_md5(str(package)),
    }
    with open(json_path, "w", encoding="utf-8") as f:
        json.dump(describe, f, indent=4, ensure_ascii=False)
        f.write("\n")
    log.info(f"生成描述文件 {json_path}")


def _progress(total: int) -> Callable[[int], None]:
    """返回回调，内部自动累计，上传新文件时重新调用即可清零"""
    transferred = 0

    def callback(chunk_size: int) -> None:
        nonlocal transferred
        transferred += chunk_size
        percent = transferred / total * 100
        bar_len = 30
        bar = "#" * int(percent / 100 * bar_len)
        sys.stdout.write(
            f"\r[{bar:<{bar_len}}] {percent:6.2f}%  "
            f"{_format_bytes(transferred)}/{_format_bytes(total)}"
        )
        if transferred >= total:
            sys.stdout.write("\n")
        sys.stdout.flush()

    return callback


def ensure_remote_dir(sftp: paramiko.SFTPClient, remote_dir: str) -> None:
    """逐级创建远端目录，已存在则跳过。"""
    path = ""
    for part in remote_dir.split("/"):
        if not part:
            continue
        path += "/" + part
        try:
            sftp.stat(path)
        except OSError:
            sftp.mkdir(path)


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

            ensure_remote_dir(sftp, posixpath.dirname(remote_path))

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
