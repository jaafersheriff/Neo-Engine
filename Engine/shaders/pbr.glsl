#pragma once

#define DIALECTRIC_REFLECTANCE 0.04 

#include "pbrtypes.glsl"

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

float D_GGX(float NoH, float roughness) {
    float a = NoH * roughness;
    float k = roughness / (1.0 - NoH * NoH + a * a);
    return k * k * (1.0 / PI);
}

vec3 F_Schlick(float u, vec3 f0) {
    float f = pow(1.0 - u, 5.0);
    return f + f0 * (1.0 - f);
}

void brdf(in PBRMaterial pbrMaterial, in PBRLight pbrLight, out PBRColor pbrColor) {
	vec3 H = normalize(pbrLight.L + pbrMaterial.V);
    float NdotH = clamp(dot(pbrMaterial.N, H), 0.0, 1.0);
    float NdotV = abs(dot(pbrMaterial.N, pbrMaterial.V)) + 1e-5;
    float NdotL = clamp(dot(pbrMaterial.N, pbrLight.L), 0.0, 1.0);
    float LdotH = clamp(dot(pbrLight.L, H), 0.0, 1.0);

    float roughness = pbrMaterial.linearRoughness * pbrMaterial.linearRoughness;

    float D = D_GGX(NdotH, roughness);
    vec3  F = F_Schlick(LdotH, pbrMaterial.F0);
    float G = GeometrySmith(NdotV, NdotL, roughness);

    vec3 Ks = F;
    vec3 Kd = (vec3(1.0) - Ks) * (1.0 - pbrMaterial.metalness);

    pbrColor.directDiffuse += (Kd * pbrMaterial.albedo / PI) * NdotL * pbrLight.radiance;
    pbrColor.directSpecular += ((D * F * G) / (4 * NdotV * dot(pbrMaterial.N, pbrLight.L))) * NdotL * pbrLight.radiance;
}
