#include "Loader.hpp"
#include "Model/Mesh.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <vector>

void Loader::init(Context &ctx) {
    this->verbose = ctx.verbose;
    this->RESOURCE_DIR = ctx.RESOURCE_DIR;
}

uint8_t* Loader::loadTextureData(Texture *tex, const std::string fileName, const bool flip) {
    uint8_t *data;

    stbi_set_flip_vertically_on_load(flip);
    data = stbi_load((RESOURCE_DIR + fileName).c_str(), &tex->width, &tex->height, &tex->components, STBI_rgb_alpha);

    if (data) {
        if (verbose) {
            std::cout << "Loaded texture (" << tex->width << ", " << tex->height << "): " << fileName << std::endl;
        }
    }
    else {
        std::cerr << "Could not find texture file " << RESOURCE_DIR << fileName << std::endl;
    }
    return data;
}

Texture* Loader::loadTexture(const std::string fileName) {
    return loadTexture(fileName, Texture::WRAP_MODE::REPEAT);
}

Texture * Loader::loadTexture(const std::string fileName, Texture::WRAP_MODE mode) {
    std::map<std::string, Texture *>::iterator it = textures.find(fileName);
    if (it != textures.end()) {
        return it->second;
    }
    Texture *texture = new Texture;
    uint8_t *data = loadTextureData(texture, fileName, true);
    if(data) {
        texture->init(data, mode);
        if (texture->textureId) {
            textures.insert(std::map<std::string, Texture*>::value_type(fileName, texture));
        }
        stbi_image_free(data);
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
    bool rc = tinyobj::LoadObj(shapes, objMaterials, errString, (RESOURCE_DIR + fileName).c_str());
    if (!rc) {
        std::cerr << errString << std::endl;
        exit(1);
    }
 
    /* Create a new empty mesh */
    Mesh *mesh = new Mesh;
    int vertCount = 0;
    /* For every shape in the loaded file */
    for (unsigned int i = 0; i < shapes.size(); i++) {
        /* Concatenate the shape's vertices, normals, and textures to the mesh */
        mesh->vertBuf.insert(mesh->vertBuf.end(), shapes[i].mesh.positions.begin(), shapes[i].mesh.positions.end());
        mesh->norBuf.insert(mesh->norBuf.end(), shapes[i].mesh.normals.begin(), shapes[i].mesh.normals.end());
        mesh->texBuf.insert(mesh->texBuf.end(), shapes[i].mesh.texcoords.begin(), shapes[i].mesh.texcoords.end());

        /* Concatenate the shape's indices to the new mesh
         * Indices need to be incremented as we concatenate shapes */
        for (unsigned int i : shapes[i].mesh.indices) {
            mesh->eleBuf.push_back(i + vertCount);
        }
        vertCount += shapes[i].mesh.positions.size() / 3;
    }

    /* Resize the mesh to be centered around origin and have vertex values [0, 1.0] */
    MeshGenerator::resize(mesh);

    /* Copy mesh data to the gpu */
    mesh->init();
    
    meshes.insert(std::map<std::string, Mesh*>::value_type(fileName, mesh));

    if (verbose) {
        std::cout << "Loaded mesh (" << vertCount << " vertices): " << fileName << std::endl;
    }

    return mesh;
}

CubeTexture* Loader::loadCubeTexture(const std::string fileNames[6]) {

    /* If texture has already been loaded, just return it */
    std::map<std::string, Texture *>::iterator it = textures.find(fileNames[0]);
    if (it != textures.end()) {
        return (CubeTexture *) it->second;
    }

    CubeTexture *cubeTexture = new CubeTexture;
    /* Load in texture data to CPU */
    uint8_t* data[6];
    for (int i = 0; i < 6; i++) {
        data[i] = loadTextureData(cubeTexture, fileNames[i], false);
    }

    /* Copy cube texture data to GPU */
    cubeTexture->init(data);
    
    /* Add to map based on first file name */
    if (cubeTexture->textureId) {
        textures.insert(std::map<std::string, Texture*>::value_type(fileNames[0], cubeTexture));
    }
    /* Free data from CPU */
    for (int i = 0; i < 6; i++) {
        stbi_image_free(data[i]);
    }
    
    return cubeTexture;
}


