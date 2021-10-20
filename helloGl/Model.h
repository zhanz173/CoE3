#pragma once
//referenced from learnopengl.com

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "vendor/stb_image.h"
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>

#include "Shader.h"
#include "Mesh.h"


namespace ASP {
    unsigned int TextureFromFile(const std::string& path, const std::string& directory, bool gamma = false);

    class Model
    {
    public:
        Model(const std::string& path)
        {
            loadModel(path);
        }

        void Draw(Renderer::Shader& shader)
        {
            for (unsigned int i = 0; i < meshes.size(); i++)
                meshes[i].Draw(shader);
        }
    private:
        // model data
        std::vector<Mesh> meshes;
        std::string directory;
        std::unordered_map<std::string, bool> textures_loaded;

        void loadModel(const std::string& path)
        {
            Assimp::Importer import;
            const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
            {
                std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
                return;
            }
            directory = path.substr(0, path.find_last_of('/'));
            meshes.reserve(scene->mNumMeshes);

            processNode(scene->mRootNode, scene);
        }

        void processNode(aiNode* node, const aiScene* scene)
        {
            // process all the node's meshes (if any)
            for (unsigned int i = 0; i < node->mNumMeshes; i++)
            {
                aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
                meshes.push_back(processMesh(mesh, scene));
            }
            // then do the same for each of its children
            for (unsigned int i = 0; i < node->mNumChildren; i++)
            {
                processNode(node->mChildren[i], scene);
            }
        }

        Mesh processMesh(aiMesh* mesh, const aiScene* scene)
        {
            std::vector<ASP::Vertex> vertices;
            std::vector<unsigned int> indices;
            std::vector<ASP::Texture> textures;

            vertices.reserve(mesh->mNumVertices);
            indices.reserve(mesh->mNumFaces * 3);       //!! hard coded triangles only 

            for (unsigned int i = 0; i < mesh->mNumVertices; i++)
            {
                Vertex vertex;
                glm::vec3 vector; 

                vector.x = mesh->mVertices[i].x;
                vector.y = mesh->mVertices[i].y;
                vector.z = mesh->mVertices[i].z;
                vertex.Position = vector;

                // normals
                if (mesh->HasNormals())
                {
                    vector.x = mesh->mNormals[i].x;
                    vector.y = mesh->mNormals[i].y;
                    vector.z = mesh->mNormals[i].z;
                    vertex.Normal = vector;
                }

                //ONLY THE FIRST TEXTURE SLOT IS RETRIVIED
                if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
                {
                    glm::vec2 TextureCoords;
                    TextureCoords.x = mesh->mTextureCoords[0][i].x;
                    TextureCoords.y = mesh->mTextureCoords[0][i].y;
                    vertex.TexCoords = TextureCoords;
                }
                else
                    vertex.TexCoords = glm::vec2(0.0f, 0.0f);

                vertices.push_back(vertex);
            }
            // process indices
            for (unsigned int i = 0; i < mesh->mNumFaces; i++)
            {
                aiFace face = mesh->mFaces[i];
                for (unsigned int j = 0; j < face.mNumIndices; j++)
                    indices.push_back(face.mIndices[j]);
            }

            // process material
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            // 1. diffuse maps
            std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, TEX::texture_diffuse);
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            // 2. specular maps
            std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, TEX::texture_specular);
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
            // 3. normal maps
            std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, TEX::texture_normal);
            textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
            // 4. height maps
            std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, TEX::texture_height);
            textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

            return Mesh(vertices, indices, textures);
        }

        std::vector<ASP::Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, TEX typeName)
        {
            std::vector<ASP::Texture> textures;
            for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
            {
                aiString str;
                mat->GetTexture(type, i, &str);
                // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
                std::string name(str.C_Str());
                bool skip = false;

                if (textures_loaded.find(name) == textures_loaded.end())
                {   // if texture hasn't been loaded already, load it
                    Texture texture;
                    texture.id = TextureFromFile(name, this->directory);
                    texture.type = typeName;
                    texture.path = name;
                    textures.push_back(texture);
                    textures_loaded[name] = true;  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
                }
            }
            return textures;
        }       
    };
        unsigned int TextureFromFile(const std::string& path, const std::string& directory, bool gamma)
        {
            std::string filename;
            filename = directory + '/' + path;

            unsigned int textureID;
            glGenTextures(1, &textureID);

            int width, height, nrComponents;
            stbi_set_flip_vertically_on_load(true);
            unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
            if (data)
            {
                GLenum format;
                if (nrComponents == 1)
                    format = GL_RED;
                else if (nrComponents == 3)
                    format = GL_RGB;
                else if (nrComponents == 4)
                    format = GL_RGBA;

                glBindTexture(GL_TEXTURE_2D, textureID);
                glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                stbi_image_free(data);
            }
            else
            {
                std::cout << "Texture failed to load at path: " << path << std::endl;
                stbi_image_free(data);
            }
            return textureID;
        }
}