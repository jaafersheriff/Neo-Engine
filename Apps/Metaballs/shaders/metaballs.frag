in vec3 fragPos;
in vec3 fragNor;

uniform bool wireframe;
uniform vec3 camPos;

uniform samplerCube cubeMap;

out vec4 color;

void main() {
    if (wireframe) {
        color = vec4(1);
    }
    else {
        vec3 V = normalize(fragPos - camPos);
        vec3 N = normalize(fragNor);

	    vec4 refl = texture(cubeMap, reflect(V, N));

        float index = 1.333;
	    float d = index * dot(V, N);
	    float k = clamp(d * d + 1.0 - index * index, 0.0, 1.0);

	    vec4 refr = texture(cubeMap, index * V - (d + sqrt(k)) * N);

        color = mix(refl, refr, k);
    }
}