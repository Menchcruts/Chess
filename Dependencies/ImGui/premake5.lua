project "ImGui"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    files { 
        "%{prj.name}/*.h", 
        "%{prj.name}/*.cpp",
        -- "%{prj.name}/misc/**.h",
        -- "%{prj.name}/misc/**.cpp",
        "%{prj.name}/backends/*glfw*.h", 
        "%{prj.name}/backends/*glfw*.cpp",
        "%{prj.name}/backends/*opengl*.h", 
        "%{prj.name}/backends/*opengl*.cpp"
    }

    includedirs
    {
        "%{prj.name}/",
        "%{prj.name}/backends/",
        "../GLFW/GLFW/include/"
    }

    links
    {
        "GLFW",
        "opengl32"
    }

    targetdir ("../bin/" .. OutputDir .. "/%{prj.name}")
    objdir ("../bin/Intermediates/" .. OutputDir .. "/%{prj.name}")

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