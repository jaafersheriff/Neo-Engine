
float getShadowVisibility(int pcfSize, sampler2D shadowMap, vec4 shadowCoord, float bias) {

    float visibility = 1.f;
    if (pcfSize > 0) {
        float shadow = 0.f;
        vec2 texelSize = 1.f / textureSize(shadowMap, 0);
        for (int x = -pcfSize; x <= pcfSize; x++) {
            for (int y = -pcfSize; y <= pcfSize; y++) {
                float pcfDepth = texture(shadowMap, shadowCoord.xy + vec2(x, y) * texelSize).r;
                shadow += shadowCoord.z - bias > pcfDepth ? 1.f : 0.f;
            }
        }
        shadow /= (2 * pcfSize + 1) * (2 * pcfSize + 1);
        visibility = 1.f - shadow;
    }
    else if (texture(shadowMap, shadowCoord.xy).r < shadowCoord.z - bias) {
        visibility = 0.f;
    }

    return visibility;
}