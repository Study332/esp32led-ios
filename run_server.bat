@echo off
chcp 65001 > nul
title ESP32 LED Web Server

:: Chuyển đến đúng thư mục chứa file .bat
cd /d "%~dp0"

echo ========================================================
echo   LOCAL WEB SERVER CHO DU AN ESP32 LED CONTROL
echo ========================================================
echo.
echo 1. May tinh truy cap tai: http://localhost:8000
echo.
echo 2. Dien thoai (trong cung mang Wi-Fi):
for /f "tokens=2 delims=:" %%a in ('ipconfig ^| findstr /c:"IPv4 Address" /c:"Dia chi IPv4"') do (
    echo    -^> http:%%a:8000
)
echo.
echo Trinh duyet se tu dong mo trang web ngay bay gio...
echo Nhan Ctrl + C neu muon dung Server.
echo ========================================================
echo.

:: Tu dong mo trinh duyet
start "" "http://localhost:8000"

:: Khoi chay Python HTTP Server o cong 8000
python -m http.server 8000
pause
