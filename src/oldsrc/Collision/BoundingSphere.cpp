#include "BoundingSphere.hpp"
#include "BoundingBox.hpp"

BoundingSphere::BoundingSphere() {
    this->position = glm::vec3(0.f, 0.f, 0.f);
    this->radius = 0.f;
}

BoundingSphere::BoundingSphere(float size) : 
    BoundingSphere() {
    this->radius = size;
}

BoundingSphere::BoundingSphere(Mesh *mesh) :    
    BoundingSphere() {

    BoundingBox box(mesh);
    this->radius = glm::distance(box.max, box.min);
}

void BoundingSphere::update(glm::vec3 &pos) {
    this->position = pos;
}

bool BoundingSphere::intersect(const BoundingSphere &other) {
    return sqrt((this->position.x - other.position.x) * (this->position.x - other.position.x) +
                (this->position.y - other.position.y) * (this->position.y - other.position.y) +
                (this->position.z - other.position.z) * (this->position.z - other.position.z))
           < (this->radius + other.radius);
}