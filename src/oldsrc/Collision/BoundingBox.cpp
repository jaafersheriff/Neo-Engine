#include "BoundingBox.hpp"

BoundingBox::BoundingBox() {
    this->min = this->worldMin = glm::vec3(-1.f);
    this->max = this->worldMax = glm::vec3(1.f);
}

BoundingBox::BoundingBox(Mesh *mesh) {
    BoundingBox();
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
}

void BoundingBox::update(glm::mat4 &M) {
    this->worldMin = glm::vec3(M * glm::vec4(min, 1.0));
    this->worldMax = glm::vec3(M * glm::vec4(max, 1.0));
}

bool BoundingBox::intersect(const BoundingBox &other) {
    return (this->worldMin.x <= other.worldMax.x && this->worldMax.x >= other.worldMin.x) &&
           (this->worldMin.y <= other.worldMax.y && this->worldMax.y >= other.worldMin.y) &&
           (this->worldMin.z <= other.worldMax.z && this->worldMax.z >= other.worldMin.z);
}
