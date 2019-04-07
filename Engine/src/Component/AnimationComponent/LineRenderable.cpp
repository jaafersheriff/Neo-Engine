#include "LineRenderable.hpp"
#include "Shader/LineShader.hpp"

void LineRenderable::init() {
    mMesh->upload(GL_LINE_STRIP);
    addShaderType<LineShader>();
    RenderableComponent::init();
}