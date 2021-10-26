

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "common_header.hpp"
#include "image.hpp"
#include <map>
#include <string>
#include <format>
#include <iostream>
#include "utils.hpp"
#include "stb_image.h"
#include "stb_image_write.h"


static std::map<std::string, Image> image_cache;

Image::~Image() {
    std::cerr << std::format("{}: Image::~Image()", (uint64_t)this) << std::endl;
    if(!buffer) return;
    if(from_file) {
        stbi_image_free(buffer);
    } else {
        free(buffer);
    }
    buffer = nullptr;


}

//unsigned char* data = stbi_load("test.jpg", &width, &height, &nrChannels, 0);

void save_image(const Image& image, const std::string& path) {
    auto [w, h, c] = image.shape();
    if(path.ends_with(".png")) {
        stbi_write_png(path.c_str(), w, h, c, image.data(), 0);
    }else if(path.ends_with(".jpg") || path.ends_with(".jpeg")) {
        stbi_write_jpg(path.c_str(), w, h, c, image.data(), 0);
    }else if(path.ends_with(".bmp")) {
        stbi_write_bmp(path.c_str(), w, h, c, image.data());
    }else{
        throw simple_exception(std::format("unexpected image format from path `{}`."
            "Supported formats: png, jpg and bmp.", path));
    }
}

Image& Image::operator=(const Image& other) {
    free(buffer);
    width = other.width;
    height = other.height;
    buffer = (unsigned char*)malloc(n_channels * width * height);
    memcpy(buffer, other.buffer, n_channels * width * height);
    return *this;
}

Image::Image(const std::string& filename, bool use_cache, bool save_to_cache) {
    if (use_cache && image_cache.count(filename)) {
        *this = image_cache[filename]; 
        return;
    }

    buffer = stbi_load(filename.c_str(), &width, &height, &n_channels, 0);
    from_file = true;

    if (save_to_cache) {
        image_cache[filename] = *this;
    }

    std::cerr << std::format("{}: Image::Image({}, {}, {})", (uint64_t)this, filename, use_cache, save_to_cache) << std::endl;
}

void Image::reset_cache() {
    image_cache.clear();
}

RGBAColor operator*(float k, const RGBAColor& c) {
    return c * k;
}