#pragma once

#include "Renderer/GLObjects/NewShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "Loader/Library.hpp"
#include "Loader/Loader.hpp"

namespace neo {

	template<typename... CompTs>
    void drawFXAA(Framebuffer& outputFBO, Texture& inputTexture) {
        MICROPROFILE_SCOPEI("FXAARenderer", "drawFXAA", MP_AUTO);

        // Where are these const chars in memory..are they being created and passed on each call?
        auto fxaaShader = Library::createShaderSource("FXAAShader", NewShader::ShaderSources {
            // TODO - this is a memory leak
            { ShaderStage::VERTEX, Loader::loadFileString("quad.vert")},
            { ShaderStage::FRAGMENT, R"(
                in vec2 fragTex;
                uniform vec2 frameSize;
                uniform sampler2D inputTexture;

                out vec4 color;
                void main() {

                    float FXAA_SPAN_MAX = 8.0;
                    float FXAA_REDUCE_MUL = 1.0/8.0;
                    float FXAA_REDUCE_MIN = 1.0/128.0;

                    vec3 rgbNW=texture2D(inputTexture,fragTex+(vec2(-1.0,-1.0)/frameSize)).xyz;
                    vec3 rgbNE=texture2D(inputTexture,fragTex+(vec2( 1.0,-1.0)/frameSize)).xyz;
                    vec3 rgbSW=texture2D(inputTexture,fragTex+(vec2(-1.0, 1.0)/frameSize)).xyz;
                    vec3 rgbSE=texture2D(inputTexture,fragTex+(vec2( 1.0, 1.0)/frameSize)).xyz;
                    vec3 rgbM= texture2D(inputTexture,fragTex).xyz;

                    vec3 luma=vec3(0.299, 0.587, 0.114);
                    float lumaNW = dot(rgbNW, luma);
                    float lumaNE = dot(rgbNE, luma);
                    float lumaSW = dot(rgbSW, luma);
                    float lumaSE = dot(rgbSE, luma);
                    float lumaM  = dot(rgbM,  luma);

                    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
                    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

                    vec2 dir;
                    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
                    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

                    float dirReduce = max(
                        (lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL),
                        FXAA_REDUCE_MIN);

                    float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);

                    dir = min(vec2( FXAA_SPAN_MAX,  FXAA_SPAN_MAX),
                          max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
                          dir * rcpDirMin)) / frameSize;

                    vec3 rgbA = (1.0/2.0) * (
                        texture2D(inputTexture, fragTex.xy + dir * (1.0/3.0 - 0.5)).xyz +
                        texture2D(inputTexture, fragTex.xy + dir * (2.0/3.0 - 0.5)).xyz);
                    vec3 rgbB = rgbA * (1.0/2.0) + (1.0/4.0) * (
                        texture2D(inputTexture, fragTex.xy + dir * (0.0/3.0 - 0.5)).xyz +
                        texture2D(inputTexture, fragTex.xy + dir * (3.0/3.0 - 0.5)).xyz);
                    float lumaB = dot(rgbB, luma);

                    if((lumaB < lumaMin) || (lumaB > lumaMax)){
                        color = vec4(rgbA, 1.0);
                    }else{
                        color = vec4(rgbB, 1.0);
                    }
                })"
            }
        });

        outputFBO.bind();

        auto resolvedShader = fxaaShader->getResolvedInstance({});
        resolvedShader.bind();
        resolvedShader.bindUniform("frameSize", glm::vec2(inputTexture.mWidth, inputTexture.mHeight));
        resolvedShader.bindTexture("inputTexture", inputTexture);

        glDisable(GL_DEPTH_TEST);
        Library::getMesh("quad").mMesh->draw();
        glEnable(GL_DEPTH_TEST);
    }
}