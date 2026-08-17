@echo off
set targetDir=%~dp0libs
set qt=C:\Qt\6.11.1\mingw_64

:: Clean
if exist "%targetDir%" (
    del /s /q "%targetDir%\*.*" 1>nul
    for /d %%d in ("%targetDir%\*") do rmdir /s /q "%%d"
) else (
    echo Creating directory: %targetDir%
    mkdir "%targetDir%"
)

:: Copy
copy "%qt%\bin\Qt6Core.dll" "%targetDir%"
copy "%qt%\bin\Qt6Gui.dll" "%targetDir%"
copy "%qt%\bin\Qt6Widgets.dll" "%targetDir%"
copy "%qt%\bin\Qt6Network.dll" "%targetDir%"
copy "%qt%\bin\libgcc_s_seh-1.dll" "%targetDir%"
copy "%qt%\bin\libstdc++-6.dll" "%targetDir%"
copy "%qt%\bin\libwinpthread-1.dll" "%targetDir%"
copy "%qt%\plugins\platforms\qwindows.dll" "%targetDir%"

@pause
