@echo off
setlocal enabledelayedexpansion

:: ============================================================
:: PostBuild.bat
:: Usage: PostBuild.bat <compiler_exe> <input_dir> <output_dir>
::
:: Example (Visual Studio Post-Build Event):
::   "$(SolutionDir)scripts\PostBuild.bat" "$(OutDir)" "$(ProjectDir)" "$(SolutionDir)Dependencies\native\" "Config\" "Content\" "*.vert *.frag *.png *.jpg *.tga"
:: ============================================================

if "%~1"=="" (
	echo [ERROR] Argument 1 missing: output directory path
	exit /b 1
)

if "%~2"=="" (
	echo [ERROR] Argument 2 missing: project directory path
	exit /b 1
)

if "%~3"=="" (
	echo [ERROR] Argument 3 missing: native binary path
	exit /b 1
)

if "%~4"=="" (
	echo [ERROR] Argument 4 missing: config directory path
	exit /b 1
)

if "%~5"=="" (
	echo [ERROR] Argument 5 missing: content directory path
	exit /b 1
)

if "%~6"=="" (
	echo [ERROR] Argument 6 missing: compiled shader extension
	exit /b 1
)

set OUTPUT_DIR=%~1
set PROJECT_DIR=%~2
set NATIVE_BIN_DIR=%~3
set CONFIG_DIR=%~4
set CONTENT_DIR=%~5
set IGNORED_EXTENSIONS=%~6

robocopy "%NATIVE_BIN_DIR%" "%OUTPUT_DIR%" /e /ns /nc /nfl /ndl /np /njh /njs
robocopy "%PROJECT_DIR%/%CONFIG_DIR%" "%OUTPUT_DIR%/%CONFIG_DIR%" /e /ns /nc /nfl /ndl /np /njh /njs
robocopy "%PROJECT_DIR%/%CONTENT_DIR%" "%OUTPUT_DIR%/%CONTENT_DIR%" /e /ns /nc /nfl /ndl /np /njh /njs /xf %IGNORED_EXTENSIONS%

exit /b 0