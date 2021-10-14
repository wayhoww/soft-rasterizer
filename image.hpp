#pragma once

#include <concepts>
#include <string>
#include <cstring>
#include <vector>
#include <cstdlib>


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


class Image {
	int width;
	int height;
	RGBAColor* buffer = nullptr;
public:

	Image(): width(0), height(0), buffer(nullptr) {}

	Image(int width, int height, bool init = true): 
		width(width), 
		height(height), 
		buffer((RGBAColor*) malloc(sizeof(RGBAColor) * width * height)) {
		
		if (init) memset(buffer, 0, sizeof(RGBAColor) * width * height);
	}

	~Image() {
		free(buffer);
	}

	Image(const Image& other) {
		width = other.width;
		height = other.height;
		buffer = (RGBAColor*)malloc(sizeof(RGBAColor) * width * height);
		memcpy(buffer, other.buffer, sizeof(RGBAColor) * width * height);
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
	RGBAColor& operator[](const std::pair<int, int>& p) {
		return buffer[(height - 1 - p.second) * width + p.first];
	}

	const RGBAColor& operator[](const std::pair<int, int>& p) const {
		return buffer[(height - 1 - p.second) * width + p.first];
	}

	void save(const std::string& filename) const;

	std::pair<int, int> size() const {
		return {width, height};
	}
};

// 返回值和参数的 std::move