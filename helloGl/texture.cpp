#include "texture.h"
#include "stb_image.h"
#include <assert.h>
#include <iostream>

namespace Renderer {
	Texture::Texture(const std::string& filepath)
		:m_Channels(0), m_Width(0), m_Height(0), m_buffer(nullptr)
	{
		stbi_set_flip_vertically_on_load(true);
		m_buffer = stbi_load(filepath.c_str(), &m_Width, &m_Height, &m_Channels, 4);

		GlCall(glGenTextures(1, &m_textureID));
		GlCall(glBindTexture(GL_TEXTURE_2D, m_textureID));

		GlCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GlCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		GlCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT));
		GlCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT));

		if (m_buffer) {
			GlCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_buffer));
			GlCall(glGenerateMipmap(GL_TEXTURE_2D));
			stbi_image_free(m_buffer);
		}
		else {
			std::cout << "texture file: " << filepath << " not found\n";
			stbi_image_free(m_buffer);
			assert(false);
		}

		GlCall(glBindTexture(GL_TEXTURE_2D, 0));
	}
	Texture::Texture(const std::vector<std::string>& faces)
		:m_Channels(0), m_Width(0), m_Height(0), m_buffer(nullptr)
	{
		stbi_set_flip_vertically_on_load(true);

		GlCall(glGenTextures(1, &m_textureID));
		GlCall(glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID));

		for (unsigned int i = 0; i < faces.size(); i++)
		{
			m_buffer = stbi_load(faces[i].c_str(), &m_Width, &m_Height, &m_Channels, 0);
			if (m_buffer)
			{
				GlCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB8, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_buffer));
				stbi_image_free(m_buffer);
			}
			else
			{
				std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
				stbi_image_free(m_buffer);
				assert(false);
			}
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		GlCall(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
	}

	Texture::~Texture()
	{
		GlCall(glDeleteTextures(1, &m_textureID));
	}

	void Texture::bind(unsigned int slot) const
	{
		GlCall(glActiveTexture(GL_TEXTURE0 + slot));
		GlCall(glBindTexture(GL_TEXTURE_2D, m_textureID));
	}

	void Texture::unbind() const
	{
		GlCall(glBindTexture(GL_TEXTURE_2D, 0));
	}
}