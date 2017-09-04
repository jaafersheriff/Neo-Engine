#include "TriangleRenderer.hpp"
#include <iostream>

void TriangleRenderer::activate(Triangle *t) {
   this->tri = t;  
   shader = new TriangleShader;
   shader->init();
}

void TriangleRenderer::setGlobals() {

}

void TriangleRenderer::render() {
   shader->bind();
   glBindVertexArray(tri->vaoId);
   int p_pos = shader->getAttribute("vPos");
   glEnableVertexAttribArray(p_pos);
   glBindBuffer(GL_ARRAY_BUFFER, tri->posId);
   glVertexAttribPointer(p_pos, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);

   int p_col = shader->getAttribute("vCol");
   glEnableVertexAttribArray(p_col);
   glBindBuffer(GL_ARRAY_BUFFER, tri->colId);
   glVertexAttribPointer(p_col, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

   shader->loadMVP(tri->mvp);

   glDrawArrays(GL_TRIANGLES, 0, 3);
   shader->unbind();
}