#include "AABBShader.hpp"

bool AABBShader::init(std::vector<Block *> *blocks) {
    /* Parent init */
    if (!blocks || !Shader::init()) {
        return false;
    }

    /* Set render target */
    this->blocks = blocks;

    addAllLocations();

    /* Create VAO */
    glGenVertexArrays(1, &vaoId);
    glBindVertexArray(vaoId);

    /* VBO */
    glGenBuffers(1, &vertexId);
    glBindBuffer(GL_ARRAY_BUFFER, vertexId);

    return true;
}

void AABBShader::addAllLocations() {
    // TODO
}

void AABBShader::setGlobals(const glm::mat4 *projection, const glm::mat4 *view) {
    loadP(projection);
    loadV(view);
}

void AABBShader::render(const World *world) {
    for (auto &b: *blocks) {
        AABB boundingBox = b->boundingBox;
        vertices[ 0] = boundingBox.worldMin.x; vertices[ 1] = boundingBox.worldMin.y; vertices[ 2] = boundingBox.worldMin.z;
        vertices[ 3] = boundingBox.worldMax.x; vertices[ 4] = boundingBox.worldMin.y; vertices[ 5] = boundingBox.worldMin.z;
        vertices[ 6] = boundingBox.worldMax.x; vertices[ 7] = boundingBox.worldMax.y; vertices[ 8] = boundingBox.worldMin.z;
        vertices[ 9] = boundingBox.worldMax.x; vertices[10] = boundingBox.worldMin.y; vertices[11] = boundingBox.worldMax.z;
        vertices[12] = boundingBox.worldMax.x; vertices[13] = boundingBox.worldMax.y; vertices[14] = boundingBox.worldMax.z;
        vertices[15] = boundingBox.worldMin.x; vertices[16] = boundingBox.worldMin.y; vertices[17] = boundingBox.worldMax.z;
        vertices[18] = boundingBox.worldMin.x; vertices[19] = boundingBox.worldMax.y; vertices[20] = boundingBox.worldMax.z;
        vertices[21] = boundingBox.worldMin.x; vertices[22] = boundingBox.worldMax.y; vertices[23] = boundingBox.worldMin.z;
        glBindVertexArray(vaoId);
        glBufferData(GL_ARRAY_BUFFER, 24, &vertices[0], GL_STATIC_DRAW);

        int pos = getAttribute("vertexPos");
        glEnableVertexAttribArray(pos);
        glBindBuffer(GL_ARRAY_BUFFER, vertexId);
        glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArrays(GL_TRIANGLES, 0, 8);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // TODO : unb->nd things 
    }
}

void AABBShader::cleanUp() {
    Shader::cleanUp();
}

void AABBShader::loadP(const glm::mat4 *p) {
    this->loadMat4(getUniform("P"), p);
}

void AABBShader::loadV(const glm::mat4 *v) {
    this->loadMat4(getUniform("V"), v);
}