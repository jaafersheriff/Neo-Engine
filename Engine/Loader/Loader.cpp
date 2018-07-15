#include "Loader.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "ext/tiny_obj_loader.h"

namespace neo {

    bool Loader::verbose = false;
    std::string Loader::RES_DIR = "";
    std::unordered_map<std::string, Mesh *> Loader::meshes;

    void Loader::init(const std::string &res, bool v) {
        RES_DIR = res;
        verbose = v;
    }
}