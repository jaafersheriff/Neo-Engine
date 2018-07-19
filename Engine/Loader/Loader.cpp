#include "Loader.hpp"

#define GLEW_STATIC
#include "GL/glew.h"
#include "Shader/GLHelper.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "ext/tiny_obj_loader.h"

#include <iostream>

namespace neo {

    bool Loader::verbose = false;
    std::string Loader::RES_DIR = "";
    std::unordered_map<std::string, Mesh *> Loader::meshes;

    void Loader::init(const std::string &res, bool v) {
        RES_DIR = res;
        verbose = v;
    }

    Mesh * Loader::getMesh(const std::string &fileName) {
        /* Search map first */
        auto it = meshes.find(fileName);
        if (it != meshes.end()) {
            return it->second;
        }

        /* If mesh was not found in map, read it in */
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> objMaterials;
        std::string errString;
        bool rc = tinyobj::LoadObj(shapes, objMaterials, errString, (RES_DIR + fileName).c_str());
        if (!rc) {
            std::cerr << errString << std::endl;
            exit(1);
        }

        /* Create a new empty mesh */
        Mesh * mesh = new Mesh;

        int vertCount = 0;
        /* For every shape in the loaded file */
        for (unsigned int i = 0; i < shapes.size(); i++) {
            /* Concatenate the shape's vertices, normals, and textures to the mesh */
            mesh->buffers.vertBuf.insert(mesh->buffers.vertBuf.end(), shapes[i].mesh.positions.begin(), shapes[i].mesh.positions.end());
            mesh->buffers.norBuf.insert(mesh->buffers.norBuf.end(), shapes[i].mesh.normals.begin(), shapes[i].mesh.normals.end());
            mesh->buffers.texBuf.insert(mesh->buffers.texBuf.end(), shapes[i].mesh.texcoords.begin(), shapes[i].mesh.texcoords.end());

            /* Concatenate the shape's indices to the new mesh
             * Indices need to be incremented as we concatenate shapes */
            for (unsigned int i : shapes[i].mesh.indices) {
                mesh->buffers.eleBuf.push_back(i + vertCount);
            }
            vertCount += int(shapes[i].mesh.positions.size()) / 3;
        }

        /* Provide VBO info */
        mesh->vertBufSize = int(mesh->buffers.vertBuf.size());
        mesh->norBufSize = int(mesh->buffers.norBuf.size());
        mesh->texBufSize = int(mesh->buffers.texBuf.size());
        mesh->eleBufSize = int(mesh->buffers.eleBuf.size());

        /* Add new mesh to library */
        meshes.insert({ fileName, mesh });

        /* Load mesh to GPU */
        uploadMesh(*mesh);

        if (verbose) {
            std::cout << "Loaded mesh (" << vertCount << " vertices): " << fileName << std::endl;
        }

        return mesh;
    }

/* Provided function to resize a mesh so all vertex positions are [0, 1.f] */
    void Loader::resize(Mesh::MeshBuffers & buffers) {
        float minX, minY, minZ;
        float maxX, maxY, maxZ;
        float scaleX, scaleY, scaleZ;
        float shiftX, shiftY, shiftZ;
        float epsilon = 0.001f;

        minX = minY = minZ = 1.1754E+38F;
        maxX = maxY = maxZ = -1.1754E+38F;

        //Go through all vertices to determine min and max of each dimension
        for (size_t v = 0; v < buffers.vertBuf.size() / 3; v++) {
            if (buffers.vertBuf[3 * v + 0] < minX) minX = buffers.vertBuf[3 * v + 0];
            if (buffers.vertBuf[3 * v + 0] > maxX) maxX = buffers.vertBuf[3 * v + 0];

            if (buffers.vertBuf[3 * v + 1] < minY) minY = buffers.vertBuf[3 * v + 1];
            if (buffers.vertBuf[3 * v + 1] > maxY) maxY = buffers.vertBuf[3 * v + 1];

            if (buffers.vertBuf[3 * v + 2] < minZ) minZ = buffers.vertBuf[3 * v + 2];
            if (buffers.vertBuf[3 * v + 2] > maxZ) maxZ = buffers.vertBuf[3 * v + 2];
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
        for (size_t v = 0; v < buffers.vertBuf.size() / 3; v++) {
            buffers.vertBuf[3 * v + 0] = (buffers.vertBuf[3 * v + 0] - shiftX) * scaleX;
            assert(buffers.vertBuf[3 * v + 0] >= -1.0 - epsilon);
            assert(buffers.vertBuf[3 * v + 0] <= 1.0 + epsilon);
            buffers.vertBuf[3 * v + 1] = (buffers.vertBuf[3 * v + 1] - shiftY) * scaleY;
            assert(buffers.vertBuf[3 * v + 1] >= -1.0 - epsilon);
            assert(buffers.vertBuf[3 * v + 1] <= 1.0 + epsilon);
            buffers.vertBuf[3 * v + 2] = (buffers.vertBuf[3 * v + 2] - shiftZ) * scaleZ;
            assert(buffers.vertBuf[3 * v + 2] >= -1.0 - epsilon);
            assert(buffers.vertBuf[3 * v + 2] <= 1.0 + epsilon);
        }
    }


    void Loader::uploadMesh(const Mesh & mesh) {
        /* Initialize VAO */
        CHECK_GL(glGenVertexArrays(1, (GLuint *) &mesh.vaoId));
        CHECK_GL(glBindVertexArray(mesh.vaoId));

        /* Copy vertex array */
        CHECK_GL(glGenBuffers(1, (GLuint *) &mesh.vertBufId));
        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mesh.vertBufId));
        CHECK_GL(glBufferData(GL_ARRAY_BUFFER, mesh.buffers.vertBuf.size() * sizeof(float), &mesh.buffers.vertBuf[0], GL_STATIC_DRAW));
        CHECK_GL(glEnableVertexAttribArray(0));
        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mesh.vertBufId));
        CHECK_GL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));

        /* Copy element array if it exists */
        if (!mesh.buffers.eleBuf.empty()) {
            CHECK_GL(glGenBuffers(1, (GLuint *) &mesh.eleBufId));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.eleBufId));
            CHECK_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.buffers.eleBuf.size() * sizeof(unsigned int), &mesh.buffers.eleBuf[0], GL_STATIC_DRAW));
            CHECK_GL(glEnableVertexAttribArray(1));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mesh.norBufId));
            CHECK_GL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));
        }

        /* Copy normal array if it exists */
        if (!mesh.buffers.norBuf.empty()) {
            CHECK_GL(glGenBuffers(1, (GLuint *) &mesh.norBufId));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mesh.norBufId));
            CHECK_GL(glBufferData(GL_ARRAY_BUFFER, mesh.buffers.norBuf.size() * sizeof(float), &mesh.buffers.norBuf[0], GL_STATIC_DRAW));
            CHECK_GL(glEnableVertexAttribArray(2));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mesh.texBufId));
            CHECK_GL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0));
        }

        /* Copy texture array if it exists */
        if (!mesh.buffers.texBuf.empty()) {
            CHECK_GL(glGenBuffers(1, (GLuint *) &mesh.texBufId));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mesh.texBufId));
            CHECK_GL(glBufferData(GL_ARRAY_BUFFER, mesh.buffers.texBuf.size() * sizeof(float), &mesh.buffers.texBuf[0], GL_STATIC_DRAW));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.eleBufId));
        }

        /* Unbind  */
        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
        CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

        /* Error check */
        assert(glGetError() == GL_NO_ERROR);
    }
}