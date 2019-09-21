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
        /* Check with static meshes first */
        if (!std::strcmp(fileName.c_str(), "cube")) {
            return MeshGenerator::createCube();
        }
        if (!std::strcmp(fileName.c_str(), "quad") || !std::strcmp(fileName.c_str(), "plane")) {
            return MeshGenerator::createQuad();
        }
        if (!std::strcmp(fileName.c_str(), "sphere") || !std::strcmp(fileName.c_str(), "ico_2")) {
            return MeshGenerator::createSphere(2);
        }
        if (!std::strncmp(fileName.c_str(), "ico_", 4)) {
            int recursions = std::stoi(fileName.c_str() + 4);
            return MeshGenerator::createSphere(recursions);
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

        /* Create a new empty mesh */
        Mesh * mesh = new Mesh;

        int vertCount = 0;
        /* For every shape in the loaded file */
        for (unsigned int i = 0; i < shapes.size(); i++) {
            /* Concatenate the shape's vertices, normals, and textures to the mesh */
            mesh->mBuffers.vertices.insert(mesh->mBuffers.vertices.end(), shapes[i].mesh.positions.begin(), shapes[i].mesh.positions.end());
            mesh->mBuffers.normals.insert(mesh->mBuffers.normals.end(), shapes[i].mesh.normals.begin(), shapes[i].mesh.normals.end());
            mesh->mBuffers.texCoords.insert(mesh->mBuffers.texCoords.end(), shapes[i].mesh.texcoords.begin(), shapes[i].mesh.texcoords.end());

            /* Concatenate the shape's indices to the new mesh
             * Indices need to be incremented as we concatenate shapes */
            for (unsigned int i : shapes[i].mesh.indices) {
                mesh->mBuffers.indices.push_back(i + vertCount);
            }
            vertCount += int(shapes[i].mesh.positions.size()) / 3;
        }

        /* Provide VBO info */
        mesh->mVertexBufferSize = int(mesh->mBuffers.vertices.size());
        mesh->mNormalBufferSize = int(mesh->mBuffers.normals.size());
        mesh->mUVBufferSize = int(mesh->mBuffers.texCoords.size());
        mesh->mElementBufferSize = int(mesh->mBuffers.indices.size());

        /* Optional resize */
        if (doResize) {
            _resize(*mesh);
        }

        /* Load mesh to GPU */
        mesh->upload();

        if (mVerbose) {
            std::cout << "Loaded mesh (" << vertCount << " vertices): " << fileName << std::endl;
        }

        return mesh;
    }

    Texture2D* Loader::loadTexture(const std::string &fileName, TextureFormat format) {
        /* Create an empty texture if it is not already exist in the library */
        Texture2D* texture = new Texture2D;
        texture->mFormat = format;

        uint8_t* data = _loadSingleTexture(texture, fileName);

        texture->upload(true, &data);
        texture->generateMipMaps();

        _cleanSingleTexture(data);

        return texture;

    }

    TextureCubeMap* Loader::loadTexture(const std::string &name, const std::vector<std::string> & files) {

        /* Create an empty texture if it is not already exist in the library */
        TextureCubeMap* texture = new TextureCubeMap;
        TextureFormat format = { GL_RGBA, GL_RGBA, GL_LINEAR, GL_CLAMP_TO_EDGE };
        texture->mFormat = format;

        /* Use stbi if name is an existing file */
        uint8_t* data[6];
        for (int i = 0; i < 6; i++) {
            data[i] = _loadSingleTexture(texture, files[i], false);
        }

        /* Upload data to GPU and free from CPU */
        texture->upload(true, data);

        /* Clean */
        for (int i = 0; i < 6; i++) {
            _cleanSingleTexture(data[i]);
        }

        return texture;
    }

    uint8_t* Loader::_loadSingleTexture(Texture* texture, const std::string& fileName, bool flip) {
        /* Use stbi if name is an existing file */
        FILE *f;
        assert(!fopen_s(&f, (RES_DIR + fileName).c_str(), "rb"), "Error opening texture file");

        stbi_set_flip_vertically_on_load(flip);
        uint8_t *data = stbi_load((RES_DIR + fileName).c_str(), &texture->mWidth, &texture->mHeight, &texture->mComponents, STBI_rgb_alpha);   // TODO - allow ability to specify number of components
        assert(data, "Error reading texture file");

        if (mVerbose) {
            std::cout << "Loaded texture " << fileName << " [" << texture->mWidth << ", " << texture->mHeight << "]" << std::endl;
        }

        return data;
    }

    void Loader::_cleanSingleTexture(uint8_t* data) {
        stbi_image_free(data);
    }

    /* Provided function to resize a mesh so all vertex positions are [0, 1.f] */
    void Loader::_resize(Mesh& mesh) {
        Mesh::MeshBuffers& buffers = mesh.mBuffers;
        float minX, minY, minZ;
        float maxX, maxY, maxZ;
        float scaleX, scaleY, scaleZ;
        float shiftX, shiftY, shiftZ;
        float epsilon = 0.001f;

        minX = minY = minZ = 1.1754E+38F;
        maxX = maxY = maxZ = -1.1754E+38F;

        //Go through all vertices to determine min and max of each dimension
        for (size_t v = 0; v < buffers.vertices.size() / 3; v++) {
            if (buffers.vertices[3 * v + 0] < minX) minX = buffers.vertices[3 * v + 0];
            if (buffers.vertices[3 * v + 0] > maxX) maxX = buffers.vertices[3 * v + 0];

            if (buffers.vertices[3 * v + 1] < minY) minY = buffers.vertices[3 * v + 1];
            if (buffers.vertices[3 * v + 1] > maxY) maxY = buffers.vertices[3 * v + 1];

            if (buffers.vertices[3 * v + 2] < minZ) minZ = buffers.vertices[3 * v + 2];
            if (buffers.vertices[3 * v + 2] > maxZ) maxZ = buffers.vertices[3 * v + 2];
        }

        //From min and max compute necessary scale and shift for each dimension
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
        for (size_t v = 0; v < buffers.vertices.size() / 3; v++) {
            buffers.vertices[3 * v + 0] = (buffers.vertices[3 * v + 0] - shiftX) * scaleX;
            assert(buffers.vertices[3 * v + 0] >= -1.0 - epsilon);
            assert(buffers.vertices[3 * v + 0] <= 1.0 + epsilon);
            buffers.vertices[3 * v + 1] = (buffers.vertices[3 * v + 1] - shiftY) * scaleY;
            assert(buffers.vertices[3 * v + 1] >= -1.0 - epsilon);
            assert(buffers.vertices[3 * v + 1] <= 1.0 + epsilon);
            buffers.vertices[3 * v + 2] = (buffers.vertices[3 * v + 2] - shiftZ) * scaleZ;
            assert(buffers.vertices[3 * v + 2] >= -1.0 - epsilon);
            assert(buffers.vertices[3 * v + 2] <= 1.0 + epsilon);
        }
    }
}