layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
#ifdef TANGENTS
layout(location = 3) in vec4 vertTan;
#endif

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat3 N;

out vec4 fragPos;
out vec3 fragNor;
out vec2 fragTex;
#ifdef TANGENTS
out vec4 fragTan;
#endif

#ifdef ENABLE_SHADOWS
uniform mat4 L;
out vec4 shadowCoord;
#endif

void main() {
	fragPos = M * vec4(vertPos, 1.0);
	fragNor = N * vertNor;
	fragTex = vertTex;
#ifdef TANGENTS
	fragTan = vertTan;
#endif
	gl_Position = P * V * fragPos;

#ifdef ENABLE_SHADOWS
	shadowCoord = L * fragPos;
#endif
}
