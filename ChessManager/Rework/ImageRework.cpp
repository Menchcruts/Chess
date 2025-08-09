#include "ImageRework.h"
#include "GL/glew.h"
#include "GLFW/glfw3.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif // !STB_IMAGE_IMPLEMENTATION

#include "error_image.h"
#include <iostream>

Image::~Image() { destroy(); }

Image::Image(Image&& other) noexcept { *this = std::move(other); }

Image& Image::operator=(Image&& other) noexcept
{
	if (this == &other) return *this;
	destroy();
	texture_ = other.texture_;  other.texture_ = 0;
	width_ = other.width_;    other.width_ = 0;
	height_ = other.height_;   other.height_ = 0;
	channels_ = other.channels_; other.channels_ = 0;
	return *this;
}

Image::Image( const std::filesystem::path& path, bool flip_vertically, int force_channels, bool generate_mipmaps)
{
	loadFromFile(path, flip_vertically, force_channels, generate_mipmaps);
}

bool Image::upload_(unsigned char* pixels, int w, int h, int ch, bool generate_mipmaps)
{
	GLuint gl_tex = 0;
	glGenTextures(1, &gl_tex);
	glBindTexture(GL_TEXTURE_2D, gl_tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		generate_mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

	const GLint  internal_fmt = GL_RGBA8;
	const GLenum data_fmt = GL_RGBA;
	const GLenum data_type = GL_UNSIGNED_BYTE;

	glTexImage2D(GL_TEXTURE_2D, 0, internal_fmt, w, h, 0, data_fmt, data_type, pixels);
	if (generate_mipmaps)
		glGenerateMipmap(GL_TEXTURE_2D);

	texture_ = (ImTextureID)(uintptr_t)(gl_tex);
	width_ = w;
	height_ = h;
	channels_ = ch;

	return true;
}

bool Image::loadFromFile(const std::filesystem::path& path, bool flip_vertically, int force_channels, bool generate_mipmaps)
{
	namespace fs = std::filesystem;
	destroy();

	stbi_set_flip_vertically_on_load(flip_vertically);
	int w = 0, h = 0, ch = 0;
	int desired_channels = force_channels > 0 ? force_channels : 4;

	unsigned char* pixels = nullptr;

	if (fs::exists(path))
		pixels = stbi_load(
			path.string().c_str(), 
			&w, &h, &ch, 
			desired_channels
		);
	
	if (!pixels)
	{
		pixels = stbi_load_from_memory(
			error_image,
			error_image_len,
			&w, &h, &ch,
			desired_channels
		);

		if (!pixels)
		{
			std::cerr	<< "[Image] Failed to load both '" << path.string()
						<< "' and embedded error image. Reason: "
						<< (stbi_failure_reason() ? stbi_failure_reason() : "unknown error")
						<< "\n";
		}
		return false;
	}

	bool ok = upload_(pixels, w, h, desired_channels, generate_mipmaps);
	stbi_image_free(pixels);
	return ok;
}

bool Image::loadFromMemory(const unsigned char* bytes, int byte_count, bool flip_vertically, int force_channels, bool generate_mipmaps)
{
	destroy();
	stbi_set_flip_vertically_on_load(flip_vertically);
	int w = 0, h = 0, ch = 0;
	int desired_channels = force_channels > 0 ? force_channels : 4;

	unsigned char* pixels = stbi_load_from_memory(
		bytes, byte_count, &w, &h, &ch,
		desired_channels
	);

	if (!pixels)
	{
		pixels = stbi_load_from_memory(
			error_image,
			static_cast<int>(error_image_len),
			&w, &h, &ch,
			desired_channels
		);
		if (!pixels)
		{
			std::cerr	<< "Failed to decode from memory and fallback image. Reason: "
						<< (stbi_failure_reason() ? stbi_failure_reason() : "unknown error")
						<< "\n";
			return false;
		}
	}

	bool ok = upload_(pixels, w, h, desired_channels, generate_mipmaps);
	stbi_image_free(pixels);
	return ok;
}

void Image::destroy()
{
	if (texture_)
	{
		GLuint id = (GLuint)(uintptr_t)(texture_);
		glDeleteTextures(1, &id);
		texture_ = 0;
	}
	width_ = height_ = channels_ = 0;
}