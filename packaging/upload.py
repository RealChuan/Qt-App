# -*- coding: utf-8 -*-
"""
通用 SFTP 上传脚本
支持平台：windows_x86、windows_x64、macos_x64、macos_arm64、ubuntu_x64、ubuntu_arm64
用法：
    python upload.py windows_x86
    python upload.py windows_x64
    ...
"""

import argparse
import os
import time

import utils

# -------------------- 配置区 --------------------
APP_NAME = "Qt-App"
VERSION = "0.2.1"
CHANNEL = "stable"
BUILD_DATE = time.strftime("%Y%m%d", time.localtime())

# 本地 releases 目录
RELEASES_PATH = os.path.join(os.path.dirname(__file__), "releases")

# 远端公共根目录
REMOTE_ROOT = "/mnt/nas/data/homes/admin/packages/Qt-App"

# 各平台配置
PLATFORMS = {
    "windows_x86": {
        "platform": "windows",
        "arch": "x86",
        "src_file": f"{APP_NAME}_installer.exe",
        "dst_tpl": f"{APP_NAME}_{VERSION}_{BUILD_DATE}_{{arch}}.exe",
        "remote": f"{REMOTE_ROOT}/windows/{{arch}}",
    },
    "windows_x64": {
        "platform": "windows",
        "arch": "x64",
        "src_file": f"{APP_NAME}_installer_x64.exe",
        "dst_tpl": f"{APP_NAME}_{VERSION}_{BUILD_DATE}_{{arch}}.exe",
        "remote": f"{REMOTE_ROOT}/windows/{{arch}}",
    },
    "windows_arm64": {
        "platform": "windows",
        "arch": "arm64",
        "src_file": f"{APP_NAME}_installer_arm64.exe",
        "dst_tpl": f"{APP_NAME}_{VERSION}_{BUILD_DATE}_{{arch}}.exe",
        "remote": f"{REMOTE_ROOT}/windows/{{arch}}",
    },
    "macos_x64": {
        "platform": "macos",
        "arch": "x64",
        "src_file": f"{APP_NAME}.dmg",
        "dst_tpl": f"{APP_NAME}_{VERSION}_{BUILD_DATE}_{{arch}}.dmg",
        "remote": f"{REMOTE_ROOT}/macos/{{arch}}",
    },
    "macos_arm64": {
        "platform": "macos",
        "arch": "arm64",
        "src_file": f"{APP_NAME}.dmg",
        "dst_tpl": f"{APP_NAME}_{VERSION}_{BUILD_DATE}_{{arch}}.dmg",
        "remote": f"{REMOTE_ROOT}/macos/{{arch}}",
    },
    "macos_universal": {
        "platform": "macos",
        "arch": "universal",
        "src_file": f"{APP_NAME}.dmg",
        "dst_tpl": f"{APP_NAME}_{VERSION}_{BUILD_DATE}_{{arch}}.dmg",
        "remote": f"{REMOTE_ROOT}/macos/{{arch}}",
    },
    "ubuntu_x64": {
        "platform": "ubuntu",
        "arch": "x64",
        "src_file": f"{APP_NAME}.deb",
        "dst_tpl": f"{APP_NAME}_{VERSION}_{BUILD_DATE}_{{arch}}.deb",
        "remote": f"{REMOTE_ROOT}/ubuntu/{{arch}}",
    },
    "ubuntu_arm64": {
        "platform": "ubuntu",
        "arch": "arm64",
        "src_file": f"{APP_NAME}.deb",
        "dst_tpl": f"{APP_NAME}_{VERSION}_{BUILD_DATE}_{{arch}}.deb",
        "remote": f"{REMOTE_ROOT}/ubuntu/{{arch}}",
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
    arch = cfg["arch"]

    # 版本号非法时客户端会把新包误判为旧包，先拦在重命名之前
    utils.validate_version(VERSION)

    src_path = os.path.join(RELEASES_PATH, cfg["src_file"])
    dst_name = cfg["dst_tpl"].format(arch=arch)
    dst_path = os.path.join(RELEASES_PATH, dst_name)
    remote_dir = cfg["remote"].format(arch=arch)

    if not os.path.isfile(src_path):
        raise FileNotFoundError(f"未找到安装包: {src_path}，请先执行打包流程。")

    # 若目标文件已存在，先删除（utils.remove 内部已做存在性检查）
    utils.remove(dst_path)
    # 重命名
    os.rename(src_path, dst_path)

    # 生成 update.json
    json_path = os.path.join(RELEASES_PATH, "update.json")
    utils.write_json_describe(
        VERSION,
        dst_path,
        cfg["platform"],
        cfg["arch"],
        json_path,
        product=APP_NAME,
        channel=CHANNEL,
    )

    # 一次 SSH 连接上传主包 + update.json
    ok = utils.upload_sftp_many(
        host=SFTP_SERVER,
        port=SFTP_PORT,
        username=SFTP_USER,
        password=SFTP_PWD,
        jobs=[
            (f"{remote_dir}/{dst_name}", dst_path),
            (f"{remote_dir}/update.json", json_path),
        ],
    )
    if not ok:
        raise RuntimeError(f"[{platform_key}] 上传失败")

    print(f"[{platform_key}] 上传完成 ✔")


def main() -> None:
    parser = argparse.ArgumentParser(description="通用 SFTP 上传脚本")
    parser.add_argument(
        "platform",
        choices=list(PLATFORMS.keys()),
        help="要上传的平台标识",
    )
    args = parser.parse_args()
    upload_platform(args.platform)


if __name__ == "__main__":
    main()
