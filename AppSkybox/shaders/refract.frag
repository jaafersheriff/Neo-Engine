
in vec4 fragPos;
in vec3 fragNor;

uniform vec3 camPos;
uniform samplerCube cubeMap;
uniform float ratio;

out vec4 color;

void main() {             
    vec3 I = normalize(fragPos.xyz - camPos);
    vec3 R = refract(I, normalize(fragNor), ratio);
    color = vec4(texture(cubeMap, R).rgb, 1.0);
}