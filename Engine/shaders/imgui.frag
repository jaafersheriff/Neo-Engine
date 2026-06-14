#include "cubemap.glsl"

in vec2 fragTex;
in vec4 fragCol;
		
#if defined(TEXTURE_2D)
layout(binding = 0) uniform sampler2D Texture;
#elif defined(TEXTURE_2D_ARRAY)
layout(binding = 0) uniform sampler2DArray Texture;
#elif defined(TEXTURE_CUBE)
layout(binding = 0) uniform samplerCube Texture;
#elif defined(TEXTURE_3D)
layout(binding = 0) uniform sampler3D Texture;
#endif

uniform uint arrayLevel;
uniform uint mipLevel;

out vec4 color;

void main() {
	vec4 texColor;
		
#if defined(TEXTURE_2D)
	texColor = textureLod(Texture, fragTex, float(mipLevel));
#elif defined(TEXTURE_2D_ARRAY)
    texColor = textureLod(Texture, vec3(fragTex, float(arrayLevel)), float(mipLevel));
#elif defined(TEXTURE_CUBE)
    texColor = textureLod(Texture, cubemapSamplingCoords(fragTex, int(arrayLevel)), float(mipLevel));
#elif defined(TEXTURE_3D)
    texColor = vec4(textureLod(Texture, vec3(fragTex, float(arrayLevel)), float(mipLevel)).rgb, 1);
#else
	texColor = vec4(1, 0, 0, 1);
#endif

	color = fragCol * texColor;
}
