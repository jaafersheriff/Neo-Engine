
#define EPSILON 0.00001
#define PI 3.14159265359

in vec4 fragWorldPos;
in vec4 fragViewPos;
in vec4 fragScreenPos;
in vec3 fragViewNormal;
in vec3 fragViewTangent;
in vec3 fragViewBinormal;
in vec4 fragTex;
in vec4 fragNormalAndHeight;

uniform float time;
uniform vec4 normalMapScrollDir;
uniform vec2 normalMapScrollSpeed;
uniform sampler2D WaterNormalMap1;
uniform sampler2D WaterNormalMap2;

uniform vec3 lightPos;
uniform vec2 reflectanceFactor;
uniform float roughness;
uniform float specIntensity;

uniform sampler2D waterNoise;

uniform float refractionDistortionFactor;
uniform sampler2D gDiffuse; // this should be final light pass 
uniform sampler2D gDepth;
uniform mat4 invV;
uniform mat4 invP;
uniform float refractionHeightFactor;
uniform float refractionDistanceFactor;

uniform float depthSofteningDistance;

uniform vec3 reflectionColor;

uniform vec3 baseColor;

out vec4 color;

float CalculateNormalDistributionGGX(float a, float NdotH) {
    float a2     = a*a;
    float NdotH2 = NdotH*NdotH;
	
    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;
	
    return nom / denom;
}

float CalculateSchlickFresnelReflectance(vec3 H, vec3 V, float F0 ) {
  float base = 1.0 - dot( V, H );
  float exponential = pow( base, 5.0 );
  return exponential + F0 * ( 1.0 - exponential );
}

float GeometrySchlickGGX(float NdotV, float k) {
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return nom / denom;
}
  

float CalculateSmithGGXGeometryTerm(float k, float NdotL, float NdotV) { 
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);
	
    return ggx1 * ggx2;
}

vec3 WorldPosFromDepth(vec2 tex, float depth) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(tex * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = invP * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    vec4 worldSpacePosition = invV * viewSpacePosition;

    return worldSpacePosition.xyz;
}

void main() {
    // Calculate normals
    vec3 N = normalize(fragViewNormal);
    vec3 T = normalize(fragViewTangent);
    vec3 B = normalize(fragViewBinormal);
    mat3 texSpace = mat3(T, B, N);
 
    vec2 normalMapCoords1 = fragTex.xy + time * normalMapScrollDir.xy * normalMapScrollSpeed.x;
    vec2 normalMapCoords2 = fragTex.xy + time * normalMapScrollDir.zw * normalMapScrollSpeed.y;
 
    vec3 normalMap = texture2D(WaterNormalMap1, normalMapCoords1).rgb * 2.0 - 1.0; // LinearWrap sample
    vec3 normalMap2 = texture2D(WaterNormalMap2, normalMapCoords2).rgb * 2.0 - 1.0; //LinearWrap sample
    vec3 finalNormal = normalize(texSpace * normalMap.xyz);
    finalNormal += normalize(texSpace * normalMap2.xyz);
    finalNormal = normalize(finalNormal);


    // Calculate specular
    // TODO: this is broken
    vec3 lightDir = lightPos - fragWorldPos.xyz;
    vec3 L = normalize(lightDir);
    vec3 V = -normalize(fragViewPos.xyz);
    vec3 H = normalize(V + L);
    float linearRoughness = roughness * roughness;
    float nDotL = clamp(dot(finalNormal, L), 0.0, 1.0);
    float nDotV = abs(dot(finalNormal, V)) + EPSILON;
    float nDotH = clamp(dot(finalNormal, H), 0.0, 1.0);
    float lDotH = clamp(dot(L, H), 0.0, 1.0);
    float f0 = reflectanceFactor.x * reflectanceFactor.y * reflectanceFactor.y;
    float normalDistribution = CalculateNormalDistributionGGX(linearRoughness, nDotH);
    float fresnelReflectance = CalculateSchlickFresnelReflectance(H, V, f0);
    float geometryTerm = CalculateSmithGGXGeometryTerm(linearRoughness, nDotL, nDotV);
    // float specularNoise = texture2D(waterNoise, normalMapCoords1 * 0.5).r;
    // specularNoise *= texture2D(waterNoise, normalMapCoords2 * 0.5).r; // 0
    // specularNoise *= texture2D(waterNoise, fragTex.xy * 0.5).r;
    float specularNoise = 1.f;
    float specularFactor = geometryTerm * normalDistribution * fresnelReflectance * specIntensity * nDotL * specularNoise;
    
    vec2 hdrCoords = ((vec2(fragScreenPos.x, -fragScreenPos.y) / fragScreenPos.w) * 0.5) + 0.5;
    vec2 distortedTexCoord = (hdrCoords + ((finalNormal.xz + finalNormal.xy) * 0.5) * refractionDistortionFactor);
    float distortedDepth = texture2D(gDepth, distortedTexCoord).r;
    vec3 distortedPosition = WorldPosFromDepth(distortedTexCoord, distortedDepth);
    vec2 refractionTexCoord = (distortedPosition.y < fragWorldPos.y) ? distortedTexCoord : hdrCoords;
    vec3 waterColor = texture2D(gDiffuse, refractionTexCoord).rgb * baseColor;

    color = vec4(waterColor, 1.0);
    return;

    float sceneDepth = texture2D(gDepth, hdrCoords).r;
    vec3 scenePosition = WorldPosFromDepth(hdrCoords, sceneDepth);
    float depthSoftenedAlpha = clamp(distance(scenePosition, fragWorldPos.xyz) / depthSofteningDistance, 0.0, 1.0);

    vec3 waterSurfacePosition = (distortedPosition.y < fragWorldPos.y) ? distortedPosition : scenePosition;
    waterColor = mix(waterColor, baseColor.rgb, clamp((fragWorldPos.y - waterSurfacePosition.y) / refractionHeightFactor, 0.0, 1.0));

    float waveTopReflectionFactor = pow(1.0 - clamp(dot(fragViewNormal, V), 0.0, 1.0), 3);
    vec3 waterBaseColor = mix(waterColor, reflectionColor, clamp(clamp(length(fragViewPos.xyz) / refractionDistanceFactor, 0.0, 1.0) + waveTopReflectionFactor, 0.0, 1.0));

    vec3 finalWaterColor = waterBaseColor + specularFactor;

    color = vec4(finalWaterColor, depthSoftenedAlpha);
}