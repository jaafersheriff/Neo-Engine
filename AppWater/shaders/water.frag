
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

out vec4 color;

void main() {
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

    color = vec4(normalMap2, 1.0);
} 