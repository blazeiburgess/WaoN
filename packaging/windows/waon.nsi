; WaoN NSIS Installer Script
; Requires NSIS 3.0 or later

!include "MUI2.nsh"
!include "x64.nsh"

; General configuration
!define PRODUCT_NAME "WaoN"
!define PRODUCT_PUBLISHER "WaoN Development Team"
!define PRODUCT_WEB_SITE "https://github.com/blazeiburgess/WaoN"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\waon.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

; Version is passed as parameter: makensis -DVERSION=x.y.z
!ifndef VERSION
  !define VERSION "0.11"
!endif

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!insertmacro MUI_PAGE_LICENSE "..\..\LICENSE"
; Components page
!insertmacro MUI_PAGE_COMPONENTS
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_RUN "$INSTDIR\waon.exe"
!define MUI_FINISHPAGE_RUN_PARAMETERS "--help"
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\README.md"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; Installer attributes
Name "${PRODUCT_NAME} ${VERSION}"
OutFile "WaoN-${VERSION}-setup.exe"
InstallDir "$PROGRAMFILES64\WaoN"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show
RequestExecutionLevel admin

; Version information
VIProductVersion "${VERSION}.0"
VIAddVersionKey "ProductName" "${PRODUCT_NAME}"
VIAddVersionKey "ProductVersion" "${VERSION}"
VIAddVersionKey "CompanyName" "${PRODUCT_PUBLISHER}"
VIAddVersionKey "LegalCopyright" "GPL v2 or later"
VIAddVersionKey "FileDescription" "${PRODUCT_NAME} Installer"
VIAddVersionKey "FileVersion" "${VERSION}"

Section "WaoN Core" SEC01
  SectionIn RO
  
  SetOutPath "$INSTDIR"
  SetOverwrite try
  
  ; Core files
  File "..\..\build\waon.exe"
  File "..\..\build\pv.exe"
  File "..\..\build\libwaon.dll"
  File "..\..\README.md"
  File "..\..\LICENSE"
  
  ; Dependencies (these need to be bundled or installed separately)
  ; File "libfftw3-3.dll"
  ; File "libsndfile-1.dll"
  ; File "libao-4.dll"
  ; File "libsamplerate-0.dll"
  
  ; Documentation
  CreateDirectory "$INSTDIR\docs"
  SetOutPath "$INSTDIR\docs"
  File "..\..\docs\TIPS"
  File "..\..\docs\ChangeLog"
  
  ; Examples
  CreateDirectory "$INSTDIR\examples"
  SetOutPath "$INSTDIR\examples"
  File "..\..\examples\waonrc.example"
SectionEnd

Section "GUI Application" SEC02
  SetOutPath "$INSTDIR"
  File "..\..\build\gwaon.exe"
  
  ; GTK dependencies would go here
  ; These are typically quite large and complex
SectionEnd

Section "Python Bindings" SEC03
  ; This section would install pre-built Python wheels
  ; Implementation depends on Python distribution strategy
  
  CreateDirectory "$INSTDIR\python"
  SetOutPath "$INSTDIR\python"
  
  ; Install wheel
  ; ExecWait '"$INSTDIR\python\python.exe" -m pip install waon-${VERSION}-py3-none-win_amd64.whl'
SectionEnd

Section -AdditionalIcons
  CreateDirectory "$SMPROGRAMS\WaoN"
  CreateShortCut "$SMPROGRAMS\WaoN\WaoN.lnk" "$INSTDIR\waon.exe"
  CreateShortCut "$SMPROGRAMS\WaoN\GWaoN.lnk" "$INSTDIR\gwaon.exe"
  CreateShortCut "$SMPROGRAMS\WaoN\Uninstall.lnk" "$INSTDIR\uninst.exe"
  CreateShortCut "$DESKTOP\WaoN.lnk" "$INSTDIR\waon.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  
  ; Write registry keys
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\waon.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\waon.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  
  ; Add to PATH
  ${EnvVarUpdate} $0 "PATH" "A" "HKLM" "$INSTDIR"
SectionEnd

; Section descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC01} "Core WaoN command-line tools"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC02} "GTK+ GUI application"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC03} "Python bindings for WaoN"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Section Uninstall
  ; Remove from PATH
  ${un.EnvVarUpdate} $0 "PATH" "R" "HKLM" "$INSTDIR"
  
  ; Delete files
  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\waon.exe"
  Delete "$INSTDIR\pv.exe"
  Delete "$INSTDIR\gwaon.exe"
  Delete "$INSTDIR\libwaon.dll"
  Delete "$INSTDIR\README.md"
  Delete "$INSTDIR\LICENSE"
  
  ; Delete directories
  RMDir /r "$INSTDIR\docs"
  RMDir /r "$INSTDIR\examples"
  RMDir /r "$INSTDIR\python"
  RMDir "$INSTDIR"
  
  ; Delete shortcuts
  Delete "$SMPROGRAMS\WaoN\*.lnk"
  RMDir "$SMPROGRAMS\WaoN"
  Delete "$DESKTOP\WaoN.lnk"
  
  ; Delete registry keys
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  
  SetAutoClose true
SectionEnd