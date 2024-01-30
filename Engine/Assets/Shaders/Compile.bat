set VULKAN_SDK=C:\VulkanSDK\1.3.261.0\Bin
set SHADER_DIR=./

for %%f in (%SHADER_DIR%*.frag %SHADER_DIR%*.vert) do (
    "%VULKAN_SDK%\glslc.exe" "%%f" -o "%%f.spv"
)

echo Shader compilation complete.