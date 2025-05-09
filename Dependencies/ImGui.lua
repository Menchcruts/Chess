project "ImGui"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"
    location "./ImGui/"

    files { 
        "./ImGui/*.h", 
        "./ImGui/*.cpp",
        -- "./ImGui/misc/**.h",
        -- "./ImGui/misc/**.cpp",
        "./ImGui/backends/*glfw*.h", 
        "./ImGui/backends/*glfw*.cpp",
        "./ImGui/backends/*opengl*.h", 
        "./ImGui/backends/*opengl*.cpp"
    }

    includedirs
    {
        "./ImGui/",
        "./ImGui/backends",
        "./GLFW/include"
    }

    links
    {
        "GLFW",
        "opengl32"
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