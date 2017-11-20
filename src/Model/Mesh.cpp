#include "Mesh.hpp"
#include <GL/glew.h>
#include <cassert>

/* Constructor */
Mesh::Mesh() : 
    vaoId(0), 
    vertBufId(0), 
    norBufId(0), 
    texBufId(0), 
    eleBufId(0) 
{
}

void Mesh::init() {

    /* Initialize VAO */
    glGenVertexArrays(1, &vaoId);
    glBindVertexArray(vaoId);

    /* Copy vertex array */
    glGenBuffers(1, &vertBufId);
    glBindBuffer(GL_ARRAY_BUFFER, vertBufId);
    glBufferData(GL_ARRAY_BUFFER, vertBuf.size()*sizeof(float), &vertBuf[0], GL_STATIC_DRAW);

    /* Copy element array if it exists */
    if (!eleBuf.empty()) {
        glGenBuffers(1, &eleBufId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufId);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf.size()*sizeof(unsigned int), &eleBuf[0], GL_STATIC_DRAW);
    }

    /* Copy normal array if it exists */
    if (!norBuf.empty()) {
        glGenBuffers(1, &norBufId);
        glBindBuffer(GL_ARRAY_BUFFER, norBufId);
        glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);
    }

    /* Copy texture array if it exists */
    if (!texBuf.empty()) {
        glGenBuffers(1, &texBufId);
        glBindBuffer(GL_ARRAY_BUFFER, texBufId);
        glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);
    }

    /* Unbind  */
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    /* Error check */
    assert(glGetError() == GL_NO_ERROR);
}
