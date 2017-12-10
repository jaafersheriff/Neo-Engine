#include "Sun.hpp"

Sun::Sun(Texture *t) : 
    Billboard(t, glm::vec3(0.f, 0.f, 0.f), glm::vec2(t->width, t->height))
{}

Sun::Sun(Texture *t, float scale) :
    Sun(t) {
    this->size *= scale;
}

void Sun::update(const Camera &camera, const Light &light) {
    this->center = light.position;
}
