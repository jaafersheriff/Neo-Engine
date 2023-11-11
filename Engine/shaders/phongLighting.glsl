
vec3 getPhong(vec3 normal, vec3 worldPos, vec3 camPos, vec3 lightDir, vec3 lightAttenuation, vec3 lightCol, vec3 baseColor, vec3 specularColor, float shine) {
    vec3 N = normalize(normal);
    vec3 V = normalize(camPos - worldPos.xyz);
    float lightDistance = length(lightDir);
    vec3 L = normalize(lightDir);
    vec3 H = normalize(L + V);
    float lambert = clamp(dot(L, N), 0.0, 1.0);
    float attFactor = 1;
    if (length(lightAttenuation) > 0) {
        attFactor = lightAttenuation.x + lightAttenuation.y*lightDistance + lightAttenuation.z*lightDistance*lightDistance;
    }
    vec3 diffuseContrib  = lightCol * lambert / attFactor;
    vec3 specularContrib = lightCol * pow(clamp(dot(H, N), 0.0, 1.0), shine) / attFactor;
    return baseColor * diffuseContrib
           + specularColor * specularContrib;
}
