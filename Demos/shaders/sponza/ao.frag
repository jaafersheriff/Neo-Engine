
in vec2 fragTex;

layout(binding = 0) uniform sampler2D gNormal;
layout(binding = 1) uniform sampler2D gDepth;
layout(binding = 2) uniform sampler1D kernel;
layout(binding = 3) uniform sampler2D noise;

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
	vec3 randomVec = texture(noise, fragTex * noiseScale).xyz * vec3(2.f) - vec3(1.f);

	vec3 tangent = normalize(randomVec - fragNor * dot(randomVec, fragNor));
	vec3 biTangent = cross(fragNor, tangent);
	mat3 TBN = mat3(tangent, biTangent, fragNor);

	float occlusion = 0.f;
	int kernelSize = textureSize(kernel, 0).x;
	for (int i = 0; i < kernelSize; i++) {
		vec3 kSample = TBN * texelFetch(kernel, i, 0).rgb;
		kSample = fragPos + kSample * radius;

		vec4 offset = vec4(kSample, 1.f);
		offset	  = P * offset;
		offset.xy  /= offset.w;
		offset.xy   = offset.xy * vec2(0.5f) + vec2(0.5f);

		float sampView = reconstructViewPos(offset.xy).z;
		float rangeCheck = smoothstep(0.f, 1.f, radius / abs(fragPos.z - sampView));
		occlusion += (sampView >= kSample.z + bias ? 1.f : 0.f) * rangeCheck;
	}

	occlusion = 1.f - (occlusion / float(kernelSize));
	color = vec4(occlusion);
}
