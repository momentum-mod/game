@echo off
devtools\bin\vpc.exe /momentum +game +shaders /mksln momentum.sln /f
copy momentum.sln+sln_fix.txt momentum.sln
Powershell.exe -ExecutionPolicy Bypass -File creategameprojects_steam.ps1
PAUSE