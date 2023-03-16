
workspace "lince"
    architecture "x86_64"
    startproject "mcommand"
    warnings "Extra"

    defines {
        "GLFW_INCLUDED_NONE",
        'LINCE_DIR="lince/"'
    }

    configurations {"Debug", "Release"}
    filter "system:windows"
        systemversion "latest"
        defines {"_CRT_SECURE_NO_WARNINGS"}
        links {"opengl32"}

    filter "system:linux"
        systemversion "latest"    
        links {"GL","rt","m","dl","pthread","X11"}
        
    filter "configurations:Debug"
        symbols "on"
        defines {"LINCE_DEBUG"}

    filter "configurations:Release"
        optimize "on"
        defines {"LINCE_RELEASE"}


OutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["glfw"]    = "lince/deps/glfw/include"
IncludeDir["glad"]    = "lince/deps/glad/include"
IncludeDir["cglm"]    = "lince/deps/cglm/include"
IncludeDir["nuklear"] = "lince/deps/nuklear/include"
IncludeDir["stb"]     = "lince/deps/stb/include"
IncludeDir["miniaudio"] = "lince/deps/miniaudio/include"
IncludeDir["lince"]   = "lince/lince/src"


project "glfw"
    kind "StaticLib"
    language "C"
    staticruntime "on"
    location "lince/build/glfw"

    targetdir ("bin/" .. OutputDir .. "/%{prj.name}")
    objdir ("obj/" .. OutputDir .. "/%{prj.name}")

    SrcDir = "lince/deps/glfw/src"
    
    files {
        "lince/deps/glfw/include/GLFW/glfw3.h",
        "lince/deps/glfw/include/GLFW/glfw3native.h",
        SrcDir .. "/context.c",
        SrcDir .. "/init.c",
        SrcDir .. "/input.c",
        SrcDir .. "/monitor.c",
        SrcDir .. "/vulkan.c",
        SrcDir .. "/window.c",
        SrcDir .. "/osmesa_context.c",
    }
    
    filter "system:linux"
        systemversion "latest"
        files {
            SrcDir .. "/posix_time.c",
            SrcDir .. "/posix_thread.c",
            SrcDir .. "/x11_init.c",
            SrcDir .. "/x11_monitor.c",
            SrcDir .. "/x11_window.c",
            SrcDir .. "/egl_context.c",
            SrcDir .. "/xkb_unicode.c",
            SrcDir .. "/glx_context.c",
            SrcDir .. "/linux_joystick.c",
        }
        defines {"_GLFW_X11"}
    
    filter "system:windows"
        files{
            SrcDir .. "/win32_init.c",
            SrcDir .. "/win32_joystick.c",
            SrcDir .. "/win32_monitor.c",
            SrcDir .. "/win32_time.c",
            SrcDir .. "/win32_thread.c",
            SrcDir .. "/win32_window.c",
            SrcDir .. "/wgl_context.c",
            SrcDir .. "/egl_context.c",
        }

        defines{
            "_GLFW_WIN32",
            "_CRT_SECURE_NO_WARNINGS"
        }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"


project "glad"
    kind "StaticLib"
    language "C"
    staticruntime "On"
    location "lince/build/glad"

    targetdir ("bin/" .. OutputDir .. "/%{prj.name}")
    objdir ("obj/" .. OutputDir .. "/%{prj.name}")

    files {
        "lince/deps/glad/include/glad/glad.h",
        "lince/deps/glad/include/KHR/khrplatform.h",
        "lince/deps/glad/src/glad.c"
    }

    includedirs {"lince/deps/glad/include"}

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"


project "cglm"
    kind "StaticLib"
    language "C"
    staticruntime "On"
    location "lince/build/cglm"

    targetdir ("bin/" .. OutputDir .. "/%{prj.name}")
    objdir ("obj/" .. OutputDir .. "/%{prj.name}")

    files {
        "lince/deps/cglm/src/**.c",
        "lince/deps/cglm/include/cglm/**.h"
    }

    includedirs {"lince/deps/cglm/include"}

    defines {"CGLM_STATIC"}

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"


project "stb"
    kind "StaticLib"
    language "C"
    staticruntime "On"
    location "lince/build/stb"

    targetdir ("bin/" .. OutputDir .. "/%{prj.name}")
    objdir ("obj/" .. OutputDir .. "/%{prj.name}")

    files {
        "lince/deps/stb/src/**.c",
        "lince/deps/stb/include/**.h"
    }

    includedirs {"lince/deps/stb/include"}

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"


project "nuklear"
    kind "StaticLib"
    language "C"
    staticruntime "on"
    location "lince/build/nuklear"

    objdir ("obj/" .. OutputDir .. "/%{prj.name}" )
    targetdir ("bin/" .. OutputDir .. "/%{prj.name}" )
    
    defines "GLFW_INCLUDED_NONE"

    files {
        "lince/deps/nuklear/src/**.c",
        "lince/deps/nuklear/include/**.h",
    }
    
    includedirs {
        "lince/deps/nuklear",
        "lince/deps/nuklear/src",
        "%{IncludeDir.nuklear}",
        "%{IncludeDir.glfw}",
        "%{IncludeDir.glad}",
    }

    links {
        "GLAD",
        "glfw",
    }

    libdirs {
        "bin/" .. OutputDir .. "/glad",
        "bin/" .. OutputDir .. "/glfw",
    }


project "miniaudio"
    kind "StaticLib"
    language "C"
    staticruntime "on"
    location "lince/build/miniaudio"

    objdir ("obj/" .. OutputDir .. "/%{prj.name}" )
    targetdir ("bin/" .. OutputDir .. "/%{prj.name}" )
    
    files {
        "lince/deps/miniaudio/src/**.c",
        "lince/deps/miniaudio/include/**.h",
    }
    
    includedirs {
        "lince/deps/miniaudio/include",
    }

    links {
        "GLAD",
        "glfw",
    }

    libdirs {
        "bin/" .. OutputDir .. "/glad",
        "bin/" .. OutputDir .. "/glfw",
    }


project "lince"
    kind "StaticLib"
    language "C"
    staticruntime "on"
    location "lince/build/lince"

    targetdir ("bin/" .. OutputDir .. "/%{prj.name}")
    objdir ("obj/" .. OutputDir .. "/%{prj.name}")

    files {
        "lince/lince/src/**.c",
        "lince/lince/src/**.h",
    }
    
    includedirs {
        "lince/lince",
		"lince/lince/src",
        "lince/lince/src/lince",
        "%{IncludeDir.glfw}",
        "%{IncludeDir.glad}",
        "%{IncludeDir.cglm}",
        "%{IncludeDir.nuklear}",
        "%{IncludeDir.stb}",
        "%{IncludeDir.miniaudio}"
    }

    links {
        "glad",
        "glfw",
        "cglm",
        "nuklear",
        "stb",
        "miniaudio"
    }

    libdirs {
        "bin/" .. OutputDir .. "/glad",
        "bin/" .. OutputDir .. "/glfw",
        "bin/" .. OutputDir .. "/nuklear",
        "bin/" .. OutputDir .. "/cglm",
        "bin/" .. OutputDir .. "/stb",
        "bin/" .. OutputDir .. "/miniaudio",
    }


project "tests"
    kind "ConsoleApp"
    language "C"
    staticruntime "on"
    location "lince/build/tests"

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
        "%{IncludeDir.cglm}"
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


project "editor"
    kind "ConsoleApp"
    language "C"
    staticruntime "on"
    location "lince/build/editor"

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
        "%{IncludeDir.nuklear}",
        "%{IncludeDir.stb}"
    }

    links {
        "lince",
        "glad",
        "glfw",
        "cglm",
        "stb",
        "nuklear"
    }

    libdirs {"bin/" .. OutputDir .. "/lince"}


project "sandbox"
    kind "ConsoleApp"
    language "C"
    staticruntime "on"
    location "lince/build/sandbox"
    
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
        "%{IncludeDir.nuklear}",
        "%{IncludeDir.stb}",
        "%{IncludeDir.miniaudio}"
    }

    links {
        "lince",
        "glad",
        "glfw",
        "cglm",
        "stb",
        "nuklear",
        "miniaudio"
    }

    libdirs {"bin/" .. OutputDir .. "/lince"}


include "projects.lua"

