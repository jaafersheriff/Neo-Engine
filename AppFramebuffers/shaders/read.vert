
layout(location = 0) in vec3 vertPos;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out vec2 fragTex;

void main() {
    gl_Position = P * V * M * vec4(vertPos, 1);
    fragTex = vertPos.xy + 0.5;
    fragTex.x = -(fragTex.x-1); // flip x to mimic a camera
}