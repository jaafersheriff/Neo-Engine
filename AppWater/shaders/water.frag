
#define EPSILON 0.00001
#define PI 3.14159265359

in vec4 fragWorldPos;
in vec4 fragViewPos;
in vec4 fragScreenPos;
in vec3 fragNormal;
in vec3 fragTangent;
in vec3 fragBinormal;
in vec4 fragTex;
in float fragHeight;

uniform float time;
uniform vec4 normalMapScrollDir;
uniform vec2 normalMapScrollSpeed;
uniform sampler2D WaterNormalMap1;
uniform sampler2D WaterNormalMap2;

uniform vec3 camPos;
uniform mat4 P;
uniform mat4 V;

uniform vec3 lightPos;
uniform vec2 reflectanceFactor;
uniform float reflectionPow;
uniform vec4 ssrSettings;
uniform float roughness;
uniform float specIntensity;

uniform sampler2D waterNoise;

uniform float refractionDistortionFactor;
uniform sampler2D gDiffuse; // this should be final light pass 
uniform sampler2D gWorld;
uniform sampler2D gNormal;
uniform sampler2D gDepth;
uniform mat4 invV;
uniform mat4 invP;
uniform float refractionHeightFactor;
uniform float refractionDistanceFactor;
uniform vec3 refractionColor;

uniform float depthSofteningDistance;

uniform samplerCube cubeMap;
uniform vec3 surfaceColor;

out vec4 color;

float CalculateNormalDistributionGGX(float a, float NdotH) {
    float a2     = a*a;
    float NdotH2 = NdotH*NdotH;
	
    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;
	
    return nom / denom;
}

float CalculateSchlickFresnelReflectance(vec3 H, vec3 Vd, float F0 ) {
  float base = 1.0 - dot( Vd, H );
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

vec3 DecodeSphereMap(vec2 e) {
    vec3 n;
    vec2 tmp = e - e * e;
    float f = tmp.x + tmp.y;
    float m = sqrt(4.0f * f - 1.0f);
    n.xy = m * (e * 4.0f - 2.0f);
    n.z = 3.0f - 8.0f * f;
    return n;
}

void main() {
    // Calculate normals
    vec3 N = normalize(fragNormal);
    vec3 T = normalize(fragTangent);
    vec3 B = normalize(fragBinormal);
    mat3 texSpace = mat3(T, B, N);
 
    vec2 normalMapCoords1 = fragTex.xy + time * normalMapScrollDir.xy * normalMapScrollSpeed.x;
    vec2 normalMapCoords2 = fragTex.xy + time * normalMapScrollDir.zw * normalMapScrollSpeed.y;
 
    vec3 normalMap = texture(WaterNormalMap1, normalMapCoords1).rgb * 2.0 - 1.0; // LinearWrap sample
    vec3 normalMap2 = texture(WaterNormalMap2, normalMapCoords2).rgb * 2.0 - 1.0; //LinearWrap sample
    vec3 finalNormal = normalize(texSpace * normalMap.xyz);
    finalNormal += normalize(texSpace * normalMap2.xyz);
    finalNormal = normalize(finalNormal);

    // Calculate specular
    vec3 lightDir = lightPos - fragWorldPos.xyz;
    vec3 L = normalize(lightDir);
    vec3 VDir = normalize(camPos - fragWorldPos.xyz);
    vec3 H = normalize(VDir + L);
    float linearRoughness = roughness * roughness;
    float nDotL = clamp(dot(finalNormal, L), 0.0, 1.0);
    float nDotV = abs(dot(finalNormal, VDir)) + EPSILON;
    float nDotH = clamp(dot(finalNormal, H), 0.0, 1.0);
    float lDotH = clamp(dot(L, H), 0.0, 1.0);
    float f0 = reflectanceFactor.x * reflectanceFactor.y * reflectanceFactor.y;
    float normalDistribution = CalculateNormalDistributionGGX(linearRoughness, nDotH);
    float fresnelReflectance = CalculateSchlickFresnelReflectance(H, VDir, f0);
    float geometryTerm = CalculateSmithGGXGeometryTerm(linearRoughness, nDotL, nDotV);

    // TODO : uncomment later
    float specularNoise = 1.0; //texture(waterNoise, normalMapCoords1 * 0.5).r;
    // specularNoise *= texture(waterNoise, normalMapCoords2 * 0.5).r;
    // specularNoise *= texture(waterNoise, fragTex.xy * 0.5).r;
    float specularFactor = geometryTerm * normalDistribution * fresnelReflectance * specIntensity * nDotL * specularNoise;

    vec2 hdrCoords = ((vec2(fragScreenPos.x, fragScreenPos.y) / fragScreenPos.w) * 0.5) + 0.5;
    float sceneZ = 0;
    float stepCount = 0;
    float forwardStepCount = ssrSettings.y;
    vec3 rayMarchPosition = fragViewPos.xyz;
    vec4 rayMarchTexPosition = vec4(0,0,0,0);
    vec3 reflectionVector = normalize(reflect(fragViewPos.xyz, normalize(vec3(V * vec4(finalNormal, 1.0))))); 
     
    while(stepCount < ssrSettings.y) {
        // march ray
        rayMarchPosition += reflectionVector.xyz * ssrSettings.x;

        // get screen tex coords from ray
        rayMarchTexPosition = P * vec4(-rayMarchPosition, 1.0);
        if(abs(rayMarchTexPosition.w) < EPSILON) {
            rayMarchTexPosition.w = EPSILON;
        }
        rayMarchTexPosition.xy /= rayMarchTexPosition.w;
        rayMarchTexPosition.xy = vec2(rayMarchTexPosition.x, -rayMarchTexPosition.y) * 0.5 + 0.5; 

        // get view pos of ray march
        // this works btw
        vec3 viewPos = vec3(V * vec4(texture(gWorld, rayMarchTexPosition.xy).rgb, 1.0));
        sceneZ = viewPos.z;

        // if ray passed an object, set ray march to max and exit loop
        if (sceneZ <= rayMarchPosition.z) {
            forwardStepCount = stepCount;
            stepCount = ssrSettings.y;
        }
        // if object in scene can still be hit, keep going
        else {
            stepCount++;
        }
    }

    // if we passed an object or haven't hit one yet
    if (forwardStepCount < ssrSettings.y) {
        stepCount = 0;		
        while(stepCount < ssrSettings.z) {
            // bring ray back
            rayMarchPosition -= reflectionVector.xyz * ssrSettings.x / ssrSettings.z;

            rayMarchTexPosition = P * vec4(rayMarchPosition, 1.0);
            if(abs(rayMarchTexPosition.w) < EPSILON) {
                rayMarchTexPosition.w = EPSILON;
            }
            rayMarchTexPosition.xy /= rayMarchTexPosition.w;
            rayMarchTexPosition.xy = vec2(rayMarchTexPosition.x, -rayMarchTexPosition.y) * 0.5 + 0.5; 

            vec3 viewPos = vec3(V * vec4(texture(gWorld, rayMarchTexPosition.xy).rgb, 1.0));
            sceneZ = viewPos.z;

            // if we passed an object
            if(sceneZ > rayMarchPosition.z) {
                stepCount = ssrSettings.z;
            }					
            else {
                stepCount++;
            }
        }
    }

    vec3 ssrReflectionNormal = texture(gNormal, rayMarchTexPosition.xy).rgb;
    vec2 ssrDistanceFactor = vec2(distance(0.5, hdrCoords.x), distance(0.5, hdrCoords.y)) * 2.0;
    float ssrFactor = (1.0 - abs(nDotV))
                      * (1.0 - forwardStepCount / ssrSettings.y)
                      * clamp(1.0 - ssrDistanceFactor.x - ssrDistanceFactor.y, 0.0, 1.0)
                      * (1.0 / (1.0 + abs(sceneZ - rayMarchPosition.z) / ssrSettings.w))
                      * (1.0 - clamp(dot(ssrReflectionNormal, finalNormal), 0.0, 1.0));
 
    vec3 reflectionColor = texture(gDiffuse, rayMarchTexPosition.xy).rgb;
    reflectionColor *= (length(texture(gWorld, rayMarchTexPosition.xy).rgb) > 0 ? 1.0 : 0.0); // cull skybox
    vec3 skyboxColor = texture(cubeMap, normalize(reflect(normalize(VDir), finalNormal))).rgb;
    reflectionColor = mix(skyboxColor, reflectionColor, ssrFactor) * surfaceColor.rgb;
    color = vec4(reflectionColor, 1.0); return;
    
    vec2 distortedTexCoord = (hdrCoords + ((finalNormal.xz + finalNormal.xy) * 0.5) * refractionDistortionFactor);
    vec3 distortedPosition = texture(gWorld, distortedTexCoord).rgb;
    vec2 refractionTexCoord = (distortedPosition.y < fragWorldPos.y) ? distortedTexCoord : hdrCoords;
    vec3 waterColor = texture(gDiffuse, refractionTexCoord).rgb * refractionColor;

    vec3 scenePosition = texture(gWorld, hdrCoords).rgb;
    float depthSoftenedAlpha = clamp(abs(distance(scenePosition, fragWorldPos.xyz)) / depthSofteningDistance, 0.0, 1.0);

    vec3 waterSurfacePosition = (distortedPosition.y < fragWorldPos.y) ? distortedPosition : scenePosition;
    waterColor = mix(waterColor, refractionColor, clamp((fragWorldPos.y - waterSurfacePosition.y) / refractionHeightFactor, 0.0, 1.0));

    float waveTopReflectionFactor = pow(1.0 - clamp(dot(fragNormal, VDir), 0.0, 1.0), reflectionPow);
    waterColor = mix(waterColor, reflectionColor, clamp(clamp(length(fragViewPos.xyz) / refractionDistanceFactor, 0.0, 1.0) + waveTopReflectionFactor, 0.0, 1.0));

    vec3 finalWaterColor = waterColor + clamp(specularFactor, 0.0, 1.0);

    color = vec4(finalWaterColor, depthSoftenedAlpha);
}