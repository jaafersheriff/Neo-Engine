
#extension GL_EXT_shader_io_blocks : enable
layout( std140, binding=0 ) buffer Pos {
    vec4 pos[];
};
uniform mat4 P, V, M;

out vec4 fragCol;
out vec2 fragTex;

void main() {

    int particleID = gl_VertexID >> 2;
    vec4 particlePos = pos[particleID];

    fragCol = vec4(0.5, 0.2, 0.1, 1.0);

    vec2 quadPos = vec2( ((gl_VertexID - 1) & 2) >> 1, (gl_VertexID % 2) >> 1);

    vec4 particleEye = M * V * particlePos;
    vec4 vertEyePos = particleEye + vec4((quadPos * 2.0 - 1.0) * 0.015f, 0.0, 0.0);

    fragTex = quadPos;

    gl_Position = P * vertEyePos;
    // gl_Position = P * V * vec4(particlePos.xyz, 1.0);
}
