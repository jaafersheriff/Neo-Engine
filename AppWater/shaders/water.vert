
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec4 vertTex;

out vec3 controlPos;
out vec4 controlTex;

void main() {
    controlPos = vertPos;
    controlTex = vertTex;
}