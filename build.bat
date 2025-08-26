@echo off
echo Building WW3 project...
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
echo Cleaning previous build...
msbuild WW3.sln /p:Configuration=Debug /p:Platform=x64 /t:Clean
echo Building project...
msbuild WW3.sln /p:Configuration=Debug /p:Platform=x64
if %ERRORLEVEL% EQU 0 (
    echo Build successful!
    echo Running the application...
    .\x64\Debug\WW3.exe
) else (
    echo Build failed!
    pause
)
