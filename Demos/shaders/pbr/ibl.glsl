
#include "pbr/pbrtypes.glsl"

vec3 getIndirectSpecular(vec3 R, PBRMaterial pbrMaterial, int numMips, sampler2D dfgLUT, samplerCube ibl) {
    vec2 f_ab = texture(dfgLUT, vec2(saturate(dot(pbrMaterial.N, pbrMaterial.V)), pbrMaterial.linearRoughness)).rg;
    vec3 radiance = textureLod(ibl, R, pbrMaterial.linearRoughness * numMips - 0.2).rgb; // -0.2 so that at least some things get 0th mip
    return (pbrMaterial.F0 * f_ab.x + f_ab.y) * radiance;
}

float GeometrySchlickGGXIBL(float NdotV, float roughness) {
    float k = (roughness * roughness) / 2.0; // TODO - why do we use a different k here?
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return num / denom;
}

float GeometrySmithIBL(float NdotV, float NdotL, float roughness) {
    float ggx2 = GeometrySchlickGGXIBL(NdotV, roughness);
    float ggx1 = GeometrySchlickGGXIBL(NdotL, roughness);
    return ggx1 * ggx2;
}

// Taken from https://github.com/SaschaWillems/Vulkan-glTF-PBR/blob/master/data/shaders/genbrdflut.frag
// Based on http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
vec2 hammersley(uint i, uint N)
{
    uint bits = (i << 16u) | (i >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    float rdi = float(bits) * 2.3283064365386963e-10;
    return vec2(float(i) /float(N), rdi);
}

// Based on Karis 2014
vec3 importanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float a = roughness * roughness;
    // Sample in spherical coordinates
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    // Construct tangent space vector
    vec3 H;
    H.x = sinTheta * cos(phi);
    H.y = sinTheta * sin(phi);
    H.z = cosTheta;

    // Tangent to world space
    vec3 upVector = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangentX = normalize(cross(upVector, N));
    vec3 tangentY = cross(N, tangentX);
    return tangentX * H.x + tangentY * H.y + N * H.z;
}
