
#include "vct/voxels.glsl"

in vec4 fragPos;

uniform vec3 cameraPos;

uniform int volumeDimension;
uniform vec3 volumeMin; // world space
uniform vec3 volumeMax; // world space

layout(std430, binding = 0) buffer VoxelFragments {
	VoxelFragment fragments[];
};

out vec4 outColor;

void main() {
	const int fragmentBufferSize = volumeDimension * volumeDimension * volumeDimension;
	vec3 rayOrigin = cameraPos;
	vec3 rayDir = normalize(fragPos.xyz - cameraPos);

	vec3 _volumeMin = volumeMin; // vec3(volumeMin.x, volumeMax.y, volumeMin.z);
	vec3 _volumeMax = volumeMax; // vec3(volumeMax.x, volumeMin.y, volumeMax.z);

	// A step size roughly matching a fraction of a voxel unit keeps it accurate.
	float voxelSize = abs((_volumeMax.x - _volumeMin.x)) / float(volumeDimension);
	float stepSize = voxelSize * 0.5; 
	int maxSteps = 32;

	vec3 currentPos = fragPos.xyz; // Start right at the surface of the debug box
	outColor = vec4(0,0,0, 1);

	for (int step = 0; step < maxSteps; ++step) {
		if (currentPos.x < _volumeMin.x || currentPos.x > _volumeMax.x ||
			currentPos.y < _volumeMin.y || currentPos.y > _volumeMax.y ||
			currentPos.z < _volumeMin.z || currentPos.z > _volumeMax.z) {
			outColor = vec4(1,0,0, 1.0);
			break;
		}

		// Calculate what integer voxel coordinate this 3D point lands in
		vec3 normalizedPos = (currentPos - _volumeMin) / (_volumeMax - _volumeMin);
		ivec3 targetVoxelIndex = ivec3(normalizedPos * volumeDimension);

		// BRUTE FORCE SEARCH
		for (int i = 0; i < fragmentBufferSize; ++i) {
			// Optimization: Stop checking if we hit completely uninitialized fragments
			if (fragments[i].worldPosition == vec3(0.0)) {
				continue;
			}

			// Convert the stored fragment's world position to its voxel coordinates
			vec3 fragNormPos = (fragments[i].worldPosition - _volumeMin) / (_volumeMax - _volumeMin);
			ivec3 fragVoxelIndex = ivec3(fragNormPos);
			// // If the fragment in the buffer matches where our ray currently is...
			if (targetVoxelIndex == fragVoxelIndex) {
			// 	// 	// Perfect hit! Draw it and stop marching this ray
			outColor = vec4(fragments[i].albedo.rgb, 1.0);
			return;
			}
		}

		// // Advance the ray deeper into the volume
		currentPos += rayDir * stepSize;
	}

}

