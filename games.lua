

project "pong"
    kind "ConsoleApp"
    language "C"
    staticruntime "on"
    location "pong"

    defines {"LINCE_DIR=\"lince/\""}

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

    libdirs {
        "lince/bin/" .. OutputDir .. "/lince",
        "lince/bin/" .. OutputDir .. "/glad",
        "lince/bin/" .. OutputDir .. "/glfw",
        "lince/bin/" .. OutputDir .. "/cglm",
        "lince/bin/" .. OutputDir .. "/nuklear",
        "lince/bin/" .. OutputDir .. "/stb",
        "lince/bin/" .. OutputDir .. "/miniaudio",
    }

    
project "mcommand"
    kind "ConsoleApp"
    language "C"
    staticruntime "on"
    location "mcommand"

    defines {"LINCE_DIR=\"lince/\""}
    
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
        "lince/%{IncludeDir.nuklear}"
    }

    links {
        "lince",
        "glad",
        "glfw",
        "cglm",
        "nuklear",
        "stb"
    }

    libdirs {
        "lince/bin/" .. OutputDir .. "/lince",
        "lince/bin/" .. OutputDir .. "/glad",
        "lince/bin/" .. OutputDir .. "/glfw",
        "lince/bin/" .. OutputDir .. "/cglm",
        "lince/bin/" .. OutputDir .. "/nuklear",
        "lince/bin/" .. OutputDir .. "/stb",
        "lince/bin/" .. OutputDir .. "/miniaudio",
    }

