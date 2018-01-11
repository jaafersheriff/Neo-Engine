/* Loader class - loads meshes and textures
 * Includes references to loaded textures and meshes so they are only loaded once */
#pragma once
#ifndef _LOADER_HPP_
#define _LOADER_HPP_

#define GLEW_STATIC
#include <GL/glew.h>

#include "Skybox/CubeTexture.hpp"
#include "Model/Texture.hpp"
#include "Context/Context.hpp"

#include <map>      /* map      */
#include <vector>   /* vector   */

class Mesh;
class Loader {
    public:
        void init(Context &);

        /* Create a texture for a provided file name and optional wrpa mode */
        Texture* loadTexture(const std::string);
        Texture* loadTexture(const std::string, Texture::WRAP_MODE);

        /* Create entire skybox with cube texture generated from provided file names */ 
        CubeTexture* loadCubeTexture(const std::string[6]);

        /* Create a mesh for a provided file name */
        Mesh* loadObjMesh(const std::string);

    private:
        /* Resize a mesh so all of the vertices are [0, 1] */
        void resize(Mesh*);

        /* Return pointer to loaded stbi_image data 
         * Update members in Texture pointer */ 
        uint8_t* loadTextureData(Texture *, const std::string, const bool);

        /* Collections that prevent loading textures/meshes more than once  */
        std::map<std::string, Texture*> textures;
        std::map<std::string, Mesh*> meshes;

        bool verbose;
        std::string RESOURCE_DIR;
};

#endif
