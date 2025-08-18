project "ChessManager"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    cdialect "C17"
    staticruntime "off"

    files { "./**.h", "./**.hpp", "./**.cpp" }

    includedirs
    {
        -- Include Core libraries
        "../Core/Chess/",
        "../Core/ChessBot/",
        "../Core/Utils/",

        -- Include dependencies
        "../Dependencies/GLFW/GLFW/include/",
        "../Dependencies/ImGui/ImGui/",
        "../Dependencies/ImGui/ImGui/backends/",
        "../Dependencies/stb_image/",
        "../Dependencies/GLEW/GLEW/include/",
        "../Dependencies/miniaudio/"
    }

    links
    {
        "GLFW",
        "GLEW",
        "ImGui",
        "opengl32",
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
        symbols "Off"
        kind "WindowedApp"
        entrypoint "mainCRTStartup"
        