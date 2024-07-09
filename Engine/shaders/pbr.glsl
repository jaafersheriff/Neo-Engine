#define DIALECTRIC_REFLECTANCE 0.04 

#include "pbrtypes.glsl"

vec3 reconstructWorldPos(vec2 texCoord, float depth, mat4 invP, mat4 invV) {
    // Clip space
    vec3 ndc = vec3(texCoord, depth) * 2.0 - vec3(1.0);
    // View space
    vec4 view = invP * vec4(ndc, 1.0);

    // Perspective divide
    view.xyz /= view.w;

    // World space
    return (invV * vec4(view.xyz, 1.0)).xyz;
}

vec3 calculateIndirectDiffuse(vec3 albedo, float metalness, float ambient = 0.03) {
	return albedo.rgb * ambient * (1.0 - metalness);
}

vec3 calculateF0(vec3 albedo, float metalness) {
	return mix(vec3(DIALECTRIC_REFLECTANCE), albedo, vec3(metalness));
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return num / denom;
}

float GeometrySmith(float NdotV, float NdotL, float roughness) {
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

float D_GGX(float NdotH, float roughness) {
    float a = NdotH * roughness;
    float k = roughness / (1.0 - NdotH * NdotH + a * a + EP);
    return k * k * (1.0 / PI);
}

vec3 F_Schlick(float u, vec3 f0) {
    float f = pow(1.0 - u, 5.0);
    return f + f0 * (1.0 - f);
}

void brdf(in PBRMaterial pbrMaterial, in PBRLight pbrLight, out PBRColor pbrColor) {
	vec3 H = normalize(pbrLight.L + pbrMaterial.V);
    float NdotH = saturate(dot(pbrMaterial.N, H));
    float NdotV = abs(dot(pbrMaterial.N, pbrMaterial.V));
    float NdotL = saturate(dot(pbrMaterial.N, pbrLight.L));
    float LdotH = saturate(dot(pbrLight.L, H));

    float roughness = pbrMaterial.linearRoughness * pbrMaterial.linearRoughness;

    float D = D_GGX(NdotH, roughness);
    vec3  F = F_Schlick(LdotH, pbrMaterial.F0);
    float G = GeometrySmith(NdotV, NdotL, roughness);

    vec3 Ks = F;
    vec3 Kd = (vec3(1.0) - Ks) * (1.0 - pbrMaterial.metalness);

    pbrColor.directDiffuse += (Kd * pbrMaterial.albedo / PI) * NdotL * pbrLight.radiance * pbrMaterial.ao;
    pbrColor.directSpecular += ((D * F * G) / (4 * NdotV * dot(pbrMaterial.N, pbrLight.L))) * NdotL * pbrLight.radiance * pbrMaterial.ao;
}
