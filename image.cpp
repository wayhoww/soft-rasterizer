#define cimg_display 0

#include "image.hpp"
#include "CImg.h"
#include <map>
#include <string>

static std::map<std::string, Image> image_cache;


void Image::save(const std::string& filename) const {
	cimg_library::CImg<unsigned char>(buffer, (int)width, (int)height, 1, 4, false).save(filename.c_str());
}

Image& Image::operator=(const Image& other) {
	free(buffer);
	width = other.width;
	height = other.height;
	buffer = (unsigned char*)malloc(PIXEL_SIZE * width * height);
	memcpy(buffer, other.buffer, PIXEL_SIZE * width * height);
	return *this;
}

Image::Image(const std::string& filename, bool use_cache, bool save_to_cache) {
	if (use_cache && image_cache.count(filename)) {
		*this = image_cache[filename]; //  为什么取下标会要求默认构造函数？
		return;
	}

	cimg_library::CImg<unsigned char> image;
	image.load(filename.c_str());
	auto byte_buffer = image.data();
	width = image.width();
	height = image.height();
	buffer = (unsigned char*) malloc(width * height * PIXEL_SIZE);

	memcpy(buffer, byte_buffer, width * height * image.spectrum());
	if (image.spectrum() == 3) {
		memset(buffer + 3 * width * height, 1, width * height);
	}

	if (save_to_cache) {
		image_cache[filename] = *this;
	}
}


RGBAColor operator*(float k, const RGBAColor& c) {
	return c*k;
}