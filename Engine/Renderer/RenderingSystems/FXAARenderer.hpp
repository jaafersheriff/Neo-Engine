#pragma once

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "Loader/Library.hpp"
#include "Loader/Loader.hpp"

namespace neo {

	template<typename... CompTs>
    void drawFXAA(Framebuffer& outputFBO, Texture& inputTexture) {
        TRACY_GPUN("drawFXAA");

        // Where are these const chars in memory..are they being created and passed on each call?
        static auto fxaaShader = Library::createShaderSource("FXAAShader", SourceShader::ConstructionArgs{
            { ShaderStage::VERTEX, "quad.vert"},
            { ShaderStage::FRAGMENT, "fxaa.frag" }
        });

        glViewport(0, 0, outputFBO.mTextures[0]->mWidth, outputFBO.mTextures[0]->mHeight);

        auto resolvedShader = fxaaShader->getResolvedInstance({});
        resolvedShader.bind();
        resolvedShader.bindUniform("frameSize", glm::vec2(inputTexture.mWidth, inputTexture.mHeight));
        resolvedShader.bindTexture("inputTexture", inputTexture);

        glDisable(GL_DEPTH_TEST);
        Library::getMesh("quad").mMesh->draw();
        glEnable(GL_DEPTH_TEST);
    }
}