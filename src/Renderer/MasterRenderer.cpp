#include "MasterRenderer.hpp"

#include "World/World.hpp"

#include "Entity/EntityShader/EntityShader.hpp"
#include "Skybox/SkyboxShader/SkyboxShader.hpp"
#include "Cloud/CloudShader/CloudShader.hpp"
#include "Sun/SunShader/SunShader.hpp"
#include "Atmosphere/AtmosphereShader/AtmosphereShader.hpp"

#include "Renderer/GLSL.hpp"


#include <iostream>

/* Master render function */
void MasterRenderer::render(const World *world) {
    /* Reset rendering display */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.f, 0.f, 0.f, 1.f);

    /* Loop through active subrenderers */
    for (auto &shader : shaders) {
        /* Bind subrenderer's shader */
        shader->bind();
        /* Load world-shader uniforms */
        loadUniforms(world, shader);
        /* Subrenderer render */
        shader->render(world);
        /* Unbind subrenderer's shader */
        shader->unbind();
    }
}

void MasterRenderer::loadUniforms(const World *world, Shader *shader) {
    if (shader->type == ERROR) {
        return;
    }

    std::vector<World::UniformData *> uniforms = world->uniforms.at(shader->type);
    for (auto uniformData : uniforms) {
        if (uniformData->dataptr) {
            switch (uniformData->type) {
                case(UniformType::Bool):
                    shader->loadBool(shader->getUniform(uniformData->location), (bool)uniformData->dataptr);
                    break;
                case(UniformType::SignedInt8):
                    // TODO 
                    break;
                case(UniformType::SignedInt16):
                    // TODO
                    break;
                case(UniformType::SignedInt32):
                    // TODO 
                    break;
                case(UniformType::UnsignedInt8):
                    // TODO
                    break;
                case(UniformType::UnsignedInt16):
                    // TODO
                    break;
                case(UniformType::UnsignedInt32):
                    // TODO
                    break;
                case(UniformType::Float):
                    shader->loadFloat(shader->getUniform(uniformData->location), *(float *) uniformData->dataptr);
                    break;
                case(UniformType::Vec2):
                    shader->loadVec2(shader->getUniform(uniformData->location), *(glm::vec2 *)uniformData->dataptr);
                    break;
                case(UniformType::Vec3):
                    shader->loadVec3(shader->getUniform(uniformData->location), *(glm::vec3 *)uniformData->dataptr);
                    break;
                case(UniformType::Vec4):
                    // TODO
                    break;
                case(UniformType::Mat3):
                    // TODO
                    break;
                case(UniformType::Mat4):
                    shader->loadMat4(shader->getUniform(uniformData->location), (glm::mat4 *)uniformData->dataptr);
                    break;
                default:
                    break;
            }
        }
    }
}

void MasterRenderer::init(const Context &ctx) {
    this->verbose = ctx.verbose;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND); 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void MasterRenderer::activateEntityShader(std::vector<Entity *> *entities) {
    EntityShader *eShader = new EntityShader;
    if (eShader->init(entities)) {
        shaders.push_back(eShader);
        if (verbose) {
            std::cout << "Entity Shader activated" << std::endl;
        }
    }
    else {
        delete eShader;
        if (verbose) {
            std::cout << "Entity Shader failed to activate" << std::endl;
        }
    }
}

void MasterRenderer::activateSkyboxShader(Skybox *sb) {
    SkyboxShader *sShader = new SkyboxShader;
    if (sShader->init(sb)) {
        shaders.push_back(sShader);
        if (verbose) {
            std::cout << "Skybox Shader activated" << std::endl;
        }
    }
    else {
        delete sShader;
        if (verbose) {
            std::cout << "Skybox Shader failed to activate" << std::endl;
       }
    }
}

void MasterRenderer::activateCloudShader(std::vector<CloudBillboard *> *billboards) {
    CloudShader *cShader = new CloudShader;
    if (cShader->init(billboards)) {
        shaders.push_back(cShader);
        if (verbose) {
            std::cout << "Cloud Shader activated" << std::endl;
        }
    }
    else {
        delete cShader;
        if (verbose) {
            std::cout << "Cloud Shader failed to activate" << std::endl;
        }
    }
}

void MasterRenderer::activateSunShader(Sun *sun) {
    SunShader *sShader = new SunShader;
    if (sShader->init(sun)) {
        shaders.push_back(sShader);
        if (verbose) {
            std::cout << "Sun Shader activated" << std::endl;
        }
    }
    else {
        delete sShader;
        if (verbose) {
            std::cout << "Sun Shader failed to activate" << std::endl;
        }
    }
}

void MasterRenderer::activateAtmosphereShader(Atmosphere *atm) {
    AtmosphereShader *aShader = new AtmosphereShader;
    if (aShader->init(atm)) {
        shaders.push_back(aShader);
        if (verbose) {
            std::cout << "Atmosphere Shader activated" << std::endl;
        }
    }
    else {
        delete aShader;
        if (verbose) {
            std::cout << "Atmosphere Shader failed to activate" << std::endl;
        }
    }
}

void MasterRenderer::cleanUp() {
    /* Clean up all active subrenderers */
    for (auto &shader : shaders) {
        shader->cleanUp();
    }
}

void MasterRenderer::toggleWireFrameMode() {
    wireFrame = !wireFrame;
    glPolygonMode(GL_FRONT_AND_BACK, wireFrame ? GL_LINE : GL_FILL);
}
