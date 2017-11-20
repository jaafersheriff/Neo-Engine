#pragma once
#ifndef _CUBE_TEXTURE_HPP_
#define _CUBE_TEXTURE_HPP_

#include "Toolbox/Loader.hpp"
#include "Model/Texture.hpp"

class CubeTexture : public Texture {
    void init(Loader &, const std::string, const std::string, const std::string, const std::string, const std::string, const std::string);
};

#endif