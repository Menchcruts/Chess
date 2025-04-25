#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif // !STB_IMAGE_IMPLEMENTATION

#ifndef STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"
#endif // !STB_IMAGE_RESIZE_IMPLEMENTATION

#include "Images.h"
#include <iostream>

bool LoadImageData( std::filesystem::path FilePath, GLFWimage* OutImage, float Scale )
{
    if ( FILE* file = fopen( FilePath.string().c_str(), "r" ) )
    {
        fclose( file );
        int width, height, channels;
        unsigned char* data = stbi_load( FilePath.string().c_str(), &width, &height, &channels, 0 );
        if ( !data )
            return false;

        OutImage->width = width;
        OutImage->height = height;
        OutImage->pixels = data;
        std::cout << "Loaded image: " << FilePath.string() << "\n";
        return true;
    }
    return false;
}

bool LoadImageTexture( std::filesystem::path FilePath, Image* OutImage, float Scale )
{
    GLFWimage image;
    if ( !LoadImageData( FilePath, &image, Scale ) )
        return false;

    int ScaledWidth = static_cast<int>(image.width * Scale);
    int ScaledHeight = static_cast<int>(image.height * Scale);

    unsigned char* ScaledPixels = (unsigned char*)malloc( ScaledWidth * ScaledHeight * 4 );
    if ( !ScaledPixels )
    {
        stbi_image_free( image.pixels );
        return false;
    }

    if ( !stbir_resize_uint8_linear( image.pixels, image.width, image.height, 0,
                                     ScaledPixels, ScaledWidth, ScaledHeight, 0,
                                     stbir_pixel_layout::STBIR_4CHANNEL ) )
    {
        free( ScaledPixels );
        stbi_image_free( image.pixels );
        return false;
    }
    stbi_image_free( image.pixels );

    GLuint texture;
    glGenTextures( 1, &texture );
    glBindTexture( GL_TEXTURE_2D, texture );

    // Setup filtering parameters for display
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // Upload pixels into texture
    glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, ScaledWidth, ScaledHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, ScaledPixels );

    free( ScaledPixels );

    OutImage->Texture = texture;
    OutImage->Width = ScaledWidth;
    OutImage->Height = ScaledHeight;
    return true;
}
