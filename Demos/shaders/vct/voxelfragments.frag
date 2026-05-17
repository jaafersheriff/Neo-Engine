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
    vec3 volumeNormPos = volumePos;
    if (
    volumeNormPos.x < 0.0 || volumeNormPos.x > 1.0
    || volumeNormPos.y < 0.0 || volumeNormPos.y > 1.0
    || volumeNormPos.z < 0.0 || volumeNormPos.z > 1.0
    ) {
        return;
    }
    
    // Scale up to your actual integer grid address (e.g., 0 to 255)
    ivec3 voxelIndex = clamp(ivec3(volumeNormPos * float(volumeDimension)), ivec3(0), ivec3(volumeDimension-1));
    uint flatVoxelIndex = getFlattenedIndex(voxelIndex, volumeDimension);
    
    int lock = atomicCompSwap(locks[flatVoxelIndex], -1, 1);

    // First come first serve, only one fragment written out per voxel
    if (lock == -1) {
        uint bufferIdx = atomicAdd(fragmentCounter, 1);
        if (bufferIdx < outputBufferSize) {
            fragments[bufferIdx].worldPosition = fragPos;
            fragments[bufferIdx].albedo = vec4(albedo, 1.0);
            fragments[bufferIdx].normal = normalize(fragNor);
            color = vec3(1);
        }
        else {
        color = vec3(0,1,0);
        }
    }
    else {
        color = vec3(1,0,0);
    }
}