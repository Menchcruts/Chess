#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui.h"

#include <filesystem>


struct Image
{
    ImTextureID Texture = 0;
    int Width = 0;
    int Height = 0;

    Image() = default;
    Image(std::filesystem::path ImagePath );
};