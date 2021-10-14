#define cimg_display 0

#include "image.hpp"
#include "CImg.h"
#include <map>
#include <string>

static std::map<std::string, Image> image_cache;


void Image::save(const std::string& filename) const {
	auto byte_buffer = new unsigned char[width * height * 4];
	for(int i = 0; i < height; i++) {
		for(int j = 0; j < width; j++) {
			byte_buffer[ height * width * 0 + i * width + j ] = lround(buffer[i * width + j].r * 255.0);
			byte_buffer[ height * width * 1 + i * width + j ] = lround(buffer[i * width + j].g * 255.0);
			byte_buffer[ height * width * 2 + i * width + j ] = lround(buffer[i * width + j].b * 255.0);
			byte_buffer[ height * width * 3 + i * width + j ] = lround(buffer[i * width + j].a * 255.0);
		}
	}
	cimg_library::CImg<unsigned char>(byte_buffer, (int)width, (int)height, 1, 4, false).save(filename.c_str());
	delete [] byte_buffer;
}

Image& Image::operator=(const Image& other) {
	free(buffer);
	width = other.width;
	height = other.height;
	buffer = (RGBAColor*)malloc(sizeof(RGBAColor) * width * height);
	memcpy(buffer, other.buffer, sizeof(RGBAColor) * width * height);
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
	buffer = (RGBAColor*) malloc(width * height * sizeof(RGBAColor));

	for(int i = 0; i < height; i++) {
		for(int j = 0; j < width; j++) {
			buffer[i * width + j].r = byte_buffer[ height * width * 0 + i * width + j ] / 255.0;
			buffer[i * width + j].g = byte_buffer[ height * width * 1 + i * width + j ] / 255.0;
			buffer[i * width + j].b = byte_buffer[ height * width * 2 + i * width + j ] / 255.0;
			if(image.spectrum() == 4) {
				buffer[i * width + j].a = byte_buffer[ height * width * 3 + i * width + j ] / 255.0;
			} else {
				buffer[i * width + j].a = 0;
			}
		}
	}

	if (save_to_cache) {
		image_cache[filename] = *this;
	}
}


RGBAColor operator*(float k, const RGBAColor& c) {
	return c*k;
}