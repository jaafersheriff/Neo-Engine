in vec3 fragTex;
layout(binding = 0) uniform samplerCube cubeMap;
out vec4 color;
void main() {
	color = texture(cubeMap, fragTex);
}