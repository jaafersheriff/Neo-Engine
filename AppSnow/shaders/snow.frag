
#include "phong.glsl"

in vec4 fragPos;
in vec3 fragNor;

uniform float ambient;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float shine;

uniform vec3 camPos;

uniform vec3 lightPos;
uniform vec3 lightCol;
uniform vec3 lightAtt;

uniform vec3 snowAngle;
uniform vec3 snowColor;
uniform float snowSize;
uniform vec3 rimColor;
uniform float rimPower;

out vec4 color;

void main() {
    vec3 N = normalize(fragNor);
    vec3 V = normalize(camPos - fragPos.xyz);
    if (dot(N, snowAngle) >= snowSize) {
        float rim = 1.f - clamp(dot(V, N), 0.f, 1.f);
        color.rgb += snowColor + rimColor * pow(rim, rimPower);
    }
    else {
        color.rgb = getPhong(N, fragPos.xyz, camPos, lightPos, lightAtt, lightCol, diffuseColor, specularColor, shine);
    }

    color.a = 1;

}