
layout(location = 0) in vec3 vertPos;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

void main() {
	gl_Position = P * V * M * vec4(vertPos, 1.f);
}