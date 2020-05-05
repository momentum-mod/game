@echo off
setlocal

rem Use dynamic shaders to build .inc files only
rem set dynamic_shaders=1
rem == Setup path to nmake.exe, from vc 2005 common tools directory ==
call "%VS120COMNTOOLS%vsvars32.bat"

set TTEXE=..\..\devtools\bin\timeprecise.exe
if not exist %TTEXE% goto no_ttexe
goto no_ttexe_end

:no_ttexe
set TTEXE=time /t
:no_ttexe_end

rem echo.
rem echo ~~~~~~ buildsdkshaders %* ~~~~~~
%TTEXE% -cur-Q
set tt_all_start=%ERRORLEVEL%
set tt_all_chkpt=%tt_start%

set BUILD_SHADER=call buildshaders_inc.bat
set ARG_EXTRA=

%BUILD_SHADER% momentum_dx9_20b -builddynamic -game %GAMEDIR% -source %SOURCEDIR% -20
%BUILD_SHADER% momentum_dx9_30 -builddynamic -game %GAMEDIR% -source %SOURCEDIR% -force30 -dx9_30 -force

rem echo.
if not "%dynamic_shaders%" == "1" (
  rem echo Finished full buildallshaders %*
) else (
  rem echo Finished dynamic buildallshaders %*
)

rem %TTEXE% -diff %tt_all_start% -cur
rem echo.
