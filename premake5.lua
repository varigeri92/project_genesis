outDir = "bin/%{cfg.buildcfg}"
VulkanLibDir = "C:/VulkanSDK/1.3.275.0/Lib"
VulkanIncludeDir = "C:/VulkanSDK/1.3.275.0/Include"

workspace "GenesisEngine"
    language "C++"
    cppdialect "C++20"
    configurations { "Debug", "Release", "Profile", "Dist" }
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

    filter "configurations:Profile"
        defines { "DEBUG", "_DEBUG", "_CONSOLE", "ENABLE_PROFILER", "TRACE_ALLOCATION" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

    filter "configurations:Dist"
        defines { "NDEBUG" }
        optimize "On"
    
project "Engine"
    kind "SharedLib"
    targetdir(outDir)
    objdir "bin-int/Engine/%{cfg.buildcfg}"
    location "Engine" 
    pchheader "gnspch.h"
    pchsource "Engine/src/gnspch.cpp"

    libdirs { 
        "Engine/vendor/lib",
        "vendor/lib",
        VulkanLibDir 
    }
    links {
        "SDL2.lib",
        "SDL2main.lib",
        "vulkan-1.lib",
        "assimp-vc143-mt.lib",
        --"yaml-cpp.lib",
        "yaml-cppd.lib"
    }
    defines {"BUILD_ENGINE_LIB"}
    includedirs { "vendor/include", VulkanIncludeDir, "Engine/API", "Engine/vendor/include","Engine/src/pch" }
    files { "Engine/**.h", "Engine/**.c", "Engine/**.cpp", "Engine/**.hpp" } 
    
project "Sandbox"
    kind "ConsoleApp"
    targetdir(outDir)
    objdir "bin-int/Sandbox/%{cfg.buildcfg}"
    location "Sandbox"
    libdirs { "libs", outDir }
    links { "Engine.lib", "Editor.lib"  }
    defines {"BUILD_SANDBOX_EXE"}
    includedirs { "vendor/include", VulkanIncludeDir, "Engine/API" }
    files { "Sandbox/**.h", "Sandbox/**.c", "Sandbox/**.cpp", "Sandbox/**.hpp" }
    dependson { "Engine", "Game", "Editor" }
    debugargs { "-p", "D:\\Genesis-TestProjects\\TestProj_1", "-r", "D:\\GenesisEngine\\Resources\\" }

project "Game"
    kind "SharedLib"
    targetdir(outDir)
    objdir "bin-int/Game/%{cfg.buildcfg}"
    location "Game"
    libdirs { "libs", outDir }
    links { "Engine.lib" }
    defines {"BUILD_GAME_LIB"}
    includedirs { "vendor/include", VulkanIncludeDir, "Engine/API" }
    files { "Game/**.h", "Game/**.c", "Game/**.cpp", "Game/**.hpp" }
    dependson { "Engine" }

project "Editor"
    kind "SharedLib"
    targetdir(outDir)
    objdir "bin-int/Editor/%{cfg.buildcfg}"
    location "Editor"
    libdirs { "vendor/lib", outDir }
    links { "Engine.lib", "yaml-cppd.lib"}
    defines {"BUILD_EEDITOR_LIB"}
    includedirs { "vendor/include", VulkanIncludeDir, "Engine/API" }
    files { "Editor/**.h", "Editor/**.c", "Editor/**.cpp", "Editor/**.hpp" }
    dependson { "Engine" }