# Windows 本机开发脚本

两个脚本，都复用 `packaging/windows/Enter-VsDevShell.ps1` 进入 MSVC 环境。

## dev.ps1 —— 配置 / 编译 / 运行 / 测试

```powershell
.\scripts\windows\dev.ps1 -Action configure                     # 配置，生成 build\compile_commands.json
.\scripts\windows\dev.ps1                                       # 编译（默认 -Action build）
.\scripts\windows\dev.ps1 -Action build -Target extensionsystem
.\scripts\windows\dev.ps1 -Action test                          # 构建并跑 ctest
.\scripts\windows\dev.ps1 -Action run -AppArgs '--console','--log-level=debug'
```

参数默认值就是本机环境，需要时再覆盖：

| 参数         | 默认值                                            |
| ------------ | ------------------------------------------------- |
| `-Action`    | `build`（`configure` / `build` / `run` / `test`） |
| `-Target`    | `all`（`clean`、`extensionsystem`、`Qt-App` 等）  |
| `-BuildType` | `Debug`                                           |
| `-BuildDir`  | `build`                                           |
| `-QtDir`     | `C:\Qt\6.12.0\msvc2022_64`                        |
| `-AppArgs`   | 空，仅 `run` 使用                                 |

产物在 `binaries/cmake/x64/bin/<config>/`：`Qt-App.exe`、`plugins/qt-app/`、`resources/`、`translations/`。

## format.ps1 —— clang-format

```powershell
.\scripts\windows\format.ps1                  # 格式化 src 与 tests
.\scripts\windows\format.ps1 -Check           # 只检查，有差异时非零退出（CI 用这个）
.\scripts\windows\format.ps1 -Path src\plugins -IncludeVendored
```

| 参数               | 默认值                                        |
| ------------------ | --------------------------------------------- |
| `-Path`            | `src`、`tests`                                |
| `-ClangFormat`     | Qt Creator 自带的 clang-format，找不到用 PATH |
| `-Check`           | 关，开启后不修改文件                          |
| `-IncludeVendored` | 关，开启后一并格式化 `src/3rdparty`           |

风格取自仓库根的 `.clang-format`。

## 编码约定：`.ps1` 一律纯 ASCII（UTF-8 无 BOM）

Windows PowerShell 5.1 对**无 BOM** 的脚本按系统 ANSI 码页（本机 936/GBK）解码，
脚本里写 UTF-8 中文会被当乱码并直接导致**解析失败**（PS 7 无此问题）。

因此仓库里的 `.ps1` 全部只用 ASCII：注释和提示信息写英文，中文说明放在 Markdown 里。
新增脚本时请沿用该约定；加中文注释就必须同时加 UTF-8 BOM，两者只能选一个。

自查（应无任何输出）：

```powershell
Get-ChildItem . -Recurse -Filter *.ps1 | ForEach-Object {
  $e = $null
  [void][System.Management.Automation.Language.Parser]::ParseFile($_.FullName, [ref]$null, [ref]$e)
  if ($e) { "$($_.FullName): $($e[0].Message)" }
}
```

## 其它说明

- `configure` / `build` / `test` 都会进 DevShell；`run` 只把 `QtDir\bin` 加进 `PATH` 后启动 exe。
- `test` 会在同一个构建目录里补一次 `-DBUILD_TESTS=ON`，所以跑完再 `build` 不会回到「不含测试」的状态；不需要测试时删掉 `build\` 重新 `configure`。
- 改动 `CMakeLists.txt` 后直接 `build` 也会触发 CMake 自动重新配置。
- 打包流程见 `packaging/windows/package.ps1`（同样遵守上述 ASCII 约定）。
