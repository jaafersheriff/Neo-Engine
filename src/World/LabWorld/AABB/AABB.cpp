#include "AABB.hpp"

AABB::AABB() {
    this->min = this->max = this->worldMin = this->worldMax = glm::vec3(0.f);
    this->entity = nullptr;
}

AABB::AABB(Entity *entity) {
    Mesh *m = entity->mesh;
    if (!m) {
        AABB();
        return;
    }
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

    this->min = this->worldMin = glm::vec3(minX, minY, minZ);
    this->max = this->worldMax = glm::vec3(maxX, maxY, maxZ);
    this->entity = entity;
}

bool AABB::intersect(const AABB &other) {
    // TODO : use entity->M
    // this->worldMin = M * min
    // this->worldMax = M * max
    return (worldMin.x <= other.worldMax.x && worldMax.x >= other.worldMin.x) &&
           (worldMin.y <= other.worldMax.y && worldMax.y >= other.worldMin.y) &&
           (worldMin.z <= other.worldMax.z && worldMax.z >= other.worldMin.z);
}