@echo off
cd /d %~dp0
devtools\bin\vpc.exe /momentum +game /mksln momentum.sln /f
Powershell.exe -ExecutionPolicy Bypass -File creategameprojects.ps1
PAUSE