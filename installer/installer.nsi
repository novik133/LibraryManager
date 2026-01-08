; Library Manager Installer Script
; Author: Dawid Papaj
; NSIS Installer for Library Manager

!include "MUI2.nsh"

; General settings
Name "Library Manager"
OutFile "LibraryManager_Setup.exe"
InstallDir "$PROGRAMFILES\LibraryManager"
InstallDirRegKey HKLM "Software\LibraryManager" "InstallDir"
RequestExecutionLevel admin

; Interface settings
!define MUI_ABORTWARNING
!define MUI_ICON "..\src\app.ico"
!define MUI_UNICON "..\src\app.ico"

; Welcome page
!define MUI_WELCOMEPAGE_TITLE "Welcome to Library Manager Setup"
!define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of Library Manager.$\r$\n$\r$\nAuthor: Dawid Papaj$\r$\n$\r$\nClick Next to continue."

; Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\LICENSE.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Language
!insertmacro MUI_LANGUAGE "English"

; Version info
VIProductVersion "1.0.0.0"
VIAddVersionKey "ProductName" "Library Manager"
VIAddVersionKey "CompanyName" "Dawid Papaj"
VIAddVersionKey "LegalCopyright" "Copyright (c) 2026 Dawid Papaj"
VIAddVersionKey "FileDescription" "Library Manager Installer"
VIAddVersionKey "FileVersion" "1.0.0.0"
VIAddVersionKey "ProductVersion" "1.0.0.0"

; Installer section
Section "Install"
    SetOutPath "$INSTDIR"
    
    ; Copy files
    File "..\build\Release\LibraryManager.exe"
    File "..\src\app.ico"
    File "..\LICENSE.txt"
    
    ; Create start menu shortcuts
    CreateDirectory "$SMPROGRAMS\Library Manager"
    CreateShortcut "$SMPROGRAMS\Library Manager\Library Manager.lnk" "$INSTDIR\LibraryManager.exe" "" "$INSTDIR\app.ico"
    CreateShortcut "$SMPROGRAMS\Library Manager\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
    
    ; Create desktop shortcut
    CreateShortcut "$DESKTOP\Library Manager.lnk" "$INSTDIR\LibraryManager.exe" "" "$INSTDIR\app.ico"
    
    ; Write registry keys
    WriteRegStr HKLM "Software\LibraryManager" "InstallDir" "$INSTDIR"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LibraryManager" "DisplayName" "Library Manager"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LibraryManager" "UninstallString" "$INSTDIR\Uninstall.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LibraryManager" "DisplayIcon" "$INSTDIR\app.ico"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LibraryManager" "Publisher" "Dawid Papaj"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LibraryManager" "DisplayVersion" "1.0.0"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LibraryManager" "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LibraryManager" "NoRepair" 1
    
    ; Create uninstaller
    WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

; Uninstaller section
Section "Uninstall"
    ; Remove files
    Delete "$INSTDIR\LibraryManager.exe"
    Delete "$INSTDIR\app.ico"
    Delete "$INSTDIR\LICENSE.txt"
    Delete "$INSTDIR\library.db"
    Delete "$INSTDIR\Uninstall.exe"
    
    ; Remove shortcuts
    Delete "$SMPROGRAMS\Library Manager\Library Manager.lnk"
    Delete "$SMPROGRAMS\Library Manager\Uninstall.lnk"
    RMDir "$SMPROGRAMS\Library Manager"
    Delete "$DESKTOP\Library Manager.lnk"
    
    ; Remove directories
    RMDir "$INSTDIR"
    
    ; Remove registry keys
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LibraryManager"
    DeleteRegKey HKLM "Software\LibraryManager"
SectionEnd
