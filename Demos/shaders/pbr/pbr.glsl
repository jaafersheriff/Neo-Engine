
#define PI 3.141592653589

struct PBRData {
    vec3 albedo;
    vec3 N;
    vec3 V;
    vec3 L;
    float linearRoughness;
    float metalness;
};

float V_SmithGGXCorrelated(float NoV, float NoL, float roughness) {
    float a2 = roughness * roughness;
    float GGXV = NoL * sqrt(NoV * NoV * (1.0 - a2) + a2);
    float GGXL = NoV * sqrt(NoL * NoL * (1.0 - a2) + a2);
    return 0.5 / (GGXV + GGXL);
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

vec3 doPBR(in PBRData pbrData) {
	vec3 H = normalize(pbrData.L + pbrData.V);
    float NdotH = clamp(dot(pbrData.N, H), 0.0, 1.0);
    float LdotH = clamp(dot(pbrData.L, H), 0.0, 1.0);
    float NdotV = abs(dot(pbrData.N, pbrData.V)) + 1e-5;
    float NdotL = clamp(dot(pbrData.N, pbrData.L), 0.0, 1.0);

    float roughness = pbrData.linearRoughness * pbrData.linearRoughness;

    float D = D_GGX(NdotH, pbrData.linearRoughness);
    vec3  F = F_Schlick(LdotH, vec3(0.04));
    float V = V_SmithGGXCorrelated(NdotV, NdotL, roughness);

    vec3 Fr = D * V * F;

    vec3 diffuseColor = (1.0 - pbrData.metalness) * pbrData.albedo;
    vec3 Fd = diffuseColor * (1.0 / PI);

    return vec3(D);
}
