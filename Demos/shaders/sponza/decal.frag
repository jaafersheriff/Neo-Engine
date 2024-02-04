
uniform sampler2D gNormal;
uniform sampler2D gDepth;

uniform mat4 invPV;
uniform mat4 invM;

uniform sampler2D decalTexture;

out vec4 color;

vec3 WorldPosFromDepth(vec2 tex, float depth) {
	float z = depth * 2.0 - 1.0;
	vec3 clip = vec3(tex * 2 - 1, z);
	vec4 wPos = invPV * vec4(clip, 1.0);
	return wPos.xyz / wPos.w;
}

void main() {
	/* Generate tex coords [0, 1] */
	vec2 fragTex = gl_FragCoord.xy / vec2(textureSize(gDepth, 0));
	
	/* Calculate fragment's world position */
	float depth = texture(gDepth, fragTex).r;

	vec3 worldPos = WorldPosFromDepth(fragTex, depth);
	vec4 objPos = invM * vec4(worldPos, 1.0);
	vec3 clip = vec3(0.5) - abs(objPos.xyz);

	if(clip.x < 0.0 || clip.y < 0.0 || clip.z < 0.0) {
		discard;
	}

	vec2 tex = objPos.xz + 0.5;
	color = texture(decalTexture, tex);
}