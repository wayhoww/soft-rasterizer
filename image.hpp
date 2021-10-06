#pragma once

#include <concepts>
#include <string>
#include <cstring>
#include <vector>


struct RGBColor {
	double r = 0;
	double g = 0;
	double b = 0;

	RGBColor& operator*=(double k) {
		r *= k;
		g *= k;
		b *= k;
		return *this;
	}

	RGBColor operator*(double k) const {
		auto out = *this;
		return out *= k;
	}

	RGBColor& operator+=(const RGBColor& c) {
		r += c.r;
		g += c.g;
		b += c.b;
		return *this;
	}

	RGBColor operator+(const RGBColor& c) const {
		auto out = *this;
		return out += c;
	}
};

RGBColor operator*(double k, const RGBColor& c);


class Image {
	int width;
	int height;
	std::vector<std::vector<RGBColor>> buffer;
public:
	Image(int width, int height): 
		width(width), 
		height(height), 
		buffer(std::vector<std::vector<RGBColor>>(height, std::vector<RGBColor>(width, RGBColor()))) {}


	RGBColor& operator[](const std::pair<int, int>& p) {
		return buffer[p.second][height - 1 - p.first];
	}

	const RGBColor& operator[](const std::pair<int, int>& p) const {
		return (*this)[p];
	}

	void save(const std::string& filename) const;
};

// 返回值和参数的 std::move