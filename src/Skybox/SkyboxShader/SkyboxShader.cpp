#include "SkyboxShader.hpp"

#include "glm/gtc/matrix_transform.hpp"

bool SkyboxShader::init(Skybox *sb) {
    if (!sb || !Shader::init()) {
        return false;
    }

    /* Set render target */
    this->skybox = sb;

    /* Set enum type */
    this->type = MasterRenderer::ShaderTypes::SKYBOX_SHADER;

    addAllLocations();

    return true;
}

void SkyboxShader::addAllLocations() {
    /* Attributes */
    addAttribute("vertexPos");

    /* Matrix uniforms */
    addUniform("P");
    addUniform("V");

    /* Cube texture */
    addUniform("cubeMap");
}

void SkyboxShader::render(const World *world) {
    /* Bind vertices */
    glBindVertexArray(skybox->mesh->vaoId);
    int pos = getAttribute("vertexPos");
    glEnableVertexAttribArray(pos);
    glBindBuffer(GL_ARRAY_BUFFER, skybox->mesh->vertBufId);
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *) 0);

    /* Bind texture */
    loadCubeTexture(skybox->cubeTexture);
    glActiveTexture(GL_TEXTURE0 + skybox->cubeTexture->textureId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->cubeTexture->textureId);
    
    /* Draw */
    glDrawArrays(GL_TRIANGLES, 0, skybox->mesh->vertBuf.size() / 3);

    /* Unbind */
    glDisableVertexAttribArray(getAttribute("vertexPos"));
    Shader::unloadMesh();
    Shader::unloadTexture(skybox->cubeTexture->textureId);
}

void SkyboxShader::cleanUp() {
    Shader::cleanUp();
}

void SkyboxShader::loadCubeTexture(const CubeTexture *ct) {
    this->loadInt(getUniform("cubeMap"), ct->textureId);
}
