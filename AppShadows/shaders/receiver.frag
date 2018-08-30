#version 330 core

in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;
in vec4 shadowCoord;

uniform float ambient;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float shine;

uniform bool useTexture;
uniform sampler2D diffuseMap;

uniform vec3 camPos;

uniform vec3 lightPos;
uniform vec3 lightCol;
uniform vec3 lightAtt;

uniform bool useDotBias;
uniform sampler2D shadowMap;
uniform float bias;
uniform int pcfSize;
uniform bool usePCF;

out vec4 color;

void main() {
    vec4 albedo = vec4(diffuseColor, 1.f);
    if (useTexture) {
        albedo = texture(diffuseMap, fragTex);
        if (albedo.a < 0.1f) {
            discard;
        }
    }

    vec3 N = normalize(fragNor);
    vec3 viewDir = camPos - fragPos.xyz;
    vec3 V = normalize(viewDir);
    vec3 lightDir = lightPos - fragPos.xyz;
    float lightDistance = length(lightDir);
    vec3 L = normalize(lightDir);

    float attFactor = 1;
    if (length(lightAtt) > 0) {
        attFactor = lightAtt.x + lightAtt.y*lightDistance + lightAtt.z*lightDistance*lightDistance;
    }

    float lambert = dot(L, N);
    vec3 H = normalize(L + V);
    vec3 diffuseContrib  = lightCol * max(lambert, 0.0f) / attFactor;
    vec3 specularContrib = lightCol * pow(max(dot(H, N), 0.0), shine) / attFactor;

    // Shadow
    float Tbias = bias;

    if (useDotBias) {
        Tbias = bias*tan(acos(clamp(lambert, 0, 1)));
        Tbias = clamp(Tbias, 0,0.01);
    }

    float visibility = 1.f;
    if (usePCF) {
        float shadow = 0.f;
        vec2 texelSize = 1.f / textureSize(shadowMap, 0);
        for (int x = -pcfSize; x <= pcfSize; x++) {
            for (int y = -pcfSize; y <= pcfSize; y++) {
                float pcfDepth = texture(shadowMap, shadowCoord.xy + vec2(x, y) * texelSize).r;
                shadow += shadowCoord.z - Tbias > pcfDepth ? 1.f : 0.f;
            }
        }
        shadow /= (2 * pcfSize + 1) * (2 * pcfSize + 1);
        visibility = 1.f - shadow;
    }
    else if (texture(shadowMap, shadowCoord.xy).r < shadowCoord.z - Tbias) {
        visibility = 0.f;
    }

    color.rgb = albedo.rgb * ambient + 
                visibility * albedo.rgb * diffuseContrib + 
                visibility * specularColor * specularContrib;
    color.a = albedo.a;
}