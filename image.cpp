#define cimg_display 0

#include "image.hpp"
#include <CImg.h>


void Image::save(const std::string& filename) const {
	auto byte_buffer = new unsigned char[width * height * 4];
	for(int i = 0; i < height; i++) {
		for(int j = 0; j < width; j++) {
			byte_buffer[ height * width * 0 + i * width + j ] = lround(buffer[i][j].r * 255.0);
			byte_buffer[ height * width * 1 + i * width + j ] = lround(buffer[i][j].g * 255.0);
			byte_buffer[ height * width * 2 + i * width + j ] = lround(buffer[i][j].b * 255.0);
			byte_buffer[ height * width * 3 + i * width + j ] = lround(buffer[i][j].a * 255.0);
		}
	}
	cimg_library::CImg<unsigned char>(byte_buffer, (int)width, (int)height, 1, 4, false).save(filename.c_str());
	delete [] byte_buffer;
}


Image::Image(const std::string& filename) {
	cimg_library::CImg<unsigned char> image;
	image.load(filename.c_str());
	auto byte_buffer = image.data();
	width = image.width();
	height = image.height();
	buffer = std::vector<std::vector<RGBAColor>>(height, std::vector<RGBAColor>(width, RGBAColor()));

	for(int i = 0; i < height; i++) {
		for(int j = 0; j < width; j++) {
			buffer[i][j].r = byte_buffer[ height * width * 0 + i * width + j ] / 255.0;
			buffer[i][j].g = byte_buffer[ height * width * 1 + i * width + j ] / 255.0;
			buffer[i][j].b = byte_buffer[ height * width * 2 + i * width + j ] / 255.0;
			if(image.spectrum() == 4) {
				buffer[i][j].a = byte_buffer[ height * width * 3 + i * width + j ] / 255.0;
			}
		}
	}
}


RGBAColor operator*(double k, const RGBAColor& c) {
	return c*k;
}