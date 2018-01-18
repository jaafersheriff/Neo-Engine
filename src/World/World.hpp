/* Abstract parent World class */
#pragma once
#ifndef _WORLD_HPP_
#define _WORLD_HPP_

#include "Renderer/MasterRenderer.hpp"
#include "Context/Context.hpp"
#include "Camera/Camera.hpp"
#include "Light/Light.hpp"
#include "Toolbox/Loader.hpp"
#include "Toolbox/Enum.hpp"

#include <string>
#include <vector>
#include <unordered_map>

class World {
    public:
        struct UniformData {
            UniformType type;
            std::string location;
            void *dataptr;
        };

        /* Include world-specific data structure to be rendered  */
        World(const std::string n) : name(n) { }
        std::string name;

        /* World members */
        Camera *camera;
        Light *light;

        /* Uniform map */
        std::unordered_map<MasterRenderer::ShaderTypes, std::vector<UniformData *>> uniforms;
        virtual void prepareUniforms() = 0;

        /* Create objects, initialize rendering data structure */
        virtual void init(Context &, Loader &) = 0;

        /* Activate feature renderers in MasterRenderer and pass in proper
         * data structure */
        virtual void prepareRenderer(MasterRenderer *) = 0;

        /* Update features, call takeInput() */
        virtual void update(Context &) = 0;

        /* Any necessary clean up */
        virtual void cleanUp() = 0;
    private:
        /* Process any user input */
        virtual void takeInput(Mouse &, Keyboard &, const float) = 0;
};

#endif
