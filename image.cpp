#define cimg_display 0

#include "image.hpp"
#include <CImg.h>


void Image::save(const std::string& filename) const {
	auto byte_buffer = new unsigned char[width * height * 3];
	for(int i = 0; i < height; i++) {
		for(int j = 0; j < width; j++) {
			byte_buffer[ height * width * 0 + i * width + j ] = lround(buffer[i][j].r * 255.0);
			byte_buffer[ height * width * 1 + i * width + j ] = lround(buffer[i][j].g * 255.0);
			byte_buffer[ height * width * 2 + i * width + j ] = lround(buffer[i][j].b * 255.0);
		}
	}
	cimg_library::CImg<unsigned char>(byte_buffer, (int)width, (int)height, 1, 3, false).save(filename.c_str());
	delete [] byte_buffer;
}

RGBColor operator*(double k, const RGBColor& c) {
	return c*k;
}