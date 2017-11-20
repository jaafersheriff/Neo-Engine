/* Loader class - loads meshes and textures
 * Includes references to loaded textures and meshes so they are only loaded once */
#pragma once
#ifndef _LOADER_HPP_
#define _LOADER_HPP_

#define GLEW_STATIC
#include <GL/glew.h>

#include "Model/Texture.hpp"

#include <map>      /* map */
#include <vector>   /* vector */

class Mesh;
class Loader {
    public:
        /* Create TextureData for a provided file name */
        Texture::TextureData getTextureData(const std::string);

        /* Create a texture for a provided file name */
        Texture loadTexture(const std::string);

        /* Create a mesh for a provided file name */
        Mesh* loadObjMesh(const std::string);
        
    private:
        /* Resize a mesh so all of the vertices are [0, 1] */
        void resize(Mesh*);

        /* Collections that prevent loading textures/meshes more than once  */
        std::map<std::string, GLint> textures;
        std::map<std::string, Mesh*> meshes;
};

#endif