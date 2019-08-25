
vec3 getPhong(vec3 normal, vec3 worldPos, vec3 camPos, vec3 lightPos, vec3 lightAttenuation, vec3 lightCol, vec3 baseColor, vec3 specularColor, float shine) {
    vec3 N = normalize(normal);
    vec3 V = normalize(camPos - worldPos.xyz);
    vec3 lightDir = lightPos - worldPos.xyz;
    float lightDistance = length(lightDir);
    vec3 L = normalize(lightDir);
    float attFactor = 1;
    if (length(lightAttenuation) > 0) {
        attFactor = lightAttenuation.x + lightAttenuation.y*lightDistance + lightAttenuation.z*lightDistance*lightDistance;
    }
    float lambert = dot(L, N);
    vec3 H = normalize(L + V);
    vec3 diffuseContrib  = lightCol * max(lambert, 0.0f) / attFactor;
    vec3 specularContrib = lightCol * pow(max(dot(H, N), 0.0), shine) / attFactor;
    return baseColor * diffuseContrib +
           specularColor * specularContrib;
}
