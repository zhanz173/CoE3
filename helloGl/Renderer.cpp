#pragma once

#include "Macro.h"
#include "Renderer.h"
#include <iostream>


void GlClearError()
{
    while (glGetError() != GL_NO_ERROR);
}
bool GlLogCall(const char* function, const char* file, int line)
{
    while (GLenum error = glGetError()) {
        std::cout << "OpenGl errors: " << error << "  " << function << "  "
            << file << ":" << line << std::endl;
        return false;
    }
    return true;
}

namespace Renderer {
    void renderer::clear() const
    {
        glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    }

    void renderer::Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const
    {
        shader.bind();
        va.bind();
        ib.bind();

        GlCall(glDrawElements(GL_TRIANGLES, ib.getElement(), GL_UNSIGNED_INT, (void*)0));
    }
}