outDir = "bin/%{cfg.buildcfg}"
VulkanDir = "C:/VulkanSDK/1.3.261.0/Lib"

workspace "GenesisEngine"
    language "C++"
    cppdialect "C++20"
    configurations { "Debug", "Release" }
    platforms { "Win64" }
    startproject "Sandbox"
    toolset "msc"
    debugenvs { "TRACEDESIGNTIME = true" }
    filter { "platforms:Win64" }
        system "Windows"
        architecture "x86_64"

    filter "configurations:Debug"
        defines { "DEBUG", "_DEBUG", "_CONSOLE" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

    
project "Engine"
    kind "SharedLib"
    targetdir(outDir)
    objdir "bin-int/Engine/%{cfg.buildcfg}"
    location "Engine"
    libdirs { 
        "Engine/vendor/lib",
        VulkanDir 
    }
    links {
        "SDL2.lib",
        "SDL2main.lib",
        "vulkan-1.lib",
        "yaml-cpp.lib",
        "assimp-vc143-mt.lib"
    }
    defines {"BUILD_DLL"}
    includedirs { "vendor/include", "C:/VulkanSDK/1.3.261.0/Include", "Engine/API", "Engine/vendor/include" }
    files { "Engine/**.h", "Engine/**.c", "Engine/**.cpp", "Engine/**.hpp" } 
    
project "Sandbox"
    kind "ConsoleApp"
    language "C++"
    targetdir(outDir)
    objdir "bin-int/Sandbox/%{cfg.buildcfg}"
    location "Sandbox"
    libdirs { "libs", outDir }
    links { "Engine.lib" }
    includedirs { "vendor/include", "C:/VulkanSDK/1.3.261.0/Include", "Engine/API" }
    files { "Sandbox/**.h", "Sandbox/**.c", "Sandbox/**.cpp", "Sandbox/**.hpp" }
    dependson { "Engine" }