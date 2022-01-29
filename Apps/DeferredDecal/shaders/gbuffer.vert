
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;

uniform mat4 P, V, M;
uniform mat3 N;

out vec4 fragPos;
out vec3 fragNor;
out vec2 fragTex;

void main() {
    fragPos = M * vec4(vertPos, 1.0);
    fragNor = N * vertNor;
    fragTex = vertTex;
    gl_Position = P * V * fragPos;
}