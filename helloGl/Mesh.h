#pragma once
#include "glm/glm.hpp"
#include <string>
#include <vector>
#include <memory>

#include "Shader.h"
#include"VertexBuffer.h"
#include"IndexBuffer.h"
#include"VertexArray.h"
#include"VertexBufferLayout.h"

namespace ASP {
    enum struct TEX {
        texture_diffuse = 1,
        texture_specular = 2,
        texture_normal = 3,
        texture_height = 4
    };

    struct Vertex {
        // position
        glm::vec3 Position;
        // normal
        glm::vec3 Normal;
        // texCoords
        glm::vec2 TexCoords;
    };

    struct Texture {
        unsigned int id;
        TEX type;
        std::string path;
    };

    class Mesh {
    public:
        // mesh data
        std::vector<Vertex>       vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture>      textures;

        Mesh(std::vector<Vertex>& vertices,std::vector<unsigned int>& indices, std::vector<Texture>& textures)
        {
            this->vertices = std::move(vertices);
            this->indices = std::move(indices);
            this->textures = std::move(textures);

            // Now that we have all the required data, set the vertex buffers and its attribute pointers.
            this->setupMesh();
        }
        void Draw(Renderer::Shader& shader)
        {
            unsigned int diffuseNr = 1;
            unsigned int specularNr = 1;
            unsigned int normalNr = 1;
            unsigned int heightNr = 1;

            for (unsigned int i = 0; i < textures.size(); i++)
            {
                glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
                // retrieve texture number (the N in diffuse_textureN)
                std::string number;

                if (textures[i].type == TEX::texture_diffuse) {
                    number = std::to_string(diffuseNr++);
                    shader.SetUniform1i(("texture_diffuse" + number).c_str(), i);
                    glBindTexture(GL_TEXTURE_2D, textures[i].id);
                }
                else if (textures[i].type == TEX::texture_specular) {
                    number = std::to_string(specularNr++); // transfer unsigned int to stream
                }
                else if (textures[i].type == TEX::texture_normal)
                    number = std::to_string(normalNr++); // transfer unsigned int to stream
                else if (textures[i].type == TEX::texture_height)
                    number = std::to_string(heightNr++); // transfer unsigned int to stream
            }
            glActiveTexture(GL_TEXTURE0);

            // draw mesh
            VAO->bind();
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        }

    private:
        // render data 
        std::unique_ptr<Renderer::VertexArray> VAO;
        std::unique_ptr<Renderer::VertexBuffer> VBO;
        std::unique_ptr<Renderer::IndexBuffer> EBO;

        // initializes all the buffer objects/arrays
        void setupMesh()
        {
            VAO = std::make_unique<Renderer::VertexArray>();
            VBO = std::make_unique<Renderer::VertexBuffer>(&vertices[0], vertices.size() * sizeof(Vertex));
            EBO = std::make_unique<Renderer::IndexBuffer>(&indices[0], indices.size());

            Renderer::VertexBufferLayout layout;
            layout.push<float>(3);
            layout.push<float>(3);
            layout.push<float>(2);
            VAO->AddBuffer(VBO, layout);

            VAO->unbind();
        }
    };
}