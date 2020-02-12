#pragma once

#include "Renderer/Shader/PostProcessShader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "Window/Window.hpp"
#include "Messaging/Messenger.hpp"

#include "Loader/Library.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"
#include "Renderer/GLObjects/Texture1D.hpp"

using namespace neo;

class PostShader : public PostProcessShader {

    public:

        glm::vec2 maxCoc = glm::vec2(5.2f, 14.9f);
        float radiusScale = 0.24f;
        int poissonSize = 32; 

        PostShader(const std::string& frag) :
            PostProcessShader("Post Shader", frag)
        {
            Texture *poissonTex = Library::createEmptyTexture<Texture1D>("poisson", { GL_RG16F, GL_RG, GL_NEAREST, GL_REPEAT, GL_FLOAT });
            generatePoisson(poissonSize);
        }

        virtual void render() override {
            const auto& defaultFBO = *Library::getFBO("default")->mTextures[0];
            loadTexture("inputFBO", defaultFBO);
            loadUniform("scenePixelSize", 1.f / glm::vec2(defaultFBO.mWidth, defaultFBO.mHeight));

            const auto& dofBlur = *Library::getFBO("dofblur")->mTextures[0];
            loadTexture("dofblur", dofBlur);
            loadUniform("dofPixelSize", 1.f / glm::vec2(dofBlur.mWidth, dofBlur.mHeight));

            loadTexture("dofinfo", *Library::getFBO("dofinfo")->mTextures[0]);

            loadUniform("maxCoc", maxCoc);
            loadUniform("radiusScale", radiusScale);

            loadUniform("poissonSize", poissonSize);
            loadTexture("poissonTex", *Library::getTexture("poisson"));
        }

        virtual void imguiEditor() override {
            ImGui::SliderFloat2("max coc", &maxCoc[0], 1.f, 30.f);
            ImGui::SliderFloat("radius scale", &radiusScale, 0.f, 1.f);
            if (ImGui::SliderInt("poisson size", &poissonSize, 1, 12)) {
                generatePoisson(poissonSize);
            }
        }

        void generatePoisson(int size) {
            std::vector<float> kernel;
            for (unsigned i = 0; i < poissonSize; i++) {
                glm::vec2 sample(
                    Util::genRandom(-1.f, 1.f),
                    Util::genRandom(-1.f, 1.f)
                );
                sample = glm::normalize(sample);
                sample *= Util::genRandom(0.f, 1.f);

                float scale = (float)i / (float)poissonSize;
                scale = Util::lerp(0.1f, 1.f, scale * scale);
                sample *= scale;

                kernel.push_back(sample.x);
                kernel.push_back(sample.y);
            };
            Texture *tex = Library::getTexture("poisson");
            tex->update(glm::uvec2(poissonSize, 1), kernel.data());
        }
};
