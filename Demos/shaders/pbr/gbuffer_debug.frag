in vec2 fragTex;

layout(binding = 0) uniform sampler2D gAlbedoAO;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D gWorldRoughness;
layout(binding = 3) uniform sampler2D gEmissiveMetalness;
layout(binding = 4) uniform sampler2D gDepth;

out vec4 color;

void main() {

#ifdef NORMAL
	color.rgb = texture(gNormal, fragTex).rgb * 2.0 - 1.0;
#elif defined ALBEDO
	color.rgb = texture(gAlbedoAO, fragTex).rgb;
#elif defined AO
	color.rgb = vec3(texture(gAlbedoAO, fragTex).a);
#elif defined WORLD
	color.rgb = texture(gWorldRoughness, fragTex).rgb;
#elif defined ROUGHNESS
	color.rgb = vec3(texture(gWorldRoughness, fragTex).a);
#elif defined EMISSIVE
	color.rgb = texture(gEmissiveMetalness, fragTex).rgb;
#elif defined METALNESS
	color.rgb = vec3(texture(gEmissiveMetalness, fragTex).a);
#elif defined DEPTH
	color.rgb = vec3(texture(gDepth, fragTex).r);
#else
	color = vec4(0.0);
#endif
	color.a = 1.0;
}
