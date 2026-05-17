@echo off
setlocal EnableExtensions DisableDelayedExpansion

:: Check for Administrator privileges
cmd /c exit /b 0
net session >nul 2>&1
if %errorLevel% == 0 (
    goto :isAdmin
) else (
    goto :elevate
)

:elevate
echo Elevating privileges...
echo Set UAC = CreateObject("Shell.Application") > "%temp%\getadmin.vbs"
echo UAC.ShellExecute "%~s0", "", "", "runas", 1 >> "%temp%\getadmin.vbs"
"%temp%\getadmin.vbs"
del "%temp%\getadmin.vbs"
exit /b

:isAdmin
:: Get the root repo directory where this script runs
set "REPO_ROOT=%~dp0"
if "%REPO_ROOT:~-1%"=="\" set "REPO_ROOT=%REPO_ROOT:~0,-1%"

:: Target the deep build directory containing offgit.exe
set "TARGET_DIR=%REPO_ROOT%\offgit\offgit\x64\Release"

echo Target directory to add to PATH: 
echo "%TARGET_DIR%"
echo.

:: Verify the folder actually exists before adding it
if not exist "%TARGET_DIR%" (
    echo [ERROR] Could not find the release folder. 
    echo Make sure you compiled the project in Release mode first!
    goto :end
)

:: Fetch current User PATH from Registry
for /f "tokens=2*" %%A in ('reg query HKCU\Environment /v PATH 2^>nul') do set "USER_PATH=%%B"

:: Check if the directory is already in the path
echo %USER_PATH% | findstr /I /C:";%TARGET_DIR%;" >nul && set "ALREADY_HERE=1"
echo %USER_PATH% | findstr /I /C:"%TARGET_DIR%;" >nul && set "ALREADY_HERE=1"

if "%ALREADY_HERE%"=="1" (
    echo [INFO] This directory is already in your PATH.
    goto :end
)

:: Append the new directory to the existing User PATH
if "%USER_PATH%"=="" (
    set "NEW_PATH=%TARGET_DIR%"
) else (
    if "%USER_PATH:~-1%"==";" (
        set "NEW_PATH=%USER_PATH%%TARGET_DIR%"
    ) else (
        set "NEW_PATH=%USER_PATH%;%TARGET_DIR%"
    )
)

:: Save back to User Environment variables permanently
reg add HKCU\Environment /v PATH /t REG_SZ /d "%NEW_PATH%" /f >nul

if %errorLevel% == 0 (
    echo [SUCCESS] "%TARGET_DIR%" successfully added to PATH!
    echo [NOTE] Restart your terminal/command prompt, then you can type 'offgit' anywhere.
) else (
    echo [ERROR] Failed to update PATH.
)

:end
echo.
pause
exit /b
