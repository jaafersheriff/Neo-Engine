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


bool rayAABBIntersect(vec3 rayOrigin, vec3 rayDir, vec3 boxMin, vec3 boxMax, out float tNear, out float tFar) {
	// Correctly handle division by zero for all directions without breaking negative signs
    vec3 invD = 1.0 / (rayDir + sign(rayDir) * vec3(rayDir.x == 0.0 ? EP : 0.0, rayDir.y == 0.0 ? EP : 0.0, rayDir.z == 0.0 ? EP : 0.0));
    
    vec3 t0s = (boxMin - rayOrigin) * invD;
    vec3 t1s = (boxMax - rayOrigin) * invD;
    
    vec3 tmin3 = min(t0s, t1s);
    vec3 tmax3 = max(t0s, t1s);
    
    tNear = max(max(tmin3.x, tmin3.y), tmin3.z);
    tFar  = min(min(tmax3.x, tmax3.y), tmax3.z);
    
    // An intersection happens if tFar is greater than tNear, 
    // AND tFar is in front of the ray (tFar >= 0.0)
    return tFar >= tNear && tFar >= 0.0;
}


void main() {
	vec3 rayOrigin = cameraPos;
	vec3 rayDir = normalize(fragPos.xyz - cameraPos);

	bool cameraInside =
		(cameraPos.x >= volumeMin.x && cameraPos.x <= volumeMax.x) &&
		(cameraPos.y >= volumeMin.y && cameraPos.y <= volumeMax.y) &&
		(cameraPos.z >= volumeMin.z && cameraPos.z <= volumeMax.z);

	// world-space voxel size
	vec3 voxelSize = abs(volumeMax - volumeMin) / float(volumeDimension);

	// intersect with box to find entry point
	float tNear, tFar;
	if (!rayAABBIntersect(rayOrigin, rayDir, volumeMin, volumeMax, tNear, tFar)) {
		outColor = vec4(0.0);
		return;
	}

	float t = max(tNear, 0.0);
	vec3 pos = rayOrigin + rayDir * (t + 1e-4); // stepSize slightly in to avoid numerical boundary issues

	// compute starting voxel index
	ivec3 voxel = ivec3(clamp(floor((pos - volumeMin) / voxelSize), vec3(0), vec3(float(volumeDimension) - 1.0)));

	// DDA setup
	ivec3 stepSize = ivec3(sign(rayDir));
	vec3 voxelBoundX = volumeMin + (vec3(voxel) + vec3(stepSize.x > 0 ? 1.0 : 0.0, 0.0, 0.0)) * voxelSize.x;
	vec3 voxelBoundY = volumeMin + (vec3(voxel) + vec3(0.0, stepSize.y > 0 ? 1.0 : 0.0, 0.0)) * voxelSize.y;
	vec3 voxelBoundZ = volumeMin + (vec3(voxel) + vec3(0.0, 0.0, stepSize.z > 0 ? 1.0 : 0.0)) * voxelSize.z;

	float tMaxX = (abs(rayDir.x) < EP) ? INF : (voxelBoundX.x - rayOrigin.x) / rayDir.x;
	float tMaxY = (abs(rayDir.y) < EP) ? INF : (voxelBoundY.y - rayOrigin.y) / rayDir.y;
	float tMaxZ = (abs(rayDir.z) < EP) ? INF : (voxelBoundZ.z - rayOrigin.z) / rayDir.z;

	float tDeltaX = (abs(rayDir.x) < EP) ? INF : (voxelSize.x / abs(rayDir.x));
	float tDeltaY = (abs(rayDir.y) < EP) ? INF : (voxelSize.y / abs(rayDir.y));
	float tDeltaZ = (abs(rayDir.z) < EP) ? INF : (voxelSize.z / abs(rayDir.z));

	int maxSteps = 256;
	outColor = vec4(0, 0, 0, 0);

	vec3 finalColor = vec3(0.0);
	bool found = false;

	for (int i = 0; i < maxSteps; ++i) {
		// Ensure voxel is inside bounds
		if (voxel.x < 0 || voxel.x >= volumeDimension ||
			voxel.y < 0 || voxel.y >= volumeDimension ||
			voxel.z < 0 || voxel.z >= volumeDimension) {
			break;
		}

		// Look up voxel contents
		int index = getFlattenedIndex(voxel, volumeDimension);
		int header = headerPointers[index];

		if (header != -1) {
			// Make a solid per-voxel color (averaging nodes if multiple), not continuous ray accumulation
			vec3 voxelColor = vec3(0.0);
			int nodeCount = 0;
			int safety = 32;
			while (header != -1 && safety-- > 0) {
				vec4 albedo = unpackRGBA8(nodes[header].albedo);
				vec3 emissive = unpackR11G11B10(nodes[header].emissive);
				vec3 normal = unpackNormal(nodes[header].normal);
				voxelColor += albedo.rgb + emissive;

				nodeCount++;
				header = nodes[header].header;
			}
			if (nodeCount > 0) {
				voxelColor /= float(nodeCount);
			}

			finalColor = voxelColor;
			found = true;
			break; // stop at first occupied voxel -> solid voxel appearance
		}

		// Advance to next voxel using Amanatides & Woo
		if (tMaxX < tMaxY) {
			if (tMaxX < tMaxZ) {
				voxel.x += stepSize.x;
				t = tMaxX;
				tMaxX += tDeltaX;
			}
			else {
				voxel.z += stepSize.z;
				t = tMaxZ;
				tMaxZ += tDeltaZ;
			}
		}
		else {
			if (tMaxY < tMaxZ) {
				voxel.y += stepSize.y;
				t = tMaxY;
				tMaxY += tDeltaY;
			}
			else {
				voxel.z += stepSize.z;
				t = tMaxZ;
				tMaxZ += tDeltaZ;
			}
		}
		// bail out if we've passed the exit plane
		if (t > tFar) break;

		pos = rayOrigin + rayDir * (t + 1e-4); // update sample position for edge calculation
	}

	if (found) {
		outColor = vec4(finalColor, 1.0);
	}
	else {
		outColor = vec4(1, 0, 0, 0);
	}
}
