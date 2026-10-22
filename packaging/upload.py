# -*- coding: utf-8 -*-
"""
Qt-App SFTP 上传脚本（全平台）

用法：
    python upload.py windows_x64
    python upload.py windows_arm64
    python upload.py macos_x86_64
    python upload.py macos_aarch64
    python upload.py macos_universal
    python upload.py ubuntu_x86_64
    python upload.py ubuntu_aarch64

前置：
    pip install -r packaging/requirements.txt
    # 先生成 packaging/releases 下对应平台的安装包
    # （Windows 由 package.ps1 产出，macOS/Ubuntu 由各自 package.sh 产出）

说明：
    - src_file 为各平台构建产物名，与构建脚本的输出严格对齐
    - dst_tpl 为上传到 NAS 的稳定名，带版本号与构建日期
    - macOS universal 与 x86_64 / aarch64 共用 src_file "Qt-App.dmg"，
      因为每次只上传一个平台产物；跨平台批量上传请分多次调用
"""

import os
import time
import argparse

import utils

# -------------------- 配置区 --------------------
APP_NAME = "Qt-App"
VERSION = "0.2.1"
BUILD_DATE = time.strftime("%Y%m%d", time.localtime())

# 本地 releases 目录（与 app.iss / package.sh 的产物输出目录一致）
RELEASES_PATH = os.path.join(os.path.dirname(__file__), "releases")

# 远端公共根目录
REMOTE_ROOT = "/mnt/nas/data/homes/admin/packages/qt-app"

PLATFORMS = {
    # ---- Windows ----
    "windows_x64": {
        "src_file": f"{APP_NAME}_installer_x64.exe",
        "dst_tpl": f"{APP_NAME}_{VERSION}_{BUILD_DATE}_x64.exe",
        "remote": f"{REMOTE_ROOT}/windows/x64",
    },
    "windows_arm64": {
        "src_file": f"{APP_NAME}_installer_arm64.exe",
        "dst_tpl": f"{APP_NAME}_{VERSION}_{BUILD_DATE}_arm64.exe",
        "remote": f"{REMOTE_ROOT}/windows/arm64",
    },
    # ---- macOS ----
    "macos_x86_64": {
        "src_file": f"{APP_NAME}.dmg",
        "dst_tpl": f"{APP_NAME}_{VERSION}_{BUILD_DATE}_x86_64.dmg",
        "remote": f"{REMOTE_ROOT}/macos/x86_64",
    },
    "macos_aarch64": {
        "src_file": f"{APP_NAME}.dmg",
        "dst_tpl": f"{APP_NAME}_{VERSION}_{BUILD_DATE}_aarch64.dmg",
        "remote": f"{REMOTE_ROOT}/macos/aarch64",
    },
    "macos_universal": {
        "src_file": f"{APP_NAME}.dmg",
        "dst_tpl": f"{APP_NAME}_{VERSION}_{BUILD_DATE}_universal.dmg",
        "remote": f"{REMOTE_ROOT}/macos/universal",
    },
    # ---- Ubuntu ----
    "ubuntu_x86_64": {
        "src_file": f"{APP_NAME}.deb",
        "dst_tpl": f"{APP_NAME}_{VERSION}_{BUILD_DATE}_x86_64.deb",
        "remote": f"{REMOTE_ROOT}/ubuntu/x86_64",
    },
    "ubuntu_aarch64": {
        "src_file": f"{APP_NAME}.deb",
        "dst_tpl": f"{APP_NAME}_{VERSION}_{BUILD_DATE}_aarch64.deb",
        "remote": f"{REMOTE_ROOT}/ubuntu/aarch64",
    },
}

# SFTP 连接信息（优先环境变量，未设置时使用默认值）
SFTP_SERVER = os.environ.get("SFTP_SERVER", "10.1.1.127")
SFTP_PORT = int(os.environ.get("SFTP_PORT", "22"))
SFTP_USER = os.environ.get("SFTP_USER", "root")
SFTP_PWD = os.environ.get("SFTP_PWD", "123456")


# -------------------- 逻辑区 --------------------
def upload_platform(platform_key: str) -> None:
    if platform_key not in PLATFORMS:
        raise ValueError(f"未知的 platform_key: {platform_key}")

    cfg = PLATFORMS[platform_key]

    src_path = os.path.join(RELEASES_PATH, cfg["src_file"])
    dst_name = cfg["dst_tpl"]
    dst_path = os.path.join(RELEASES_PATH, dst_name)

    if not os.path.isfile(src_path):
        raise FileNotFoundError(f"未找到安装包: {src_path}，请先执行该平台打包流程。")

    # 若目标文件已存在，先删除（utils.remove 内部已做存在性检查）
    utils.remove(dst_path)
    # 重命名为带版本号 + 日期的稳定名
    os.rename(src_path, dst_path)

    # 生成 update.json（含 version / buildVersion / name / md5）
    json_path = os.path.join(RELEASES_PATH, "update.json")
    utils.write_json_describe(VERSION, dst_path, dst_name, json_path)

    # 一次 SSH 连接上传主包 + update.json
    ok = utils.upload_sftp_many(
        host=SFTP_SERVER,
        port=SFTP_PORT,
        username=SFTP_USER,
        password=SFTP_PWD,
        jobs=[
            (f"{cfg['remote']}/{dst_name}", dst_path),
            (f"{cfg['remote']}/update.json", json_path),
        ],
    )
    if not ok:
        raise RuntimeError(f"[{platform_key}] 上传失败")

    print(f"[{platform_key}] 上传完成 ✔")
    print(f"  本地: {dst_path}")
    print(f"  远端: {cfg['remote']}/{dst_name}")
    print(f"  清单: {cfg['remote']}/update.json")


def main() -> None:
    parser = argparse.ArgumentParser(description="Qt-App SFTP 上传脚本（全平台）")
    parser.add_argument(
        "platform",
        choices=list(PLATFORMS.keys()),
        help="要上传的平台标识",
    )
    args = parser.parse_args()
    upload_platform(args.platform)


if __name__ == "__main__":
    main()
