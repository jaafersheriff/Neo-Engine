#include "Loader.hpp"

#include "Library.hpp"

#include "Renderer/GLObjects/Mesh.hpp"
#include "Renderer/GLObjects/Texture.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

#include "Util/Log/Log.hpp"

#pragma warning(push)
#pragma warning(disable: 4706)
#define TINYOBJLOADER_IMPLEMENTATION
#include "ext/tiny_obj_loader.h"
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4244)
#define STB_IMAGE_IMPLEMENTATION
#include "ext/stb_image.h"
#pragma warning(pop)

namespace neo {

    std::string Loader::APP_RES_DIR = "";
    std::string Loader::ENGINE_RES_DIR = "../Engine/res/";

    void Loader::init(const std::string &res) {
        APP_RES_DIR = res;
    }

    MeshData Loader::loadMesh(const std::string &fileName, bool doResize) {
        MICROPROFILE_SCOPEI("Loader", "loadMesh", MP_AUTO);

        /* Create mesh */
        MeshData meshData;
        Mesh* mesh = new Mesh;
        meshData.mMesh = mesh;

        /* If mesh was not found in map, read it in */
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> objMaterials;
        std::string errString;
        std::string _fileName = APP_RES_DIR + fileName;
        if (!util::fileExists(_fileName.c_str())) {
            _fileName = ENGINE_RES_DIR + fileName;
        }
        NEO_ASSERT(util::fileExists(_fileName.c_str()), "Unable to find file: %s after checking:\n\t%s\n\t%s\n", fileName.c_str(), APP_RES_DIR.c_str(), ENGINE_RES_DIR.c_str());
        // TODO : use assimp or another optimized asset loader
        bool rc = tinyobj::LoadObj(shapes, objMaterials, errString, _fileName.c_str());
        NEO_ASSERT(rc, errString.c_str());

        /* Create empty mesh buffers */
        std::vector<float> vertices;
        std::vector<float> normals;
        std::vector<float> texCoords;
        std::vector<uint32_t> indices;

        int vertCount = 0;
        /* For every shape in the loaded file */
        for (uint32_t i = 0; i < shapes.size(); i++) {
            /* Concatenate the shape's vertices, normals, and textures to the mesh */
            vertices.insert(vertices.end(), shapes[i].mesh.positions.begin(), shapes[i].mesh.positions.end());
            normals.insert(normals.end(), shapes[i].mesh.normals.begin(), shapes[i].mesh.normals.end());
            texCoords.insert(texCoords.end(), shapes[i].mesh.texcoords.begin(), shapes[i].mesh.texcoords.end());

            /* Concatenate the shape's indices to the new mesh
             * Indices need to be incremented as we concatenate shapes */
            for (uint32_t j : shapes[i].mesh.indices) {
                indices.push_back(j + vertCount);
            }
            vertCount += int(shapes[i].mesh.positions.size()) / 3;
        }

        /* Optional resize and find min/max */
        _findMetaData(meshData, vertices, doResize);

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

        NEO_LOG_I("Loaded mesh (%d vertices): %s", vertCount, fileName.c_str());
        return meshData;
    }
   std::vector<Asset> Loader::loadMultiAsset(const std::string &fileName) {
        /* If mesh was not found in map, read it in */
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> objMaterials;

        std::string resDir = APP_RES_DIR;
        if (!util::fileExists((resDir + fileName).c_str())) {
            resDir = ENGINE_RES_DIR;
            NEO_ASSERT(util::fileExists((resDir + fileName).c_str()), "Unable to find file: %s", fileName.c_str());
        }
        // TODO : use assimp or another optimized asset loader
        std::string errString;
        bool rc = tinyobj::LoadObj(shapes, objMaterials, errString, (resDir + fileName).c_str(), resDir.c_str());
        NEO_ASSERT(rc, errString.c_str());

        std::vector<Asset> ret;

        for (auto& shape : shapes) {
            Asset asset;

            Mesh* mesh = new Mesh;
            asset.meshData.mMesh = mesh;

            _findMetaData(asset.meshData, shape.mesh.positions, true);

            /* Upload */
            mesh->mPrimitiveType = GL_TRIANGLE_STRIP;
            if (shape.mesh.positions.size()) {
                mesh->addVertexBuffer(VertexType::Position, 0, 3, shape.mesh.positions);
            }
            if (shape.mesh.normals.size()) {
                mesh->addVertexBuffer(VertexType::Normal, 1, 3, shape.mesh.normals);
            }
            if (shape.mesh.texcoords.size()) {
                mesh->addVertexBuffer(VertexType::Texture0, 2, 2, shape.mesh.texcoords);
            }
            if (shape.mesh.indices.size()) {
                mesh->mPrimitiveType = GL_TRIANGLES;
                mesh->addElementBuffer(shape.mesh.indices);
            }

            NEO_LOG_I("Loaded mesh (%d vertices): %s of %s", shape.mesh.positions.size(), shape.name.c_str(), fileName.c_str());

            for (auto materialID : shape.mesh.material_ids) {
                if (materialID >= 0) {
                    auto& material = objMaterials[materialID];
                    asset.material.mAmbient = glm::vec3(material.ambient[0], material.ambient[1], material.ambient[2]);
                    asset.material.mDiffuse = glm::vec3(material.diffuse[0], material.diffuse[1], material.diffuse[2]);
                    asset.material.mSpecular = glm::vec3(material.specular[0], material.specular[1], material.specular[2]);
                    asset.material.mShininess = material.shininess;
                    asset.material.mTransmittance = glm::vec3(material.transmittance[0], material.transmittance[1], material.transmittance[2]);
                    asset.material.mEmission = glm::vec3(material.emission[0], material.emission[1], material.emission[2]);
                    asset.material.mIOR = material.ior;
                    asset.material.mDissolve = material.dissolve;

                    TextureFormat format;
                    format.mInternalFormat = GL_RGB8;
                    format.mBaseFormat = GL_RGB;

                    if (material.ambient_texname.size()) {
                        asset.ambient_tex = Library::loadTexture(material.ambient_texname, format);
                    }
                    if (material.diffuse_texname.size()) {
                        asset.diffuse_tex = Library::loadTexture(material.diffuse_texname, format);
                    }
                    if (material.specular_texname.size()) {
                        asset.specular_tex = Library::loadTexture(material.specular_texname, format);
                    }
                    if (material.displacement_texname.size()) {
                        asset.displacement_tex = Library::loadTexture(material.displacement_texname, format);
                    }
                }
            }

            ret.push_back(asset);
        }

        return ret;
    }

    Texture2D* Loader::loadTexture(const std::string &fileName, TextureFormat format) {
        MICROPROFILE_SCOPEI("Loader", "loadTexture", MP_AUTO);
        /* Create an empty texture if it is not already exist in the library */
        int width, height, components;
        uint8_t* data = _loadTextureData(width, height, components, fileName, format);

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
            data.push_back(_loadTextureData(_width, _height, _components, files[i], {}, false));
            sizes.push_back(glm::uvec2(_width, _height));
        }

        /* Upload data to GPU and free from CPU */
        TextureFormat format = { GL_RGBA, GL_RGBA, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_UNSIGNED_BYTE };
        TextureCubeMap* texture = new TextureCubeMap(format, sizes, reinterpret_cast<void**>(data.data()));

        /* Clean */
        for (int i = 0; i < 6; i++) {
            _cleanTextureData(data[i]);
        }

        NEO_LOG_I("Loaded cubemap (%s)", name.c_str());

        return texture;
    }

    uint8_t* Loader::_loadTextureData(int& width, int& height, int& components, const std::string& fileName, TextureFormat format, bool flip) {
        std::string _fileName = APP_RES_DIR + fileName;
        if (!util::fileExists(_fileName.c_str())) {
            _fileName = ENGINE_RES_DIR + fileName;
            NEO_ASSERT(util::fileExists(_fileName.c_str()), "Unable to find file %s", fileName.c_str());
        }

        /* Use stbi if name is an existing file */
        stbi_set_flip_vertically_on_load(flip);
        uint8_t *data = stbi_load(_fileName.c_str(), &width, &height, &components, format.mBaseFormat == GL_RGB ? STBI_rgb : STBI_rgb_alpha);
        NEO_ASSERT(data, "Error reading texture file");

        NEO_LOG_I("Loaded texture %s [%d, %d]", fileName.c_str(), width, height);

        return data;
    }

    void Loader::_cleanTextureData(uint8_t* data) {
        stbi_image_free(data);
    }

    /* Provided function to resize a mesh so all vertex positions are [0, 1.f] */
    void Loader::_findMetaData(MeshData& meshData, std::vector<float>& vertices, bool doResize) {
        float minX, minY, minZ;
        float maxX, maxY, maxZ;
        float scaleX, scaleY, scaleZ;
        float shiftX, shiftY, shiftZ;

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

        //From min and max compute necessary scale and shift for each dimension
        float xExtent, yExtent, zExtent;
        xExtent = maxX - minX;
        yExtent = maxY - minY;
        zExtent = maxZ - minZ;

        float maxExtent = 0;
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

        if (doResize) {
            //Go through all verticies shift and scale them
            minX = minY = minZ = 1.1754E+38F;
            maxX = maxY = maxZ = -1.1754E+38F;
            for (size_t v = 0; v < vertices.size() / 3; v++) {
                vertices[3 * v + 0] = (vertices[3 * v + 0] - shiftX) * scaleX;
                minX = std::min(minX, vertices[3 * v + 0]);
                maxX = std::max(maxX, vertices[3 * v + 0]);
                assert(vertices[3 * v + 0] >= -1.0 - 0.001f);
                assert(vertices[3 * v + 0] <= 1.0 + 0.001f);
                vertices[3 * v + 1] = (vertices[3 * v + 1] - shiftY) * scaleY;
                minY = std::min(minY, vertices[3 * v + 1]);
                maxY = std::max(maxY, vertices[3 * v + 1]);
                assert(vertices[3 * v + 1] >= -1.0 - 0.001f);
                assert(vertices[3 * v + 1] <= 1.0 + 0.001f);
                vertices[3 * v + 2] = (vertices[3 * v + 2] - shiftZ) * scaleZ;
                minZ = std::min(minZ, vertices[3 * v + 2]);
                maxZ = std::max(maxZ, vertices[3 * v + 2]);
                assert(vertices[3 * v + 2] >= -1.0 - 0.001f);
                assert(vertices[3 * v + 2] <= 1.0 + 0.001f);
            }
        }

        meshData.mMin = glm::vec3(minX, minY, minZ);
        meshData.mMax = glm::vec3(maxX, maxY, maxZ);
        meshData.mBasePosition = glm::vec3(shiftX, shiftY, shiftZ);
        meshData.mBaseScale = glm::vec3(maxExtent) / 2.f;
    }
}
