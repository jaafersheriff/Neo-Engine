#include "Loader.hpp"
#include "Model/Mesh.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <vector>

Texture::TextureData Loader::getTextureData(const std::string fileName, const bool flip) {
    Texture::TextureData td;
    stbi_set_flip_vertically_on_load(flip);
    td.data = stbi_load(fileName.c_str(), &td.width, &td.height, &td.components, STBI_rgb_alpha);
    if (td.data) {
        std::cout << "Loaded texture (" << td.width << ", " << td.height << "): " << fileName << std::endl;
    }
    else {
        std::cerr << "Could not find texture file " << fileName << std::endl;
    }

    return td;
}

Texture Loader::loadTexture(const std::string fileName) {
    Texture texture;
    std::map<std::string, GLint>::iterator it = textures.find(fileName);
    if (it != textures.end()) {
        texture.textureId = it->second;
    }
    else {
        Texture::TextureData td = getTextureData(fileName, true);
        if(td.data) {
            texture.init(td);
            if (texture.textureId) {
                textures.insert(std::map<std::string, GLint>::value_type(fileName, texture.textureId));
            }
            stbi_image_free(td.data);
        }
    }
    return texture;
}

Mesh* Loader::loadObjMesh(const std::string fileName) {
    std::map<std::string, Mesh*>::iterator it = meshes.find(fileName);
    if (it != meshes.end()) {
        return it->second;
    }

    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> objMaterials;
    std::string errString;
    bool rc = tinyobj::LoadObj(shapes, objMaterials, errString, fileName.c_str());
    if (!rc) {
        std::cerr << errString << std::endl;
        exit(1);
    }
 
    /* Create a new empty mesh */
    Mesh *mesh = new Mesh;
    int vertCount = 0;
    /* For every shape in the loaded file */
    for (int i = 0; i < shapes.size(); i++) {
        /* Concatenate the shape's vertices to the new mesh */
        mesh->vertBuf.insert(mesh->vertBuf.end(), shapes[i].mesh.positions.begin(), shapes[i].mesh.positions.end());
        /* Concatenate the shape's normals to the new mesh */
        mesh->norBuf.insert(mesh->norBuf.end(), shapes[i].mesh.normals.begin(), shapes[i].mesh.normals.end());
        /* Concatenate the shape's texture coordinates to the new mesh */
        mesh->texBuf.insert(mesh->texBuf.end(), shapes[i].mesh.texcoords.begin(), shapes[i].mesh.texcoords.end());
        /* Concatenate the shape's indices to the new mesh
         * Indices need to be incremented as we concatenate shapes */
        for (unsigned int i : shapes[i].mesh.indices) {
            mesh->eleBuf.push_back(i + vertCount);
        }
        vertCount += shapes[i].mesh.positions.size() / 3;
    }

    /* Resize the mesh to be centered around origin and have vertex values [0, 1.0] */
    resize(mesh);

    /* Copy mesh data to the gpu */
    mesh->init();
    
    meshes.insert(std::map<std::string, Mesh*>::value_type(fileName, mesh));

    std::cout << "Loaded mesh (" << vertCount << " vertices): " << fileName << std::endl;

    return mesh;
}

CubeTexture Loader::loadCubeTexture(const std::string fileNames[6]) {
    CubeTexture cubeTexture;

    /* If texture has already been loaded, just return it */
    std::map<std::string, GLint>::iterator it = textures.find(fileNames[0]);
    if (it != textures.end()) {
        cubeTexture.textureId = it->second;
    }
    else {
        /* Load in texture data to CPU */
        Texture::TextureData tds[6];
        for (int i = 0; i < 6; i++) {
            tds[i] = getTextureData(fileNames[i], false);
        }

        /* Copy cube texture data to GPU */
        cubeTexture.init(tds);
        
        /* Add to map based on first file name */
        if (cubeTexture.textureId) {
            textures.insert(std::map<std::string, GLint>::value_type(fileNames[0], cubeTexture.textureId));
        }
        /* Free data from CPU */
        for (int i = 0; i < 6; i++) {
            stbi_image_free(tds[i].data);
        }
    }
    
    return cubeTexture;
}

/* Provided function to resize a mesh so all vertex positions are [0, 1.f] */
void Loader::resize(Mesh * mesh) {
    float minX, minY, minZ;
    float maxX, maxY, maxZ;
    float scaleX, scaleY, scaleZ;
    float shiftX, shiftY, shiftZ;
    float epsilon = 0.001;

    minX = minY = minZ = 1.1754E+38F;
    maxX = maxY = maxZ = -1.1754E+38F;

    //Go through all vertices to determine min and max of each dimension
    for (size_t v = 0; v < mesh->vertBuf.size() / 3; v++) {
        if(mesh->vertBuf[3*v+0] < minX) minX = mesh->vertBuf[3*v+0];
        if(mesh->vertBuf[3*v+0] > maxX) maxX = mesh->vertBuf[3*v+0];

        if(mesh->vertBuf[3*v+1] < minY) minY = mesh->vertBuf[3*v+1];
        if(mesh->vertBuf[3*v+1] > maxY) maxY = mesh->vertBuf[3*v+1];

        if(mesh->vertBuf[3*v+2] < minZ) minZ = mesh->vertBuf[3*v+2];
        if(mesh->vertBuf[3*v+2] > maxZ) maxZ = mesh->vertBuf[3*v+2];
    }

    //From min and max compute necessary scale and shift for each dimension
    float maxExtent, xExtent, yExtent, zExtent;
    xExtent = maxX-minX;
    yExtent = maxY-minY;
    zExtent = maxZ-minZ;
    if (xExtent >= yExtent && xExtent >= zExtent) {
    maxExtent = xExtent;
    }
    if (yExtent >= xExtent && yExtent >= zExtent) {
        maxExtent = yExtent;
    }
    if (zExtent >= xExtent && zExtent >= yExtent) {
        maxExtent = zExtent;
    }
    scaleX = 2.0 /maxExtent;
    shiftX = minX + (xExtent/ 2.0);
    scaleY = 2.0 / maxExtent;
    shiftY = minY + (yExtent / 2.0);
    scaleZ = 2.0/ maxExtent;
    shiftZ = minZ + (zExtent)/2.0;

    //Go through all verticies shift and scale them
	for (size_t v = 0; v < mesh->vertBuf.size() / 3; v++) {
		mesh->vertBuf[3*v+0] = (mesh->vertBuf[3*v+0] - shiftX) * scaleX;
		assert(mesh->vertBuf[3*v+0] >= -1.0 - epsilon);
		assert(mesh->vertBuf[3*v+0] <= 1.0 + epsilon);
		mesh->vertBuf[3*v+1] = (mesh->vertBuf[3*v+1] - shiftY) * scaleY;
		assert(mesh->vertBuf[3*v+1] >= -1.0 - epsilon);
		assert(mesh->vertBuf[3*v+1] <= 1.0 + epsilon);
		mesh->vertBuf[3*v+2] = (mesh->vertBuf[3*v+2] - shiftZ) * scaleZ;
		assert(mesh->vertBuf[3*v+2] >= -1.0 - epsilon);
        assert(mesh->vertBuf[3*v+2] <= 1.0 + epsilon);
	}
}
