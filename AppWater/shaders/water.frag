
#define EPSILON 0.00001
in vec4 fragPos;
in vec4 fragWorldPos;
in vec4 fragWorldViewPos;
in vec3 fragNormal;
in vec3 fragTangent;
in vec3 fragBinormal;
in vec4 fragTex;
in vec4 fragWorldNormalAndHeight;

uniform float time;
uniform vec4 normalMapScrollDir;
uniform vec2 normalMapScrollSpeed;
uniform sampler2D WaterNormalMap1;
uniform sampler2D WaterNormalMap2;

uniform vec3 lightPos;
// uniform vec2 reflectanceFactor;
uniform float shine;

out vec4 color;

void main() {
    // Calculate normals
    vec3 N = normalize(fragNormal);
    vec3 T = normalize(fragTangent);
    vec3 B = normalize(fragBinormal);
    mat3 texSpace = mat3(T, B, N);
 
    vec2 normalMapCoords1 = fragTex.xy + time * normalMapScrollDir.xy * normalMapScrollSpeed.x;
    vec2 normalMapCoords2 = fragTex.xy + time * normalMapScrollDir.zw * normalMapScrollSpeed.y;
    vec2 hdrCoords = ((vec2(fragPos.x, -fragPos.y) / fragPos.w) * 0.5) + 0.5;
 
    vec3 normalMap = texture2D(WaterNormalMap1, normalMapCoords1).rgb * 2.0 - 1.0; // LinearWrap sample
    vec3 normalMap2 = texture2D(WaterNormalMap2, normalMapCoords2).rgb * 2.0 - 1.0; //LinearWrap sample
    vec3 finalNormal = normalize(texSpace * normalMap.xyz);
    finalNormal += normalize(texSpace * normalMap2.xyz);
    finalNormal = normalize(finalNormal);

    // Calculate specular
    vec3 lightDir = lightPos - fragWorldPos.xyz;
    vec3 L = -normalize(lightDir);
    vec3 V = -normalize(fragWorldViewPos.xyz);
    vec3 H = normalize(V + L);
    /*
    float linearRoughness = roughness * roughness;
    float nDotL = clamp(dot(finalNormal, L), 0.0, 1.0);
    float nDotV = abs(dot(finalNormal, V)) + EPSILON;
    float nDotH = clamp(dot(finalNormal, H), 0.0, 1.0);
    float lDotH = clamp(dot(L, H), 0.0, 1.0);
    vec3 f0 = reflectanceFactor.x * reflectance.y * reflectance.y;
    float normalDistribution = CalculateNormalDistributionGGX(linearRoughness, nDotH);
    vec3 fresnelReflectance = CalculateSchlickFresnelReflectance(lDotH, f0);
    float geometryTerm = CalculateSmithGGXGeometryTerm(linearRoughness, nDotL, nDotV);
    float specularNoise = WaterNoiseMap.Sample(LinearWrapSampler, normalMapCoords1 * 0.5).r;
    specularNoise *= WaterNoiseMap.Sample(LinearWrapSampler, normalMapCoords2 * 0.5).r;
    specularNoise *= WaterNoiseMap.Sample(LinearWrapSampler, input.texCoord0.xy * 0.5).r;
    vec3 specularFactor = (geometryTerm * normalDistribution) * fresnelReflectance * specIntensity * nDotL * specularNoise;
    */
    vec3 specularFactor = vec3(1.0) * pow(clamp(dot(H, N), 0.0, 1.0), shine);

    color = vec4(specularFactor, 1.0);
} 