#pragma once

#include "ECS/Component/RenderingComponent/ShaderComponent.hpp"

#include "Loader/Library.hpp"

using namespace neo;

struct GBufferShaderComponent : public ShaderComponent {
    GBufferShaderComponent() : ShaderComponent({
        Library::createSourceShader("GBufferShader", SourceShader::ConstructionArgs{
            { ShaderStage::VERTEX, "sponza/gbuffer.vert"},
            { ShaderStage::FRAGMENT, "sponza/gbuffer.frag" }
        })
        })
    {}

    virtual std::string getName() const override {
        return "GBufferShaderComponent";
    }
};