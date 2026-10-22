#!/bin/bash -ex
# ============================================================
# 平铺 download-artifact 解出来的产物目录
#
#   releases/<artifact 名>/<文件>  ->  releases/<文件>
#
# GitHub Release 上传时按「文件名」去重（同名后传的会覆盖先传的），
# 所以先做重名检查，冲突就整步失败，避免安静地丢掉某个平台的包。
# ============================================================

cd "$(dirname "$0")"
cd ..
releases_dir="$(pwd)/packaging/releases"
cd "$releases_dir"

mapfile -t files < <(find . -mindepth 2 -type f | sort)
[ ${#files[@]} -gt 0 ] || {
	echo "releases 下没有可平铺的文件" >&2
	exit 1
}

declare -A seen
conflicts=()
for f in "${files[@]}"; do
	name="$(basename "$f")"
	if [ -n "${seen[$name]:-}" ]; then
		conflicts+=("$name  <=  ${seen[$name]}  与  $f")
	elif [ -e "./$name" ]; then
		conflicts+=("$name  <=  releases 根目录已存在同名文件")
	fi
	seen["$name"]="$f"
done

if [ ${#conflicts[@]} -gt 0 ]; then
	echo "以下文件重名，上传 Release 时会互相覆盖：" >&2
	printf '  %s\n' "${conflicts[@]}" >&2
	exit 1
fi

for f in "${files[@]}"; do
	mv -v "$f" .
done

# 清理搬空后的 artifact 目录（含多层空目录）
find . -mindepth 1 -type d -empty -delete

ls -lh .
