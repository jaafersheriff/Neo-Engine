#include "vct/voxels.glsl"
#include "alphaDiscard.glsl"
#include "color.glsl"

in vec3 fragPos;
in vec3 fragNor;
in vec2 fragTex;

uniform int volumeDimension;
uniform vec3 volumeMin;
uniform vec3 volumeMax;
uniform int numVoxels;
uniform int numNodes;

uniform vec4 albedo;
#ifdef ALBEDO_MAP
layout(binding = 0) uniform sampler2D albedoMap;
#endif

layout(std430, binding = 1) coherent buffer VoxelNodes {
	VoxelNode nodes[];
};

layout(std430, binding = 2) coherent buffer HeaderPointers {
	int headerPointers[];
};

layout(std430, binding = 3) coherent buffer NodeCounter {
	uint nodeCounter; 
};

out vec3 outColor;

void main() {
	ivec3 targetVoxelIndex = getVoxelIndex(fragPos, volumeMin, volumeMax, volumeDimension);
	int flattenedIndex = getFlattenedIndex(targetVoxelIndex, volumeDimension);

	int nodeIdx = int(atomicAdd(nodeCounter, 1));

	if (nodeIdx >= numNodes) {
		outColor = vec3(1,0,0);
		discard;
	}

	vec4 fAlbedo = albedo;
#ifdef ALBEDO_MAP
	fAlbedo *= srgbToLinear(texture(albedoMap, fragTex));
#endif
	alphaDiscard(fAlbedo.a);

	int oldHeader = atomicExchange(headerPointers[flattenedIndex], nodeIdx);
	nodes[nodeIdx].aR = fAlbedo.r;
	nodes[nodeIdx].aG = fAlbedo.g;
	nodes[nodeIdx].aB = fAlbedo.b;
	nodes[nodeIdx].aA = fAlbedo.a;
	vec3 n = normalize(fragNor);
	nodes[nodeIdx].nX = n.x;
	nodes[nodeIdx].nY = n.y;
	nodes[nodeIdx].nZ = n.z;
	nodes[nodeIdx].header = oldHeader;

	// TODO - remove heheheheheh
	// memoryBarrierBuffer();

	outColor = vec3(targetVoxelIndex) / float(volumeDimension);
}