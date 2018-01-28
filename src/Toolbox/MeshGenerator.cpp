#include "MeshGenerator.hpp"
#include "Toolbox.hpp"

#include "Collision/BoundingBox.hpp"

Mesh* MeshGenerator::generateCube(float scale) {
    Mesh *mesh = new Mesh;
    mesh->vertBuf = { 
            -scale,  scale, -scale,
            -scale, -scale, -scale,
             scale, -scale, -scale,
             scale, -scale, -scale,
             scale,  scale, -scale,
            -scale,  scale, -scale,
            
            -scale, -scale,  scale,
            -scale, -scale, -scale,
            -scale,  scale, -scale,
            -scale,  scale, -scale,
            -scale,  scale,  scale,
            -scale, -scale,  scale,
            
             scale, -scale, -scale,
             scale, -scale,  scale,
             scale,  scale,  scale,
             scale,  scale,  scale,
             scale,  scale, -scale,
             scale, -scale, -scale,
            
            -scale, -scale,  scale,
            -scale,  scale,  scale,
             scale,  scale,  scale,
             scale,  scale,  scale,
             scale, -scale,  scale,
            -scale, -scale,  scale,
            
            -scale,  scale, -scale,
             scale,  scale, -scale,
             scale,  scale,  scale,
             scale,  scale,  scale,
            -scale,  scale,  scale,
            -scale,  scale, -scale,
            
            -scale, -scale, -scale,
            -scale, -scale,  scale,
             scale, -scale, -scale,
             scale, -scale, -scale,
            -scale, -scale,  scale,
             scale, -scale,  scale
        };

    mesh->init();
    return mesh;
}

Mesh* MeshGenerator::generateSphere(int smoothness) {
    Mesh *mesh = new Mesh;

    /* Vertices */
    float t = (float) ((1.0 + sqrt(3.0)) / 2.0);
    mesh->vertBuf = std::vector<float>{
        -1,  t,  0,
         1,  t,  0,
        -1, -t,  0,
         1, -t,  0,

         0, -1,  t,
         0,  1,  t,
         0, -1, -t,
         0,  1, -t,

         t,  0, -1,
         t,  0,  1,
        -t,  0, -1,
        -t,  0,  1,
    };


    mesh->eleBuf = std::vector<unsigned int>{
        /* 3 faces around point 0 */
         0, 11,  5,
         0,  5,  1,
         0,  1,  7,
         0,  7, 10,
         0, 10, 11,
        /* 3 adjacent faces */
         1,  5,  9,
         5, 11,  4,
        11, 10,  2,
        10,  7,  6,
         7,  1,  8,
        /* 3 faces around point 3 */
         3,  9,  4,
         3,  4,  2,
         3,  2,  6,
         3,  6,  8,
         3,  8,  9,
        /* 3 adjacent faces */
         4,  9,  5,
         2,  4, 11,
         6,  2, 10,
         8,  6,  7,
         9,  8,  1
    };

    /* Add refinements */
    for (int i = 0; i < smoothness; i++) {
        std::vector<float> newVerts;
        for (int j = 0; j < mesh->vertBuf.size(); j += 9) {
            /* Load 3 vertices of a triangle */
            glm::vec3 v0(mesh->vertBuf[i    ], mesh->vertBuf[i + 1], mesh->vertBuf[i + 2]);
            glm::vec3 v1(mesh->vertBuf[i + 3], mesh->vertBuf[i + 4], mesh->vertBuf[i + 3]);
            glm::vec3 v2(mesh->vertBuf[i + 6], mesh->vertBuf[i + 7], mesh->vertBuf[i + 8]);
            /* Find normalized midpoints between each vertex */
            glm::vec3 v3 = glm::normalize(Toolbox::interpolate(v0, v1, 0.3f));
            glm::vec3 v4 = glm::normalize(Toolbox::interpolate(v0, v2, 0.3f));
            glm::vec3 v5 = glm::normalize(Toolbox::interpolate(v1, v2, 0.5f));
            /* Normalize original vertices */
            glm::normalize(v0);
            glm::normalize(v1);
            glm::normalize(v2);
            /* Store all normalized vertices */
            newVerts.push_back(v0.x); newVerts.push_back(v0.y); newVerts.push_back(v0.z);
            newVerts.push_back(v3.x); newVerts.push_back(v3.y); newVerts.push_back(v3.z);
            newVerts.push_back(v4.x); newVerts.push_back(v4.y); newVerts.push_back(v4.z);
            newVerts.push_back(v4.x); newVerts.push_back(v4.y); newVerts.push_back(v4.z);
            newVerts.push_back(v2.x); newVerts.push_back(v2.y); newVerts.push_back(v2.z);
            newVerts.push_back(v5.x); newVerts.push_back(v5.y); newVerts.push_back(v5.z);
            newVerts.push_back(v3.x); newVerts.push_back(v3.y); newVerts.push_back(v3.z);
            newVerts.push_back(v4.x); newVerts.push_back(v4.y); newVerts.push_back(v4.z);
            newVerts.push_back(v5.x); newVerts.push_back(v5.y); newVerts.push_back(v5.z);
            newVerts.push_back(v3.x); newVerts.push_back(v3.y); newVerts.push_back(v3.z);
            newVerts.push_back(v1.x); newVerts.push_back(v1.y); newVerts.push_back(v1.z);
            newVerts.push_back(v5.x); newVerts.push_back(v5.y); newVerts.push_back(v5.z);
        }
        mesh->vertBuf = newVerts;
    }

    resize(mesh);
    mesh->init();

    return mesh;
}

/* Resize a mesh so all vertex positions are [0, 1.f] */
void MeshGenerator::resize(Mesh *mesh) {
    float scaleX, scaleY, scaleZ;
    float shiftX, shiftY, shiftZ;
    float epsilon = 0.001f;

    /* Find BoundingBox from mesh */
    BoundingBox boundingBox(mesh);

    //From min and max compute necessary scale and shift for each dimension
    float maxExtent, xExtent, yExtent, zExtent;
    xExtent = boundingBox.max.x-boundingBox.min.x;
    yExtent = boundingBox.max.y-boundingBox.min.y;
    zExtent = boundingBox.max.z-boundingBox.min.z;
    
    if (xExtent >= yExtent && xExtent >= zExtent) {
        maxExtent = xExtent;
    }
    if (yExtent >= xExtent && yExtent >= zExtent) {
        maxExtent = yExtent;
    }
    if (zExtent >= xExtent && zExtent >= yExtent) {
        maxExtent = zExtent;
    }
    scaleX = 2.f /maxExtent;
    shiftX = boundingBox.min.x + (xExtent/ 2.f);
    scaleY = 2.f / maxExtent;
    shiftY = boundingBox.min.y + (yExtent / 2.f);
    scaleZ = 2.f/ maxExtent;
    shiftZ = boundingBox.min.z + (zExtent)/2.f;

    //Go through all verticies shift and scale them
	for (size_t v = 0; v < mesh->vertBuf.size() / 3; v++) {
		mesh->vertBuf[3*v+0] = (mesh->vertBuf[3*v+0] - shiftX) * scaleX;
		assert(mesh->vertBuf[3*v+0] >= -1.0 - epsilon);
		assert(mesh->vertBuf[3*v+0] <= 1.0 + epsilon);
		mesh->vertBuf[3*v+1] = (mesh->vertBuf[3*v+1] - shiftY) * scaleY;
		assert(mesh->vertBuf[3*v+1] >= -1.0 - epsilon);
		assert(mesh->vertBuf[3*v+1] <= 1.0 + epsilon);
		mesh->vertBuf[3*v+2] = (mesh->vertBuf[3*v+2] - shiftZ) * scaleZ;
		assert(mesh->vertBuf[3*v+2] >= -1.0 - epsilon);
        assert(mesh->vertBuf[3*v+2] <= 1.0 + epsilon);
	}
}
