#pragma once

#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include <memory>

namespace Renderer {
	class VertexArray {
	private:
		unsigned int m_RendererID;
	public:
		VertexArray();
		~VertexArray();

		void AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);
		void AddBuffer(std::unique_ptr<VertexBuffer>& vb, const VertexBufferLayout& layout);
		void bind() const;
		void unbind() const;
	};
}