
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

	// A step size roughly matching a fraction of a voxel unit keeps it accurate.
	float voxelSize = (volumeMax.x - volumeMin.x) / float(volumeDimension);
	float stepSize = voxelSize * 0.75;
	int maxSteps = 8;

	vec3 currentPos = fragPos.xyz + rayDir * 0.1; // Start right at the surface of the debug box

	int voxelCount = 0;
	for (int step = 0; step < maxSteps; ++step) {
		if (currentPos.x < volumeMin.x || currentPos.x > volumeMax.x ||
			currentPos.y < volumeMin.y || currentPos.y > volumeMax.y ||
			currentPos.z < volumeMin.z || currentPos.z > volumeMax.z) {
			// outColor = vec4(1,0,0, 1.0);
			break;
		}
	
		// Calculate what integer voxel coordinate this 3D point lands in
		ivec3 targetVoxelIndex = getVoxelIndex(currentPos, volumeMin, volumeMax, volumeDimension);
		// outColor.rgb = vec3(targetVoxelIndex) / float(volumeDimension); // return;
	
		// BRUTE FORCE SEARCH
		for (int i = 0; i < fragmentBufferSize; ++i) {
			// Optimization: Stop checking if we hit completely uninitialized fragments
			if (fragments[i].worldPosition == vec3(0.0)) {
				continue;
			}
	
			// Convert the stored fragment's world position to its voxel coordinates
			ivec3 fragVoxelIndex = getVoxelIndex(fragments[i].worldPosition, volumeMin, volumeMax, volumeDimension);
			// // If the fragment in the buffer matches where our ray currently is...
			if (targetVoxelIndex == fragVoxelIndex) {
				// Perfect hit! Draw it and stop marching this ray
				outColor += vec4(fragments[i].albedo.rgb, 1.0);
				voxelCount++;
			}
		}
	
		// // Advance the ray deeper into the volume
		currentPos += rayDir * stepSize;
	}
	outColor.rgb /= float(voxelCount);

}

