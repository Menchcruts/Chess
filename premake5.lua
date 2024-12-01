-- premake5.lua
workspace "Chess_V1.0"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "ChessManager"

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

group "Core"
	include "Core/Chess"
    include "Core/ChessBotLocal"

group "Dependencies"
    include "Dependencies/GLFW"
    include "Dependencies/ImGui"
    
group ""
    include "ChessManager"