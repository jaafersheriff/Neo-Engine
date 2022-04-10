
in vec4 mock_gl_Position;

uniform sampler3D volume;

out vec4 color;

// ivec3 getVoxelIndex() {
// }

void main() {

    color = vec4(mock_gl_Position);
}
