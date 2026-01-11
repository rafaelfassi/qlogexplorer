; Will be replaced by the release script
#define ProjName "qlogexplorer"
#define AppName "QLogExplorer"
#define AppVersion "1.1.2"
#define AppURL "https://rafaelfassi.github.io/qlogexplorer/"
; Calculated and fixed values
#define AppAssocExt ".log"
#define AppPublisher AppName + " Project"
#define AppAssocKey AppName + ".id"
#define AppExeName ProjName + ".exe"
#define AppAssocName AppName
#define PlatformType "Win64"

; Needs to be provided by the command line args: /DSrcDir=<src-dir> /DOutBinDir=<build-dir>
;#define SrcDir "<src-dir>"
;#define OutBinDir "<build-dir>"

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
OutputDir={#OutBinDir}
OutputBaseFilename={#AppName}-{#PlatformType}-Setup
SetupIconFile={#SrcDir}\packaging\win\{#ProjName}.ico
Compression=lzma
SolidCompression=yes
WizardStyle=modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#OutBinDir}\{#AppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#OutBinDir}\*.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#OutBinDir}\generic\*"; DestDir: "{app}\generic"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#OutBinDir}\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#OutBinDir}\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#OutBinDir}\networkinformation\*"; DestDir: "{app}\networkinformation"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#OutBinDir}\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#OutBinDir}\styles\*"; DestDir: "{app}\styles"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#OutBinDir}\tls\*"; DestDir: "{app}\tls"; Flags: ignoreversion recursesubdirs createallsubdirs
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
