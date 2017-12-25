#include "Sun.hpp"

Sun::Sun(Texture *t, float scale) : 
    Billboard(t, glm::vec3(0.f, 0.f, 0.f), glm::vec2(t->width, t->height)*scale) {
}

Sun::Sun(glm::vec3 c1, glm::vec3 c2, float in, float out) :
    Billboard(glm::vec3(0.f, 0.f, 0.f), glm::vec2(out)) {
    this->innerColor = c1;
    this->outerColor = c2;
    this->innerRadius = in;
    this->outerRadius = out;
}

void Sun::update(const Light *light) {
    this->center = light->position;
}

void Sun::updateInnerRadius(const float in) {
    if (innerRadius + in >= 0 && innerRadius + in <= outerRadius) {
        innerRadius += in;
    }
}

void Sun::updateOuterRadius(const float in) {
    if (outerRadius + in <= innerRadius) {
        updateInnerRadius(in);
    }
    if (outerRadius += in >= 0) {
        outerRadius += in;
    }
    this->size = glm::vec2(outerRadius);
}
