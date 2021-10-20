#pragma once
#include "Renderer.h"

namespace Renderer {
	enum BufferType {
		Texturebuffer_color = 1,
		Texturebuffer_depth = 2,
		Renderbuffer = 3
	};

	class Framebuffer {
	public:
		unsigned int m_FramebufferID;
		int m_width, m_height;
		BufferType m_type;
		union{
			unsigned int m_textureID;
			unsigned int m_renderbufferID;
		};
		
		Framebuffer(int scr_width, int scr_height, BufferType type);
		~Framebuffer();
		void bind();
		void unbind();
		void bindTexture(unsigned int slot = 0);
		void unbindTexture();
	private:
		void Create_Renderbuffer();
		void Create_Texturebuffer(BufferType type);
	};
}