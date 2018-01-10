#include "SunShader.hpp"

bool SunShader::init(Sun *sun) {
    /* Parent init */
    if (!sun || !Shader::init()) {
        return false;
    }

    /* Set render target */
    this->sun = sun;

    addAllLocations();

    return true;
}

void SunShader::addAllLocations() {
    /* Attributes */
    addAttribute("vertexPos");

    /* Projection and view */
    addUniform("P");
    addUniform("V");

    /* Billboard things */
    addUniform("center");
    addUniform("size");

    /* Texture */
    addUniform("usesTexture");
    addUniform("textureImage");

    /* Untextured */
    addUniform("innerColor");
    addUniform("outerColor");
    addUniform("innerRadius");
    addUniform("outerRadius");
}

void SunShader::setGlobals(const glm::mat4 *projection, const glm::mat4 *view) {
    loadP(projection);
    loadV(view);
}

void SunShader::render(const World *world) {
    glDisable(GL_DEPTH_TEST);

    /* Bind vertices */
    glBindVertexArray(sun->mesh->vaoId);
    int pos = getAttribute("vertexPos");
    glEnableVertexAttribArray(pos);
    glBindBuffer(GL_ARRAY_BUFFER, sun->mesh->vertBufId);
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

    /* Load billboard's position and size */
    loadCenter(sun->center);
    loadSize(sun->size);

    if(sun->texture) {
        loadUsesTexture(true);
        loadTexture(sun->texture);
        glActiveTexture(GL_TEXTURE0 + sun->texture->textureId);
        glBindTexture(GL_TEXTURE_2D, sun->texture->textureId);
    }
    else {
        loadUsesTexture(false);
        loadInnerColor(sun->innerColor);
        loadOuterColor(sun->outerColor);
        loadInnerRadius(sun->innerRadius);
        loadOuterRadius(sun->outerRadius);
    }

    /* Draw */
    glDrawArrays(GL_TRIANGLE_STRIP, 0, sun->mesh->vertBuf.size() / 3);

    /* Unbind */
    glDisableVertexAttribArray(getAttribute("vertexPos"));
    Shader::unloadMesh();
    if (sun->texture) {
        Shader::unloadTexture(sun->texture->textureId);
    }
    glDisable(GL_DEPTH_TEST);
}

void SunShader::cleanUp() {
    Shader::cleanUp();
}

void SunShader::loadP(const glm::mat4 *p) {
    this->loadMat4(getUniform("P"), p);
}

void SunShader::loadV(const glm::mat4 *v) {
    this->loadMat4(getUniform("V"), v);
}

void SunShader::loadCenter(const glm::vec3 c) {
    this->loadVec3(getUniform("center"), c);
}

void SunShader::loadSize(const glm::vec2 s) {
    this->loadVec2(getUniform("size"), s);
}

void SunShader::loadTexture(const Texture *texture) {
    this->loadInt(getUniform("textureImage"), texture->textureId);
}

void SunShader::loadUsesTexture(bool b) {
    this->loadBool(getUniform("usesTexture"), b);
}

void SunShader::loadInnerColor(glm::vec3 col) {
    this->loadVec3(getUniform("innerColor"), col);
}

void SunShader::loadOuterColor(glm::vec3 col) {
    this->loadVec3(getUniform("outerColor"), col);
}

void SunShader::loadInnerRadius(float in) {
    this->loadFloat(getUniform("innerRadius"), in);
}

void SunShader::loadOuterRadius(float out) {
    this->loadFloat(getUniform("outerRadius"), out);
}