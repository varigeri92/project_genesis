outDir = "bin/%{cfg.buildcfg}"
VulkanLibDir = "C:/VulkanSDK/1.3.261.1/Lib"
VulkanIncludeDir = "C:/VulkanSDK/1.3.261.1/Include"
p = path.getabsolute("./")
p=p:gsub('/','\\\\')..'\\\\'
rootpath = '"'..p..'"'

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
        VulkanLibDir 
    }
    links {
        "SDL2.lib",
        "SDL2main.lib",
        "vulkan-1.lib",
        "yaml-cpp.lib",
        "assimp-vc143-mt.lib"
    }
    defines {"BUILD_DLL"}
    includedirs { "vendor/include", VulkanIncludeDir, "Engine/API", "Engine/vendor/include" }
    files { "Engine/**.h", "Engine/**.c", "Engine/**.cpp", "Engine/**.hpp" } 
    
project "Sandbox"
    kind "ConsoleApp"
    language "C++"
    targetdir(outDir)
    objdir "bin-int/Sandbox/%{cfg.buildcfg}"
    location "Sandbox"
    libdirs { "libs", outDir }
    links { "Engine.lib" }
    includedirs { "vendor/include", VulkanIncludeDir, "Engine/API" }
    files { "Sandbox/**.h", "Sandbox/**.c", "Sandbox/**.cpp", "Sandbox/**.hpp" }
    dependson { "Engine" }


    function ReplacePath(path)
        srcfile = "./Engine/API/path.h"
        local file = io.open(srcfile, "r")
    
        if not file then
            print("Error: Unable to open file for reading.")
            return
        end
        
        -- Read the entire content of the file
        local content = file:read("*a")
        
        -- Close the file
        file:close()
        
        -- Replace the specified string
        local modifiedContent, replacements = string.gsub(content, '{ROOT}', path)
        
        -- Open the file in write mode to overwrite it
        file = io.open(srcfile, "w")
        
        if not file then
            print("Error: Unable to open file for writing.")
            return
        end
        
        -- Write the modified content back to the file
        file:write(modifiedContent)
        
        -- Close the file
        file:close()
        
        print("Replaced", "{ROOT}", "occurrences of", searchString, "with", replaceString, "in", filename)
     end

     ReplacePath(rootpath)