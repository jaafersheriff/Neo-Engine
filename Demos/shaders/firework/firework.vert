
layout(location = 0) in vec4 positionIntensity;
layout(location = 1) in vec3 velocity;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

 out vec3 gPos;
 out vec3 gVelocity;
 out float gIntensity;

void main() {
	vec4 pos = M * vec4(positionIntensity.xyz, 1.0);
	gl_Position = P * V * pos;
	gPos = pos.xyz;
	gIntensity = positionIntensity.a;
	gVelocity = velocity;
}
