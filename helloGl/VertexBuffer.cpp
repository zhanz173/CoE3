#include "VertexBuffer.h"
#include "Renderer.h"


namespace Renderer {
	VertexBuffer::VertexBuffer(const void* data, unsigned int size)
	{
		GlCall(glGenBuffers(1, &m_Renderer_id));
		GlCall(glBindBuffer(GL_ARRAY_BUFFER, m_Renderer_id));
		GlCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
	}

	VertexBuffer::~VertexBuffer()
	{
		GlCall(glDeleteBuffers(1, &m_Renderer_id));
	}

	void VertexBuffer::bind() const
	{
		GlCall(glBindBuffer(GL_ARRAY_BUFFER, m_Renderer_id));
	}

	void VertexBuffer::unbind() const
	{
		GlCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}
}
