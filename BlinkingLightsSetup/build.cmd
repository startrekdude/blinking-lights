@echo off
SET SEVENZIP="C:\Program Files\7-Zip\7z.exe"
SET SFX=%1\7zS.sfx
SET CONFIG=%1%\config.txt

cscript /nologo %1%\ARPNOMODIFY.js
%SEVENZIP% a BlinkingLightsSetup.7z BlinkingLightsSetup.msi setup.exe
copy /b %SFX% + %CONFIG% + BlinkingLightsSetup.7z BlinkingLightsSetup.exe
del BlinkingLightsSetup.7z