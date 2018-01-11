#include "AABB.hpp"

AABB::AABB() {
    this->min = glm::vec3(0.f);
    this->max = glm::vec3(0.f);
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
}

AABB::AABB(const glm::vec3 min, const glm::vec3 max) {
    this->min = min;
    this->max = max;
}

bool AABB::intersect(const AABB &other) {
    return (this->min.x <= other.max.x && this->max.x >= other.min.x) &&
           (this->min.y <= other.max.y && this->max.y >= other.min.y) &&
           (this->min.z <= other.max.z && this->max.z >= other.min.z);
}