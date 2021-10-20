#pragma once
#include "Renderer.h"

namespace Renderer {
	enum class TEXTURE_TYPE {
		texture_diffuse = 1,
		texture_specular,
		texture_normal,
		texture_height
	};

	class Texture {
	public:
		unsigned int m_textureID;
		unsigned char* m_buffer;
		int m_Width, m_Height, m_Channels;

	public:
		Texture(const std::string& filepath);
		Texture(const std::vector<std::string>& faces);
		~Texture();

		void bind(unsigned int slot = 0) const;
		void unbind() const;
	};
}