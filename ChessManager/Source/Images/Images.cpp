#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif // !STB_IMAGE_IMPLEMENTATION

//#ifndef STB_IMAGE_RESIZE_IMPLEMENTATION
//#define STB_IMAGE_RESIZE_IMPLEMENTATION
//#include "stb_image_resize2.h"
//#endif // !STB_IMAGE_RESIZE_IMPLEMENTATION

#include "Images.h"
#include <iostream>

Image::Image( std::filesystem::path ImagePath )
{    
    namespace fs = std::filesystem;

    GLFWimage image;
    if ( !fs::exists( ImagePath ) )
    {
        // Load dummy image later
        throw std::runtime_error( "Error loading image at " + ImagePath.string() );
    }

    int width, height, channels;
    unsigned char* data = stbi_load( ImagePath.string().c_str(), &width, &height, &channels, 0 );
    if ( !data )
    {
        // Load dummy image later
        throw std::runtime_error( "Error loading image at " + ImagePath.string() );
    }

    image.width = width;
    image.height = height;
    image.pixels = data;

    GLuint texture;
    glGenTextures( 1, &texture );
    glBindTexture( GL_TEXTURE_2D, texture );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.pixels );

    stbi_image_free( image.pixels );

    this->Width = image.width;
    this->Height = image.height;
    this->Texture = texture;
}