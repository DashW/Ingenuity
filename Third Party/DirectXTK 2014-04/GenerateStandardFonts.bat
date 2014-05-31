@echo off
setlocal
set error=0
set fontSize=64
set exeDir=MakeSpriteFont\bin\Release\
set outDir=StandardFonts\

mkdir "%outDir%"

call :GenerateFont "Arial"
call :GenerateFont "Times New Roman"
call :GenerateFont "Courier New"
call :GenerateFont "Tahoma"

endlocal
exit /b

:GenerateFont
"%exedir%MakeSpriteFont.exe" "%~1" "%outDir%%~1.spritefont" /FontSize:%fontSize%
"%exedir%MakeSpriteFont.exe" "%~1" "%outDir%%~1B.spritefont" /FontSize:%fontSize% /FontStyle:bold
"%exedir%MakeSpriteFont.exe" "%~1" "%outDir%%~1I.spritefont" /FontSize:%fontSize% /FontStyle:italic
exit /b