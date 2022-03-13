; Will be replaced by the release script
#define ProjName "qlogexplorer"
#define AppName "QLogExplorer"
#define AppVersion "0.0.1"
#define AppURL "https://rafaelfassi.github.io/qlogexplorer/"
; Calculated and fixed values
#define AppAssocExt ".log"
#define AppPublisher AppName + " Project"
#define AppAssocKey AppName + ".id"
#define AppExeName ProjName + ".exe"
#define AppAssocName AppName
; Needs to be set manually before running innosetup
#define SrcDir "<project-source-folder>"
#define BuildBinDir "<build-folder>"
#define QtDir "<qt-folder>"
#define PlatformType "Win64"

[Setup]
AppId={{52CADC64-418D-43B8-8A1A-D5C47619D786}
AppName={#AppName}
AppVersion={#AppVersion}
;AppVerName={#AppName} {#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}
AppUpdatesURL={#AppURL}
DefaultDirName={autopf}\{#AppName}
ChangesAssociations=yes
DisableProgramGroupPage=yes
LicenseFile={#SrcDir}\LICENSE
; Remove the following line to run in administrative install mode (install for all users.)
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
OutputBaseFilename={#AppName}-{#AppVersion}-{#PlatformType}
SetupIconFile={#SrcDir}\packaging\win\{#ProjName}.ico
Compression=lzma
SolidCompression=yes
WizardStyle=modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#BuildBinDir}\{#AppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Gui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Network.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\plugins\platforms\qminimal.dll"; DestDir: "{app}\platforms"; Flags: ignoreversion
Source: "{#QtDir}\plugins\platforms\qoffscreen.dll"; DestDir: "{app}\platforms"; Flags: ignoreversion
Source: "{#QtDir}\plugins\platforms\qwebgl.dll"; DestDir: "{app}\platforms"; Flags: ignoreversion
Source: "{#QtDir}\plugins\platforms\qwindows.dll"; DestDir: "{app}\platforms"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Registry]
Root: HKA; Subkey: "Software\Classes\{#AppAssocKey}"; ValueType: string; ValueName: ""; ValueData: "{#AppAssocName}"; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\{#AppAssocKey}\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#AppExeName},0"
Root: HKA; Subkey: "Software\Classes\{#AppAssocKey}\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#AppExeName}"" ""%1"""
; Open With...
Root: HKA; Subkey: "Software\Classes\*\shell\OpenWith{#AppName}\"; ValueType: string; ValueData: "Open with {#AppName}"; Flags: uninsdeletekey 
Root: HKA; Subkey: "Software\Classes\*\shell\OpenWith{#AppName}\"; ValueType: string; ValueName: "icon"; ValueData: """{app}\{#AppExeName}"""
Root: HKA; Subkey: "Software\Classes\*\shell\OpenWith{#AppName}\command"; ValueType: string; ValueData: """{app}\{#AppExeName}"" ""%1"""

[Icons]
Name: "{autoprograms}\{#AppName}"; Filename: "{app}\{#AppExeName}"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#AppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(AppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
