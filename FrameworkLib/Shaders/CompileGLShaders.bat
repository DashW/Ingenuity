SET shader_dir=.

REM PUSHD %shader_dir%

for %%F in (*.cgfx) do (
"%CG_BIN_PATH%\cgc.exe" %%~dpnxF -strict -profile glslv -o %%~dpnF.glsl
REM admin import-xml "-Dimport.file=%%~dpnxF" -Dadmin.db.password=test123
)

REM POPD