;NSIS Modern User Interface version 1.70
;Basic Example Script
;Written by Joost Verburg

;--------------------------------
;Include Modern UI

	!addincludedir			"..\..\nsis"
	!include				"MUI2.nsh"
	!include				"Library.nsh"
	!include				"tlbplugins.nsh"
	
	SetCompressor /SOLID lzma
	RequestExecutionLevel admin

;--------------------------------
;Component data
	!define TLBSOURCEPATH	"..\..\..\..\.."
	!define INSTALLARTPATH	"..\..\nsis\img"
	!define OUTPATH			"..\..\nsis\bin"

 	!define GUID 			"{ADE143D4-3FA6-4D21-86BB-6235A57ADCBB}"
	!define UDISPLAYNAME 	"TLB: Address Book plugin"
	!define DISPLAYNAME		"Address Book plugin"
	!define VERSION  		"4.1"
	!define FULLVERSION		"4.1.0.0"
	!define PLUGDIR 		"abook"
	!define PLUGFILE 		"abook"
	!define SRCFOLDER32		"..\Win32\Release"
	!define SRCFOLDER64		"..\x64\Release"
	!define DATAFOLDER 		"..\"
	!define SKINFOLDER 		"..\default-skins"
	!define LOCFILE 		"abook.xml"
	!define TLBREF32		"..\..\..\tlbUtils\tlbref\Release"
	!define TLBREF64		"..\..\..\tlbUtils\tlbref\Release64"
	!define OUTFILE			"tlbabook.exe"

	!define HAVESKINS
		!define SKINFILE 		"abook.ini"
		!define SKINSECTION 	"options"
		!define SKINNAME 		"name"

!insertmacro TLBPLUGINS_START
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"
  ;OutFile "${OUTPATH}\${OUTFILE}"

;--------------------------------
;Installer Sections

Section "Core Components" SecCore
	SectionIn RO

	!insertmacro TLBPLUGINS_WRITEDEFVALUES

	CreateDirectory "$INSTDIR\skins"
	
	CreateDirectory "$INSTDIR\skins\Default"
  	SetOutPath "$INSTDIR\skins\Default"
	File /r "${SKINFOLDER}\Default\*.*"
	
	CreateDirectory "$INSTDIR\skins\Mini Blue"
  	SetOutPath "$INSTDIR\skins\Mini Blue"
	File /r "${SKINFOLDER}\Mini Blue\*.*"

	CreateDirectory "$INSTDIR\skins\Mini Orange"
  	SetOutPath "$INSTDIR\skins\Mini Orange"
	File /r "${SKINFOLDER}\Mini Orange\*.*"

	CreateDirectory "$INSTDIR\skins\Round Blue"
  	SetOutPath "$INSTDIR\skins\Round Blue"
	File /r "${SKINFOLDER}\Round Blue\*.*"

	CreateDirectory "$INSTDIR\skins\Round Orange"
  	SetOutPath "$INSTDIR\skins\Round Orange"
	File /r "${SKINFOLDER}\Round Orange\*.*"

	CreateDirectory "$INSTDIR\skins\Simple"
  	SetOutPath "$INSTDIR\skins\Simple"
	File /r "${SKINFOLDER}\Simple\*.*"

	!ifndef INNER
		SetOutPath $INSTDIR
		File $%TEMP%\uninstall.exe
	!endif
	;WriteUninstaller "$INSTDIR\Uninstall.exe"

	!insertmacro TLBPLUGINS_SETREBOOTEVENT
SectionEnd

;--------------------------------
;Descriptions

	;Language strings
	LangString DESC_SecCore ${LANG_ENGLISH} "${DISPLAYNAME} core files."

	;Assign language strings to sections
	!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecCore} $(DESC_SecCore)
	!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"
	!insertmacro TLBPLUGINS_UWRITEDEFVALUES
	
	!insertmacro TLBPLUGINS_DEFUNINSTALL
SectionEnd
