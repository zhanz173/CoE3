#pragma once

namespace Renderer {
	class IndexBuffer {
		unsigned int m_Renderer_id;
		unsigned int m_count;

	public:
		IndexBuffer(const unsigned int* data, unsigned int count);
		~IndexBuffer();

		void bind() const;
		void unbind() const;
		int getElement() const;
	};
}