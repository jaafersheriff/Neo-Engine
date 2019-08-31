
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

    if (dot(N, snowAngle) >= snowSize) {
        float rim = 1.f - clamp(dot(V, N), 0.f, 1.f);
        color.rgb += snowColor + rimColor * pow(rim, rimPower);
    }
    else {
        color.rgb = diffuseColor * ambient +
                    diffuseColor * diffuseContrib +
                    specularColor * specularContrib;
    }

    color.a = 1;

}