


----------------------------
--------- MINIGAMES --------
----------------------------

project "pong"
    kind "ConsoleApp"
    language "C"
    staticruntime "on"
    location "pong"

    targetdir ("bin/" .. OutputDir .. "/%{prj.name}")
    objdir ("obj/" .. OutputDir .. "/%{prj.name}")

    files {
        "%{prj.name}/src/**.c",
        "%{prj.name}/src/**.h",
    }
    
    includedirs {
        "%{prj.name}",
        "%{prj.name}/src",
        "lince/%{IncludeDir.lince}",
        "lince/%{IncludeDir.glfw}",
        "lince/%{IncludeDir.glad}",
        "lince/%{IncludeDir.cglm}",
        "lince/%{IncludeDir.nuklear}",
        "lince/%{IncludeDir.miniaudio}",
    }

    links {
        "lince",
        "glad",
        "glfw",
        "cglm",
        "nuklear",
        "stb",
        "miniaudio"
    }

    libdirs {"bin/" .. OutputDir .. "/lince"}

    
project "mcommand"
    kind "ConsoleApp"
    language "C"
    staticruntime "on"
    location "mcommand"
    
    targetdir ("bin/" .. OutputDir .. "/%{prj.name}")
    objdir ("obj/" .. OutputDir .. "/%{prj.name}")

    files {
        "%{prj.name}/src/**.c",
        "%{prj.name}/src/**.h",
    }
    
    includedirs {
        "%{prj.name}",
        "%{prj.name}/src",
        "%{IncludeDir.lince}",
        "%{IncludeDir.glfw}",
        "%{IncludeDir.glad}",
        "%{IncludeDir.cglm}",
        "%{IncludeDir.nuklear}"
    }

    links {
        "lince",
        "glad",
        "glfw",
        "cglm",
        "nuklear",
        "stb"
    }

    libdirs {"bin/" .. OutputDir .. "/lince"}

