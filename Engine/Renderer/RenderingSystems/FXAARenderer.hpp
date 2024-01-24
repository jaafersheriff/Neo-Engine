#pragma once

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "Loader/Library.hpp"
#include "Loader/Loader.hpp"

namespace neo {

	template<typename... CompTs>
    void drawFXAA(Framebuffer& outputFBO, Texture& inputTexture) {
        TRACY_GPU();

        // Where are these const chars in memory..are they being created and passed on each call?
        auto* fxaaShader = Library::createSourceShader("FXAAShader", SourceShader::ConstructionArgs{
            { ShaderStage::VERTEX, "quad.vert"},
            { ShaderStage::FRAGMENT, "fxaa.frag" }
        });

        outputFBO.bind();
        glViewport(0, 0, outputFBO.mTextures[0]->mWidth, outputFBO.mTextures[0]->mHeight);

        auto& resolvedShader = fxaaShader->getResolvedInstance({});
        resolvedShader.bind();
        resolvedShader.bindUniform("frameSize", glm::vec2(inputTexture.mWidth, inputTexture.mHeight));
        resolvedShader.bindTexture("inputTexture", inputTexture);

        bool oldDepthState = glIsEnabled(GL_DEPTH_TEST);
        glDisable(GL_DEPTH_TEST);
        Library::getMesh("quad").mMesh->draw();
        if (oldDepthState) {
            glEnable(GL_DEPTH_TEST);
        }
    }
}