#include "vct/voxels.glsl"
#include "alphaDiscard.glsl"
#include "color.glsl"

in vec3 fragPos;
in vec3 fragNor;
in vec2 fragTex;

uniform uint volumeDimension;
uniform vec3 volumeMin;
uniform vec3 volumeMax;
uniform uint brickSize;
uniform uint maxBricks;
uniform ivec3 brickResolution;

uniform vec4 albedo;
uniform vec3 emissive;

layout(binding = 0) uniform sampler2D albedoMap;

layout(r32i, binding = 1) coherent uniform iimage3D BrickPointers;
layout(rgba32ui, binding = 2) writeonly uniform uimage3D BrickTexture;
layout(std430, binding = 3) coherent buffer BrickCounter {
       uint brickCounter;
};

out vec3 outColor;

void main() {
	vec4 fAlbedo = albedo;
#ifdef ALBEDO_MAP
	fAlbedo *= srgbToLinear(texture(albedoMap, fragTex));
	alphaDiscard(fAlbedo.a);
#endif

	// Logical space
	uvec3 voxelIndex = getVoxelIndex(fragPos, volumeMin, volumeMax, volumeDimension);
	uvec3 macroGridIdx = voxelIndex / int(brickSize);
	uvec3 localVoxelIdx = voxelIndex % int(brickSize);
	
	int brickIdx = imageLoad(BrickPointers, ivec3(macroGridIdx)).r;
	if (brickIdx == -1) {
		// Try to claim a new brick ID safely using an atomic exchange loop
		// or pre-pass allocation. For an injection pass, atomicCompSwap protects allocation:
		int newBrickIdx = int(atomicAdd(brickCounter, 1));
		
		if (newBrickIdx >= maxBricks) {
			outColor = vec3(1, 0, 0); // Out of memory indicator
			discard;
		}
		
		// Attempt to store our allocated brick ID into the header pointer map
		int oldIdx = imageAtomicCompSwap(BrickPointers, ivec3(macroGridIdx), -1, newBrickIdx);

		if (oldIdx == -1) {
			// Success! We won the race condition and allocated this brick
			brickIdx = newBrickIdx;
		} else {
			// We lost the race; another fragment allocated a brick here already.
			// Give our allocated index back (or just let it leak for this frame, 
			// though compaction passes usually handle this cleaner).
			brickIdx = oldIdx;
		}
	}

	// Physical space
	ivec3 brick3DOffset = ivec3(
		brickIdx % brickResolution.x,
		(brickIdx / brickResolution.x) % brickResolution.y,
		brickIdx / (brickResolution.x * brickResolution.y)
	);
	ivec3 brickPhysicalStart = brick3DOffset * (int(brickSize) + 2); // Padding
	ivec3 writeTexelCoord = brickPhysicalStart + ivec3(1) + ivec3(localVoxelIdx); // +1 for padding
	
	// TODO - use atomicMaxExchange or something
	imageStore(BrickTexture, writeTexelCoord, uvec4(
		0u, // TODO - luminance/albedo
		packRGBA8(fAlbedo),
		packR11G11B10(emissive),
		packNormal(fragNor)
	));

	memoryBarrierBuffer();

	outColor = vec3(voxelIndex) / float(volumeDimension);
	// outColor = fAlbedo.rgb;
}