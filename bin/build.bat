@echo off

set compiler_flags=-Od -MTd -nologo -fp:fast -fp:except- -Gm- -GR- -EHa- -Zo -Oi -WX -W4 -wd4201 -wd4100 -wd4214 -wd4101 -wd4189 -wd4505 -wd4127 -wd4706 -wd4702 -wd4204 -FC -Z7 -I%PROJECT_PATH%/src -I"D:\Dev\vulkan\current\include"
set linker_flags= -incremental:no -opt:ref %PROJECT_LINKER_FLAGS%

IF NOT exist %BUILD_PATH% ( mkdir %BUILD_PATH% )
pushd %BUILD_PATH%

D:\Dev\vulkan\current\Bin32\glslc.exe ..\ui\data\shader\vulkan_shader.vert -o ..\ui\data\shader\vulkan_vert.spv
D:\Dev\vulkan\current\Bin32\glslc.exe ..\ui\data\shader\vulkan_shader.frag -o ..\ui\data\shader\vulkan_frag.spv

cl %compiler_flags% %PROJECT_PATH%\src\main.c -Fe%PROJECT_NAME%.exe /link %linker_flags%

popd
