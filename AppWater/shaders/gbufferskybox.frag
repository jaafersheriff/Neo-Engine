in vec3 fragTex;

uniform samplerCube cubeMap;

layout (location = 2) out vec4 gDiffuse;

void main() {
    gDiffuse = vec4(texture(cubeMap, fragTex).rgb, 1.0);
}
