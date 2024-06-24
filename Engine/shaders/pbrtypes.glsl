
struct PBRMaterial {
    vec3 albedo;
    vec3 N;
    vec3 V;
    float linearRoughness;
    float metalness;
    vec3 F0;
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

