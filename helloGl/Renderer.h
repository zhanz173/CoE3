#pragma once
#include <GLEW/glew.h>
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"

namespace Renderer {
    class renderer {
    public:
        void clear() const;
        void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const;
    };
}