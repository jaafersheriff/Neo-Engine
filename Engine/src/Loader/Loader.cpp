#include "Loader.hpp"

#include "MeshGenerator.hpp"

#include "GLObjects/Mesh.hpp"
#include "GLObjects/Texture.hpp"
#include "GLObjects/GLHelper.hpp"
#include "GLObjects/Framebuffer.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "ext/tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "ext/stb_image.h"

#include <iostream>

namespace neo {

    bool Loader::mVerbose = false;
    std::string Loader::RES_DIR = "";

    void Loader::init(const std::string &res, bool v) {
        RES_DIR = res;
        mVerbose = v;
    }

    Mesh* Loader::loadMesh(const std::string &fileName, bool doResize) {
        MICROPROFILE_SCOPEI("Loader", "loadMesh", MP_AUTO);

        /* Create mesh */
        Mesh* mesh = new Mesh;

        /* Check with static meshes first */
        if (!std::strcmp(fileName.c_str(), "cube")) {
            MeshGenerator::generateCube(mesh);
            mesh->mPrimitiveType = GL_TRIANGLES;
            return mesh;
        }
        if (!std::strcmp(fileName.c_str(), "quad") || !std::strcmp(fileName.c_str(), "plane")) {
            MeshGenerator::generateQuad(mesh);
            mesh->mPrimitiveType = GL_TRIANGLES;
            return mesh;
        }
        if (!std::strcmp(fileName.c_str(), "sphere") || !std::strcmp(fileName.c_str(), "ico_2")) {
            MeshGenerator::generateSphere(mesh, 2);
            mesh->mPrimitiveType = GL_TRIANGLES;
            return mesh;
        }
        if (!std::strncmp(fileName.c_str(), "ico_", 4)) {
            int recursions = std::stoi(fileName.c_str() + 4);
            MeshGenerator::generateSphere(mesh, recursions);
            mesh->mPrimitiveType = GL_TRIANGLES;
            return mesh;
        }

        /* If mesh was not found in map, read it in */
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> objMaterials;
        std::string errString;
        // TODO : use assimp or another optimized asset loader
        bool rc = tinyobj::LoadObj(shapes, objMaterials, errString, (RES_DIR + fileName).c_str());
        if (!rc) {
            std::cerr << errString << std::endl;
            std::cin.get();
            exit(1);
        }

        /* Create empty mesh buffers */
        std::vector<float> vertices;
        std::vector<float> normals;
        std::vector<float> texCoords;
        std::vector<unsigned> indices;

        int vertCount = 0;
        /* For every shape in the loaded file */
        for (unsigned int i = 0; i < shapes.size(); i++) {
            /* Concatenate the shape's vertices, normals, and textures to the mesh */
            vertices.insert(vertices.end(), shapes[i].mesh.positions.begin(), shapes[i].mesh.positions.end());
            normals.insert(normals.end(), shapes[i].mesh.normals.begin(), shapes[i].mesh.normals.end());
            texCoords.insert(texCoords.end(), shapes[i].mesh.texcoords.begin(), shapes[i].mesh.texcoords.end());

            /* Concatenate the shape's indices to the new mesh
             * Indices need to be incremented as we concatenate shapes */
            for (unsigned int i : shapes[i].mesh.indices) {
                indices.push_back(i + vertCount);
            }
            vertCount += int(shapes[i].mesh.positions.size()) / 3;
        }



        /* Optional resize and find min/max */
        _resize(mesh, vertices, doResize);

        /* Upload */
        mesh->mPrimitiveType = GL_TRIANGLE_STRIP;
        if (vertices.size()) {
            mesh->addVertexBuffer(VertexType::Position, 0, 3, vertices);
        }
        if (normals.size()) {
            mesh->addVertexBuffer(VertexType::Normal, 1, 3, normals);
        }
        if (texCoords.size()) {
            mesh->addVertexBuffer(VertexType::Texture0, 2, 2, texCoords);
        }
        if (indices.size()) {
            mesh->mPrimitiveType = GL_TRIANGLES;
            mesh->addElementBuffer(indices);
        }

        if (mVerbose) {
            std::cout << "Loaded mesh (" << vertCount << " vertices): " << fileName << std::endl;
        }

        return mesh;
    }

    Texture2D* Loader::loadTexture(const std::string &fileName, TextureFormat format) {
        MICROPROFILE_SCOPEI("Loader", "loadTexture", MP_AUTO);
        /* Create an empty texture if it is not already exist in the library */
        int width, height, components;
        uint8_t* data = _loadTextureData(width, height, components, fileName);

        Texture2D* texture = new Texture2D(format, glm::uvec2(width, height), data);
        texture->generateMipMaps();

        _cleanTextureData(data);

        return texture;

    }

    TextureCubeMap* Loader::loadTexture(const std::string &name, const std::vector<std::string>& files) {
        MICROPROFILE_SCOPEI("Loader", "loadCubemap", MP_AUTO);

        NEO_ASSERT(files.size() == 6, "Attempting to create cube map without 6 files");

        std::vector<uint8_t*> data;
        std::vector<glm::uvec2> sizes;
        for (int i = 0; i < 6; i++) {
            int _width, _height, _components;
            data.push_back(_loadTextureData(_width, _height, _components, files[i], false));
            sizes.push_back(glm::uvec2(_width, _height));
        }

        /* Upload data to GPU and free from CPU */
        TextureFormat format = { GL_RGBA, GL_RGBA, GL_LINEAR, GL_CLAMP_TO_EDGE };
        TextureCubeMap* texture = new TextureCubeMap(format, sizes, data.data());

        /* Clean */
        for (int i = 0; i < 6; i++) {
            _cleanTextureData(data[i]);
        }

        return texture;
    }

    uint8_t* Loader::_loadTextureData(int& width, int& height, int& components, const std::string& fileName, bool flip) {
        /* Use stbi if name is an existing file */
        FILE *f;
        NEO_ASSERT(!fopen_s(&f, (RES_DIR + fileName).c_str(), "rb"), "Error opening texture file");

        stbi_set_flip_vertically_on_load(flip);
        uint8_t *data = stbi_load((RES_DIR + fileName).c_str(), &width, &height, &components, STBI_rgb_alpha);   // TODO - allow ability to specify number of components
        NEO_ASSERT(data, "Error reading texture file");

        if (mVerbose) {
            std::cout << "Loaded texture " << fileName << " [" << width << ", " << height << "]" << std::endl;
        }

        return data;
    }

    void Loader::_cleanTextureData(uint8_t* data) {
        stbi_image_free(data);
    }

    /* Provided function to resize a mesh so all vertex positions are [0, 1.f] */
    void Loader::_resize(Mesh* mesh, std::vector<float>& vertices, bool doResize) {
        float minX, minY, minZ;
        float maxX, maxY, maxZ;
        float scaleX, scaleY, scaleZ;
        float shiftX, shiftY, shiftZ;
        float epsilon = 0.001f;

        minX = minY = minZ = 1.1754E+38F;
        maxX = maxY = maxZ = -1.1754E+38F;

        //Go through all vertices to determine min and max of each dimension
        for (size_t v = 0; v < vertices.size() / 3; v++) {
            if (vertices[3 * v + 0] < minX) minX = vertices[3 * v + 0];
            if (vertices[3 * v + 0] > maxX) maxX = vertices[3 * v + 0];

            if (vertices[3 * v + 1] < minY) minY = vertices[3 * v + 1];
            if (vertices[3 * v + 1] > maxY) maxY = vertices[3 * v + 1];

            if (vertices[3 * v + 2] < minZ) minZ = vertices[3 * v + 2];
            if (vertices[3 * v + 2] > maxZ) maxZ = vertices[3 * v + 2];
        }

        mesh->mMin = glm::vec3(minX, minY, minZ);
        mesh->mMax = glm::vec3(maxX, maxY, maxZ);

        //From min and max compute necessary scale and shift for each dimension
        if (doResize) {
            float maxExtent, xExtent, yExtent, zExtent;
            xExtent = maxX - minX;
            yExtent = maxY - minY;
            zExtent = maxZ - minZ;
            if (xExtent >= yExtent && xExtent >= zExtent) {
                maxExtent = xExtent;
            }
            if (yExtent >= xExtent && yExtent >= zExtent) {
                maxExtent = yExtent;
            }
            if (zExtent >= xExtent && zExtent >= yExtent) {
                maxExtent = zExtent;
            }
            scaleX = 2.f / maxExtent;
            shiftX = minX + (xExtent / 2.f);
            scaleY = 2.f / maxExtent;
            shiftY = minY + (yExtent / 2.f);
            scaleZ = 2.f / maxExtent;
            shiftZ = minZ + (zExtent) / 2.f;

            //Go through all verticies shift and scale them
            for (size_t v = 0; v < vertices.size() / 3; v++) {
                vertices[3 * v + 0] = (vertices[3 * v + 0] - shiftX) * scaleX;
                assert(vertices[3 * v + 0] >= -1.0 - epsilon);
                assert(vertices[3 * v + 0] <= 1.0 + epsilon);
                vertices[3 * v + 1] = (vertices[3 * v + 1] - shiftY) * scaleY;
                assert(vertices[3 * v + 1] >= -1.0 - epsilon);
                assert(vertices[3 * v + 1] <= 1.0 + epsilon);
                vertices[3 * v + 2] = (vertices[3 * v + 2] - shiftZ) * scaleZ;
                assert(vertices[3 * v + 2] >= -1.0 - epsilon);
                assert(vertices[3 * v + 2] <= 1.0 + epsilon);
            }
        }
    }
}