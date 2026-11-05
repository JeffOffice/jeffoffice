; JeffOffice Windows installer (Inno Setup 6.6 or later). Built by CI from dist\JeffOffice:
;   ISCC.exe /DVERSION=0.20.0 [/DWITH_WRITE] [/DWITH_CRUNCH] packaging\windows\installer.iss
; after tools\reflow_license.py --rtf LICENSE packaging\windows\license.rtf --append ADDITIONAL-TERMS.md (the license page).
; Installs the suite's apps that are ready (JeffPub and JeffDeck, and JeffWrite and JeffCrunch as betas) into one folder,
; per user: no administrator rights needed. The apps still being built are listed as coming soon and can't be chosen yet.
; JeffWrite and JeffCrunch were among those until the owner decided each ships: building with /DWITH_WRITE or
; /DWITH_CRUNCH (the workflow does, when its SHIP_WRITE or SHIP_CRUNCH is "true") makes it a beta that is not ticked
; by default (it is ticked when it is already installed).
; Setups through 0.20.0 were made with NSIS; this one takes over from them (see RemoveNsisInstall).
#if Ver < EncodeVer(6, 6, 0)
  #error Inno Setup 6.6.0 or later is needed: the artwork is drawn for its image sizes.
#endif
#ifndef VERSION
  #define VERSION "0.1.0"
#endif
#define Suite "JeffOffice"
#define SuiteKey "Software\JeffOffice\JeffOffice"
#define PubSetKey "Software\JeffOffice\JeffPub"
#define DeckSetKey "Software\JeffOffice\JeffDeck"
#define WriteSetKey "Software\JeffOffice\JeffWrite"
#define CrunchSetKey "Software\JeffOffice\JeffCrunch"
#define TaskStats "Send anonymous usage statistics (never your files)"
#define PubExts ".pub,.jpub"
#define DeckExts ".pptx,.potx,.ppsx,.ppt,.pot,.pps,.odp,.otp"
#define WriteExts ".docx,.docm,.dotx,.dotm,.doc,.dot,.odt,.ott,.rtf"
#define CrunchExts ".xlsx,.xlsm,.xltx,.xltm,.xls,.ods,.ots,.csv"

; What the texts say, by what this build contains.
#ifdef WITH_WRITE
  #ifdef WITH_CRUNCH
    #define RunningList "JeffPub, JeffDeck, JeffWrite, or JeffCrunch"
    #define WelcomeApps ", and JeffWrite, for documents, and JeffCrunch, for spreadsheets, come as betas. JeffMail and JeffBase are coming soon."
    #define AppsDesc "JeffPub and JeffDeck are ready. JeffWrite and JeffCrunch are betas."
  #else
    #define RunningList "JeffPub, JeffDeck, or JeffWrite"
    #define WelcomeApps ", and JeffWrite, for documents, comes as a beta. JeffCrunch, JeffMail, and JeffBase are coming soon."
    #define AppsDesc "JeffPub and JeffDeck are ready. JeffWrite is a beta."
  #endif
#else
  #ifdef WITH_CRUNCH
    #define RunningList "JeffPub, JeffDeck, or JeffCrunch"
    #define WelcomeApps ", and JeffCrunch, for spreadsheets, comes as a beta. JeffWrite, JeffMail, and JeffBase are coming soon."
    #define AppsDesc "JeffPub and JeffDeck are ready. JeffCrunch is a beta."
  #else
    #define RunningList "JeffPub or JeffDeck"
    #define WelcomeApps ". JeffWrite, JeffCrunch, JeffMail, and JeffBase are coming soon."
    #define AppsDesc "JeffPub and JeffDeck are ready now."
  #endif
#endif
; The folder dist\JeffOffice holds every app's program; only the app's own component installs it.
#ifdef WITH_WRITE
  #define NotWrite ",JeffWrite.exe"
#else
  #define NotWrite ""
#endif
#ifdef WITH_CRUNCH
  #define NotCrunch ",JeffCrunch.exe"
#else
  #define NotCrunch ""
#endif
#define NotApps "JeffPub.exe,JeffDeck.exe" + NotWrite + NotCrunch

[Setup]
AppId=JeffOffice
AppName={#Suite}
AppVersion={#VERSION}
AppVerName={#Suite} {#VERSION}
AppPublisher=JeffOffice LLC
AppPublisherURL=https://github.com/JeffOffice/jeffoffice
AppCopyright=Copyright (c) 2026 JeffOffice LLC. Free software under the GNU GPL v3.
VersionInfoVersion={#VERSION}.0
VersionInfoProductVersion={#VERSION}
VersionInfoCompany=JeffOffice LLC
VersionInfoDescription={#Suite} installer
VersionInfoProductName={#Suite}
VersionInfoCopyright=Copyright (c) 2026 JeffOffice LLC. Free software under the GNU GPL v3.
; Per user, no administrator rights; the folder is the one the NSIS setups used.
PrivilegesRequired=lowest
DefaultDirName={code:GetDefaultDir}
DisableProgramGroupPage=yes
; An earlier setup's folder is expected to exist (the NSIS setups made it), so Setup doesn't ask about it.
DirExistsWarning=no
DisableReadyPage=yes
DisableWelcomePage=no
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
; The program is never closed for the person: Setup asks them to (see ClosedFirst).
CloseApplications=no
RestartApplications=no
SetupMutex=JeffOfficeSetup
ChangesAssociations=yes
OutputDir=.
OutputBaseFilename=JeffOffice-Setup
SetupIconFile=jeffoffice.ico
UninstallDisplayIcon={app}\jeffoffice.ico
UninstallDisplayName={#Suite}
Compression=lzma2/max
SolidCompression=yes
; The look: the modern style follows the system's light or dark setting. The side panel and the corner mark are drawn
; at the exact size Setup asks for at each display scale (tools/make_installer_art.py), so they are never resized.
WizardStyle=modern dynamic
WizardImageFile=art\welcome-202x386.png,art\welcome-269x515.png,art\welcome-336x643.png,art\welcome-403x772.png,art\welcome-430x824.png,art\welcome-498x953.png,art\welcome-534x1022.png
WizardSmallImageFile=art\small-58.png,art\small-77.png,art\small-97.png,art\small-116.png,art\small-124.png,art\small-143.png,art\small-159.png
LicenseFile=license.rtf

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Messages]
WelcomeLabel1=Welcome to {#Suite}
WelcomeLabel2={#Suite} {#VERSION} is a free, open-source office suite.%n%nJeffPub, for desktop publishing, and JeffDeck, for presentations, are ready now{#WelcomeApps} If JeffPub is already installed, setup moves it into {#Suite}, with your settings.%n%n{#Suite} is an independent open-source project.%n%nNo administrator rights are needed.
WizardLicense=License
LicenseLabel=Please read the license.
LicenseLabel3={#Suite} is free software under the GNU General Public License v3.0, with Additional Terms under section 7 after it. To install {#Suite}, you must accept its terms.
LicenseAccepted=I &accept the terms of the GNU General Public License v3.0 and the Additional Terms
LicenseNotAccepted=I &do not accept the terms
SelectComponentsDesc=Choose what to install.
SelectComponentsLabel2=Apps marked coming soon come in an update.
WizardSelectTasks=Shortcuts and file types
SelectTasksDesc=Shortcuts, file types, and usage statistics.
SelectTasksLabel2=If you send usage statistics, they go once a day: version, system, language, command counts, and crashes. Details: www.jeffoffice.com/privacy
FinishedHeadingLabel={#Suite} is ready
FinishedLabel={#Suite} {#VERSION} is installed.
FinishedLabelNoIcons={#Suite} {#VERSION} is installed.
ConfirmUninstall=Remove %1, with its settings, from this computer?
UninstalledAll=%1 was removed from this computer.

[Components]
Name: "pub"; Description: "JeffPub: publications"
Name: "deck"; Description: "JeffDeck: presentations"
#ifdef WITH_WRITE
Name: "write"; Description: "JeffWrite: documents (beta)"
#else
Name: "write"; Description: "JeffWrite: documents (coming soon)"
#endif
#ifdef WITH_CRUNCH
Name: "crunch"; Description: "JeffCrunch: spreadsheets (beta)"
#else
Name: "crunch"; Description: "JeffCrunch: spreadsheets (coming soon)"
#endif
Name: "mail"; Description: "JeffMail: email and calendar (coming soon)"
Name: "base"; Description: "JeffBase: databases (coming soon)"

[Tasks]
Name: "desktopicon"; Description: "Add shortcuts to the apps to your desktop"; GroupDescription: "Shortcuts:"
Name: "jpub"; Description: "Open .jpub files with JeffPub"; GroupDescription: "File types:"; Components: pub
Name: "pubdefault"; Description: "Make JeffPub the default for .pub files (off keeps your current program)"; GroupDescription: "File types:"; Components: pub; Flags: unchecked
Name: "stats"; Description: "{#TaskStats}"; GroupDescription: "Usage statistics:"

[Dirs]
; The folder is removed with the program even when an earlier setup made it (NSIS, through 0.20.0).
Name: "{app}"; Flags: uninsalwaysuninstall

[Files]
Source: "..\..\dist\JeffOffice\*"; DestDir: "{app}"; Excludes: "{#NotApps}"; Flags: recursesubdirs createallsubdirs ignoreversion
Source: "jeffoffice.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\dist\JeffOffice\JeffPub.exe"; DestDir: "{app}"; Components: pub; Flags: ignoreversion
Source: "..\..\dist\JeffOffice\JeffDeck.exe"; DestDir: "{app}"; Components: deck; Flags: ignoreversion
#ifdef WITH_WRITE
Source: "..\..\dist\JeffOffice\JeffWrite.exe"; DestDir: "{app}"; Components: write; Flags: ignoreversion
#endif
#ifdef WITH_CRUNCH
Source: "..\..\dist\JeffOffice\JeffCrunch.exe"; DestDir: "{app}"; Components: crunch; Flags: ignoreversion
#endif

[Icons]
Name: "{userprograms}\JeffPub"; Filename: "{app}\JeffPub.exe"; Components: pub
Name: "{userprograms}\JeffDeck"; Filename: "{app}\JeffDeck.exe"; Components: deck
; A desktop shortcut made for an old folder follows it, and so does a taskbar pin (pins are shortcuts kept in the
; folder below; setup remakes the ones that exist, and the uninstaller leaves them, as the NSIS setups did).
Name: "{userdesktop}\JeffPub"; Filename: "{app}\JeffPub.exe"; Components: pub; Check: WantDesktop('JeffPub')
Name: "{userdesktop}\JeffDeck"; Filename: "{app}\JeffDeck.exe"; Components: deck; Check: WantDesktop('JeffDeck')
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\JeffPub"; Filename: "{app}\JeffPub.exe"; Components: pub; Check: PinExists('JeffPub'); Flags: uninsneveruninstall
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\JeffDeck"; Filename: "{app}\JeffDeck.exe"; Components: deck; Check: PinExists('JeffDeck'); Flags: uninsneveruninstall
#ifdef WITH_WRITE
Name: "{userprograms}\JeffWrite"; Filename: "{app}\JeffWrite.exe"; Components: write
Name: "{userdesktop}\JeffWrite"; Filename: "{app}\JeffWrite.exe"; Components: write; Check: WantDesktop('JeffWrite')
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\JeffWrite"; Filename: "{app}\JeffWrite.exe"; Components: write; Check: PinExists('JeffWrite'); Flags: uninsneveruninstall
#endif
#ifdef WITH_CRUNCH
Name: "{userprograms}\JeffCrunch"; Filename: "{app}\JeffCrunch.exe"; Components: crunch
Name: "{userdesktop}\JeffCrunch"; Filename: "{app}\JeffCrunch.exe"; Components: crunch; Check: WantDesktop('JeffCrunch')
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\JeffCrunch"; Filename: "{app}\JeffCrunch.exe"; Components: crunch; Check: PinExists('JeffCrunch'); Flags: uninsneveruninstall
#endif

[Code]
const
  NsisKey = 'Software\Microsoft\Windows\CurrentVersion\Uninstall\JeffOffice';
  // Through 0.6.10, setup installed JeffPub on its own; this setup moves it in.
  PubKey = 'Software\Microsoft\Windows\CurrentVersion\Uninstall\JeffPub';
  // JeffPub was called JeffPub 79 through version 0.5.0; setup replaces that install too.
  OldKey = 'Software\Microsoft\Windows\CurrentVersion\Uninstall\JeffPub79';
  Cls = 'Software\Classes\';

var
  // A desktop shortcut JeffPub 79 had: JeffPub takes its place.
  DeskExtra: Boolean;
  // The folder the apps' presets were last read from, and the description under the components.
  PresetFolder: String;
  DescLabel: TNewStaticText;
  ListFixed: Boolean;
  StatsPreset: Boolean;

#define Pfx ""
#include "shared.isi"
#undef Pfx
#define Pfx "Un"
#include "shared.isi"
#undef Pfx

function TrimSlash(const S: String): String;
begin
  Result := S;
  // The folder page once showed "...\JeffOffice\": a backslash at the end is taken off, but not from a drive's own folder.
  if (Length(Result) > 3) and (Result[Length(Result)] = '\') then
    Delete(Result, Length(Result), 1);
end;

// The folder of an earlier install (either setup), else the usual one.
function GetDefaultDir(Param: String): String;
var
  S: String;
begin
  if RegQueryStringValue(HKCU, 'Software\JeffOffice', 'InstallDir', S) and (S <> '') then
    Result := TrimSlash(S)
  else
    Result := ExpandConstant('{localappdata}\Programs\JeffOffice');
end;

function InitializeSetup(): Boolean;
begin
  Result := ClosedFirst(GetDefaultDir(''), WizardSilent);
end;

function InitializeUninstall(): Boolean;
begin
  Result := UnClosedFirst(ExpandConstant('{app}'), UninstallSilent);
end;

// The folder is checked again for the folder the person chose.
function PrepareToInstall(var NeedsRestart: Boolean): String;
begin
  Result := '';
  if AppsRunning(WizardDirValue) then
    Result := '{#RunningList} is running.' + #13#10#13#10 + 'Save your work and close it, then click Next to try again.';
end;

function WantDesktop(App: String): Boolean;
begin
  Result := WizardIsTaskSelected('desktopicon') or FileExists(ExpandConstant('{userdesktop}\') + App + '.lnk')
    or (DeskExtra and (App = 'JeffPub'));
end;

function PinExists(App: String): Boolean;
begin
  Result := FileExists(ExpandConstant('{userappdata}\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\') + App + '.lnk');
end;

// ---- the wizard ----

function Described(I: Integer): String;
begin
  case I of
    0: Result := 'Desktop publishing: flyers, newsletters, and cards. Opens .pub files.';
    1: Result := 'Presentations and slide shows. Opens and saves .pptx and .odp.';
#ifdef WITH_WRITE
    2: Result := 'Documents and reports. A beta: opens .docx, .doc, .rtf, and .odt files, and saves .docx and .odt.';
#else
    2: Result := 'Documents, letters, and reports. Coming soon: it can''t be installed yet.';
#endif
#ifdef WITH_CRUNCH
    3: Result := 'Spreadsheets and charts. A beta: opens .xlsx, .xls, .ods, and .csv files, and saves them.';
#else
    3: Result := 'Spreadsheets and charts. Coming soon: it can''t be installed yet.';
#endif
    4: Result := 'Email and calendars. Coming soon: it can''t be installed yet.';
    5: Result := 'Databases. Coming soon: it can''t be installed yet.';
  else
    Result := 'Click an item to see what it does.';
  end;
end;

procedure ShowDescription(Sender: TObject);
begin
  DescLabel.Caption := Described(WizardForm.ComponentsList.ItemIndex);
end;

procedure ShowDescriptionKey(Sender: TObject; var Key: Word; Shift: TShiftState);
begin
  ShowDescription(Sender);
end;

// An update keeps JeffWrite and JeffCrunch where they are installed: they start ticked, so their files are updated
// with the rest and never left a version behind. (The folder of an earlier NSIS or Inno install counts too.) JeffPub
// starts ticked, except in an update of an install that was made without it.
procedure PresetApps(const Dir: String);
var
  Old: String;
begin
  Old := GetDefaultDir('');
  WizardForm.ComponentsList.Checked[0] := FileExists(Dir + '\JeffPub.exe') or not (FileExists(Dir + '\JeffDeck.exe')
    or FileExists(Dir + '\JeffWrite.exe') or FileExists(Dir + '\JeffCrunch.exe'));
  if FileExists(Dir + '\JeffWrite.exe') or FileExists(Old + '\JeffWrite.exe') then
    WizardForm.ComponentsList.Checked[2] := True;
  if FileExists(Dir + '\JeffCrunch.exe') or FileExists(Old + '\JeffCrunch.exe') then
    WizardForm.ComponentsList.Checked[3] := True;
end;

// The position of a task in the tasks list (its caption holds the words), or -1.
function TaskIndex(const Words: String): Integer;
var
  I: Integer;
begin
  Result := -1;
  for I := 0 to WizardForm.TasksList.Items.Count - 1 do
    if Pos(Words, WizardForm.TasksList.Items[I]) > 0 then
    begin
      Result := I;
      Break;
    end;
end;

// An update shows the choice already made: turned off in an app (or in JeffPub before it kept the choice for the
// suite), the statistics start unticked, and setup never turns them on for someone who said no.
procedure PresetStats;
var
  V: String;
  Found: Boolean;
  I: Integer;
begin
  V := '';
  Found := RegQueryStringValue(HKCU, '{#SuiteKey}\telemetry', 'enabled', V) and (V <> '');
  if not Found then
  begin
    V := '';
    Found := RegQueryStringValue(HKCU, '{#PubSetKey}\telemetry', 'enabled', V) and (V <> '');
  end;
  if not Found then
  begin
    V := '';
    Found := RegQueryStringValue(HKCU, 'Software\JeffPub79\JeffPub79\telemetry', 'enabled', V) and (V <> '');
  end;
  if Found and (V = 'false') then
  begin
    I := TaskIndex('anonymous usage statistics');
    if I >= 0 then
      WizardForm.TasksList.Checked[I] := False;
  end;
end;

// The apps not ready yet are listed, unticked, and can't be ticked.
procedure ComingSoon;
var
  I: Integer;
begin
  for I := 2 to 5 do
  begin
#ifdef WITH_WRITE
    if I = 2 then Continue;
#endif
#ifdef WITH_CRUNCH
    if I = 3 then Continue;
#endif
    WizardForm.ComponentsList.Checked[I] := False;
    WizardForm.ComponentsList.ItemEnabled[I] := False;
  end;
end;

procedure InitializeWizard;
var
  I: Integer;
begin
  PresetFolder := '';
  // With no setup types every component starts ticked: only JeffPub and JeffDeck do (PresetApps sets JeffPub). The same goes for the file type
  // that makes JeffPub the default for .pub files: it starts unticked.
  WizardForm.ComponentsList.Checked[1] := True;
  for I := 2 to 5 do
    WizardForm.ComponentsList.Checked[I] := False;
  I := TaskIndex('default for .pub files');
  if I >= 0 then
    WizardForm.TasksList.Checked[I] := False;
  PresetApps(WizardDirValue);
  // The lists leave room for a check box of the 100% size, but Windows draws a larger one on a scaled display, and its
  // left edge was cut off. The space before the box (and after it) grows by the difference.
  WizardForm.ComponentsList.Offset := ScaleX(13) - 13 + ScaleX(4);
  WizardForm.TasksList.Offset := ScaleX(13) - 13 + ScaleX(4);
  // The description of the chosen app sits under the list.
  DescLabel := TNewStaticText.Create(WizardForm);
  DescLabel.Parent := WizardForm.SelectComponentsPage;
  DescLabel.AutoSize := False;
  DescLabel.WordWrap := True;
  DescLabel.Caption := Described(-1);
  WizardForm.ComponentsList.OnClick := @ShowDescription;
  WizardForm.ComponentsList.OnClickCheck := @ShowDescription;
  WizardForm.ComponentsList.OnKeyUp := @ShowDescriptionKey;
end;

// Builds "To start an app, open the Start menu and click A, B, or C." from the apps that were installed, with "or"
// before the last and commas from three up.
function FinishText: String;
var
  Names: array of String;
  N, I: Integer;
  Head, Lst: String;
begin
  SetArrayLength(Names, 4);
  N := 0;
  if WizardIsComponentSelected('pub') then begin Names[N] := 'JeffPub'; N := N + 1; end;
  if WizardIsComponentSelected('deck') then begin Names[N] := 'JeffDeck'; N := N + 1; end;
#ifdef WITH_WRITE
  if WizardIsComponentSelected('write') then begin Names[N] := 'JeffWrite'; N := N + 1; end;
#endif
#ifdef WITH_CRUNCH
  if WizardIsComponentSelected('crunch') then begin Names[N] := 'JeffCrunch'; N := N + 1; end;
#endif
  Head := '{#Suite} {#VERSION} is installed.' + #13#10#13#10;
  if N = 1 then
    Result := Head + 'To start it, open the Start menu and click ' + Names[0] + '.'
  else if N = 2 then
    Result := Head + 'To start an app, open the Start menu and click ' + Names[0] + ' or ' + Names[1] + '.'
  else
  begin
    Lst := '';
    for I := 0 to N - 2 do
      Lst := Lst + Names[I] + ', ';
    Result := Head + 'To start an app, open the Start menu and click ' + Lst + 'or ' + Names[N - 1] + '.';
  end;
end;

procedure CurPageChanged(CurPageID: Integer);
var
  L: TNewCheckListBox;
  I: Integer;
begin
  if CurPageID = wpSelectComponents then
  begin
    if PresetFolder <> WizardDirValue then
    begin
      PresetFolder := WizardDirValue;
      PresetApps(WizardDirValue);
    end;
    ComingSoon;
    L := WizardForm.ComponentsList;
    if not ListFixed then
    begin
      ListFixed := True;
      // The list of setup types has nothing to choose: its room goes to the list.
      WizardForm.TypesCombo.Visible := False;
      I := L.Top - WizardForm.TypesCombo.Top;
      L.Top := WizardForm.TypesCombo.Top;
      L.Height := L.Height + I - ScaleY(46);
      DescLabel.SetBounds(L.Left, L.Top + L.Height + ScaleY(8), L.Width, ScaleY(38));
    end;
    DescLabel.Caption := Described(L.ItemIndex);
  end
  else if CurPageID = wpSelectTasks then
  begin
    // Setup builds the tasks list again after the components page, so the statistics preset is made when it is shown.
    if not StatsPreset then
    begin
      StatsPreset := True;
      PresetStats;
    end;
  end
  else if CurPageID = wpFinished then
    WizardForm.FinishedLabel.Caption := FinishText;
end;

// Setup with no app ticked would install nothing anyone can open.
function NextButtonClick(CurPageID: Integer): Boolean;
begin
  Result := True;
  if (CurPageID = wpSelectComponents) and not (WizardIsComponentSelected('pub') or WizardIsComponentSelected('deck')
#ifdef WITH_WRITE
    or WizardIsComponentSelected('write')
#endif
#ifdef WITH_CRUNCH
    or WizardIsComponentSelected('crunch')
#endif
    ) then
  begin
    MsgBox('Choose at least one app to install.', mbInformation, MB_OK);
    Result := False;
  end;
end;

// ---- install ----

// Through 0.6.10, JeffPub had its own folder and Settings > Apps entry. The files setup put in that folder go, one by
// one so nothing else there is touched, then the folder if that left it empty; and the entry goes. Its settings and
// files stay: the apps read them where they always were.
procedure MoveFromJeffPub;
var
  Loc: String;
begin
  if RegQueryStringValue(HKCU, PubKey, 'InstallLocation', Loc) and (Loc <> '') then
  begin
    Loc := TrimSlash(Loc);
    if (CompareText(Loc, ExpandConstant('{app}')) <> 0) and FileExists(Loc + '\JeffPub.exe') then
    begin
      DeleteFile(Loc + '\JeffPub.exe');
      DeleteFile(Loc + '\Uninstall.exe');
      DelTree(Loc + '\Qt6*.dll', False, True, False);
      DeleteFile(Loc + '\libgcc_s_seh-1.dll');
      DeleteFile(Loc + '\libstdc++-6.dll');
      DeleteFile(Loc + '\libwinpthread-1.dll');
      DeleteFile(Loc + '\pdfium.dll');
      DeleteFile(Loc + '\LICENSE');
      DeleteFile(Loc + '\README.md');
      DelTree(Loc + '\fonts', True, True, True);
      DelTree(Loc + '\dict', True, True, True);
      DelTree(Loc + '\licenses', True, True, True);
      DelTree(Loc + '\generic', True, True, True);
      DelTree(Loc + '\iconengines', True, True, True);
      DelTree(Loc + '\imageformats', True, True, True);
      DelTree(Loc + '\networkinformation', True, True, True);
      DelTree(Loc + '\platforms', True, True, True);
      DelTree(Loc + '\styles', True, True, True);
      DelTree(Loc + '\tls', True, True, True);
      RemoveDir(Loc);
    end;
    RegDeleteKeyIncludingSubkeys(HKCU, PubKey);
    RegDeleteValue(HKCU, 'Software\JeffOffice', 'JeffPubInstallDir');
  end;
end;

// Takes away JeffPub 79 (0.5.0 and earlier): its program folder, shortcuts, file types and Settings > Apps entry. Its
// settings stay: JeffPub copies them the first time it starts. A .pub default that pointed at JeffPub 79 moves over,
// and so does a desktop shortcut.
procedure RemoveJeffPub79;
var
  Loc, V: String;
begin
  if RegQueryStringValue(HKCU, OldKey, 'InstallLocation', Loc) and (Loc <> '') then
  begin
    Loc := TrimSlash(Loc);
    // Only a folder that really holds JeffPub 79 is deleted.
    if FileExists(Loc + '\JeffPub79.exe') and (CompareText(Loc, ExpandConstant('{app}')) <> 0) then
      DelTree(Loc, True, True, True);
    DeleteFile(ExpandConstant('{userprograms}\JeffPub 79.lnk'));
    if FileExists(ExpandConstant('{userdesktop}\JeffPub 79.lnk')) then
    begin
      DeleteFile(ExpandConstant('{userdesktop}\JeffPub 79.lnk'));
      DeskExtra := True;
    end;
    RegDeleteKeyIncludingSubkeys(HKCU, OldKey);
  end;
  if RegQueryStringValue(HKCU, Cls + '.pub', '', V) and (V = 'JeffPub79.PubFile') then
    RegWriteStringValue(HKCU, Cls + '.pub', '', 'JeffPub.PubFile');
  if RegQueryStringValue(HKCU, Cls + '.jpub', '', V) and (V = 'JeffPub79.Publication') then
    RegWriteStringValue(HKCU, Cls + '.jpub', '', 'JeffPub.Publication');
  RegDeleteValue(HKCU, Cls + '.pub\OpenWithProgids', 'JeffPub79.PubFile');
  RegDeleteKeyIncludingSubkeys(HKCU, Cls + 'JeffPub79.PubFile');
  RegDeleteKeyIncludingSubkeys(HKCU, Cls + 'JeffPub79.Publication');
  RegDeleteKeyIncludingSubkeys(HKCU, Cls + 'Applications\JeffPub79.exe');
  RegDeleteValue(HKCU, 'Software\JeffPub79', 'InstallDir');
end;

// The NSIS setups (0.20.0 and earlier) made their own Settings > Apps entry and an Uninstall.exe. Setup takes over from
// them without running that uninstaller, which deletes the apps' settings (and the statistics choice): the entry and the
// Uninstall.exe go, and so do the files of the old install this version doesn't ship (CleanProgramFolder, which only
// touches the names setup installs). The new uninstaller is the one in Settings > Apps from here on.
procedure RemoveNsisInstall;
var
  Loc: String;
begin
  if not RegKeyExists(HKCU, NsisKey) then
    Exit;
  if RegQueryStringValue(HKCU, NsisKey, 'InstallLocation', Loc) and (Loc <> '') then
  begin
    Loc := TrimSlash(Loc);
    if CompareText(Loc, ExpandConstant('{app}')) <> 0 then
      // The new setup goes to another folder: the old one is emptied (the apps are installed again in the new one).
      CleanProgramFolder(Loc, True, False)
    else
      DeleteFile(Loc + '\Uninstall.exe');
  end;
  RegDeleteKeyIncludingSubkeys(HKCU, NsisKey);
end;

// A shortcut to a program that isn't there (an app left unticked in a new folder) goes.
procedure DropDanglingShortcut(const App: String);
begin
  if not FileExists(ExpandConstant('{app}\') + App + '.exe') then
  begin
    DeleteFile(ExpandConstant('{userprograms}\') + App + '.lnk');
    DeleteFile(ExpandConstant('{userdesktop}\') + App + '.lnk');
  end;
end;

// An app lists the types it opens under "Open with", and is never made their default.
procedure OpenWith(Exts, ProgId, ExeName: String);
var
  P: Integer;
  E: String;
begin
  Exts := Exts + ',';
  while Exts <> '' do
  begin
    P := Pos(',', Exts);
    E := Copy(Exts, 1, P - 1);
    Delete(Exts, 1, P);
    RegWriteStringValue(HKCU, Cls + E + '\OpenWithProgids', ProgId, '');
    RegWriteStringValue(HKCU, Cls + 'Applications\' + ExeName + '\SupportedTypes', E, '');
  end;
end;

procedure RegisterProgId(Id, Title, ExeName: String);
begin
  RegWriteStringValue(HKCU, Cls + Id, '', Title);
  RegWriteStringValue(HKCU, Cls + Id + '\DefaultIcon', '', ExpandConstant('{app}\') + ExeName + ',0');
  RegWriteStringValue(HKCU, Cls + Id + '\shell\open\command', '', '"' + ExpandConstant('{app}\') + ExeName + '" "%1"');
end;

procedure RegisterTypes;
var
  V: String;
begin
  RegWriteStringValue(HKCU, 'Software\JeffOffice', 'InstallDir', ExpandConstant('{app}'));
  // Usage statistics: off unless chosen, written where the apps keep the choice. A silent install leaves it to the
  // program, which asks when it first starts.
  if not WizardSilent then
  begin
    RegWriteStringValue(HKCU, '{#SuiteKey}\telemetry', 'enabled', 'false');
    if WizardIsTaskSelected('stats') then
      RegWriteStringValue(HKCU, '{#SuiteKey}\telemetry', 'enabled', 'true');
  end;
  if WizardIsComponentSelected('pub') then
  begin
    // .pub files list JeffPub under "Open with" without becoming the default.
    RegisterProgId('JeffPub.PubFile', '.pub publication', 'JeffPub.exe');
    RegWriteStringValue(HKCU, Cls + '.pub\OpenWithProgids', 'JeffPub.PubFile', '');
    RegWriteStringValue(HKCU, Cls + 'Applications\JeffPub.exe\SupportedTypes', '.pub', '');
    RegWriteStringValue(HKCU, Cls + 'Applications\JeffPub.exe\SupportedTypes', '.jpub', '');
    // .jpub files that open in JeffPub keep opening in it, from its new folder.
    if RegQueryStringValue(HKCU, Cls + 'JeffPub.Publication\shell\open\command', '', V) and (V <> '') then
      RegisterProgId('JeffPub.Publication', 'JeffPub publication', 'JeffPub.exe');
  end;
  if WizardIsComponentSelected('deck') then
  begin
    RegisterProgId('JeffDeck.Presentation', 'Presentation', 'JeffDeck.exe');
    OpenWith('{#DeckExts}', 'JeffDeck.Presentation', 'JeffDeck.exe');
  end;
#ifdef WITH_WRITE
  if WizardIsComponentSelected('write') then
  begin
    RegisterProgId('JeffWrite.Document', 'Document', 'JeffWrite.exe');
    OpenWith('{#WriteExts}', 'JeffWrite.Document', 'JeffWrite.exe');
  end;
#endif
#ifdef WITH_CRUNCH
  if WizardIsComponentSelected('crunch') then
  begin
    RegisterProgId('JeffCrunch.Workbook', 'Workbook', 'JeffCrunch.exe');
    OpenWith('{#CrunchExts}', 'JeffCrunch.Workbook', 'JeffCrunch.exe');
  end;
#endif
  if WizardIsComponentSelected('pub') and WizardIsTaskSelected('jpub') then
  begin
    RegWriteStringValue(HKCU, Cls + '.jpub', '', 'JeffPub.Publication');
    RegWriteStringValue(HKCU, Cls + 'JeffPub.Publication', '', 'JeffPub publication');
    RegisterProgId('JeffPub.Publication', 'JeffPub publication', 'JeffPub.exe');
  end;
  if WizardIsComponentSelected('pub') and WizardIsTaskSelected('pubdefault') then
    RegWriteStringValue(HKCU, Cls + '.pub', '', 'JeffPub.PubFile');
end;

procedure UnOpenWith(Exts, ProgId: String);
var
  P: Integer;
begin
  Exts := Exts + ',';
  while Exts <> '' do
  begin
    P := Pos(',', Exts);
    RegDeleteValue(HKCU, Cls + Copy(Exts, 1, P - 1) + '\OpenWithProgids', ProgId);
    Delete(Exts, 1, P);
  end;
end;

// An app left unticked in an update goes: its program, shortcuts (a taskbar pin too, which would open nothing), and
// file types. Its settings stay, for when it is ticked again.
procedure RemoveApp(const App, ProgId, Exts: String);
begin
  DeleteFile(ExpandConstant('{app}\') + App + '.exe');
  DeleteFile(ExpandConstant('{userprograms}\') + App + '.lnk');
  DeleteFile(ExpandConstant('{userdesktop}\') + App + '.lnk');
  DeleteFile(ExpandConstant('{userappdata}\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\') + App + '.lnk');
  UnOpenWith(Exts, ProgId);
  RegDeleteKeyIncludingSubkeys(HKCU, Cls + ProgId);
  RegDeleteKeyIncludingSubkeys(HKCU, Cls + 'Applications\' + App + '.exe');
end;

procedure RemoveUnticked;
var
  V: String;
begin
  if not WizardIsComponentSelected('pub') then
  begin
    RemoveApp('JeffPub', 'JeffPub.PubFile', '.pub');
    if RegQueryStringValue(HKCU, Cls + '.pub', '', V) and (V = 'JeffPub.PubFile') then
      RegDeleteValue(HKCU, Cls + '.pub', '');
    if RegQueryStringValue(HKCU, Cls + '.jpub', '', V) and (V = 'JeffPub.Publication') then
      RegDeleteKeyIncludingSubkeys(HKCU, Cls + '.jpub');
    RegDeleteKeyIncludingSubkeys(HKCU, Cls + 'JeffPub.Publication');
  end;
  if not WizardIsComponentSelected('deck') then
    RemoveApp('JeffDeck', 'JeffDeck.Presentation', '{#DeckExts}');
#ifdef WITH_WRITE
  if not WizardIsComponentSelected('write') then
    RemoveApp('JeffWrite', 'JeffWrite.Document', '{#WriteExts}');
#endif
#ifdef WITH_CRUNCH
  if not WizardIsComponentSelected('crunch') then
    RemoveApp('JeffCrunch', 'JeffCrunch.Workbook', '{#CrunchExts}');
#endif
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssInstall then
  begin
    // An earlier JeffPub of its own is moved in only when JeffPub is ticked; otherwise it is left as it is.
    if WizardIsComponentSelected('pub') then
    begin
      MoveFromJeffPub;
      RemoveJeffPub79;
    end;
    RemoveNsisInstall;
    // The files an earlier version had and this one doesn't ship are cleared before the new ones are copied.
    CleanProgramFolder(ExpandConstant('{app}'), False, False);
    RemoveUnticked;
  end
  else if CurStep = ssPostInstall then
  begin
    RegisterTypes;
    DropDanglingShortcut('JeffPub');
    DropDanglingShortcut('JeffDeck');
    DropDanglingShortcut('JeffWrite');
    DropDanglingShortcut('JeffCrunch');
  end;
end;

// ---- uninstall: everything setup added, and nothing else ----

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  V: String;
begin
  if CurUninstallStep <> usPostUninstall then
    Exit;
  DeleteFile(ExpandConstant('{userprograms}\JeffPub.lnk'));
  DeleteFile(ExpandConstant('{userdesktop}\JeffPub.lnk'));
  DeleteFile(ExpandConstant('{userprograms}\JeffDeck.lnk'));
  DeleteFile(ExpandConstant('{userdesktop}\JeffDeck.lnk'));
  DeleteFile(ExpandConstant('{userprograms}\JeffWrite.lnk'));
  DeleteFile(ExpandConstant('{userdesktop}\JeffWrite.lnk'));
  DeleteFile(ExpandConstant('{userprograms}\JeffCrunch.lnk'));
  DeleteFile(ExpandConstant('{userdesktop}\JeffCrunch.lnk'));
  UnOpenWith('{#WriteExts}', 'JeffWrite.Document');
  RegDeleteKeyIncludingSubkeys(HKCU, Cls + 'JeffWrite.Document');
  RegDeleteKeyIncludingSubkeys(HKCU, Cls + 'Applications\JeffWrite.exe');
  RegDeleteKeyIncludingSubkeys(HKCU, '{#WriteSetKey}');
  UnOpenWith('{#CrunchExts}', 'JeffCrunch.Workbook');
  RegDeleteKeyIncludingSubkeys(HKCU, Cls + 'JeffCrunch.Workbook');
  RegDeleteKeyIncludingSubkeys(HKCU, Cls + 'Applications\JeffCrunch.exe');
  RegDeleteKeyIncludingSubkeys(HKCU, '{#CrunchSetKey}');
  UnOpenWith('{#DeckExts}', 'JeffDeck.Presentation');
  RegDeleteKeyIncludingSubkeys(HKCU, Cls + 'JeffDeck.Presentation');
  RegDeleteKeyIncludingSubkeys(HKCU, Cls + 'Applications\JeffDeck.exe');
  RegDeleteKeyIncludingSubkeys(HKCU, '{#DeckSetKey}');
  // The apps and what earlier setups left beside them (the files this setup installed went already).
  UnCleanProgramFolder(ExpandConstant('{app}'), True, True);
  if RegQueryStringValue(HKCU, Cls + '.pub', '', V) and (V = 'JeffPub.PubFile') then
    RegDeleteValue(HKCU, Cls + '.pub', '');
  if RegQueryStringValue(HKCU, Cls + '.jpub', '', V) and (V = 'JeffPub.Publication') then
    RegDeleteKeyIncludingSubkeys(HKCU, Cls + '.jpub');
  RegDeleteValue(HKCU, Cls + '.pub\OpenWithProgids', 'JeffPub.PubFile');
  RegDeleteKeyIncludingSubkeys(HKCU, Cls + 'JeffPub.PubFile');
  RegDeleteKeyIncludingSubkeys(HKCU, Cls + 'JeffPub.Publication');
  RegDeleteKeyIncludingSubkeys(HKCU, Cls + 'Applications\JeffPub.exe');
  RegDeleteKeyIncludingSubkeys(HKCU, '{#SuiteKey}');
  RegDeleteKeyIncludingSubkeys(HKCU, '{#PubSetKey}');
  RegDeleteValue(HKCU, 'Software\JeffOffice', 'InstallDir');
  RegDeleteValue(HKCU, 'Software\JeffOffice', 'JeffPubInstallDir');
  RegDeleteKeyIfEmpty(HKCU, 'Software\JeffOffice');
  RegDeleteKeyIncludingSubkeys(HKCU, 'Software\JeffPub79');
end;
