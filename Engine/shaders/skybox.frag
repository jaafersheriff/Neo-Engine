in vec3 fragTex;

#ifdef EQUIRECTANGULAR
layout(binding = 0) uniform sampler2D cubeMap;
#else
layout(binding = 0) uniform samplerCube cubeMap;
#endif

out vec4 color;

// https://learnopengl.com/PBR/IBL/Diffuse-irradiance
const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main() {

#ifdef EQUIRECTANGULAR
	color = texture(cubeMap, SampleSphericalMap(normalize(fragTex)));
#else
	color = texture(cubeMap, fragTex);
#endif
}