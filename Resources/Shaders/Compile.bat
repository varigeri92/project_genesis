echo off
set VULKAN_SDK=C:\VulkanSDK\1.3.275.0\Bin
set SHADER_DIR=./
 "%VULKAN_SDK%\glslc.exe" -h
for %%f in (%SHADER_DIR%*.frag %SHADER_DIR%*.vert) do (
    echo "%%f"
    "%VULKAN_SDK%\glslc.exe" "%%f" -o "%%f.spv" -E
)

echo Shader compilation complete.