project "GLEW"
	kind "StaticLib"
	language "C"
	staticruntime "off"
	warnings "off"

	targetdir ("../bin/" .. OutputDir .. "/%{prj.name}")
	objdir ("../bin/Intermediates/" .. OutputDir .. "/%{prj.name}")

	includedirs
	{
		"include"
	}

	files
	{
		"src/glew.c",
		"src/glewinfo.c",
		"src/visualinfo.c"
	}

	filter "system:windows"
		defines
		{
			"_CRT_SECURE_NO_WARNINGS"
		}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "speed"

    filter "configurations:Dist"
		runtime "Release"
		optimize "speed"
        symbols "off"
