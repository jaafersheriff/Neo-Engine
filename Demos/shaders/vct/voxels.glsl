struct VoxelNode {
	float aR, aG, aB, aA; // TODO - pack
    float nX, nY, nZ; // TODO - pack
    int header;
};

ivec3 getVoxelIndex(vec3 worldPos, vec3 volumeWorldMin, vec3 volumeWorldMax, int volumeDimension) {
    vec3 _vmin = min(volumeWorldMin, volumeWorldMax);
    vec3 _vmax = max(volumeWorldMin, volumeWorldMax);
    vec3 relativePos = (worldPos - _vmin) / (_vmax - _vmin);
    return clamp(ivec3(floor(relativePos * float(volumeDimension))), ivec3(0), ivec3(volumeDimension-1));
}

uint generateMorton3D(ivec3 v) {
    uint x = uint(v.x);
    uint y = uint(v.y);
    uint z = uint(v.z);
    x = (x | (x << 16)) & 0x030000FF;
    x = (x | (x << 8)) & 0x0300F00F;
    x = (x | (x << 4)) & 0x030C30C3;
    x = (x | (x << 2)) & 0x09249249;
    y = (y | (y << 16)) & 0x030000FF;
    y = (y | (y << 8)) & 0x0300F00F;
    y = (y | (y << 4)) & 0x030C30C3;
    y = (y | (y << 2)) & 0x09249249;
    z = (z | (z << 16)) & 0x030000FF;
    z = (z | (z << 8)) & 0x0300F00F;
    z = (z | (z << 4)) & 0x030C30C3;
    z = (z | (z << 2)) & 0x09249249;
    return x | (y << 1) | (z << 2);
}

int getFlattenedIndex(ivec3 voxelIndex, int volumeDimension) {
    return 
    // generateMorton3D(ivec3(
        voxelIndex.x 
        + voxelIndex.y * volumeDimension
        + voxelIndex.z * volumeDimension * volumeDimension
    // ))
    ;
}

