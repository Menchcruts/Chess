project "ChessManager"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    files { "Source/**.h", "Source/**.cpp" }

    includedirs
    {
        "Source",

        -- Include Core libraries
        "../Core/Chess/Source/",
        "../Core/ChessBotLocal/Source/",
        "../Core/Utils",

        -- Include dependencies
        "../Dependencies/GLFW/include/",
        "../Dependencies/ImGui/",
        "../Dependencies/ImGui/backends/",
        "../Dependencies/stb_image/",
        "../Dependencies/GLEW/include/",
        "../Dependencies/irrKlang-64bit-1.6.0/include/"
    }

    libdirs 
    {
        "../Dependencies/irrKlang-64bit-1.6.0/lib/Winx64-visualStudio/"
    }

    links
    {
        "GLFW",
        "GLEW",
        "ImGui",
        "opengl32.lib",
        "irrKlang",
        "Chess",
        "ChessBot"
    }

    defines
    {
        "GLEW_STATIC"
    }

    targetdir ("../bin/" .. OutputDir .. "/%{prj.name}")
    objdir ("../bin/Intermediates/" .. OutputDir .. "/%{prj.name}")

    filter "system:windows"
        systemversion "latest"
        defines { "WINDOWS" }
        postbuildcommands 
        { 
            "{COPY} ../Dependencies/irrKlang-64bit-1.6.0/dlls/winx64-visualStudio/irrKlang.dll %{cfg.targetdir}",
            "{COPY} ../Dependencies/irrKlang-64bit-1.6.0/dlls/winx64-visualStudio/ikpMP3.dll %{cfg.targetdir}"
        }

    filter "system:linux"
        defines { "LINUX" }
        postbuildcommands 
        { 
            "{COPY} ../Dependencies/irrKlang-64bit-1.6.0/dlls/linux-gcc-64/libIrrKlang.so %{cfg.targetdir}",
            "{COPY} ../Dependencies/irrKlang-64bit-1.6.0/dlls/linux-gcc-64/ikpMP3.so %{cfg.targetdir}"
        }

    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "RELEASE" }
        runtime "Release"
        optimize "On"
        symbols "On"

    filter "configurations:Dist"
        defines { "DIST" }
        runtime "Release"
        optimize "On"
        symbols "Off"