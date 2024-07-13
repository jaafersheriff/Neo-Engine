

// TODO - move this to include
 struct Data {
 	vec3 position;
 	vec3 velocity;
 	float intensity;
 	float unused;
 };

layout(location = 0) in Data data;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

 out vec3 gPos;
 out vec3 gVelocity;
 out float gIntensity;

void main() {
	vec4 pos = M * vec4(data.position, 1.0);
	gl_Position = P * V * pos;
	gPos = pos.xyz;
	gVelocity = data.velocity;
	gIntensity = data.intensity;
}
