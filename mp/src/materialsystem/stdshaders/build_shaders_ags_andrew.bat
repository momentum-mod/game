@echo off
setlocal
@echo=====================================AGS-Renegade Shader Compiler=======================================
rem ================================
rem ==== MOD PATH CONFIGURATIONS ===

rem == Set the absolute path to your mod's game directory here ==
set GAMEDIR=%cd%\..\..\..\game\momentum

rem == Set the relative or absolute path to Source SDK Base 2013 Singleplayer\bin ==
set SDKBINDIR=%cd%\..\..\devtools\bin\shadercompile\valve
rem ==  Set the Path to your mod's root source code ==
rem This should already be correct, accepts relative paths only!
set SOURCEDIR=..\..

rem ==== MOD PATH CONFIGURATIONS END ===
rem ====================================

@echo===========================================Building .INC files==========================================
call buildsdkshaders_inc.bat 
@echo===========================================Building .VCS files==========================================
call buildsdkshaders_vcs.bat 
@echo===========================================Copying  .VCS files==========================================
xcopy %cd%\shaders %GAMEDIR%\shaders /e /v

@echo========================================Shader Compile is Finished======================================
@pause
