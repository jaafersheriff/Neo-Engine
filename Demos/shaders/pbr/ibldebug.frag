in vec3 fragTex;
layout(binding = 0) uniform samplerCube cubeMap;
uniform float mip;
out vec4 color;
void main() {
	color = textureLod(cubeMap, fragTex, mip);
}
