#pragma once
#include <vector>;
#include <GLEW/glew.h>

namespace Renderer {
	struct VertexBufferElement {
		unsigned int type;
		unsigned int count;
		unsigned char normalized;
	};

	class VertexBufferLayout {
	private:
		std::vector<VertexBufferElement> m_elements;
		unsigned int m_stride;

	public:
		VertexBufferLayout() : m_stride(0) {};

		template<typename T>
		void push(unsigned int count) {
			static_assert(false);
		}

		template<typename T>
		void push_empty(unsigned int count) {
			static_assert(false);
		}

		template<>
		void push<float>(unsigned int count) {
			m_elements.push_back({ GL_FLOAT , count, GL_FALSE });
			m_stride += count * sizeof(GL_FLOAT);
		}

		template<>
		void push_empty<float>(unsigned int count) {
			m_stride += count * sizeof(GL_FLOAT);
		}

		template<>
		void push<unsigned int>(unsigned int count) {
			m_elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
			m_stride += count * sizeof(GL_UNSIGNED_INT);
		}

		template<>
		void push<unsigned char>(unsigned int count) {
			m_elements.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
			m_stride += count * sizeof(GL_UNSIGNED_BYTE);
		}

		const std::vector<VertexBufferElement>& GetElement() const {
			return m_elements;
		}

		unsigned int Getstride() const {
			return m_stride;
		}
	};
}