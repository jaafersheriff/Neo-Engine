
#include "vct/voxels.glsl"

in vec4 fragPos;

uniform vec3 cameraPos;
uniform vec3 cameraDir;

uniform int volumeDimension;
uniform vec3 volumeMin; // world space
uniform vec3 volumeMax; // world space

layout(std430, binding = 0) coherent buffer VoxelNodes {
	VoxelNode nodes[];
};

layout(std430, binding = 1) coherent buffer HeaderPointers {
	int headerPointers[];
};

out vec4 outColor;

void main() {
	vec3 rayOrigin = cameraPos;
	vec3 rayDir = normalize(fragPos.xyz - cameraPos);
	bool cameraInside =
		(cameraPos.x >= volumeMin.x && cameraPos.x <= volumeMax.x) &&
		(cameraPos.y >= volumeMin.y && cameraPos.y <= volumeMax.y) &&
		(cameraPos.z >= volumeMin.z && cameraPos.z <= volumeMax.z);
	if (cameraInside) {
		rayDir = -rayDir;
	}
 
	// A step size roughly matching a fraction of a voxel unit keeps it accurate.
	float voxelSize = abs(volumeMax.x - volumeMin.x) / float(volumeDimension);
	float stepSize = voxelSize * 0.25;
	int maxSteps = 256;

	vec3 currentPos = fragPos.xyz + rayDir * 0.01; // Start right at the surface of the debug box

	outColor = vec4(0,0,0,0);
	int nodeCount = 0;
	for (int step = 0; step < maxSteps; ++step) {
		// Stepped outside the volume
		if (currentPos.x < volumeMin.x || currentPos.x > volumeMax.x ||
			currentPos.y < volumeMin.y || currentPos.y > volumeMax.y ||
			currentPos.z < volumeMin.z || currentPos.z > volumeMax.z) {
			break;
		}
		// Tracing behind the camera
		if (dot(currentPos - cameraPos, cameraDir) < 0) {
			break;
		}

		// Calculate what integer voxel coordinate this 3D point lands in
		ivec3 targetVoxelIndex = getVoxelIndex(currentPos, volumeMin, volumeMax, volumeDimension);
		int flattenedIndex = getFlattenedIndex(targetVoxelIndex, volumeDimension);

		// Grab the topmost node for this voxel
		int header = headerPointers[flattenedIndex];
		int safety = maxSteps;
		while (header != -1 && safety > 0) {
			// vec3 normalizedCoord = vec3(targetVoxelIndex) / float(volumeDimension);
			// outColor.rgb += normalizedCoord;
			outColor.r += nodes[header].aR;
			outColor.g += nodes[header].aG;
			outColor.b += nodes[header].aB;
			outColor.a = nodes[header].aA;
			header = int(nodes[header].header);
			nodeCount++;
			safety--;
		}
		if (safety == 0) {
			outColor.rgb = vec3(1,0,0); return;
		}

		// Advance the ray deeper into the volume
		currentPos += rayDir * stepSize;
	}
	outColor.rgb /= float(nodeCount) + EP;

}

