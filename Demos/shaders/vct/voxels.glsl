struct VoxelFragment {
	vec3 worldPosition;
	vec4 albedo;
    vec3 normal;
};


int getFlattenedIndex(ivec3 voxelIndex, int volumeDimension) {
    return voxelIndex.x 
        + voxelIndex.y * volumeDimension
        + voxelIndex.z * volumeDimension * volumeDimension;

}