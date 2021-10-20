#include "Framebuffer.h"
#include <iostream>

namespace Renderer {
	Framebuffer::Framebuffer(int scr_width, int scr_height, BufferType type):
		m_width(scr_width), m_height(scr_height), m_type(type)
	{
		glGenFramebuffers(1, &m_FramebufferID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);

		if (type == Renderbuffer) {
			Create_Renderbuffer();
		}
		else {
			Create_Texturebuffer(type);
		}
		ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	Framebuffer::~Framebuffer()
	{
		if (m_type == Texturebuffer_color || m_type == Texturebuffer_depth) GlCall(glDeleteTextures(1, &m_textureID);)
		else GlCall(glDeleteRenderbuffers(1, &m_renderbufferID));
		GlCall(glDeleteFramebuffers(1, &m_FramebufferID));
	}
	void Framebuffer::bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);
	}
	void Framebuffer::unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	void Framebuffer::Create_Renderbuffer()
	{
		GlCall(glGenRenderbuffers(1, &m_renderbufferID));
		GlCall(glBindRenderbuffer(GL_RENDERBUFFER, m_renderbufferID));
		GlCall(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height));
		GlCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_renderbufferID));
	}
	void Framebuffer::Create_Texturebuffer(BufferType type)
	{
		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_2D, m_textureID);

		if (type == Texturebuffer_color) {
			GlCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL));
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureID, 0);
		}
		else if (type == Texturebuffer_depth) {
			GlCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_width, m_height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL));
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			float boardColor[] = { 1.0, 1.0, 1.0, 1.0 };
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, boardColor);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_textureID, 0);
		}
		else
		{
			std::cout << "ERROR: unknown texturebuffer type\n";
			ASSERT(false);
		}
		GlCall(glBindTexture(GL_TEXTURE_2D, 0));
	}

	void Framebuffer::bindTexture(unsigned int slot)
	{
		if (m_type == Renderbuffer) {
			std::cout << "ERROR: expected a texture type\n";
			return;
		}
		GlCall(glActiveTexture(GL_TEXTURE0 + slot));
		GlCall(glBindTexture(GL_TEXTURE_2D, m_textureID));
	}

	void Framebuffer::unbindTexture()
	{
		if (m_type == Renderbuffer) {
			std::cout << "ERROR: expected a texture type\n";
			return;
		}
		GlCall(glBindTexture(GL_TEXTURE_2D, 0));
	}
}