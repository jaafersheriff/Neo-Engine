
layout(location = 0) in vec4 positionDecay;
layout(location = 2) in vec3 velocity;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out vec3 gPos;
out vec3 gVelocity;
out float gDecay;

void main() {
	vec4 pos = M * vec4(positionDecay.xyz, 1.0);
	gl_Position = P * V * pos;
	// gPos = pos.xyz;
	// gVelocity = velocity;
	// gDecay = positionDecay.a;
}
