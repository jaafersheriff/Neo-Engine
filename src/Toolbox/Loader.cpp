#include "Loader.hpp"
#include "Model/Mesh.hpp"
#include "BoundingBox/BoundingBox.hpp"

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
    resize(mesh);

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

/* Provided function to resize a mesh so all vertex positions are [0, 1.f] */
void Loader::resize(Mesh *mesh) {
    float scaleX, scaleY, scaleZ;
    float shiftX, shiftY, shiftZ;
    float epsilon = 0.001f;

    /* Find BoundingBox from mesh */
    BoundingBox boundingBox(mesh);

    //From min and max compute necessary scale and shift for each dimension
    float maxExtent, xExtent, yExtent, zExtent;
    xExtent = boundingBox.max.x-boundingBox.min.x;
    yExtent = boundingBox.max.y-boundingBox.min.y;
    zExtent = boundingBox.max.z-boundingBox.min.z;
    
    if (xExtent >= yExtent && xExtent >= zExtent) {
        maxExtent = xExtent;
    }
    if (yExtent >= xExtent && yExtent >= zExtent) {
        maxExtent = yExtent;
    }
    if (zExtent >= xExtent && zExtent >= yExtent) {
        maxExtent = zExtent;
    }
    scaleX = 2.f /maxExtent;
    shiftX = boundingBox.min.x + (xExtent/ 2.f);
    scaleY = 2.f / maxExtent;
    shiftY = boundingBox.min.y + (yExtent / 2.f);
    scaleZ = 2.f/ maxExtent;
    shiftZ = boundingBox.min.z + (zExtent)/2.f;

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

Mesh* Loader::loadCubeMesh(const float scale) {
    Mesh *cube = meshGenerator.generateCube(scale);
    cube->init();
    return cube;
}