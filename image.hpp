#pragma once

#include <concepts>
#include <string>
#include <cstring>
#include <vector>


struct RGBAColor {
	double r = 0;
	double g = 0;
	double b = 0;
	double a = 1;

	RGBAColor& operator*=(double k) {
		r *= k;
		g *= k;
		b *= k;
		a *= k;
		return *this;
	}

	RGBAColor operator*(double k) const {
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
		r = std::max<double>(0, std::min<double>(r, 1));
		g = std::max<double>(0, std::min<double>(g, 1));
		b = std::max<double>(0, std::min<double>(b, 1));
		a = std::max<double>(0, std::min<double>(a, 1));
		return *this;
	}
};

RGBAColor operator*(double k, const RGBAColor& c);


class Image {
	int width;
	int height;
	std::vector<std::vector<RGBAColor>> buffer;
public:
	Image(int width, int height): 
		width(width), 
		height(height), 
		buffer(std::vector<std::vector<RGBAColor>>(height, std::vector<RGBAColor>(width, RGBAColor()))) {}

	Image(const std::string& filename);

	// [{x, y}], 左下坐标系
	RGBAColor& operator[](const std::pair<int, int>& p) {
		return buffer[height - 1 - p.second][p.first];
	}

	const RGBAColor& operator[](const std::pair<int, int>& p) const {
		return buffer[height - 1 - p.second][p.first];
	}

	void save(const std::string& filename) const;

	std::pair<int, int> size() const {
		return {width, height};
	}
};

// 返回值和参数的 std::move