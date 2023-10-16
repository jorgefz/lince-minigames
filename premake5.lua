

-- Folder name for compilation outputs inside 'build/' and 'bin/'.
LinceOutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Dependencies' include directories
LinceIncludeDir = {}
LinceIncludeDir["glfw"]      = "lince/deps/glfw/include"
LinceIncludeDir["glad"]      = "lince/deps/glad/include"
LinceIncludeDir["cglm"]      = "lince/deps/cglm/include"
LinceIncludeDir["nuklear"]   = "lince/deps/nuklear/include"
LinceIncludeDir["stb"]       = "lince/deps/stb/include"
LinceIncludeDir["miniaudio"] = "lince/deps/miniaudio/include"
LinceIncludeDir["lince"]     = "lince/lince/src"


workspace "minigames"
    architecture "x86_64"
    startproject "empty"
    warnings "Extra"

    configurations {"Debug", "Release"}
    flags {"MultiProcessorCompile"}

    group "lince"
        include "lince/deps/premake5.lua"
        include "lince/lince"
    group ""