; JeffOffice Windows installer (NSIS). Built by CI from dist\JeffPub.
; Installs the suite's apps that are ready (JeffPub, for now) into one folder,
; per user: no administrator rights needed. The apps still being built are
; listed as coming soon and can't be chosen yet.
Unicode true
ManifestDPIAware true
!include "MUI2.nsh"
!include "FileFunc.nsh"
!include "Sections.nsh"

!ifndef VERSION
  !define VERSION "0.1.0"
!endif
!define SUITE "JeffOffice"
!define APP "JeffPub"
!define EXE "JeffPub.exe"
!define UNKEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\JeffOffice"
; Through 0.6.10, setup installed JeffPub on its own; this setup moves it in.
!define PUBKEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\JeffPub"
; JeffPub was called JeffPub 79 through version 0.5.0; setup replaces that install too.
!define OLDKEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\JeffPub79"
; Where the apps keep their settings (QSettings, organization JeffOffice): the
; suite's own, which holds the statistics choice, and JeffPub's.
!define SUITEKEY "Software\JeffOffice\JeffOffice"
!define PUBSETKEY "Software\JeffOffice\JeffPub"
; Taskbar pins are shortcuts kept here.
!define PINS "$APPDATA\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar"

Name "${SUITE}"
OutFile "JeffOffice-Setup.exe"
RequestExecutionLevel user
InstallDir "$LOCALAPPDATA\Programs\${SUITE}"
InstallDirRegKey HKCU "Software\JeffOffice" "InstallDir"
BrandingText "${SUITE} ${VERSION}"
VIProductVersion "${VERSION}.0"
VIAddVersionKey "ProductName" "${SUITE}"
VIAddVersionKey "FileDescription" "${SUITE} installer"
VIAddVersionKey "FileVersion" "${VERSION}"
VIAddVersionKey "ProductVersion" "${VERSION}"
VIAddVersionKey "CompanyName" "JeffOffice LLC"
VIAddVersionKey "LegalCopyright" "Copyright (c) 2026 JeffOffice LLC. Free software under the GNU GPL v3."

!define MUI_ICON "jeffoffice.ico"
!define MUI_UNICON "jeffoffice.ico"
!define MUI_ABORTWARNING
; Branded artwork (tools/make_installer_art.py), drawn at twice the size so it
; stays sharp on high-DPI screens.
!define MUI_WELCOMEFINISHPAGE_BITMAP "welcome.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "welcome.bmp"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP "header.bmp"
!define MUI_HEADERIMAGE_UNBITMAP "header.bmp"
!define MUI_WELCOMEPAGE_TITLE "Welcome to ${SUITE}"
!define MUI_WELCOMEPAGE_TEXT "This will install ${SUITE} ${VERSION}, a free, open-source office suite.$\r$\n$\r$\n${APP}, for desktop publishing, is ready now. JeffDeck, JeffWrite, JeffCrunch, JeffMail, and JeffBase are coming soon. If ${APP} is already installed, setup moves it into ${SUITE}, with your settings.$\r$\n$\r$\n${SUITE} is an independent open-source project.$\r$\n$\r$\nNo administrator rights are needed."
!insertmacro MUI_PAGE_WELCOME
; The GNU GPL v3.0: Next stays disabled until the box is checked.
!define MUI_LICENSEPAGE_TEXT_TOP "${SUITE} is free software under the GNU General Public License v3.0."
!define MUI_LICENSEPAGE_TEXT_BOTTOM "Please read the license. To install ${SUITE}, you must accept its terms."
!define MUI_LICENSEPAGE_CHECKBOX
!define MUI_LICENSEPAGE_CHECKBOX_TEXT "I accept the terms of the GNU General Public License v3.0"
!insertmacro MUI_PAGE_LICENSE "..\..\LICENSE"
!define MUI_COMPONENTSPAGE_TEXT_TOP "Choose what to install. The apps marked coming soon can't be installed yet; they'll come in an update."
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_TITLE "${SUITE} is ready"
!define MUI_FINISHPAGE_TEXT "${SUITE} ${VERSION} is installed. You'll find ${APP} in the Start menu."
!define MUI_FINISHPAGE_RUN "$INSTDIR\${EXE}"
!define MUI_FINISHPAGE_RUN_TEXT "Start ${APP}"
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

; Files in use can't be replaced, so JeffPub must be closed first. Since
; 0.1.6 it holds a named mutex while it runs; any version keeps its .exe
; locked, which catches older ones too.
!macro WaitForAppToClose
  check_running:
    StrCpy $R9 0
    System::Call 'kernel32::OpenMutexW(i 0x00100000, i 0, w "JeffPubRunning") p .r1'
    IntCmp $1 0 no_mutex
      System::Call 'kernel32::CloseHandle(p r1)'
      StrCpy $R9 1
  no_mutex:
    System::Call 'kernel32::OpenMutexW(i 0x00100000, i 0, w "JeffPub79Running") p .r1'
    IntCmp $1 0 no_old_mutex
      System::Call 'kernel32::CloseHandle(p r1)'
      StrCpy $R9 1
  no_old_mutex:
    IfFileExists "$INSTDIR\${EXE}" 0 decide
      ClearErrors
      FileOpen $2 "$INSTDIR\${EXE}" a
      IfErrors locked
      FileClose $2
      Goto decide
    locked:
      StrCpy $R9 1
  decide:
    IntCmp $R9 0 not_running
      MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION "${APP} is running.$\r$\n$\r$\nSave your work and close ${APP}, then click OK to continue. Click Cancel to stop." IDOK check_running
      Abort
  not_running:
!macroend

Function .onInit
  !insertmacro WaitForAppToClose
  Call ComingSoon
  Call PresetStats
FunctionEnd

Function un.onInit
  !insertmacro WaitForAppToClose
FunctionEnd

SectionGroup /e "Apps" SecApps

Section "${APP}: publications" SecMain
  SectionIn RO
  Call MoveFromJeffPub
  Call RemoveJeffPub79
  SetOutPath "$INSTDIR"
  File /r "..\..\dist\JeffPub\*.*"
  File "jeffoffice.ico"
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  CreateShortcut "$SMPROGRAMS\${APP}.lnk" "$INSTDIR\${EXE}"
  ; A desktop shortcut or taskbar pin made for the old folder follows it.
  IfFileExists "$DESKTOP\${APP}.lnk" 0 +2
    CreateShortcut "$DESKTOP\${APP}.lnk" "$INSTDIR\${EXE}"
  IfFileExists "${PINS}\${APP}.lnk" 0 +2
    CreateShortcut "${PINS}\${APP}.lnk" "$INSTDIR\${EXE}"
  WriteRegStr HKCU "Software\JeffOffice" "InstallDir" "$INSTDIR"
  ; Usage statistics: off unless their section below is chosen, written where
  ; the apps keep the choice. A silent install leaves it to the program,
  ; which asks when it first starts.
  IfSilent +2
  WriteRegStr HKCU "${SUITEKEY}\telemetry" "enabled" "false"
  ; One entry in Settings > Apps, for the whole suite.
  WriteRegStr HKCU "${UNKEY}" "DisplayName" "${SUITE}"
  WriteRegStr HKCU "${UNKEY}" "DisplayVersion" "${VERSION}"
  WriteRegStr HKCU "${UNKEY}" "Publisher" "JeffOffice LLC"
  WriteRegStr HKCU "${UNKEY}" "DisplayIcon" "$INSTDIR\jeffoffice.ico"
  WriteRegStr HKCU "${UNKEY}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegStr HKCU "${UNKEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr HKCU "${UNKEY}" "URLInfoAbout" "https://github.com/JeffOffice/jeffpub"
  WriteRegDWORD HKCU "${UNKEY}" "NoModify" 1
  WriteRegDWORD HKCU "${UNKEY}" "NoRepair" 1
  ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
  WriteRegDWORD HKCU "${UNKEY}" "EstimatedSize" $0
  ; .pub files list JeffPub under "Open with" without becoming the default.
  WriteRegStr HKCU "Software\Classes\JeffPub.PubFile" "" ".pub publication"
  WriteRegStr HKCU "Software\Classes\JeffPub.PubFile\DefaultIcon" "" "$INSTDIR\${EXE},0"
  WriteRegStr HKCU "Software\Classes\JeffPub.PubFile\shell\open\command" "" '"$INSTDIR\${EXE}" "%1"'
  WriteRegStr HKCU "Software\Classes\.pub\OpenWithProgids" "JeffPub.PubFile" ""
  WriteRegStr HKCU "Software\Classes\Applications\${EXE}\SupportedTypes" ".pub" ""
  WriteRegStr HKCU "Software\Classes\Applications\${EXE}\SupportedTypes" ".jpub" ""
  ; .jpub files that open in JeffPub keep opening in it, from its new folder.
  ReadRegStr $0 HKCU "Software\Classes\JeffPub.Publication\shell\open\command" ""
  StrCmp $0 "" +3
    WriteRegStr HKCU "Software\Classes\JeffPub.Publication\DefaultIcon" "" "$INSTDIR\${EXE},0"
    WriteRegStr HKCU "Software\Classes\JeffPub.Publication\shell\open\command" "" '"$INSTDIR\${EXE}" "%1"'
SectionEnd

; Coming soon: listed so people see what's ahead (ComingSoon makes them read-only).
Section /o "JeffDeck: presentations (coming soon)" SecDeck
SectionEnd
Section /o "JeffWrite: documents (coming soon)" SecWrite
SectionEnd
Section /o "JeffCrunch: spreadsheets (coming soon)" SecCrunch
SectionEnd
Section /o "JeffMail: email and calendar (coming soon)" SecMail
SectionEnd
Section /o "JeffBase: databases (coming soon)" SecBase
SectionEnd

SectionGroupEnd

SectionGroup /e "Options" SecOptions

Section "Desktop shortcut" SecDesktop
  CreateShortcut "$DESKTOP\${APP}.lnk" "$INSTDIR\${EXE}"
SectionEnd

Section "Open JeffPub publications (.jpub) with ${APP}" SecJpub
  WriteRegStr HKCU "Software\Classes\.jpub" "" "JeffPub.Publication"
  WriteRegStr HKCU "Software\Classes\JeffPub.Publication" "" "JeffPub publication"
  WriteRegStr HKCU "Software\Classes\JeffPub.Publication\DefaultIcon" "" "$INSTDIR\${EXE},0"
  WriteRegStr HKCU "Software\Classes\JeffPub.Publication\shell\open\command" "" '"$INSTDIR\${EXE}" "%1"'
  System::Call 'shell32::SHChangeNotify(i 0x08000000, i 0, p 0, p 0)'
SectionEnd

Section /o "Make ${APP} the default for .pub files" SecPub
  WriteRegStr HKCU "Software\Classes\.pub" "" "JeffPub.PubFile"
  System::Call 'shell32::SHChangeNotify(i 0x08000000, i 0, p 0, p 0)'
SectionEnd

Section "Send anonymous usage statistics" SecStats
  IfSilent +2
  WriteRegStr HKCU "${SUITEKEY}\telemetry" "enabled" "true"
SectionEnd

SectionGroupEnd

; The apps not ready yet are listed, unticked, and can't be ticked.
; (After the sections: their ids are defined there.)
Function ComingSoon
  SectionSetFlags ${SecDeck} ${SF_RO}
  SectionSetFlags ${SecWrite} ${SF_RO}
  SectionSetFlags ${SecCrunch} ${SF_RO}
  SectionSetFlags ${SecMail} ${SF_RO}
  SectionSetFlags ${SecBase} ${SF_RO}
FunctionEnd

; An update shows the choice already made: turned off in an app (or in
; JeffPub before it kept the choice for the suite), the statistics start
; unticked.
Function PresetStats
  ReadRegStr $0 HKCU "${SUITEKEY}\telemetry" "enabled"
  StrCmp $0 "" 0 +2
    ReadRegStr $0 HKCU "${PUBSETKEY}\telemetry" "enabled"
  StrCmp $0 "" 0 +2
    ReadRegStr $0 HKCU "Software\JeffPub79\JeffPub79\telemetry" "enabled"
  StrCmp $0 "false" 0 +2
  SectionSetFlags ${SecStats} 0
FunctionEnd

; Through 0.6.10, JeffPub had its own folder and Settings > Apps entry. The
; files setup put in that folder go, one by one so nothing else there is
; touched, then the folder if that left it empty; and the entry goes. Its
; settings and files stay: the apps read them where they always were.
Function MoveFromJeffPub
  ReadRegStr $0 HKCU "${PUBKEY}" "InstallLocation"
  StrCmp $0 "" done
  StrCmp $0 $INSTDIR forget
  IfFileExists "$0\${EXE}" 0 forget
    Delete "$0\${EXE}"
    Delete "$0\Uninstall.exe"
    Delete "$0\Qt6*.dll"
    Delete "$0\libgcc_s_seh-1.dll"
    Delete "$0\libstdc++-6.dll"
    Delete "$0\libwinpthread-1.dll"
    Delete "$0\pdfium.dll"
    Delete "$0\LICENSE"
    Delete "$0\README.md"
    RMDir /r "$0\fonts"
    RMDir /r "$0\dict"
    RMDir /r "$0\licenses"
    RMDir /r "$0\generic"
    RMDir /r "$0\iconengines"
    RMDir /r "$0\imageformats"
    RMDir /r "$0\networkinformation"
    RMDir /r "$0\platforms"
    RMDir /r "$0\styles"
    RMDir /r "$0\tls"
    RMDir "$0"
  forget:
  DeleteRegKey HKCU "${PUBKEY}"
  DeleteRegValue HKCU "Software\JeffOffice" "JeffPubInstallDir"
  done:
FunctionEnd

; Takes away JeffPub 79 (0.5.0 and earlier): its program folder, shortcuts,
; file types and Settings > Apps entry. Its settings stay: JeffPub copies them
; the first time it starts. A .pub default that pointed at JeffPub 79 moves
; over, and so does a desktop shortcut.
Function RemoveJeffPub79
  ReadRegStr $0 HKCU "${OLDKEY}" "InstallLocation"
  StrCmp $0 "" types
  ; Only a folder that really holds JeffPub 79 is deleted.
  IfFileExists "$0\JeffPub79.exe" 0 entries
    StrCmp $0 $INSTDIR +2
      RMDir /r "$0"
  entries:
  Delete "$SMPROGRAMS\JeffPub 79.lnk"
  IfFileExists "$DESKTOP\JeffPub 79.lnk" 0 +3
    Delete "$DESKTOP\JeffPub 79.lnk"
    CreateShortcut "$DESKTOP\${APP}.lnk" "$INSTDIR\${EXE}"
  DeleteRegKey HKCU "${OLDKEY}"
  types:
  ReadRegStr $1 HKCU "Software\Classes\.pub" ""
  StrCmp $1 "JeffPub79.PubFile" 0 +2
    WriteRegStr HKCU "Software\Classes\.pub" "" "JeffPub.PubFile"
  ReadRegStr $1 HKCU "Software\Classes\.jpub" ""
  StrCmp $1 "JeffPub79.Publication" 0 +2
    WriteRegStr HKCU "Software\Classes\.jpub" "" "JeffPub.Publication"
  DeleteRegValue HKCU "Software\Classes\.pub\OpenWithProgids" "JeffPub79.PubFile"
  DeleteRegKey HKCU "Software\Classes\JeffPub79.PubFile"
  DeleteRegKey HKCU "Software\Classes\JeffPub79.Publication"
  DeleteRegKey HKCU "Software\Classes\Applications\JeffPub79.exe"
  DeleteRegValue HKCU "Software\JeffPub79" "InstallDir"
FunctionEnd

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecApps} "The ${SUITE} apps. ${APP} is ready now; the others are coming soon."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecMain} "Desktop publishing: flyers, newsletters, cards, and more. Opens .pub files. Comes with its fonts and spelling dictionaries."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDeck} "Presentations and slide shows. Coming soon: it can't be installed yet."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecWrite} "Documents, letters, and reports. Coming soon: it can't be installed yet."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecCrunch} "Spreadsheets and charts. Coming soon: it can't be installed yet."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecMail} "Email and calendars. Coming soon: it can't be installed yet."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecBase} "Databases. Coming soon: it can't be installed yet."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecOptions} "Shortcuts, file types, and usage statistics."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecStats} "Once a day (and when updated), the apps send their version, your operating system and language, and how often each of their commands is used; after a crash, also where in the program it happened. Never your files, their names, or anything in them. You can change this later in File > Options."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} "Adds a shortcut to your desktop."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecJpub} "Double-clicking a .jpub file opens it in ${APP}."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecPub} "Double-clicking a .pub file opens it in ${APP} instead of the program that opens it now. Leave this off to keep that program as the default; ${APP} is always available under Open with."
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Section "Uninstall"
  Delete "$SMPROGRAMS\${APP}.lnk"
  Delete "$DESKTOP\${APP}.lnk"
  RMDir /r "$INSTDIR"
  ReadRegStr $0 HKCU "Software\Classes\.pub" ""
  StrCmp $0 "JeffPub.PubFile" 0 +2
    DeleteRegValue HKCU "Software\Classes\.pub" ""
  ReadRegStr $0 HKCU "Software\Classes\.jpub" ""
  StrCmp $0 "JeffPub.Publication" 0 +2
    DeleteRegKey HKCU "Software\Classes\.jpub"
  DeleteRegValue HKCU "Software\Classes\.pub\OpenWithProgids" "JeffPub.PubFile"
  DeleteRegKey HKCU "Software\Classes\JeffPub.PubFile"
  DeleteRegKey HKCU "Software\Classes\JeffPub.Publication"
  DeleteRegKey HKCU "Software\Classes\Applications\${EXE}"
  DeleteRegKey HKCU "${UNKEY}"
  DeleteRegKey HKCU "${SUITEKEY}"
  DeleteRegKey HKCU "${PUBSETKEY}"
  DeleteRegValue HKCU "Software\JeffOffice" "InstallDir"
  DeleteRegValue HKCU "Software\JeffOffice" "JeffPubInstallDir"
  DeleteRegKey /ifempty HKCU "Software\JeffOffice"
  DeleteRegKey HKCU "Software\JeffPub79"
  System::Call 'shell32::SHChangeNotify(i 0x08000000, i 0, p 0, p 0)'
SectionEnd
