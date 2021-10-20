#include "VertexArray.h"
#include "Renderer.h"


namespace Renderer {
	VertexArray::VertexArray()
	{
		GlCall(glGenVertexArrays(1, &m_RendererID));
		GlCall(glBindVertexArray(m_RendererID));
	}

	VertexArray::~VertexArray()
	{
		GlCall(glDeleteVertexArrays(1, &m_RendererID));
	}

	void VertexArray::AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout)
	{
		vb.bind();
		const auto& elements = layout.GetElement();
		unsigned int offset = 0;

		for (unsigned int i = 0; i < elements.size(); i++)
		{
			const auto& element = elements[i];
			GlCall(glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.Getstride(), (const void*)offset));
			GlCall(glEnableVertexAttribArray(i));
			offset += element.count * sizeof(element.type);
		}
	}

	void VertexArray::AddBuffer(std::unique_ptr<VertexBuffer>& vb, const VertexBufferLayout& layout)
	{
		vb->bind();
		const auto& elements = layout.GetElement();
		unsigned int offset = 0;

		for (unsigned int i = 0; i < elements.size(); i++)
		{
			const auto& element = elements[i];
			GlCall(glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.Getstride(), (const void*)offset));
			GlCall(glEnableVertexAttribArray(i));
			offset += element.count * sizeof(element.type);
		}
	}

	void VertexArray::bind() const
	{
		GlCall(glBindVertexArray(m_RendererID));
	}

	void VertexArray::unbind() const
	{
		GlCall(glBindVertexArray(0));
	}
}
