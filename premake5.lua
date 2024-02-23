outDir = "bin/%{cfg.buildcfg}"
--C:\VulkanSDK\1.3.275.0
--1.3.261.1
VulkanLibDir = "C:/VulkanSDK/1.3.275.0/Lib"
VulkanIncludeDir = "C:/VulkanSDK/1.3.275.0/Include"
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


     function replaceRootValue(filePath, newValue)
        -- Open the file in read mode
        local file = io.open(filePath, "r")
        if not file then
            print("Error opening file")
            return
        end
    
        -- Read all lines into a table
        local lines = {}
        for line in file:lines() do
            table.insert(lines, line)
        end
    
        -- Close the file
        file:close()
    
        -- Find and replace the line starting with "#define ROOT"
        for i, line in ipairs(lines) do
            if line:match("^#define ROOT") then
                -- Replace the old value with the new one
                lines[i] = "#define ROOT_DIR " .. newValue
                break
            end
        end
    
        -- Open the file in write mode
        file = io.open(filePath, "w")
        if not file then
            print("Error opening file for writing")
            return
        end
    
        -- Write the modified lines back to the file
        file:write(table.concat(lines, "\n"))
    
        -- Close the file
        file:close()
    
        print("Replacement completed successfully")
    end
    
    -- Example usage
    local filePath = "./Engine/API/path.h"
    replaceRootValue(filePath, rootpath)