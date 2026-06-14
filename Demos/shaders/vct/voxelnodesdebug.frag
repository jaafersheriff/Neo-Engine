#include "vct/voxels.glsl"
#include "color.glsl"

in vec4 fragPos;

uniform vec3 cameraPos;
uniform vec3 cameraDir;

uniform uint volumeDimension;
uniform vec3 volumeMin; // world space
uniform vec3 volumeMax; // world space

uniform uint bricksPerAxis;
uniform uint voxelsPerBrick;
uniform ivec3 brickResolution; // Dimensions of the BrickTexture in terms of total bricks (e.g., 16x16x16 bricks)

layout(r32i, binding = 1) coherent uniform iimage3D BrickPointers;
layout(rgba32ui, binding = 2) readonly uniform uimage3D BrickTexture; 
out vec4 outColor;

#ifdef DEBUG_BRICKS
vec3 brickColor(int id) {
	if (id < 0) return vec3(0.0);

	uint x = uint(id) * 1664525u + 1013904223u;
	float r = float((x >>  0) & 255u) / 255.0;
	float g = float((x >>  8) & 255u) / 255.0;
	float b = float((x >> 16) & 255u) / 255.0;
	return vec3(r, g, b);
}
#endif

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

	// intersect with box to find entry point
	float tNear, tFar;
	if (!rayAABBIntersect(rayOrigin, rayDir, volumeMin, volumeMax, tNear, tFar)) {
		outColor = vec4(0.0);
		return;
	}

	// DDA setup
	float t = max(tNear, 0.0);
	vec3 pos = rayOrigin + rayDir * (t + EP); // stepDir slightly in to avoid numerical boundary issues
	ivec3 stepDir = ivec3(sign(rayDir));

	// compute starting index
#ifdef DEBUG_BRICKS
	vec3 brickSize = abs(volumeMax - volumeMin) / float(bricksPerAxis); // brick size world space
	vec3 volumePos = (pos - volumeMin) / (volumeMax - volumeMin); // start pos volume space
	ivec3 currentBrick = ivec3(clamp(floor(volumePos * float(bricksPerAxis)), vec3(0), vec3(bricksPerAxis - 1)));

	vec3 stepSize = brickSize;
	ivec3 currentIndex = currentBrick;
	int maxDimension = int(bricksPerAxis);
#else
	vec3 voxelSize = abs(volumeMax - volumeMin) / float(volumeDimension); // world space
	ivec3 currentVoxel = ivec3(clamp(floor((pos - volumeMin) / voxelSize), vec3(0), vec3(float(volumeDimension) - 1.0))); 

	vec3 stepSize = voxelSize;
	ivec3 currentIndex = currentVoxel;
	int maxDimension = int(volumeDimension);
#endif

	vec3 nextBoundaryX = volumeMin + (vec3(currentIndex) + vec3(stepDir.x > 0 ? 1.0 : 0.0, 0.0, 0.0)) * stepSize.x;
	vec3 nextBoundaryY = volumeMin + (vec3(currentIndex) + vec3(0.0, stepDir.y > 0 ? 1.0 : 0.0, 0.0)) * stepSize.y;
	vec3 nextBoundaryZ = volumeMin + (vec3(currentIndex) + vec3(0.0, 0.0, stepDir.z > 0 ? 1.0 : 0.0)) * stepSize.z;

	float tMaxX = (abs(rayDir.x) < EP) ? INF : (nextBoundaryX.x - rayOrigin.x) / rayDir.x;
	float tMaxY = (abs(rayDir.y) < EP) ? INF : (nextBoundaryY.y - rayOrigin.y) / rayDir.y;
	float tMaxZ = (abs(rayDir.z) < EP) ? INF : (nextBoundaryZ.z - rayOrigin.z) / rayDir.z;

	float tDeltaX = (abs(rayDir.x) < EP) ? INF : (stepSize.x / abs(rayDir.x));
	float tDeltaY = (abs(rayDir.y) < EP) ? INF : (stepSize.y / abs(rayDir.y));
	float tDeltaZ = (abs(rayDir.z) < EP) ? INF : (stepSize.z / abs(rayDir.z));

	int maxSteps = 256;
	outColor = vec4(0, 0, 0, 0);

	vec3 finalColor = vec3(0.0);
	bool found = false;

	for (int i = 0; i < maxSteps; ++i) {
		// Ensure voxel is inside bounds
		if (currentIndex.x < 0 || currentIndex.x >= maxDimension ||
			currentIndex.y < 0 || currentIndex.y >= maxDimension ||
			currentIndex.z < 0 || currentIndex.z >= maxDimension) {
			break;
		}


#ifdef DEBUG_BRICKS
		// Indexing directly into the pointer map grid
		int brickID = imageLoad(BrickPointers, currentIndex).r;
		if (brickID >= 0) {
			found = true;
			finalColor = brickColor(brickID);
			break;
		}
#else
		// 1. Calculate macro grid (brick) index and local voxel within that brick
		ivec3 macroGridIdx = currentIndex / int(voxelsPerBrick);
		ivec3 localVoxelIdx = currentIndex % int(voxelsPerBrick);

		int brickIdx = imageLoad(BrickPointers, macroGridIdx).r;

		// Check if brick is allocated (matching your initialization default of -1)
		if (brickIdx == -1) {
		}
		else {
			
			// 2. Map the 1D flat brick ID into physical 3D brick grid coordinates
			ivec3 brick3DOffset = ivec3(
				brickIdx % brickResolution.x,
				(brickIdx / brickResolution.x) % brickResolution.y,
				brickIdx / (brickResolution.x * brickResolution.y)
			);
			
			// 3. Match the allocation spacing: stride by (voxelsPerBrick + 2) due to padding 
			ivec3 brickPhysicalStart = brick3DOffset * int(voxelsPerBrick + 2);
			
			// 4. Offset by 1 to skip the boundary padding texel and add local coordinate
			ivec3 readTexelCoord = brickPhysicalStart + ivec3(1) + localVoxelIdx;

			// 5. Read and decode the data exactly how you packed it
			uvec4 packedData = imageLoad(BrickTexture, readTexelCoord);
			
			// If the voxel is completely unwritten/empty (checking albedo payload alpha or rgb)
			if (packedData.g != 0u || packedData.b != 0u) {
				vec4 albedo = unpackRGBA8(packedData.g);
				vec3 emissive = unpackR11G11B10(packedData.b);
				vec3 normal = normalize(unpackNormal(packedData.a));

				// Visual representation of the voxel data
				finalColor = albedo.rgb + emissive;
				found = true;
				break; 
			}
		}
#endif

		// Advance to next voxel using Amanatides & Woo
		if (tMaxX < tMaxY) {
			if (tMaxX < tMaxZ) {
				currentIndex.x += stepDir.x;
				t = tMaxX;
				tMaxX += tDeltaX;
			}
			else {
				currentIndex.z += stepDir.z;
				t = tMaxZ;
				tMaxZ += tDeltaZ;
			}
		}
		else {
			if (tMaxY < tMaxZ) {
				currentIndex.y += stepDir.y;
				t = tMaxY;
				tMaxY += tDeltaY;
			}
			else {
				currentIndex.z += stepDir.z;
				t = tMaxZ;
				tMaxZ += tDeltaZ;
			}
		}
		// bail out if we've passed the exit plane
		if (t > tFar) break;

		pos = rayOrigin + rayDir * (t + EP); // update sample position for edge calculation
	}

	if (found) {
		outColor = vec4(finalColor, 1.0);
	}
	else {
		outColor = vec4(0);
	}
}
