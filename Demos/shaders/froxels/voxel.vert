layout(location = 0) in vec3 vertPos;
layout(location = 3) in vec3 voxelPosition;
layout(location = 4) in vec4 voxelData;

uniform mat4 P;
uniform mat4 V;
uniform float voxelSize;

out vec4 voxData;

void main() {
    mat4 M = mat4(1.f);
    M[0][0] = voxelSize;
    M[1][1] = voxelSize;
    M[2][2] = voxelSize;
    M[3][3] = 1.f;
    M[3][0] = voxelPosition.x;
    M[3][1] = voxelPosition.y;
    M[3][2] = voxelPosition.z;
    gl_Position = P * V * M * vec4(vertPos, 1.f);
    voxData = voxelData;
}