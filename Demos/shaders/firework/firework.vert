
layout(location = 0) in vec4 positionIntensity;
layout(location = 1) in vec4 velocity;

 out vec3 gPos;
 out vec4 gVelocity;
 out float gIntensity;

void main() {
	gl_Position = vec4(positionIntensity.xyz, 1.0);
	gIntensity = positionIntensity.a;
	gVelocity = velocity;
}
