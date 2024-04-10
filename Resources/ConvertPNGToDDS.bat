@echo off

rem Define the path to TextureConverter.exe (modify if needed)
set "TEXTURE_CONVERTER=texconv.exe"

rem Check if TextureConverter exists
if not exist "%TEXTURE_CONVERTER%" (
  echo Error: TextureConverter.exe not found! Please adjust the path in the script.
  exit /b 1
)

for /r %%a in (*.png *.jpg) do (
  "%TEXTURE_CONVERTER%" "%%a")

echo Finished converting PNG files to DDS in all subfolders.
