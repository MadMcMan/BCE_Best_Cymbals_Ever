; Cymbals VST3/CLAP Plugin Installer for Windows
; Inno Setup 6 script
; Unsigned distribution (no code signing configured)
;
; Build locally:
;   iscc scripts\installer_win\cymbals64.iss
;
; Override build dir and version at compile time:
;   iscc /DBuildDir=..\..\build-win\bin /DMyAppVersion=X.Y.Z scripts\installer_win\cymbals64.iss

#define AppName      "Cymbals"
#define AppPublisher "MadMcMan"
#define AppURL       ""

#ifndef MyAppVersion
  #define MyAppVersion "0.9.0"
#endif
#define AppVersion MyAppVersion
#define AppExeName   "MadMcManBCE"

; Default build output dir (relative to .iss file location)
#ifndef BuildDir
  #define BuildDir "..\..\build-lvgl\bin"
#endif

#ifndef OutputDir
  #define OutputDir "Output"
#endif

[Setup]
AppId={{B4D2E9A1-7C3F-4E8D-9F5A-2B6C0D1E3047}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}
AppUpdatesURL={#AppURL}
DefaultDirName={autopf}\MadMcMan\{#AppName}
DefaultGroupName={#AppPublisher}\{#AppName}
DisableProgramGroupPage=yes
OutputDir={#OutputDir}
OutputBaseFilename={#AppName}_Setup_{#AppVersion}
Compression=lzma2/ultra64
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64
WizardStyle=modern
; Require admin for installing into Program Files and VST3 system dirs
PrivilegesRequired=admin
ShowLanguageDialog=no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

; ------------------------------------------------------------------
; Installation types presented in the wizard
; ------------------------------------------------------------------
[Types]
Name: "full";    Description: "Full installation (VST3 + CLAP + Standalone)"
Name: "custom";  Description: "Custom installation";              Flags: iscustom

; ------------------------------------------------------------------
; Components (checkboxes)
; ------------------------------------------------------------------
[Components]
Name: "VST3";       Description: "VST3 Plugin";        Types: full custom
Name: "CLAP";       Description: "CLAP Plugin";        Types: full custom
Name: "Standalone"; Description: "Standalone Application"; Types: full custom

; ------------------------------------------------------------------
; Installation directories per component
; ------------------------------------------------------------------
[Dirs]
; Windows default discovery paths: C:\Program Files\Common Files\VST3 and CLAP.
Name: "{autocf64}\VST3";                        Components: VST3
Name: "{autocf64}\CLAP";                        Components: CLAP
Name: "{autopf64}\MadMcMan\MadMcManBCE";         Components: Standalone

; ------------------------------------------------------------------
; Files
; ------------------------------------------------------------------
[Files]
; VST3 bundle directory (recursive)
Source: "{#BuildDir}\{#AppExeName}.vst3\*"; \
  DestDir: "{autocf64}\VST3\{#AppExeName}.vst3"; \
  Flags: ignoreversion recursesubdirs createallsubdirs; \
  Components: VST3

; CLAP single file
Source: "{#BuildDir}\{#AppExeName}.clap"; \
  DestDir: "{autocf64}\CLAP"; \
  Flags: ignoreversion; \
  Components: CLAP

; Standalone/JACK executable (DPF builds the JACK target as MadMcManBCE.exe).
Source: "{#BuildDir}\{#AppExeName}.exe"; \
  DestDir: "{autopf64}\MadMcMan\MadMcManBCE"; \
  DestName: "{#AppExeName}.exe"; \
  Flags: ignoreversion; \
  Components: Standalone

; ------------------------------------------------------------------
; Icons (Start Menu)
; ------------------------------------------------------------------
[Icons]
Name: "{group}\{#AppName} Standalone"; \
  Filename: "{autopf64}\MadMcMan\MadMcManBCE\{#AppExeName}.exe"; \
  Flags: createonlyiffileexists; \
  Components: Standalone
Name: "{group}\Uninstall {#AppName}"; \
  Filename: "{uninstallexe}"

; ------------------------------------------------------------------
; Run after install
; ------------------------------------------------------------------
[Run]
Filename: "{autopf64}\MadMcMan\MadMcManBCE\{#AppExeName}.exe"; \
  Description: "Launch {#AppName} Standalone"; \
  Flags: nowait postinstall skipifsilent skipifdoesntexist; \
  Components: Standalone
