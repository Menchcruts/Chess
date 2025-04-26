project "ImGui"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"
    location "./ImGui/"

    files { "/ImGui/*.h", "/ImGui/*.cpp", "/ImGui/backends/**.h", "/ImGui/backends/**.cpp" }

    includedirs
    {
        "/ImGui/",
        "/ImGui/backends",
        "/GLFW/include"
    }

    links
    {
        "GLFW",
        "opengl32.lib"
    }

    targetdir ("./bin/" .. OutputDir .. "/%{prj.name}")
    objdir ("./bin/Intermediates/" .. OutputDir .. "/%{prj.name}")

    filter "system:windows"
        systemversion "latest"
        defines { }

    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "RELEASE" }
        runtime "Release"
        optimize "On"
        symbols "Off"