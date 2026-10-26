; JeffOffice Windows installer (NSIS). Built by CI from dist\JeffOffice.
; Installs the suite's apps that are ready (JeffPub and JeffDeck, and
; JeffWrite and JeffCrunch as previews) into one folder, per user: no administrator rights needed. The
; apps still being built are listed as coming soon and can't be chosen yet.
; JeffWrite and JeffCrunch were among those until the owner decided each ships: building with
; /DWITH_WRITE or /DWITH_CRUNCH (the workflow does, when its SHIP_WRITE or SHIP_CRUNCH is "true") makes
; it a preview that is not ticked by default (it is ticked when it is already installed).
Unicode true
ManifestDPIAware true
!include "MUI2.nsh"
!include "FileFunc.nsh"
!include "Sections.nsh"
!include "LogicLib.nsh"

!ifndef VERSION
  !define VERSION "0.1.0"
!endif
!define SUITE "JeffOffice"
!define APP "JeffPub"
!define EXE "JeffPub.exe"
!define DECK "JeffDeck"
!define DECKEXE "JeffDeck.exe"
!ifdef WITH_WRITE
  !define WRITE "JeffWrite"
  !define WRITEEXE "JeffWrite.exe"
  ; The folder dist\JeffOffice then holds JeffWrite.exe, which only its own section installs.
  !define NOTWRITE '/x "${WRITEEXE}"'
!else
  !define NOTWRITE ""
!endif
!ifdef WITH_CRUNCH
  !define CRUNCH "JeffCrunch"
  !define CRUNCHEXE "JeffCrunch.exe"
  ; The folder dist\JeffOffice then holds JeffCrunch.exe, which only its own section installs.
  !define NOTCRUNCH '/x "${CRUNCHEXE}"'
!else
  !define NOTCRUNCH ""
!endif
!define UNKEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\JeffOffice"
; Through 0.6.10, setup installed JeffPub on its own; this setup moves it in.
!define PUBKEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\JeffPub"
; JeffPub was called JeffPub 79 through version 0.5.0; setup replaces that install too.
!define OLDKEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\JeffPub79"
; Where the apps keep their settings (QSettings, organization JeffOffice): the
; suite's own, which holds the statistics choice, and JeffPub's.
!define SUITEKEY "Software\JeffOffice\JeffOffice"
!define PUBSETKEY "Software\JeffOffice\JeffPub"
!define DECKSETKEY "Software\JeffOffice\JeffDeck"
!ifdef WITH_WRITE
  !define WRITESETKEY "Software\JeffOffice\JeffWrite"
!endif
!ifdef WITH_CRUNCH
  !define CRUNCHSETKEY "Software\JeffOffice\JeffCrunch"
!endif
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
!define WEL_START "This will install ${SUITE} ${VERSION}, a free, open-source office suite.$\r$\n$\r$\n${APP}, for desktop publishing, and ${DECK}, for presentations, are ready now"
!define WEL_END " If ${APP} is already installed, setup moves it into ${SUITE}, with your settings.$\r$\n$\r$\n${SUITE} is an independent open-source project.$\r$\n$\r$\nNo administrator rights are needed."
!ifdef WITH_WRITE
!ifdef WITH_CRUNCH
!define MUI_WELCOMEPAGE_TEXT "${WEL_START}, and ${WRITE}, for documents, and ${CRUNCH}, for spreadsheets, come as previews. JeffMail and JeffBase are coming soon.${WEL_END}"
!else
!define MUI_WELCOMEPAGE_TEXT "${WEL_START}, and ${WRITE}, for documents, comes as a preview. JeffCrunch, JeffMail, and JeffBase are coming soon.${WEL_END}"
!endif
!else
!ifdef WITH_CRUNCH
!define MUI_WELCOMEPAGE_TEXT "${WEL_START}, and ${CRUNCH}, for spreadsheets, comes as a preview. JeffWrite, JeffMail, and JeffBase are coming soon.${WEL_END}"
!else
!define MUI_WELCOMEPAGE_TEXT "${WEL_START}. JeffWrite, JeffCrunch, JeffMail, and JeffBase are coming soon.${WEL_END}"
!endif
!endif
!insertmacro MUI_PAGE_WELCOME
; The GNU GPL v3.0: Next stays disabled until the box is checked.
!define MUI_LICENSEPAGE_TEXT_TOP "${SUITE} is free software under the GNU General Public License v3.0."
!define MUI_LICENSEPAGE_TEXT_BOTTOM "Please read the license. To install ${SUITE}, you must accept its terms."
!define MUI_LICENSEPAGE_CHECKBOX
!define MUI_LICENSEPAGE_CHECKBOX_TEXT "I accept the terms of the GNU General Public License v3.0"
!insertmacro MUI_PAGE_LICENSE "..\..\LICENSE"
; The description goes under the list, so the list is as wide as the page (ComponentsShow lays the rest out).
!define MUI_COMPONENTSPAGE_SMALLDESC
!define MUI_COMPONENTSPAGE_TEXT_TOP "Choose what to install. Apps marked coming soon come in an update."
!define MUI_PAGE_CUSTOMFUNCTION_SHOW ComponentsShow
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_TITLE "${SUITE} is ready"
!define MUI_FINISHPAGE_TEXT "${SUITE} ${VERSION} is installed. You'll find its apps in the Start menu."
!define MUI_FINISHPAGE_RUN "$INSTDIR\${EXE}"
!define MUI_FINISHPAGE_RUN_TEXT "Start ${APP}"
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

; Files in use can't be replaced, so the apps must be closed first. JeffPub
; (since 0.1.6) and JeffDeck hold a named mutex while they run; any version
; keeps its .exe locked, which catches older ones too.
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
    System::Call 'kernel32::OpenMutexW(i 0x00100000, i 0, w "JeffDeckRunning") p .r1'
    IntCmp $1 0 no_deck_mutex
      System::Call 'kernel32::CloseHandle(p r1)'
      StrCpy $R9 1
  no_deck_mutex:
!ifdef WITH_WRITE
    System::Call 'kernel32::OpenMutexW(i 0x00100000, i 0, w "JeffWriteRunning") p .r1'
    IntCmp $1 0 no_write_mutex
      System::Call 'kernel32::CloseHandle(p r1)'
      StrCpy $R9 1
  no_write_mutex:
!endif
!ifdef WITH_CRUNCH
    System::Call 'kernel32::OpenMutexW(i 0x00100000, i 0, w "JeffCrunchRunning") p .r1'
    IntCmp $1 0 no_crunch_mutex
      System::Call 'kernel32::CloseHandle(p r1)'
      StrCpy $R9 1
  no_crunch_mutex:
!endif
    ; A virus scanner can hold the program for a moment after it closes,
    ; so a locked file is checked again for up to 5 seconds (unless a mutex
    ; already said it's running).
    IntCmp $R9 1 decide
    StrCpy $R8 0
  check_file:
    IfFileExists "$INSTDIR\${EXE}" 0 check_deck
      ClearErrors
      FileOpen $2 "$INSTDIR\${EXE}" a
      IfErrors 0 unlocked
        IntOp $R8 $R8 + 1
        IntCmp $R8 10 locked
        Sleep 500
        Goto check_file
    unlocked:
      FileClose $2
  check_deck:
    IfFileExists "$INSTDIR\${DECKEXE}" 0 decide
      ClearErrors
      FileOpen $2 "$INSTDIR\${DECKEXE}" a
      IfErrors 0 deck_unlocked
        IntOp $R8 $R8 + 1
        IntCmp $R8 10 locked
        Sleep 500
        Goto check_deck
    deck_unlocked:
      FileClose $2
      Goto decide
    locked:
      StrCpy $R9 1
  decide:
    IntCmp $R9 0 not_running
!ifdef WITH_WRITE
!ifdef WITH_CRUNCH
      MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION "${APP}, ${DECK}, ${WRITE}, or ${CRUNCH} is running.$\r$\n$\r$\nSave your work and close it, then click OK to continue. Click Cancel to stop." IDOK check_running
!else
      MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION "${APP}, ${DECK}, or ${WRITE} is running.$\r$\n$\r$\nSave your work and close it, then click OK to continue. Click Cancel to stop." IDOK check_running
!endif
!else
!ifdef WITH_CRUNCH
      MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION "${APP}, ${DECK}, or ${CRUNCH} is running.$\r$\n$\r$\nSave your work and close it, then click OK to continue. Click Cancel to stop." IDOK check_running
!else
      MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION "${APP} or ${DECK} is running.$\r$\n$\r$\nSave your work and close it, then click OK to continue. Click Cancel to stop." IDOK check_running
!endif
!endif
      Abort
  not_running:
!macroend

; A presentation type lists JeffDeck under "Open with" (and JeffDeck lists the
; type), never as its default; uninstalling takes both away again.
!macro DeckOpensWith ext
  WriteRegStr HKCU "Software\Classes\${ext}\OpenWithProgids" "JeffDeck.Presentation" ""
  WriteRegStr HKCU "Software\Classes\Applications\${DECKEXE}\SupportedTypes" "${ext}" ""
!macroend
!macro un.DeckOpensWith ext
  DeleteRegValue HKCU "Software\Classes\${ext}\OpenWithProgids" "JeffDeck.Presentation"
!macroend

!ifdef WITH_WRITE
; A document type lists JeffWrite under "Open with", never as its default; uninstalling takes both away again.
!macro WriteOpensWith ext
  WriteRegStr HKCU "Software\Classes\${ext}\OpenWithProgids" "JeffWrite.Document" ""
  WriteRegStr HKCU "Software\Classes\Applications\${WRITEEXE}\SupportedTypes" "${ext}" ""
!macroend
!macro un.WriteOpensWith ext
  DeleteRegValue HKCU "Software\Classes\${ext}\OpenWithProgids" "JeffWrite.Document"
!macroend
!endif

!ifdef WITH_CRUNCH
; A workbook type lists JeffCrunch under "Open with", never as its default; uninstalling takes both away again.
!macro CrunchOpensWith ext
  WriteRegStr HKCU "Software\Classes\${ext}\OpenWithProgids" "JeffCrunch.Workbook" ""
  WriteRegStr HKCU "Software\Classes\Applications\${CRUNCHEXE}\SupportedTypes" "${ext}" ""
!macroend
!macro un.CrunchOpensWith ext
  DeleteRegValue HKCU "Software\Classes\${ext}\OpenWithProgids" "JeffCrunch.Workbook"
!macroend
!endif

Function .onInit
  !insertmacro WaitForAppToClose
  Call ComingSoon
  Call PresetWrite
  Call PresetCrunch
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
  File /r /x "${DECKEXE}" ${NOTWRITE} ${NOTCRUNCH} "..\..\dist\JeffOffice\*.*"
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
  WriteRegStr HKCU "${UNKEY}" "URLInfoAbout" "https://github.com/JeffOffice/jeffoffice"
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

; Installed unless unticked, beside JeffPub (the files they share
; come with JeffPub's section).
Section "${DECK}: presentations" SecDeck
  SetOutPath "$INSTDIR"
  File "..\..\dist\JeffOffice\${DECKEXE}"
  CreateShortcut "$SMPROGRAMS\${DECK}.lnk" "$INSTDIR\${DECKEXE}"
  IfFileExists "${PINS}\${DECK}.lnk" 0 +2
    CreateShortcut "${PINS}\${DECK}.lnk" "$INSTDIR\${DECKEXE}"
  ; Presentations list JeffDeck under "Open with" without it becoming their default.
  WriteRegStr HKCU "Software\Classes\JeffDeck.Presentation" "" "Presentation"
  WriteRegStr HKCU "Software\Classes\JeffDeck.Presentation\DefaultIcon" "" "$INSTDIR\${DECKEXE},0"
  WriteRegStr HKCU "Software\Classes\JeffDeck.Presentation\shell\open\command" "" '"$INSTDIR\${DECKEXE}" "%1"'
  !insertmacro DeckOpensWith ".pptx"
  !insertmacro DeckOpensWith ".potx"
  !insertmacro DeckOpensWith ".ppsx"
  !insertmacro DeckOpensWith ".ppt"
  !insertmacro DeckOpensWith ".pot"
  !insertmacro DeckOpensWith ".pps"
  !insertmacro DeckOpensWith ".odp"
  !insertmacro DeckOpensWith ".otp"
  System::Call 'shell32::SHChangeNotify(i 0x08000000, i 0, p 0, p 0)'
SectionEnd

; Coming soon: listed so people see what's ahead (ComingSoon makes them read-only).
!ifdef WITH_WRITE
; A preview, not ticked until the person ticks it.
Section /o "${WRITE}: documents (preview)" SecWrite
  SetOutPath "$INSTDIR"
  File "..\..\dist\JeffOffice\${WRITEEXE}"
  CreateShortcut "$SMPROGRAMS\${WRITE}.lnk" "$INSTDIR\${WRITEEXE}"
  IfFileExists "${PINS}\${WRITE}.lnk" 0 +2
    CreateShortcut "${PINS}\${WRITE}.lnk" "$INSTDIR\${WRITEEXE}"
  ; Documents list JeffWrite under "Open with" without it becoming their default.
  WriteRegStr HKCU "Software\Classes\JeffWrite.Document" "" "Document"
  WriteRegStr HKCU "Software\Classes\JeffWrite.Document\DefaultIcon" "" "$INSTDIR\${WRITEEXE},0"
  WriteRegStr HKCU "Software\Classes\JeffWrite.Document\shell\open\command" "" '"$INSTDIR\${WRITEEXE}" "%1"'
  !insertmacro WriteOpensWith ".docx"
  !insertmacro WriteOpensWith ".docm"
  !insertmacro WriteOpensWith ".dotx"
  !insertmacro WriteOpensWith ".dotm"
  !insertmacro WriteOpensWith ".doc"
  !insertmacro WriteOpensWith ".dot"
  System::Call 'shell32::SHChangeNotify(i 0x08000000, i 0, p 0, p 0)'
SectionEnd
!else
Section /o "JeffWrite: documents (coming soon)" SecWrite
SectionEnd
!endif
!ifdef WITH_CRUNCH
; A preview, not ticked until the person ticks it.
Section /o "${CRUNCH}: spreadsheets (preview)" SecCrunch
  SetOutPath "$INSTDIR"
  File "..\..\dist\JeffOffice\${CRUNCHEXE}"
  CreateShortcut "$SMPROGRAMS\${CRUNCH}.lnk" "$INSTDIR\${CRUNCHEXE}"
  IfFileExists "${PINS}\${CRUNCH}.lnk" 0 +2
    CreateShortcut "${PINS}\${CRUNCH}.lnk" "$INSTDIR\${CRUNCHEXE}"
  ; Workbooks list JeffCrunch under "Open with" without it becoming their default.
  WriteRegStr HKCU "Software\Classes\JeffCrunch.Workbook" "" "Workbook"
  WriteRegStr HKCU "Software\Classes\JeffCrunch.Workbook\DefaultIcon" "" "$INSTDIR\${CRUNCHEXE},0"
  WriteRegStr HKCU "Software\Classes\JeffCrunch.Workbook\shell\open\command" "" '"$INSTDIR\${CRUNCHEXE}" "%1"'
  !insertmacro CrunchOpensWith ".xlsx"
  !insertmacro CrunchOpensWith ".xlsm"
  !insertmacro CrunchOpensWith ".xls"
  !insertmacro CrunchOpensWith ".ods"
  !insertmacro CrunchOpensWith ".csv"
  System::Call 'shell32::SHChangeNotify(i 0x08000000, i 0, p 0, p 0)'
SectionEnd
!else
Section /o "JeffCrunch: spreadsheets (coming soon)" SecCrunch
SectionEnd
!endif
Section /o "JeffMail: email and calendar (coming soon)" SecMail
SectionEnd
Section /o "JeffBase: databases (coming soon)" SecBase
SectionEnd

SectionGroupEnd

SectionGroup /e "Options" SecOptions

Section "Desktop shortcuts" SecDesktop
  CreateShortcut "$DESKTOP\${APP}.lnk" "$INSTDIR\${EXE}"
  ${If} ${SectionIsSelected} ${SecDeck}
    CreateShortcut "$DESKTOP\${DECK}.lnk" "$INSTDIR\${DECKEXE}"
  ${EndIf}
!ifdef WITH_WRITE
  ${If} ${SectionIsSelected} ${SecWrite}
    CreateShortcut "$DESKTOP\${WRITE}.lnk" "$INSTDIR\${WRITEEXE}"
  ${EndIf}
!endif
!ifdef WITH_CRUNCH
  ${If} ${SectionIsSelected} ${SecCrunch}
    CreateShortcut "$DESKTOP\${CRUNCH}.lnk" "$INSTDIR\${CRUNCHEXE}"
  ${EndIf}
!endif
SectionEnd

Section "Open .jpub files with ${APP}" SecJpub
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

; Lays the components page out so every option shows at once, with no scrolling: the list spans the
; page and is as tall as its 12 rows, the description and the space needed sit under it, and the
; top text and the list's label (the header says the same) are hidden. Sizes come from the page's
; own client area and the list's row height, so it holds at any display scale.
; Control ids: 1006 top text, 1021 and 1022 labels, 1023 space required, 1032 list,
; 1042 description frame, 1043 description text.
Function ComponentsShow
  FindWindow $R0 "#32770" "" $HWNDPARENT
  GetDlgItem $R1 $R0 1032
  GetDlgItem $R2 $R0 1043
  GetDlgItem $R3 $R0 1023
  GetDlgItem $0 $R0 1006
  ShowWindow $0 ${SW_HIDE}
  GetDlgItem $0 $R0 1021
  ShowWindow $0 ${SW_HIDE}
  GetDlgItem $0 $R0 1022
  ShowWindow $0 ${SW_HIDE}
  GetDlgItem $0 $R0 1042
  ShowWindow $0 ${SW_HIDE}
  ; The page's width ($R6) and height ($R7).
  System::Call '*(i 0, i 0, i 0, i 0) p .R5'
  System::Call 'user32::GetClientRect(p $R0, p $R5)'
  System::Call '*$R5(i, i, i .R6, i .R7)'
  System::Free $R5
  ; The list: 12 rows of its row height, plus its border; never so tall that under two lines of text is left below.
  SendMessage $R1 0x111C 0 0 $R8
  IntOp $R9 $R8 * 12
  System::Call 'user32::GetSystemMetrics(i 46) i .r0'
  IntOp $0 $0 * 2
  IntOp $R9 $R9 + $0
  IntOp $1 $R8 * 17
  IntOp $1 $1 / 10
  IntOp $1 $R7 - $1
  IntCmp $R9 $1 +2 +2 0
    StrCpy $R9 $1
  System::Call 'user32::SetWindowPos(p $R1, p 0, i 0, i 0, i $R6, i $R9, i 0x14)'
  ; Under it: the description on the left (with our own prompt until an item is chosen), the space needed on the right.
  SendMessage $R2 ${WM_SETTEXT} 0 "STR:Click an item to see what it does."
  IntOp $1 $R8 / 8
  IntOp $2 $R9 + $1
  IntOp $3 $R7 - $2
  IntOp $4 $R6 * 63
  IntOp $4 $4 / 100
  System::Call 'user32::SetWindowPos(p $R2, p 0, i 0, i $2, i $4, i $3, i 0x14)'
  IntOp $5 $R6 * 66
  IntOp $5 $5 / 100
  IntOp $6 $R6 - $5
  System::Call 'user32::SetWindowPos(p $R3, p 0, i $5, i $2, i $6, i $3, i 0x14)'
FunctionEnd

; The apps not ready yet are listed, unticked, and can't be ticked.
; (After the sections: their ids are defined there.)
Function ComingSoon
!ifndef WITH_WRITE
  SectionSetFlags ${SecWrite} ${SF_RO}
!endif
!ifndef WITH_CRUNCH
  SectionSetFlags ${SecCrunch} ${SF_RO}
!endif
  SectionSetFlags ${SecMail} ${SF_RO}
  SectionSetFlags ${SecBase} ${SF_RO}
FunctionEnd

; An update keeps JeffWrite where it is installed: its section starts ticked,
; so its files are updated with the rest and never left a version behind.
Function PresetWrite
!ifdef WITH_WRITE
  IfFileExists "$INSTDIR\${WRITEEXE}" 0 +2
    SectionSetFlags ${SecWrite} ${SF_SELECTED}
!endif
FunctionEnd

; The same for JeffCrunch.
Function PresetCrunch
!ifdef WITH_CRUNCH
  IfFileExists "$INSTDIR\${CRUNCHEXE}" 0 +2
    SectionSetFlags ${SecCrunch} ${SF_SELECTED}
!endif
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
!ifdef WITH_WRITE
!ifdef WITH_CRUNCH
  !insertmacro MUI_DESCRIPTION_TEXT ${SecApps} "${APP} and ${DECK} are ready. ${WRITE} and ${CRUNCH} are previews."
!else
  !insertmacro MUI_DESCRIPTION_TEXT ${SecApps} "${APP} and ${DECK} are ready. ${WRITE} is a preview."
!endif
!else
!ifdef WITH_CRUNCH
  !insertmacro MUI_DESCRIPTION_TEXT ${SecApps} "${APP} and ${DECK} are ready. ${CRUNCH} is a preview."
!else
  !insertmacro MUI_DESCRIPTION_TEXT ${SecApps} "${APP} and ${DECK} are ready now."
!endif
!endif
  !insertmacro MUI_DESCRIPTION_TEXT ${SecMain} "Desktop publishing: flyers, newsletters, and cards. Opens .pub files."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDeck} "Presentations and slide shows. Opens and saves .pptx and .odp."
!ifdef WITH_WRITE
  !insertmacro MUI_DESCRIPTION_TEXT ${SecWrite} "Documents and reports. A preview: opens and saves .docx files."
!else
  !insertmacro MUI_DESCRIPTION_TEXT ${SecWrite} "Documents, letters, and reports. Coming soon: it can't be installed yet."
!endif
!ifdef WITH_CRUNCH
  !insertmacro MUI_DESCRIPTION_TEXT ${SecCrunch} "Spreadsheets and charts. A preview: opens and saves .xlsx files."
!else
  !insertmacro MUI_DESCRIPTION_TEXT ${SecCrunch} "Spreadsheets and charts. Coming soon: it can't be installed yet."
!endif
  !insertmacro MUI_DESCRIPTION_TEXT ${SecMail} "Email and calendars. Coming soon: it can't be installed yet."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecBase} "Databases. Coming soon: it can't be installed yet."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecOptions} "Shortcuts, file types, and usage statistics."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecStats} "Once a day: version, system, language, command counts, crashes. Never your files."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} "Adds shortcuts to the apps to your desktop."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecJpub} "Double-clicking a .jpub file opens it in ${APP}."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecPub} "Double-clicking a .pub file opens it in ${APP}. Off keeps your current program."
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Section "Uninstall"
  Delete "$SMPROGRAMS\${APP}.lnk"
  Delete "$DESKTOP\${APP}.lnk"
  Delete "$SMPROGRAMS\${DECK}.lnk"
  Delete "$DESKTOP\${DECK}.lnk"
!ifdef WITH_WRITE
  Delete "$SMPROGRAMS\${WRITE}.lnk"
  Delete "$DESKTOP\${WRITE}.lnk"
  !insertmacro un.WriteOpensWith ".docx"
  !insertmacro un.WriteOpensWith ".docm"
  !insertmacro un.WriteOpensWith ".dotx"
  !insertmacro un.WriteOpensWith ".dotm"
  !insertmacro un.WriteOpensWith ".doc"
  !insertmacro un.WriteOpensWith ".dot"
  DeleteRegKey HKCU "Software\Classes\JeffWrite.Document"
  DeleteRegKey HKCU "Software\Classes\Applications\${WRITEEXE}"
  DeleteRegKey HKCU "${WRITESETKEY}"
!endif
!ifdef WITH_CRUNCH
  Delete "$SMPROGRAMS\${CRUNCH}.lnk"
  Delete "$DESKTOP\${CRUNCH}.lnk"
  !insertmacro un.CrunchOpensWith ".xlsx"
  !insertmacro un.CrunchOpensWith ".xlsm"
  !insertmacro un.CrunchOpensWith ".xls"
  !insertmacro un.CrunchOpensWith ".ods"
  !insertmacro un.CrunchOpensWith ".csv"
  DeleteRegKey HKCU "Software\Classes\JeffCrunch.Workbook"
  DeleteRegKey HKCU "Software\Classes\Applications\${CRUNCHEXE}"
  DeleteRegKey HKCU "${CRUNCHSETKEY}"
!endif
  !insertmacro un.DeckOpensWith ".pptx"
  !insertmacro un.DeckOpensWith ".potx"
  !insertmacro un.DeckOpensWith ".ppsx"
  !insertmacro un.DeckOpensWith ".ppt"
  !insertmacro un.DeckOpensWith ".pot"
  !insertmacro un.DeckOpensWith ".pps"
  !insertmacro un.DeckOpensWith ".odp"
  !insertmacro un.DeckOpensWith ".otp"
  DeleteRegKey HKCU "Software\Classes\JeffDeck.Presentation"
  DeleteRegKey HKCU "Software\Classes\Applications\${DECKEXE}"
  DeleteRegKey HKCU "${DECKSETKEY}"
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
