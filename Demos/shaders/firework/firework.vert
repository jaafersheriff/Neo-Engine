

// TODO - move this to include
// struct Data {
// 	vec3 position;
// 	vec3 velocity;
// 	float intensity;
// 	float unused;
// };

layout(location = 0) in vec3 data;

uniform mat4 P;
uniform mat4 V;
//uniform mat4 M;

// out vec3 gPos;
// out vec3 gVelocity;
// out float gDecay;

void main() {
	//vec4 pos = M * vec4(data.position, 1.0);
	//gl_Position = P * V * pos;
	gl_Position = P * V * vec4(1);
	// gPos = pos.xyz;
	// gVelocity = velocity;
	// gDecay = positionDecay.a;
}
