project "ChessManager"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    files { "Source/**.h", "Source/**.cpp", "../Dependencies/miniaudio/miniaudio.c" }

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
        "../Dependencies/miniaudio/"
    }

    links
    {
        "GLFW",
        "GLEW",
        "ImGui",
        "opengl32.lib",
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

    filter "system:linux"
        defines { "LINUX" }

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

    filter "Release or Dist"
        kind "WindowedApp"
        entrypoint "mainCRTStartup"
        -- filter "system:windows"
        --     entrypoint "mainCRTStartup"
        
        -- filter "system:linux"
        --     entrypoint "WinMainCRTStartup"
        --     buildoptions { "-mwindows" }