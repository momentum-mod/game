if (-NOT ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator"))
{
	Write-Warning "You do not have Administrator rights to run this script!`nPlease re-run this script as an Administrator!"
	pause
	exit
}

$ErrorActionPreference= 'silentlycontinue'
# This is the path to your steamworks dev folder (depo). This should be the folder with hl2.exe inside of it.
# Your local momentum folder will be sym linked INSIDE this folder as momentum
$path = "E:\Steamworks\dev\Momentum"

# Boot arguments
$hl2exe = Join-Path $path hl2.exe 
$hl2args = "-game momentum -window -w 1600 -h 900 -novid +developer 2 -console"

$momentum_sym = Join-Path $path momentum # Writing as (steamworks path)/momentum
$momentum = [System.IO.Path]::GetFullPath("..\game\momentum") # This is your local dev folder
cmd /c mklink /D $momentum_sym $momentum

$data = @"
<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="12.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <PropertyGroup Condition="'`$(Configuration)|`$(Platform)'=='Debug|Win32'">
    <LocalDebuggerCommand>$hl2exe</LocalDebuggerCommand>
    <LocalDebuggerCommandArguments>$hl2args</LocalDebuggerCommandArguments>
    <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
  </PropertyGroup>
  <PropertyGroup Condition="'`$(Configuration)|`$(Platform)'=='Release|Win32'">
    <LocalDebuggerCommand>$hl2exe</LocalDebuggerCommand>
    <LocalDebuggerCommandArguments>$hl2args</LocalDebuggerCommandArguments>
    <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
  </PropertyGroup>
</Project>
"@

$data | Out-File game\client\client_momentum.vcxproj.user utf8

"`nDebug and launch added to client_momentum.vcxproj`n"
