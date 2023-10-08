#pragma once

#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"

#include "Renderer/Shader/PostProcessShader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "Loader/Library.hpp"
#include "Engine/Engine.hpp"

namespace neo {

    class FXAAShader : public PostProcessShader {

    public:

        FXAAShader() :
            PostProcessShader("FXAAShader", R"(
                #include "postprocess.glsl"
                uniform sampler2D inputTexture;
                uniform vec2 frameSize;
                out vec4 color;
                void main() {

                    float FXAA_SPAN_MAX = 8.0;
                    float FXAA_REDUCE_MUL = 1.0/8.0;
                    float FXAA_REDUCE_MIN = 1.0/128.0;

                    vec3 rgbNW=texture2D(inputFBO,fragTex+(vec2(-1.0,-1.0)/frameSize)).xyz;
                    vec3 rgbNE=texture2D(inputFBO,fragTex+(vec2( 1.0,-1.0)/frameSize)).xyz;
                    vec3 rgbSW=texture2D(inputFBO,fragTex+(vec2(-1.0, 1.0)/frameSize)).xyz;
                    vec3 rgbSE=texture2D(inputFBO,fragTex+(vec2( 1.0, 1.0)/frameSize)).xyz;
                    vec3 rgbM= texture2D(inputFBO,fragTex).xyz;

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
                        texture2D(inputFBO, fragTex.xy + dir * (1.0/3.0 - 0.5)).xyz +
                        texture2D(inputFBO, fragTex.xy + dir * (2.0/3.0 - 0.5)).xyz);
                    vec3 rgbB = rgbA * (1.0/2.0) + (1.0/4.0) * (
                        texture2D(inputFBO, fragTex.xy + dir * (0.0/3.0 - 0.5)).xyz +
                        texture2D(inputFBO, fragTex.xy + dir * (3.0/3.0 - 0.5)).xyz);
                    float lumaB = dot(rgbB, luma);

                    if((lumaB < lumaMin) || (lumaB > lumaMax)){
                        color = vec4(rgbA, 1.0);
                    }else{
                        color = vec4(rgbB, 1.0);
                    }
            })")
        {}

         void render(const ECS& ecs) override {
             glm::vec2 size = std::get<1>(*ecs.cGetComponent<ViewportDetailsComponent>()).mSize;
             loadUniform("frameSize", size);
         }
         void imguiEditor() override {
         }
    };
}