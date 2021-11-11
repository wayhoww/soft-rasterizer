#pragma once

#include <GL/glew.h>    // include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <stdio.h>
#include <string>
#include <sstream>



class ImageDisplay {
    int width;
    int height;
    GLuint readFboId;
    GLFWwindow* window;
public:
    ImageDisplay(int width, int height): width(width), height(height) {
        if (!glfwInit()) {
            fprintf(stderr, "ERROR: could not start GLFW3\n");
            exit(-1);
        }

        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(width, height, "Hello Triangle", NULL, NULL);

        if (!window) {
            fprintf(stderr, "ERROR: could not open window with GLFW3\n");
            glfwTerminate();
            exit(-1);
        }
        glfwMakeContextCurrent(window);

        glewInit();

        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glGenFramebuffers(1, &readFboId);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, readFboId);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, texture, 0);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    }

    void show(const Image& image) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image.data());
        glGenerateMipmap(GL_TEXTURE_2D);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, readFboId);
        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

        glfwSwapBuffers(window);
    }
};

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