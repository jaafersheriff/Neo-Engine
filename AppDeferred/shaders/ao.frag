#version 330 core

in vec2 fragTex;

uniform sampler2D inputFBO;
uniform sampler2D inputDepth;

uniform sampler2D gNormal;
uniform sampler2D gDepth;

uniform sampler1D kernel;
uniform sampler2D noise;

uniform float radius;
uniform float bias;

uniform mat4 P;
uniform mat4 invP;

out vec4 color;

// Position in view space
vec3 reconstructViewPos(vec2 texCoord) {
    float depth = texture(gDepth, texCoord).r;
    vec3 ndc = vec3(texCoord, depth) * 2.0 - vec3(1.0);
    vec4 view = invP * vec4(ndc, 1.0);
    return view.xyz / view.w;
}

void main() {
    vec2 noiseScale = textureSize(gDepth, 0) / textureSize(noise, 0);

    vec3 fragPos = reconstructViewPos(fragTex);
    vec3 fragNor = normalize(texture(gNormal, fragTex).rgb * vec3(2.f) - vec3(1.f));
    vec3 randomVec = texture(noise, fragTex * noiseScale).xyz;

    vec3 tangent = normalize(randomVec - fragNor * dot(randomVec, fragNor));
    vec3 biTangent = cross(fragNor, tangent);
    mat3 TBN = mat3(tangent, biTangent, fragNor);

    float occ = 0.f;
    int kernelSize = textureSize(kernel, 0);   
    for (int i = 0; i < kernelSize; i++) {
        vec3 s = TBN * texelFetch(kernel, i, 0).rgb;
        s = fragPos + s * radius;
        vec4 offset = vec4(s, 1.f);
        offset      = P * offset;
        offset.xy  /= offset.w;
        offset.xy   = offset.xy * vec2(0.5f) + vec2(0.5f);
        float sampView = reconstructViewPos(offset.xy).z;
        float rangeCheck = smoothstep(0.f, 1.f, radius / abs(fragPos.z - sampView));
        occ += (sampView >= s.z + bias ? 1.f : 0.f) * rangeCheck;
    }

    occ = 1.f - (occ / float(kernelSize));
    color = vec4(occ);
}