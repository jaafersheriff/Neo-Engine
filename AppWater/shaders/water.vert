
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec4 vertTex;

uniform mat4 M;

out vec3 controlPos;
out vec4 controlTex;

void main() {
    controlPos = vec3(M * vec4(vertPos, 1.0));
    controlTex = vertTex;
}