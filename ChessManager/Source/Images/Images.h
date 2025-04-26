#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui.h"

#include <filesystem>


struct Image
{
    ImTextureID Texture;
    int Width;
    int Height;
};

bool LoadImageData( std::filesystem::path FilePath, GLFWimage* OutImage, float Scale );

bool LoadImageTexture( std::filesystem::path FilePath, Image* OutImage, float Scale );