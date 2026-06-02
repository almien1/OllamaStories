#define MyAppName "Llama Stories"
#define MyAppVersion "0.1"
#define MyAppExeName "LlamaStories.exe"

[Setup]
AppId={{AE858D9A-46D7-48C6-9122-DFD46C9A5806}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
DefaultDirName={autopf}\{#MyAppName}
DisableDirPage=yes
UninstallDisplayIcon={app}\{#MyAppExeName}
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
OutputDir=output
OutputBaseFilename=LlamaStories
SolidCompression=yes
WizardStyle=modern dynamic

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "build\Desktop_Qt_6_11_1_MinGW_64_bit-Debug\debug\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\6.11.1\mingw_64\bin\Qt6Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\6.11.1\mingw_64\bin\Qt6Gui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\6.11.1\mingw_64\bin\Qt6Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\6.11.1\mingw_64\bin\libgcc_s_seh-1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\6.11.1\mingw_64\bin\libstdc++-6.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\6.11.1\mingw_64\bin\libwinpthread-1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\6.11.1\mingw_64\plugins\platforms\qwindows.dll"; DestDir: "{app}/platforms"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

