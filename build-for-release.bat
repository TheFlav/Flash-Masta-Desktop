:: Batch file sets some variables courtesy of the Visual Studio command line build
:: process, so make sure changes to environment variables remain local to this script
:: only.
@ECHO OFF
SETLOCAL

:: Verify required variables are defined
IF NOT DEFINED QTDIR (
	ECHO Environment variable QTDIR not defined: terminating script
	ECHO "Example: C:\Qt\5.5"
	EXIT /B 1
)
IF NOT DEFINED VS120COMNTOOLS (
	ECHO Environment variable VS120COMNTOOLS not defined: terminating script
	ECHO "Example: C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Tools\"
	EXIT /B 1
)

:: Create build directory if it doesn't exist
IF NOT EXIST "build" (
	@ECHO ON
	mkdir build
	@ECHO OFF
)

:: Obligatory error check
IF %ERRORLEVEL% NEQ 0 (
	ECHO Terminating script: an error occured
	EXIT /B %ERRORLEVEL%
)

:: CD into build directory
@ECHO ON
cd /d build
@ECHO OFF

:: Obligatory error check
IF %ERRORLEVEL% NEQ 0 (
	ECHO Terminating script: an error occured
	EXIT /B %ERRORLEVEL%
)

:: Delete deployment directory
IF EXIST "FlashMasta" (
	@ECHO ON
	rmdir /Q /S FlashMasta
	@ECHO OFF
)

:: Obligatory error check
IF %ERRORLEVEL% NEQ 0 (
	ECHO Terminating script: an error occured
	EXIT /B %ERRORLEVEL%
)

:: Create deployment directory if it doesn't exist
IF NOT EXIST "FlashMasta" (
	@ECHO ON
	mkdir FlashMasta
	@ECHO OFF
)

:: Obligatory error check
IF %ERRORLEVEL% NEQ 0 (
	ECHO Terminating script: an error occured
	EXIT /B %ERRORLEVEL%
)

:: ============================================
:: Step 1. Have VS set up environment variables
:: ============================================

CALL "%VS120COMNTOOLS%..\..\VC\vcvarsall"

:: Obligatory error check
IF %ERRORLEVEL% NEQ 0 (
	ECHO Terminating script: an error occured
	EXIT /B %ERRORLEVEL%
)

:: =================
:: Step 2. Run QMake
:: =================

@ECHO ON
"%QTDIR%\bin\qmake" ..\FlashMasta.pro
@ECHO OFF

:: Obligatory error check
IF %ERRORLEVEL% NEQ 0 (
	ECHO Terminating script: an error occured
	EXIT /B %ERRORLEVEL%
)

:: =============================
:: Step 3. Clean build directory
:: =============================

@ECHO ON
nmake clean
@ECHO OFF

:: Obligatory error check
IF %ERRORLEVEL% NEQ 0 (
	ECHO Terminating script: an error occured
	EXIT /B %ERRORLEVEL%
)

:: ========================
:: Step 4. Build executable
:: ========================

@ECHO ON
nmake release
@ECHO OFF

:: Obligatory error check
IF %ERRORLEVEL% NEQ 0 (
	ECHO Terminating script: an error occured
	EXIT /B %ERRORLEVEL%
)

:: ===========================================
:: Step 5. Copy executable to deploy directory
:: ===========================================

@ECHO ON
copy release\FlashMasta.exe FlashMasta\FlashMasta.exe
@ECHO OFF

:: Obligatory error check
IF %ERRORLEVEL% NEQ 0 (
	ECHO Terminating script: an error occured
	EXIT /B %ERRORLEVEL%
)

:: ============================
:: Step 6. Run Qt's deploy tool
:: ============================

@ECHO ON
"%QTDIR%\bin\windeployqt" FlashMasta\FlashMasta.exe
@ECHO OFF

:: Obligatory error check
IF %ERRORLEVEL% NEQ 0 (
	ECHO Terminating script: an error occured
	EXIT /B %ERRORLEVEL%
)

:: ===================================
:: Step 7. Zip up deployment directory
:: ===================================

:: First delete existing file if it exsts
IF EXIST ..\FlashMasta.zip del ..\FlashMasta.zip

echo Creating zip file...
powershell.exe -nologo -noprofile -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::CreateFromDirectory('FlashMasta', '..\FlashMasta.zip'); }"

:: Obligatory error check
IF %ERRORLEVEL% NEQ 0 (
	ECHO Terminating script: an error occured
	EXIT /B %ERRORLEVEL%
)

:: ====
:: End!
:: ====

ECHO Build for deployment complete!
