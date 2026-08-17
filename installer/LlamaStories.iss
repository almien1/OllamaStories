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
Source: "..\output\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\installer\libs\Qt6Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\installer\libs\Qt6Gui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\installer\libs\Qt6Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\installer\libs\Qt6Network.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\installer\libs\libgcc_s_seh-1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\installer\libs\libstdc++-6.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\installer\libs\libwinpthread-1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\installer\libs\qwindows.dll"; DestDir: "{app}/platforms"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

