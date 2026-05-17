#include "vct/voxels.glsl"

in vec3 fragPos;
in vec3 volumePos;
in vec3 fragNor;
in vec2 fragTex;

uniform int volumeDimension;
uniform int outputBufferSize;

uniform vec3 albedo;

layout(std430, binding = 0) buffer VoxelFragments {
	VoxelFragment fragments[];
};

layout(std430, binding = 1) buffer VoxelLocks {
	int locks[];
};

layout(std430, binding = 2) buffer FragmentCounter {
    uint fragmentCounter; 
};

out vec3 color;

void main() {
    // Convert volume position from [-1, 1] to [0, 1] range
    vec3 voxelTexCoord = volumePos * 0.5 + 0.5;
    
    // Scale up to your actual integer grid address (e.g., 0 to 255)
    ivec3 voxelIndex = ivec3(voxelTexCoord * float(volumeDimension));
    uint flatVoxelIndex = getFlattenedIndex(voxelIndex, volumeDimension);

    
    int lock = atomicCompSwap(locks[flatVoxelIndex], -1, 1);

    // First come first serve, only one fragment written out per voxel
    if (lock == -1) {
        uint bufferIdx = atomicAdd(fragmentCounter, 1);
        if (bufferIdx < outputBufferSize) {
            fragments[bufferIdx].worldPosition = fragPos;
            fragments[bufferIdx].albedo = vec4(albedo, 1.0);
            fragments[bufferIdx].worldPosition = fragPos;
            fragments[bufferIdx].albedo = vec4(albedo, 1.0);
            fragments[bufferIdx].normal = fragNor;
            color = vec3(1);
        }
    }
    else {
        color = vec3(1,0,0);
    }
}