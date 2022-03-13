import os
import re

# Requires ImageMagick

scriptDir = os.path.dirname(os.path.realpath(__file__))
cmakeFile = os.path.join(scriptDir, '../CMakeLists.txt')
rcFile = os.path.join(scriptDir, 'win/qlogexplorer.rc')
innoFile = os.path.join(scriptDir, 'win/innosetup.iss')
svgIco = os.path.join(scriptDir, '../images/svg/qlogexplorer.svg')

# Read info from CMake file

cmakeFileContent=None

with open(cmakeFile) as f:
   cmakeFileContent = f.read()

projInfo = re.search(r'project\s*\(\s*(?P<ProjName>\w+)\s+VERSION\s+(?P<VerMajor>\d+)\.(?P<VerMinor>\d+)\.(?P<VerPatch>\d+)', cmakeFileContent)
if not projInfo:
    print("Project info not found")
    exit(-1)

nameInfo = re.search(r'set\s*\(\s*APP_NAME\s+\"(?P<AppName>[^\"]+)\"\s*\)', cmakeFileContent)
if not nameInfo:
    print("Name info not found")
    exit(-1)

urlInfo = re.search(r'set\s*\(\s*APP_URL\s+\"(?P<AppUrl>[^\"]+)\"\s*\)', cmakeFileContent)
if not nameInfo:
    print("Url info not found")
    exit(-1)

projName = projInfo.group("ProjName")
verMajor = projInfo.group("VerMajor")
verMinor = projInfo.group("VerMinor")
verPatch = projInfo.group("VerPatch")
appName = nameInfo.group("AppName")
appUrl = urlInfo.group("AppUrl")

# Updates windows RC file

rcFileContent = '''
IDI_ICON1 ICON DISCARDABLE "{ProjectName}.ico"

#include <windows.h>

VS_VERSION_INFO VERSIONINFO
FILEVERSION     {VerMajor},{VerMinor},{VerPatch},0
PRODUCTVERSION  {VerMajor},{VerMinor},{VerPatch},0
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904E4"
        BEGIN
            VALUE "CompanyName",        "{AppName} Project"
            VALUE "FileDescription",    "{AppName} - Advanced Log Viewer"
            VALUE "FileVersion",        "{VerMajor}.{VerMinor}.{VerPatch}.0\\0"
            VALUE "InternalName",       "{AppName}"
            VALUE "LegalCopyright",     "(c) 2022 {AppName} Project"
            VALUE "OriginalFilename",   "{ProjectName}.exe"
            VALUE "ProductName",        "{AppName}"
            VALUE "ProductVersion",     "{VerMajor}.{VerMinor}.{VerPatch}\\0"
        END
    END

    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x409, 1252
    END
END
'''.format(ProjectName = projName, VerMajor = verMajor, VerMinor = verMinor, VerPatch = verPatch, AppName = appName)

with open(rcFile, "w") as f:
    f.write(rcFileContent)

# Updates windows instaler

def replaceInnoVar(content, varName, value):
    res = re.sub(r'(#define\s+{}\s+\")([^\"]+)(\")'.format(varName), r'\1_ValuePlaceholder_\3', content)
    return res.replace('_ValuePlaceholder_', value)

innoFileContent = None

with open(innoFile, "r") as f:
    innoFileContent = f.read()

innoFileContent = replaceInnoVar(innoFileContent, 'ProjName', projName)
innoFileContent = replaceInnoVar(innoFileContent, 'AppName', appName)
innoFileContent = replaceInnoVar(innoFileContent, 'AppVersion', '{}.{}.{}'.format(verMajor, verMinor, verPatch))
innoFileContent = replaceInnoVar(innoFileContent, 'AppURL', appUrl)

with open(innoFile, "w") as f:
    f.write(innoFileContent)

# Update windows ico

os.system('convert -density 256x256 -background transparent {} -define icon:auto-resize -colors 256 {}/win/{}.ico'.format(
    svgIco, scriptDir, projName))

# Update xdg icons

xdgBaseIconsDir = '{}/linux/xdg/icons/hicolor'.format(scriptDir)

scalableIconDir = '{}/scalable/apps'.format(xdgBaseIconsDir)
os.system('mkdir -p {}'.format(scalableIconDir))
os.system('cp {} {}'.format(svgIco, scalableIconDir))

for xdgIconSize in [16, 24, 32, 64, 128, 256]:
    xdgIconDir = '{}/{}x{}/apps'.format(xdgBaseIconsDir, xdgIconSize, xdgIconSize)
    os.system('mkdir -p {}'.format(xdgIconDir))
    os.system('convert -define png:exclude-chunks=all -density 1200 -background transparent -resize {}x{} {} {}/{}.png'.format(
        xdgIconSize, xdgIconSize, svgIco, xdgIconDir, projName))
