#pragma once

namespace Renderer {
	class VertexBuffer {
		unsigned int m_Renderer_id;

	public:
		VertexBuffer(const void* data, unsigned int size);
		~VertexBuffer();

		void bind() const;
		void unbind() const;
	};
}