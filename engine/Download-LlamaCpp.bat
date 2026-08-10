@echo off
REM Downloads the llama.cpp engine (CUDA build, for an NVIDIA GPU) into engine\bin
REM next to this script. Point the app's Llama.cpp tab "Server executable" field at
REM the llama-server.exe this produces.
REM
REM To grab a newer version later, update RELEASE_TAG below to a tag from
REM https://github.com/ggml-org/llama.cpp/releases (check the asset list still
REM has a "win-cuda-12.4-x64" zip; the CUDA version in the name may change).

setlocal enabledelayedexpansion

set "RELEASE_TAG=b10344"
set "BASE_URL=https://github.com/ggml-org/llama.cpp/releases/download/%RELEASE_TAG%"
set "ZIP_MAIN=llama-%RELEASE_TAG%-bin-win-cuda-12.4-x64.zip"
set "ZIP_CUDART=cudart-llama-bin-win-cuda-12.4-x64.zip"
set "OUTDIR=%~dp0bin"

if not exist "%OUTDIR%" mkdir "%OUTDIR%"

echo Downloading %ZIP_MAIN% ...
curl.exe -L --fail --retry 3 -o "%~dp0%ZIP_MAIN%" "%BASE_URL%/%ZIP_MAIN%"
if errorlevel 1 (
    echo Download failed. Re-run this script to resume/retry.
    exit /b 1
)

echo Downloading %ZIP_CUDART% ...
curl.exe -L --fail --retry 3 -o "%~dp0%ZIP_CUDART%" "%BASE_URL%/%ZIP_CUDART%"
if errorlevel 1 (
    echo Download failed. Re-run this script to resume/retry.
    exit /b 1
)

echo Extracting into %OUTDIR% ...
tar -xf "%~dp0%ZIP_MAIN%" -C "%OUTDIR%"
tar -xf "%~dp0%ZIP_CUDART%" -C "%OUTDIR%"

echo.
echo Done. In LlamaStories' Llama.cpp tab, set "Server executable" to:
echo   %OUTDIR%\llama-server.exe
echo.

endlocal
pause
