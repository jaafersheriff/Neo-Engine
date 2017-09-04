#include "MasterRenderer.hpp"

void MasterRenderer::activateTriangleRenderer(Triangle *t) {
   TriangleRenderer *tR = new TriangleRenderer;
   tR->activate(t);
   renderers.push_back(tR);
}

void MasterRenderer::render() {
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   std::vector<Renderer *>::const_iterator renderer;
   for (renderer = renderers.begin(); renderer != renderers.end(); renderer++) {
      (*renderer)->setGlobals();
      (*renderer)->render();
   }
}