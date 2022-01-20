@echo off

if not exist build mkdir build
if not exist engine mkdir engine

pushd engine
if not exist shaders mkdir shaders
popd

:: @TODO: Disable exceptions (-EHa-) after removing std garbage

set compiler_args=^
-MTd ^
-nologo ^
-GR- ^
-EHsc ^
-Od -Oi ^
-WX -W4 ^
-FC ^
-Zi ^
-diagnostics:caret ^
-wd4201 ^
-wd4100 ^
-wd4505 ^
-wd4189 ^
-wd4146 ^
-Fe:vulkan_engine ^
-IE:/Tools/glfw/include/GLFW ^
-IE:/Tools/VulkanSDK/Include ^
-IE:/Tools/VulkanSDK/Third-Party/Include/glm ^
-DSLOW

set linker_args=^
E:/Tools/glfw/build/src/Debug/glfw3.lib ^
E:/Tools/VulkanSDK/Lib/vulkan-1.lib ^
user32.lib gdi32.lib shell32.lib

pushd build

glslc ../src/shaders/simple.vert -o ../engine/shaders/simple.vert.spv
glslc ../src/shaders/simple.frag -o ../engine/shaders/simple.frag.spv

cl %compiler_args% ../src/*.cpp /link /NODEFAULTLIB:library %linker_args% && echo [32mBuild successfull[0m || echo [31mBuild failed[0m

popd

