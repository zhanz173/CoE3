#include "Shader.h"
#include "Renderer.h"
#include"VertexBuffer.h"
#include"IndexBuffer.h"
#include"VertexArray.h"
#include"VertexBufferLayout.h"

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

namespace Renderer {
    Shader::Shader(const std::string& filepath)
    {
        ShaderSource source = ParseShader(filepath);
        m_RendererID = CreateShader(source.Vertex, source.Fragment);
        GlCall(glUseProgram(m_RendererID));
    }

    Shader::Shader(const std::string& VertexShader, const std::string& FragmentShader, const std::string& GeometryShader)
    {
        std::string VertexShader_source = ParseSingleShader(VertexShader);
        std::string FragmentShader_source = ParseSingleShader(FragmentShader);

        if (!GeometryShader.empty()) {
            std::string GeometryShader_source = ParseSingleShader(GeometryShader);
            m_RendererID = CreateShader(VertexShader_source, FragmentShader_source, GeometryShader_source);
        }
        else {
            m_RendererID = CreateShader(VertexShader_source, FragmentShader_source);
        }
        GlCall(glUseProgram(m_RendererID));
    }

    Shader::Shader(Shader& newshader)
    {
        unsigned int temp = m_RendererID;
        this->m_RendererID = newshader.m_RendererID;
        newshader.m_RendererID = temp;
        this->uniform_location.clear();
    }

    Shader& Shader::operator=(Shader& newshader)
    {
        unsigned int temp = m_RendererID;
        this->m_RendererID = newshader.m_RendererID;
        newshader.m_RendererID = temp;
        this->uniform_location.clear();
        return *this;
    }

    Shader::~Shader()
    {
        GlCall(glDeleteProgram(m_RendererID));
    }

    void Shader::bind() const
    {
        GlCall(glUseProgram(m_RendererID));
    }

    void Shader::unbind() const
    {
        GlCall(glUseProgram(0));
    }

    unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
    {
        unsigned int id = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(id, 1, &src, nullptr);
        GlCall(glCompileShader(id));

        int result;
        glGetShaderiv(id, GL_COMPILE_STATUS, &result);
        if (!result) {
            int length;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
            char* message = (char*)alloca(sizeof(char) * length);
            glGetShaderInfoLog(id, length, &length, message);
            glDeleteShader(id);
            std::cout << message << "\n";
            ASSERT(false);
            return 0;
        }


        return id;
    }

    unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader, const std::string& GeometryShader)
    {
        unsigned int program = glCreateProgram();
        unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
        unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);
        unsigned int gs;

        glAttachShader(program, vs);
        glAttachShader(program, fs);
        if (!GeometryShader.empty()) {
            gs = CompileShader(GL_GEOMETRY_SHADER, GeometryShader);
            glAttachShader(program, gs);
        }

        glLinkProgram(program);
        glValidateProgram(program);

        glDeleteShader(vs);
        glDeleteShader(fs);
        if (!GeometryShader.empty())  glDeleteShader(gs);

        return program;
    }

    std::string Shader::ParseSingleShader(const std::string& filepath) {
        std::ifstream stream(filepath);
        if (!stream) {
            std::cout << "Shader source not found!\n";
            ASSERT(false);
        }

        std::stringstream shader;
        shader << stream.rdbuf();
        stream.close();

        return shader.str();
    }

    ShaderSource Shader::ParseShader(const std::string& filepath)
    {
        std::ifstream stream(filepath);
        if (!stream) {
            std::cout << "Shader source not found!\n";
            ASSERT(false);
        }

        enum class ShaderType
        {
            NONE = -1, VERTEX = 0, FRAGMENT = 1
        };
        ShaderType type = ShaderType::NONE;

        std::string line;
        std::stringstream ss[2];

        while (getline(stream, line)) {
            if (line.find("#shader") != std::string::npos) {
                if (line.find("vertex") != std::string::npos)
                    type = ShaderType::VERTEX;
                else if (line.find("fragment") != std::string::npos)
                    type = ShaderType::FRAGMENT;
            }
            else if (type != ShaderType::NONE) {
                ss[(int)type] << line << '\n';
            }
            else {
                std::cout << "shader not found\n";
            }
        }
        stream.close();
        return { ss[0].str() ,ss[1].str() };
    }


    int Shader::GetUniformLocation(const std::string& name)
    {
        if (uniform_location.find(name) != uniform_location.end())
            return uniform_location[name];

        GlCall(int location = glGetUniformLocation(m_RendererID, name.c_str()));
        if (location == -1) std::cout << "Uniform " << name << " doesn't exist!\n";
        else  uniform_location[name] = location;

        return location;
    }
}