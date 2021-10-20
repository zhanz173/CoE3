#include "IndexBuffer.h"
#include "Renderer.h"


namespace Renderer {
	IndexBuffer::IndexBuffer(const unsigned int* data, unsigned int count)
		:m_count(count)
	{
		GlCall(glGenBuffers(1, &m_Renderer_id));
		GlCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Renderer_id));
		GlCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), data, GL_STATIC_DRAW));
	}

	IndexBuffer::~IndexBuffer()
	{
		GlCall(glDeleteBuffers(1, &m_Renderer_id));
	}

	void IndexBuffer::bind() const
	{
		GlCall(glBindBuffer(GL_ARRAY_BUFFER, m_Renderer_id));
	}

	void IndexBuffer::unbind() const
	{
		GlCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	int IndexBuffer::getElement() const
	{
		return m_count;
	}
}