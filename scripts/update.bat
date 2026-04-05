@echo off
:: Usage: update.bat <archive_path> <install_dir> <executable_path>
set ARCHIVE=%1
set INSTALL_DIR=%2
set EXECUTABLE=%3

timeout /t 1 /nobreak >nul
powershell -Command "Expand-Archive -Force '%ARCHIVE%' '%INSTALL_DIR%'"
xcopy /e /y /i "%INSTALL_DIR%\pong-windows-x86_64\*" "%INSTALL_DIR%"
rmdir /s /q "%INSTALL_DIR%\pong-windows-x86_64"
del "%ARCHIVE%"
start "" "%EXECUTABLE%"
del "%~f0"
