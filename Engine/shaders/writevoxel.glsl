
layout(rgba32f, binding = 0) uniform writeonly image3D volume;
uniform vec2 bufferSize;
uniform float camNear;
uniform float camFar;

float linearizeDepth(float d,float zNear,float zFar) {
	float z_n = 2.0 * d - 1.0;
    return 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
}

void writevoxel(vec4 color) {
	ivec3 dims = imageSize(volume);

	vec2 xy = gl_FragCoord.xy;

	float depth = gl_FragCoord.z * dims.x;
	//  depth /= gl_FragCoord.w;
	// float depth = linearizeDepth(gl_FragCoord.z, camNear, camFar) / camFar * dims.x;
	xy /= bufferSize;

	ivec3 volCoords = ivec3(xy * dims.xy, depth);

	imageStore(volume, volCoords, color);
}