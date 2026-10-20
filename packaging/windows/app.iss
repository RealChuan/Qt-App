; ============================================================
; Qt-App Inno Setup 脚本
;   架构选择：默认 x64
;     ISCC /DMyArch=x64   app.iss
;     ISCC /DMyArch=arm64 app.iss
;   [Files] 一行通配符：packet 目录已由 package.ps1 整理完毕
;   中文翻译：.\ChineseSimplified.isl（package.ps1 保证其就位）
; ============================================================
#ifndef MyArch
  #define MyArch "x64"
#endif

#if MyArch == "arm64"
  #define MyOutputSuffix "_arm64"
  #define MyVcRedist     "vc_redist.arm64.exe"
  ; arm64 独立 AppId（与 x64 可共存）；如需覆盖安装，改成和 x64 相同的 GUID
  #define MyAppId "B4A6E2D1-8C3F-4F1A-9A21-7E8E9D5C6B10"
#else
  #define MyOutputSuffix "_x64"
  #define MyVcRedist     "vc_redist.x64.exe"
  #define MyAppId "F9328618-8855-4E59-BD15-12C77B74E3C0"
#endif

#define MyAppName        "Qt-App"
; 版本号由 package.ps1 通过 /DMyAppVersion=... /DMyAppFileVersion=... 注入，
; 唯一来源是 packaging/version.txt；下面只是单独运行 ISCC 时的兜底值。
#ifndef MyAppVersion
#define MyAppVersion "0.0.0"
#endif
#ifndef MyAppFileVersion
#define MyAppFileVersion "0.0.0.0"
#endif
#define MyAppPublisher   "The Youth."
#define MyAppURL         "https://github.com/RealChuan/Qt-App"
#define MyAppExeName     MyAppName + ".exe"
#define MyAppAssocName   MyAppName + ""
#define MyAppAssocExt    ".myp"
#define MyAppAssocKey    StringChange(MyAppAssocName, " ", "") + MyAppAssocExt
#define MyAppUninstallName "Uninstall"
#define MyCrashAppName   "CrashReport"
#define MyCrashAppExeName "CrashReport.exe"

[Setup]
AppId={{{#MyAppId}}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
ChangesAssociations=yes
DisableProgramGroupPage=yes
LicenseFile=..\..\LICENSE
InfoAfterFile=..\..\README.md
PrivilegesRequiredOverridesAllowed=dialog
OutputDir=..\releases
OutputBaseFilename={#MyAppName}_{#MyAppVersion}_installer{#MyOutputSuffix}
SetupIconFile=..\..\src\apps\app\app.ico
UninstallDisplayIcon={app}\{#MyAppExeName}
Compression=lzma
SolidCompression=yes
WizardStyle=modern
MinVersion=10.0
DefaultGroupName={#MyAppName}
Uninstallable=yes
UninstallDisplayName={#MyAppName}

; --- 版本信息 ---
VersionInfoCompany={#MyAppPublisher}
VersionInfoCopyright={#MyAppPublisher}
VersionInfoDescription={#MyAppName}
VersionInfoOriginalFileName={#MyAppExeName}
VersionInfoVersion={#MyAppFileVersion}
VersionInfoProductVersion={#MyAppFileVersion}
VersionInfoProductTextVersion={#MyAppVersion}
VersionInfoTextVersion={#MyAppVersion}

ShowLanguageDialog=auto
; 安装包自身签名（可选）—— packet 内 exe/dll 已在 package.ps1 里签过，此处不启用
; SignTool = sha256
; SignedUninstaller = yes

; --- 架构声明 ---
#if MyArch == "arm64"
ArchitecturesAllowed=arm64
ArchitecturesInstallIn64BitMode=arm64
#else
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
#endif

CloseApplications=yes
RestartApplications=no

[Languages]
Name: "english";          MessagesFile: "compiler:Default.isl"
Name: "chinesesimplified"; MessagesFile: ".\ChineseSimplified.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: checkablealone
Name: "vcredist";    Description: "Install Microsoft Visual C++ Redistributable"; GroupDescription: "Additional Runtimes"; Flags: unchecked

[Files]
; ------------------------------------------------------------
; 1. packet 目录：由 package.ps1 整理完毕（编译产物 + windeployqt 部署 + 字体 + i18n），
;    一行通配符保持目录结构原样安装。
;    注意：Qt-App.exe / CrashReport.exe 的签名已在 package.ps1 中完成，
;          本 [Files] 不加 sign / signonce flag，避免重复签名浪费时间戳。
; ------------------------------------------------------------
Source: "..\packet\{#MyArch}\*"; DestDir: "{app}"; \
    Flags: ignoreversion recursesubdirs createallsubdirs

; ------------------------------------------------------------
; 2. VC / UCRT 运行时（备用方案，当前注释）
;    现在依赖 windeployqt --compiler-runtime 将运行库拷入 packet；
;    若 windeployqt 未能拷贝（个别 Qt 版本 / 精简 SDK），
;    可取消对应架构分支的注释，手动从系统路径拉取。
;    注意：路径中的 VS 版本号与 Windows SDK 版本号需按实际环境调整。
; ------------------------------------------------------------
#if MyArch == "arm64"
; Source: "C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\VC\Redist\MSVC\14.44.35208\arm64\Microsoft.VC143.CRT\*"; DestDir: "{app}"; Flags: ignoreversion createallsubdirs recursesubdirs
; Source: "C:\Program Files (x86)\Windows Kits\10\Redist\10.0.26100.0\ucrt\DLLs\arm64\*"; DestDir: "{app}"; Flags: ignoreversion createallsubdirs recursesubdirs
#else
; Source: "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Redist\MSVC\14.29.30133\x86\Microsoft.VC142.CRT\*"; DestDir: "{app}"; Flags: ignoreversion createallsubdirs recursesubdirs
; Source: "C:\Program Files (x86)\Windows Kits\10\Redist\10.0.19041.0\ucrt\DLLs\x86\*"; DestDir: "{app}"; Flags: ignoreversion createallsubdirs recursesubdirs
#endif

[Registry]
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocExt}\OpenWithProgids"; ValueType: string; ValueName: "{#MyAppAssocKey}"; ValueData: ""; Flags: uninsdeletevalue
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}"; ValueType: string; ValueName: ""; ValueData: "{#MyAppAssocName}"; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""
Root: HKA; Subkey: "Software\Classes\Applications\{#MyAppExeName}\SupportedTypes"; ValueType: string; ValueName: ".myp"; ValueData: ""

Root: HKA; Subkey: "SOFTWARE\Classes\CLSID\{{{#MyAppId}}"; ValueType: string; ValueData: "{#MyAppName}"; Flags: uninsdeletekey
Root: HKA; Subkey: "SOFTWARE\Classes\CLSID\{{{#MyAppId}}\DefaultIcon"; ValueType: string; ValueData: """{app}\{#MyAppExeName}"""; Flags: uninsdeletekey
Root: HKA; Subkey: "SOFTWARE\Classes\CLSID\{{{#MyAppId}}\shell\Open\Command"; ValueType: string; ValueData: """{app}\{#MyAppExeName}"" ""%1"""; Flags: uninsdeletekey
Root: HKA; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\MyComputer\NameSpace\{{{#MyAppId}}"; ValueType: string; ValueData: "{#MyAppName}"; Flags: uninsdeletekey

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:ProgramOnTheWeb,{#MyAppName}}"; Filename: "{#MyAppURL}"
Name: "{group}\{#MyAppUninstallName}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyVcRedist}"; Parameters: "/install /quiet /norestart"; StatusMsg: "Installing Microsoft Visual C++ Runtime ..."; Flags: runhidden waituntilterminated skipifdoesntexist; Tasks: vcredist
Filename: "{sys}\netsh.exe"; Parameters: "advfirewall firewall delete rule name=""{#MyAppName}"" program=""{app}\{#MyAppExeName}"" "; Flags: runhidden
Filename: "{sys}\netsh.exe"; Parameters: "advfirewall firewall add rule name=""{#MyAppName}"" program=""{app}\{#MyAppExeName}"" dir=in action=allow enable=yes"; Flags: runhidden
Filename: "{sys}\netsh.exe"; Parameters: "advfirewall firewall delete rule name=""{#MyCrashAppName}"" program=""{app}\{#MyCrashAppExeName}"" "; Flags: runhidden
Filename: "{sys}\netsh.exe"; Parameters: "advfirewall firewall add rule name=""{#MyCrashAppName}"" program=""{app}\{#MyCrashAppExeName}"" dir=in action=allow enable=yes"; Flags: runhidden
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[UninstallRun]
Filename: "{sys}\netsh.exe"; Parameters: "advfirewall firewall delete rule name=""{#MyAppName}"" program=""{app}\{#MyAppExeName}"""; Flags: runhidden; RunOnceId: "DelQtAppFirewallRule"
Filename: "{sys}\netsh.exe"; Parameters: "advfirewall firewall delete rule name=""{#MyCrashAppName}"" program=""{app}\{#MyCrashAppExeName}"""; Flags: runhidden; RunOnceId: "DelCrashAppFirewallRule"

[UninstallDelete]
Type: filesandordirs; Name: "{app}\_"
Type: filesandordirs; Name: "{localappdata}\Youth\{#MyAppName}"
Type: filesandordirs; Name: "{localappdata}\Youth\{#MyCrashAppName}"

[Code]
function IsProcessRunning(const ExeName: string): Boolean;
var
  ResultCode: Integer;
begin
  Result := Exec(
    ExpandConstant('{cmd}'),
    '/C tasklist /FI "IMAGENAME eq ' + ExeName + '" | find /I "' + ExeName + '" > nul',
    '', SW_HIDE, ewWaitUntilTerminated, ResultCode) and (ResultCode = 0);
end;

function KillProcess(const ExeName: string): Boolean;
var
  ResultCode: Integer;
begin
  Result := Exec(
    ExpandConstant('{cmd}'),
    '/C taskkill /F /IM "' + ExeName + '"',
    '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  // taskkill 对不存在的进程通常返回 128，也视为成功
  Result := Result and ((ResultCode = 0) or (ResultCode = 128));
end;

function StopProcesses: Boolean;
begin
  Result := True;
  if IsProcessRunning('{#MyAppExeName}') then
    if not KillProcess('{#MyAppExeName}') then Result := False;
  if IsProcessRunning('{#MyCrashAppExeName}') then
    if not KillProcess('{#MyCrashAppExeName}') then Result := False;
  if IsProcessRunning('crashpad_handler.exe') then
    if not KillProcess('crashpad_handler.exe') then Result := False;
end;

function PrepareToInstall(var NeedsRestart: Boolean): String;
begin
  Result := '';
  if not StopProcesses then
    Result := '无法关闭正在运行的 {#MyAppName} 相关进程，请手动关闭后重试。';
end;

function InitializeUninstall(): Boolean;
begin
  Result := StopProcesses;
  if not Result then
    MsgBox('无法关闭正在运行的 {#MyAppName} 相关进程，请手动关闭后重试。', mbError, MB_OK);
end;

{ ============================================================
  安装前清理旧的安装目录
  ------------------------------------------------------------
  背景：Inno 默认只覆盖同名文件，旧版独有文件会残留，
        版本升级时可能导致运行时行为异常。
  保留：unins*（卸载程序）、_（用户数据目录）
  ============================================================ }
function ShouldKeepOldItem(const Name: string): Boolean;
var
  LowerName: string;
begin
  LowerName := LowerCase(Name);

  if Pos('unins', LowerName) = 1 then
  begin
    Result := True;
    Exit;
  end;

  if LowerName = '_' then
  begin
    Result := True;
    Exit;
  end;

  Result := False;
end;

procedure ClearInstallDir();
var
  FindRec: TFindRec;
  BaseDir: string;
  FullPath: string;
begin
  BaseDir := ExpandConstant('{app}');
  if not DirExists(BaseDir) then Exit;

  if FindFirst(BaseDir + '\*', FindRec) then
  begin
    try
      repeat
        if (FindRec.Name = '.') or (FindRec.Name = '..') then Continue;
        if ShouldKeepOldItem(FindRec.Name) then Continue;

        FullPath := BaseDir + '\' + FindRec.Name;

        if (FindRec.Attributes and FILE_ATTRIBUTE_DIRECTORY) <> 0 then
          DelTree(FullPath, True, True, True)
        else
          DeleteFile(FullPath);
      until not FindNext(FindRec);
    finally
      FindClose(FindRec);
    end;
  end;

  Log('ClearInstallDir: 已清理 ' + BaseDir);
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssInstall then
    ClearInstallDir();
end;
