#pragma once


#include "common_header.hpp"
#include <concepts>
#include <string>
#include <cstring>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <map>
#include <string>
#include <format>
#include <tuple>
#include <iostream>
#include "utils.hpp"

// 颜色都按照 RGBA 指定
// 图片有 RGB 和 RGBA
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
    int n_channels = 3;

	unsigned char* buffer = nullptr;
    bool from_file = false;
public:

	Image() : width(0), height(0), n_channels(3), buffer(nullptr) {
		std::cerr << std::format("{}: Image::Image()", (uint64_t)this) << std::endl;
	}

	Image(int width, int height, int n_channels = 3, bool init = true): 
		width(width), 
		height(height), 
        n_channels(n_channels),
		buffer((unsigned char*) malloc(n_channels * width * height)) {
		
        if(n_channels != 3 && n_channels != 4) {
            throw simple_exception(
                std::format(
                    "error: unexpected `n_channels` = {}. It is expected to be 3 or 4.", 
                    n_channels
                )
            );
        }

		if (init) {
			memset(buffer, 0, n_channels * width * height); 
		}
		
		std::cerr << std::format("{}: Image::Image({}, {}, {}, {})", (uint64_t)this, width, height, n_channels, init) << std::endl;
	}

	~Image();

	Image(const Image& other) {
		width = other.width;
		height = other.height;
		buffer = (unsigned char*) malloc(n_channels * width * height);
		memcpy(buffer, other.buffer, n_channels * width * height);


		std::cerr << std::format("{}: Image::Image({})", (uint64_t)this, (uint64_t)&other) << std::endl;
	}

	Image(Image&& other) {
		width = other.width;
		height = other.height;
		buffer = other.buffer;
        this->from_file = other.from_file;
        other.from_file = false;
		other.buffer = nullptr;
		std::cerr << std::format("{}: Image::Image(&&{})", (uint64_t)this, (uint64_t)&other) << std::endl;
	}

	Image& operator=(const Image& other);

	Image& operator=(Image&& other) {
		free(buffer);
		width = other.width;
		height = other.height;
		buffer = other.buffer;
        this->from_file = true;
        other.from_file = false;
		other.buffer = nullptr;
		return *this;
	}

	Image(const std::string& filename, bool use_cache = true, bool save_to_cache = true);

	// [{x, y}], 左下坐标系
	RGBAColor getPixel(int x, int y) const {
		int pixel_offset = ((height - 1 - y) * width + x) * n_channels;

		return RGBAColor{
			buffer[pixel_offset + 0] / 255.0f,
			buffer[pixel_offset + 1] / 255.0f,
			buffer[pixel_offset + 2] / 255.0f,
			n_channels == 3 ? 1.0f : buffer[pixel_offset + 3] / 255.0f
		};
	}

	Image& setPixel(int x, int y, const RGBAColor& color) {
		int pixel_offset = ((height - 1 - y) * width + x) * n_channels;
		buffer[pixel_offset + 0] = std::lround(color.r * 255.0f);
		buffer[pixel_offset + 1] = std::lround(color.g * 255.0f);
		buffer[pixel_offset + 2] = std::lround(color.b * 255.0f);
		if(n_channels == 4) buffer[pixel_offset + 4] = std::lround(color.a * 255.0f);

		return *this;
	}
    
	std::pair<int, int> size() const {
		return {width, height};
	}	
    
    std::tuple<int, int, int> shape() const {
		return {width, height, n_channels};
	}


    const unsigned char* data() const {
        return buffer;
    }

	static void reset_cache();
};



void save_image(const Image& image, const std::string& path);

//
//#include <GL/glew.h>    // include GLEW and new version of GL on Windows
//#include <GLFW/glfw3.h> // GLFW helper library
//#include <stdio.h>
//#include <string>
//#include <sstream>


//
//class ImageDisplay {
//    int width;
//    int height;
//    GLuint readFboId;
//    GLFWwindow* window;
//public:
//    ImageDisplay(int width, int height): width(width), height(height) {
//        if (!glfwInit()) {
//            fprintf(stderr, "ERROR: could not start GLFW3\n");
//            exit(-1);
//        }
//
//        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
//        window = glfwCreateWindow(width, height, "Hello Triangle", NULL, NULL);
//
//        if (!window) {
//            fprintf(stderr, "ERROR: could not open window with GLFW3\n");
//            glfwTerminate();
//            exit(-1);
//        }
//        glfwMakeContextCurrent(window);
//
//        glewInit();
//
//        unsigned int texture;
//        glGenTextures(1, &texture);
//        glBindTexture(GL_TEXTURE_2D, texture);
//
//        glGenFramebuffers(1, &readFboId);
//
//        glBindFramebuffer(GL_READ_FRAMEBUFFER, readFboId);
//        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
//            GL_TEXTURE_2D, texture, 0);
//        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
//    }
//
//    void show(const Image& image) {
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image.data());
//        glGenerateMipmap(GL_TEXTURE_2D);
//
//        glBindFramebuffer(GL_READ_FRAMEBUFFER, readFboId);
//        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
//        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
//
//
//        glfwSwapBuffers(window);
//    }
//};

// 返回值和参数的 std::move

//#include "stb_image.h"

//
//unsigned compile_shader(const std::string& source, unsigned type) {
//
//    auto vshader_source = source.c_str();
//
//    unsigned int shader;
//    shader = glCreateShader(type);
//    glShaderSource(shader, 1, &vshader_source, NULL);
//    glCompileShader(shader);
//
//    int  success;
//    char infoLog[512];
//    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
//
//    if (!success) {
//        std::stringstream ss;
//        glGetShaderInfoLog(shader, 512, NULL, infoLog);
//        ss << "Failed to compile: " << infoLog << std::endl;
//        throw simple_exception(ss.str());
//    }
//
//    return shader;
//}
//
//unsigned get_shader_program(const std::string& vshader_source, const std::string& fshader_source) {
//    auto vshader = compile_shader(vshader_source, GL_VERTEX_SHADER);
//    auto fshader = compile_shader(fshader_source, GL_FRAGMENT_SHADER);
//
//    unsigned int program;
//    program = glCreateProgram();
//    glAttachShader(program, vshader);
//    glAttachShader(program, fshader);
//    glLinkProgram(program);
//
//    //    glDeleteShader(vshader);
//    //    glDeleteShader(fshader);  
//
//    int  success;
//    char infoLog[512];
//    glGetProgramiv(program, GL_LINK_STATUS, &success);
//    if (!success) {
//        std::stringstream ss;
//        glGetProgramInfoLog(program, 512, NULL, infoLog);
//        ss << "Failed to link: " << infoLog << std::endl;
//        throw simple_exception(ss.str());
//    }
//
//    return program;
//}
//
//int main2() {
//
//    int width = 1200, height = 600, nrChannels;
//    // unsigned char* data = stbi_load("test.jpg", &width, &height, &nrChannels, 0);
//    auto data = new unsigned char[width * height * 3];
//    memset(data, 100, sizeof(data));
//
//    if (!glfwInit()) {
//        fprintf(stderr, "ERROR: could not start GLFW3\n");
//        return 1;
//    }
//
//    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
//    GLFWwindow* window = glfwCreateWindow(width, height, "Hello Triangle", NULL, NULL);
//
//    if (!window) {
//        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
//        glfwTerminate();
//        return 1;
//    }
//    glfwMakeContextCurrent(window);
//
//    glewInit();
//
//    // glEnable(GL_DEPTH_TEST);
//    // glDepthFunc(GL_LESS);
//
//
//    // glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 512, 512);
//    // glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 512, 512,
//    //            GL_RGBA, GL_UNSIGNED_BYTE, data);
//
//    std::string vshader_source = R"(
//        #version 330 core
//        layout (location = 0) in vec2 aPos;
//        layout (location = 1) in vec2 aTexCoord;
//        out vec2 texCoord;
//        void main() {
//            gl_Position = vec4(aPos.x, aPos.y, 0, 1.0); // z is in [-1, +1] as well
//            texCoord = aTexCoord;
//        }
//    )";
//
//    std::string fshader_source = R"(
//        #version 330 core
//        in vec2 texCoord;
//        out vec4 FragColor;
//        uniform sampler2D ourTexture;
//        void main() {
//            FragColor = texture(ourTexture, texCoord);
//        }
//    )";
//
//
//    float vertices_data[] = {
//        -1.0, +1.0, 0.0, 0.0,
//        +1.0, +1.0, 1.0, 0.0,
//        +1.0, -1.0, 1.0, 1.0,
//        -1.0, -1.0, 0.0, 1.0
//    };
//
//    int indices_data[] = {
//        2, 1, 0,
//        2, 3, 0
//    };
//
//    auto shader_program = get_shader_program(vshader_source, fshader_source);
//
//
//    unsigned int VAO;
//    glGenVertexArrays(1, &VAO);
//
//    // GL_ARRAY_BUFFER <-> VBO
//    GLuint VBO;
//    glGenBuffers(1, &VBO);
//    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_data), vertices_data, GL_STATIC_DRAW);
//
//    glBindVertexArray(VAO); // 这是 VAO 绑定的最晚时机。指定数据格式之前必须绑定 VAO
//    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(0 * sizeof(float)));
//    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
//    glBindBuffer(GL_ARRAY_BUFFER, 0); // 这是解绑 VBO 的最早时机。指定数据格式的时候要留一个到 VBO 的指针。
//
//    glEnableVertexAttribArray(0);
//    glEnableVertexAttribArray(1);
//
//    glBindVertexArray(0);           // 指定完数据和数据格式后，才能解绑 VAO
//
//    // OpenGL 奇怪之处在于：
//    // 1. 超级弱类型
//    // 2. 所有东西都必须绑定到某些 enum 上进行操作，不能直接对 ID 进行操作
//    // 3. 使用指定时绑定的数据指针，而非现在绑定的数据指针
//
//    // 而 EBO 是相对独立的。。learn opengl 里面说 VAO 会创建一个到 EBO 的指针，暂时没发现能用
//    GLuint EBO;
//    glGenBuffers(1, &EBO);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_data), indices_data, GL_STATIC_DRAW);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//
//    unsigned int texture;
//    glGenTextures(1, &texture);
//    glBindTexture(GL_TEXTURE_2D, texture);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//    // 解释方式：按行排序，先下后上
//    // stb 加载的图片：按行排序，从下到上
//    // 需要转一下uv坐标
//    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
//    // glGenerateMipmap(GL_TEXTURE_2D);
//
//    glTexStorage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height);
//    //stbi_image_free(data);
//
//    int x = 0; // width
//    int y = 0; // height
//
//    double lastTime = glfwGetTime();
//    char title[100];
//
//    GLuint readFboId = 0;
//    glGenFramebuffers(1, &readFboId);
//    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFboId);
//    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
//        GL_TEXTURE_2D, texture, 0);
//    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
//
//    while (!glfwWindowShouldClose(window)) {
//        //int index = 3 * ((y * width) + x);
//        //data[index] += 100;
//        memset(data, x + y, width * height * 3);
//
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
//        glGenerateMipmap(GL_TEXTURE_2D);
//
//        //glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
//        //glGenerateMipmap(GL_TEXTURE_2D);
//
//        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT);
//
//        /*
//        // Draw our first triangle
//        glUseProgram(shader_program);
//
//        glBindVertexArray(VAO);
//        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//        glBindTexture(GL_TEXTURE_2D, texture);
//
//        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
//
//        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//        glBindVertexArray(0);
//        */
//
//        glBindFramebuffer(GL_READ_FRAMEBUFFER, readFboId);
//        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
//        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
//
//
//        glfwSwapBuffers(window);
//        glfwPollEvents();
//
//        x++;
//        if (x == width) {
//            x = 0;
//            y = (y + 1) % height;
//        }
//
//        double thisTime = glfwGetTime();
//        double elapsed = thisTime - lastTime;
//        double fps = 1 / elapsed;
//        lastTime = thisTime;
//
//        sprintf(title, "%.1lf fps", fps);
//        glfwSetWindowTitle(window, title);
//    }
//
//    // close GL context and any other GLFW resources
//    glfwTerminate();
//    return 0;
//}