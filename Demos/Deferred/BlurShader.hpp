#pragma once

#include "Renderer/Shader/PostProcessShader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

using namespace neo;

namespace Deferred {
    class BlurShader : public PostProcessShader {

    public:

        int blurAmount = 2;

        BlurShader(const std::string& frag) :
            PostProcessShader("Blur Shader", frag)
        {}

        virtual void render(const ECS& ecs) override {
            NEO_UNUSED(ecs);

            loadUniform("blurAmount", blurAmount);
        }

        virtual void imguiEditor() override {
            ImGui::SliderInt("Blur", &blurAmount, 0, 10);
        }
    };
}