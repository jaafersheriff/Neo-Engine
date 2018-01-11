#include "AABB.hpp"

AABB::AABB() {
    this->min = glm::vec3(0.f);
    this->max = glm::vec3(0.f);
    this->position = glm::vec3(0.f);
}

AABB::AABB(Mesh *mesh) {
    float minX, minY, minZ;
    float maxX, maxY, maxZ;
    minX = minY = minZ = 1.1754E+38F;
    maxX = maxY = maxZ = -1.1754E+38F;

    //Go through all vertices to determine min and max of each dimension
    for (size_t v = 0; v < mesh->vertBuf.size() / 3; v++) {
        if (mesh->vertBuf[3 * v + 0] < minX) minX = mesh->vertBuf[3 * v + 0];
        if (mesh->vertBuf[3 * v + 0] > maxX) maxX = mesh->vertBuf[3 * v + 0];

        if (mesh->vertBuf[3 * v + 1] < minY) minY = mesh->vertBuf[3 * v + 1];
        if (mesh->vertBuf[3 * v + 1] > maxY) maxY = mesh->vertBuf[3 * v + 1];

        if (mesh->vertBuf[3 * v + 2] < minZ) minZ = mesh->vertBuf[3 * v + 2];
        if (mesh->vertBuf[3 * v + 2] > maxZ) maxZ = mesh->vertBuf[3 * v + 2];
    }

    this->min = glm::vec3(minX, minY, minZ);
    this->max = glm::vec3(maxX, maxY, maxZ);
    this->position = glm::vec3(0.f);
}

AABB::AABB(Mesh *m, const glm::vec3 pos) :
    AABB(m) {
    this->position = pos;
}

AABB::AABB(const glm::vec3 min, const glm::vec3 max) {
    this->min = min;
    this->max = max;
    this->position = glm::vec3(0.f);
}

AABB::AABB(const glm::vec3 min, const glm::vec3 max, const glm::vec3 pos) {
    this->min = min;
    this->max = max;
    this->position = pos;
}

bool AABB::intersect(const AABB &other) {
    glm::vec3 aMin = this->min + this->position;
    glm::vec3 aMax = this->max + this->position;
    glm::vec3 bMin = other.min + other.position;
    glm::vec3 bMax = other.min + other.position;
    return (aMin.x <= bMax.x && aMax.x >= bMin.x) &&
           (aMin.y <= bMax.y && aMax.y >= bMin.y) &&
           (aMin.z <= bMax.z && aMax.z >= bMin.z);
}