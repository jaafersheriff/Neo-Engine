#include "CloudBillboard.hpp"

CloudBillboard::CloudBillboard(const glm::vec3 c, const glm::vec2 s) :
    Billboard(c, s)
{}

CloudBillboard::CloudBillboard(Texture *t, const glm::vec3 c, const glm::vec2 s) :
    Billboard(t, c, s)
{}

void CloudBillboard::update(const Camera &camera) {
    this->distance = glm::distance(this->center, camera.position);
}