@echo off

SET PROJECT_NAME=ui
SET PROJECT_MAIN_FILE=main.c
SET PROJECT_LINKER_FLAGS=-libpath:"D:\Dev\vulkan\current\Lib" user32.lib gdi32.lib winmm.lib vulkan-1.lib Shlwapi.lib

SET PATH=%userprofile%\Dev\C\bin;%userprofile%\Dev\C\bin;%PATH%

call global_shell.bat
