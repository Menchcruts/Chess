#pragma once
#include "imgui.h"
#include <filesystem>
#include <cstdint>

class Image
{
private:
	ImTextureID texture_ = 0;
	int width_			= 0;
	int height_			= 0;
	int channels_		= 0;

	bool upload_(unsigned char* pixels, int w, int h, int ch,
		bool generate_mipmaps);

public:
	Image() = default;
	explicit Image(
		const std::filesystem::path& ImagePath,
		bool flip_vertically = false,
		int force_channels = 4,
		bool generate_mipmaps = true
		);

	Image(Image&& other) noexcept;
	Image& operator =(Image&& other) noexcept;

	Image(const Image&) = delete;
	Image& operator =(const Image&) = delete;

	~Image();

	bool loadFromFile(
		const std::filesystem::path& path,
		bool flip_vertically = false,
		int force_channels = 4,
		bool generate_mipmaps = true
	);

	bool loadFromMemory(
		const unsigned char* bytes, int byte_count,
		bool flip_vertically = false,
		int force_channels = 4,
		bool generate_mipmaps = true
	);

	void destroy();

	bool valid() const { return texture_ != 0; }
	explicit operator bool() const { return valid(); }

	ImTextureID ImGuiID() const { return texture_; }
	int width() const { return width_; }
	int height() const { return height_; }
	int channels() const { return channels_; }
};