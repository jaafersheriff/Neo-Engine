
#include "vct/voxels.glsl"

in vec4 fragPos;

uniform vec3 cameraPos;

uniform int volumeDimension;
uniform vec3 volumeMin; // world space
uniform vec3 volumeMax; // world space
uniform int outputBufferSize;

layout(std430, binding = 0) buffer VoxelFragments {
	VoxelFragment fragments[];
};

out vec4 outColor;

void main() {
	vec3 rayOrigin = cameraPos;
	vec3 rayDir = normalize(fragPos.xyz - cameraPos);

	// A step size roughly matching a fraction of a voxel unit keeps it accurate.
	float voxelSize = abs((volumeMax.x - volumeMin.x)) / float(volumeDimension);
	float stepSize = voxelSize * 0.5; 
	int maxSteps =16;

	vec3 currentPos = fragPos.xyz; // Start right at the surface of the debug box
	outColor = vec4(0,0,0, 1);

	for (int step = 0; step < maxSteps; ++step) {
		if (currentPos.x < volumeMin.x || currentPos.x > volumeMax.x ||
			currentPos.y < volumeMin.y || currentPos.y > volumeMax.y ||
			currentPos.z < volumeMin.z || currentPos.z > volumeMax.z) {
			break;
		}

		// Calculate what integer voxel coordinate this 3D point lands in
		vec3 normalizedPos = (currentPos - volumeMin) / (volumeMax - volumeMin);
		ivec3 targetVoxelIndex = ivec3(normalizedPos * volumeDimension);

		// BRUTE FORCE SEARCH
		for (int i = 0; i < outputBufferSize; ++i) {
			// Optimization: Stop checking if we hit completely uninitialized fragments
			if (i > outputBufferSize) {
				continue;
			}
			if (fragments[i].worldPosition == vec3(0.0)) {
				continue;
			}


			// Convert the stored fragment's world position to its voxel coordinates
			vec3 fragNormPos = (fragments[i].worldPosition - volumeMin) / (volumeMax - volumeMin);
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

