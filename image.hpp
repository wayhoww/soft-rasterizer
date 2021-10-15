#pragma once

#include <concepts>
#include <string>
#include <cstring>
#include <vector>
#include <cstdlib>
#include <cmath>


struct RGBAColor {
	float r = 0;
	float g = 0;
	float b = 0;
	float a = 1;

	RGBAColor& operator*=(float k) {
		r *= k;
		g *= k;
		b *= k;
		a *= k;
		return *this;
	}

	RGBAColor operator*(float k) const {
		auto out = *this;
		return out *= k;
	}

	RGBAColor& operator+=(const RGBAColor& c) {
		r += c.r;
		g += c.g;
		b += c.b;
		a += c.a;
		return *this;
	}

	RGBAColor operator+(const RGBAColor& c) const {
		auto out = *this;
		return out += c;
	}	
	
	RGBAColor& operator*=(const RGBAColor& c) {
		r *= c.r;
		g *= c.g;
		b *= c.b;
		a *= c.a;
		return *this;
	}

	RGBAColor operator*(const RGBAColor& c) const {
		auto out = *this;
		return out *= c;
	}

	RGBAColor& clip() {
		r = std::max<float>(0, std::min<float>(r, 1));
		g = std::max<float>(0, std::min<float>(g, 1));
		b = std::max<float>(0, std::min<float>(b, 1));
		a = std::max<float>(0, std::min<float>(a, 1));
		return *this;
	}
};

RGBAColor operator*(float k, const RGBAColor& c);


constexpr int PIXEL_SIZE = 4;

class Image {
	int width;
	int height;
	unsigned char* buffer = nullptr;
public:

	Image(): width(0), height(0), buffer(nullptr) {}

	Image(int width, int height, bool init = true): 
		width(width), 
		height(height), 
		buffer((unsigned char*) malloc(PIXEL_SIZE* width * height)) {
		
		if (init) {
			memset(buffer, 0, PIXEL_SIZE * width * height); 
		}
	}

	~Image() {
		free(buffer);
	}

	Image(const Image& other) {
		width = other.width;
		height = other.height;
		buffer = (unsigned char*)malloc(PIXEL_SIZE * width * height);
		memcpy(buffer, other.buffer, PIXEL_SIZE * width * height);
	}

	Image(Image&& other) {
		width = other.width;
		height = other.height;
		buffer = other.buffer;
		other.buffer = nullptr;
	}

	Image& operator=(const Image& other);

	Image& operator=(Image&& other) {
		free(buffer);
		width = other.width;
		height = other.height;
		buffer = other.buffer;
		other.buffer = nullptr;
		return *this;
	}

	Image(const std::string& filename, bool use_cache = true, bool save_to_cache = true);

	// [{x, y}], 左下坐标系
	RGBAColor getPixel(int x, int y) const {
		int pixel_offset = (height - 1 - y) * width + x;
		int channel_size = height * width;
		return RGBAColor{
			buffer[0 * channel_size + pixel_offset] / 255.0f,
			buffer[1 * channel_size + pixel_offset] / 255.0f,
			buffer[2 * channel_size + pixel_offset] / 255.0f,
			buffer[3 * channel_size + pixel_offset] / 255.0f
		};
	}

	Image& setPixel(int x, int y, const RGBAColor& color) {
		int pixel_offset = (height - 1 - y) * width + x;
		int channel_size = height * width;
		buffer[0 * channel_size + pixel_offset] = std::lround(color.r * 255.0f);
		buffer[1 * channel_size + pixel_offset] = std::lround(color.g * 255.0f);
		buffer[2 * channel_size + pixel_offset] = std::lround(color.b * 255.0f);
		buffer[3 * channel_size + pixel_offset] = std::lround(color.a * 255.0f);

		return *this;
	}

	void save(const std::string& filename) const;

	std::pair<int, int> size() const {
		return {width, height};
	}
};

// 返回值和参数的 std::move