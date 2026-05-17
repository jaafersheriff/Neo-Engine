struct VoxelFragment {
	vec3 worldPosition;
	vec4 albedo;
    vec3 normal;
};

ivec3 getVoxelIndex(vec3 worldPos, vec3 volumeWorldMin, vec3 volumeWorldMax, int volumeDimension) {
    vec3 relativePos = (worldPos - volumeWorldMin) / (volumeWorldMax - volumeWorldMin);
    return clamp(ivec3(floor(relativePos * float(volumeDimension))), ivec3(0), ivec3(volumeDimension-1));
}

int getFlattenedIndex(ivec3 voxelIndex, int volumeDimension) {
    return voxelIndex.x 
        + voxelIndex.y * volumeDimension
        + voxelIndex.z * volumeDimension * volumeDimension;

}