#ifndef SHADER_HPP
#define SHADER_HPP

#include <glad/glad.h>

#include <string>
#include <iostream>
#include <string>

class Shader {
  public:
    unsigned int ID;

    // constructor reads and builds the shader
    Shader(const char* vertex_shader_path, const char* fragment_shader_path) {
        // reading shaders
        // // vertex
        std::string vertex_shader_text;
        FILE* vertex_shader_file = fopen(vertex_shader_path, "r");
        if(vertex_shader_file == NULL) {
            std::cerr << "ERROR: reading vertex shader file failed" << std::endl;
            exit(-1);
        }
        while(!feof(vertex_shader_file)) {
            vertex_shader_text += fgetc(vertex_shader_file);
        }
        fclose(vertex_shader_file);
        const char* vertex_shader_src = vertex_shader_text.c_str();

        // // fragment
        std::string fragment_shader_text;
        FILE* fragment_shader_file = fopen(fragment_shader_path, "r");
        if(fragment_shader_file == NULL) {
            std::cerr << "ERROR: reading fragment shader file failed" << std::endl;
            exit(-1);
        }
        while(!feof(fragment_shader_file)) { // TODO: fix off by one error when EOF is at the end of *_shader_text
            fragment_shader_text += fgetc(fragment_shader_file);
        }
        fclose(fragment_shader_file);

        const char* fragment_shader_src = fragment_shader_text.c_str();

        // comiplation
        unsigned int vertex, fragment;
        int success;
        char info_log[512];

        // // vertex
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vertex_shader_src, NULL);
        glCompileShader(vertex);
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if(!success) {
            glGetShaderInfoLog(vertex, 512, NULL, info_log);
            std::cerr << "ERROR: vertex shader compilation failed\n" << info_log << std::endl;
            exit(-1);
        };

        // // fragment
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fragment_shader_src, NULL);
        glCompileShader(fragment);
        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if(!success) {
            glGetShaderInfoLog(fragment, 512, NULL, info_log);
            std::cerr << "ERROR: fragment shader compilation failed\n" << info_log << std::endl;
            exit(-1);
        }

        // // program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if(!success){
            glGetProgramInfoLog(ID, 512, NULL, info_log);
            std::cerr << "ERROR: shader program compilation failed\n" << info_log << std::endl;
            exit(-1);
        }

        // delete the linked shaders
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    ~Shader() {
        glDeleteProgram(ID);
    }

    void use() {
        glUseProgram(ID);
    }
    // utility uniform functions
    void setBool(const std::string &name, bool value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    void setInt(const std::string &name, int value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setFloat(const std::string &name, float value) const {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setVec2(const std::string &name, float x, float y) const {
        glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
    }
    void setDVec2(const std::string &name, float x, float y) const {
        glUniform2d(glGetUniformLocation(ID, name.c_str()), x, y);
    }
    void setVec3(const std::string &name, float x, float y, float z) const {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
    }
    void setVec4(const std::string &name, float x, float y, float z, float w) const {
        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
    }


};


// TODO: maybe use cpp file streams

#endif
