layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat3 N;

out vec4 fragPos;
out vec3 fragNor;
out vec2 fragTex;

#ifdef ENABLE_SHADOWS
uniform mat4 L0;
uniform mat4 L1;
uniform mat4 L2;
uniform mat4 L3;
uniform float depth0;
uniform float depth1;
uniform float depth2;
uniform float depth3;

out vec4 shadowCoord0; // csm frustum transforms
out vec4 shadowCoord1;
out vec4 shadowCoord2;
out vec4 shadowCoord3;

uniform mat4 mockPV;
uniform float mockNear;

out float sceneDepth;
#endif

void main() {
	fragPos = M * vec4(vertPos, 1.0);
	fragNor = N * vertNor;
	fragTex = vertTex;
	gl_Position = P * V * fragPos;

#ifdef ENABLE_SHADOWS
	shadowCoord0 = L0 * fragPos;
	shadowCoord1 = L1 * fragPos;
	shadowCoord2 = L2 * fragPos;
	shadowCoord3 = L3 * fragPos;

 // This should just be gl_position.z
	sceneDepth = (mockPV * fragPos).z + mockNear; // Add near plane b/c GL NDC is -1, 1 ?
#endif
}
