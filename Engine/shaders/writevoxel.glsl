
layout(rgba32f, binding = 0) uniform restrict writeonly image3D volume;
uniform vec2 bufferSize;
uniform float camNear;
uniform float camFar;

float linearizeDepth(float d,float zNear,float zFar) {
	float z_n = 2.0 * d - 1.0;
    float z_r = 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
	return z_r;
}

void writevoxel(vec4 color) {
	ivec3 dims = imageSize(volume) - 1;

	vec2 xy = gl_FragCoord.xy;
	xy /= bufferSize;
	float depth = gl_FragCoord.z;

	float ldepth = linearizeDepth(gl_FragCoord.z, camNear, camFar);

	ivec3 volCoords = ivec3(xy * dims.xy, depth * dims.z);
	volCoords = clamp(volCoords, ivec3(0), dims);

	imageStore(volume, volCoords, color);
}