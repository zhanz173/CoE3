#pragma once
#include "Macro.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <GLEW/glew.h>
#include <string>
#include <unordered_map>

namespace Renderer {
	struct ShaderSource
	{
		std::string Vertex;
		std::string Fragment;
	};

	class Shader {
	public:
		unsigned int m_RendererID;
		std::unordered_map<std::string, int> uniform_location;
	public:
		Shader(const std::string& filepath);
		Shader(const std::string& VertexShader, const std::string& FragmentShader, const std::string& GeometryShader = std::string());
		Shader(Shader& newshader);
		Shader& operator= (Shader& newshader);
		~Shader();

		void bind() const;
		void unbind() const;

		void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
		void SetUniform4fv(const std::string& name, const glm::vec4& v);
		void SetUniform3f(const std::string& name, float v0, float v1, float v2);
		void SetUniform3fv(const std::string& name, const glm::vec3& v);
		void SetUniform1f(const std::string& name, float f);
		void SetUniform1i(const std::string& name, int i);
		void SetUniformMat4f(const std::string& name, const glm::mat4& matrix);

	private:
		int GetUniformLocation(const std::string& name);
		unsigned int CompileShader(unsigned int type, const std::string& source);
		unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader, const std::string& GeometryShader = std::string());
		ShaderSource ParseShader(const std::string& filepath);
		std::string ParseSingleShader(const std::string& filepath);

	};


	inline void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
	{
		GlCall(glUniform4f(GetUniformLocation(name), v0, v1, v2, v3));
	}
	inline void Shader::SetUniform4fv(const std::string& name, const glm::vec4& v)
	{
		GlCall(glUniform4fv(GetUniformLocation(name), 1, glm::value_ptr(v)));
	}

	inline void Shader::SetUniform3f(const std::string& name, float v0, float v1, float v2)
	{
		GlCall(glUniform3f(GetUniformLocation(name), v0, v1, v2));
	}

	inline void Shader::SetUniform3fv(const std::string& name, const glm::vec3& v)
	{
		GlCall(glUniform3fv(GetUniformLocation(name), 1,glm::value_ptr(v)));
	}

	inline void Shader::SetUniform1f(const std::string& name, float f)
	{
		GlCall(glUniform1f(GetUniformLocation(name), f));
	}

	inline void Shader::SetUniform1i(const std::string& name, int i)
	{
		GlCall(glUniform1i(GetUniformLocation(name), i));
	}

	inline void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix)
	{
		GlCall(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0].x));
	}
}

