
#define PI 3.141592653589

struct PBRMaterial {
    vec3 albedo;
    vec3 N;
    vec3 V;
    float linearRoughness;
    float metalness;
};

struct PBRLight {
    vec3 L;
    vec3 radiance;
};

struct PBRColor {
    vec3 directDiffuse;
    vec3 directSpecular;
    vec3 indirectDiffuse;
    vec3 indirectSpecular;
};

float V_SmithGGXCorrelated(float NoV, float NoL, float roughness) {
    float a2 = roughness * roughness;
    float GGXV = NoL * sqrt(NoV * NoV * (1.0 - a2) + a2);
    float GGXL = NoV * sqrt(NoL * NoL * (1.0 - a2) + a2);
    return 0.5 / (GGXV + GGXL + 1e-5);
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
    vec3 F0 = mix(vec3(0.04), pbrMaterial.albedo, vec3(pbrMaterial.metalness));

    float D = D_GGX(NdotH, roughness);
    vec3  F = F_Schlick(LdotH, F0);
    float V = V_SmithGGXCorrelated(NdotV, NdotL, roughness);

    vec3 Ks = F;
    vec3 Kd = (vec3(1.0) - Ks) * (1.0 - pbrMaterial.metalness);

    pbrColor.directDiffuse  += (Kd * pbrMaterial.albedo / PI) * NdotL * pbrLight.radiance;
    pbrColor.directSpecular += (D * F * V) * NdotL * pbrLight.radiance;
}
