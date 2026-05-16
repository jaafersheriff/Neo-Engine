
in vec3 fragPos;
in vec3 volumePos;
in vec3 fragNor;
in vec2 fragTex;

uniform int volumeDimension;
uniform int outputBufferSize;

struct VoxelFragment {
	vec3 worldPosition;
	vec4 albedo;
    vec3 normal;
};

layout(std430, binding = 0) buffer VoxelFragments {
	VoxelFragment fragments[];
};

out vec3 color;

void main() {
    // Convert volume position from [-1, 1] to [0, 1] range
    vec3 voxelTexCoord = volumePos * 0.5 + 0.5;
    
    // Scale up to your actual integer grid address (e.g., 0 to 255)
    ivec3 voxelIndex = ivec3(voxelTexCoord * float(volumeDimension));

    int flattenedIndex = voxelIndex.x 
        + voxelIndex.y * volumeDimension
        + voxelIndex.z * volumeDimension * volumeDimension;

    // Guard bounds just in case a floating-point error pushes a edge vertex to 256
    if (voxelIndex.x >= 0 && voxelIndex.x < volumeDimension && 
        voxelIndex.y >= 0 && voxelIndex.y < volumeDimension && 
        voxelIndex.z >= 0 && voxelIndex.z < volumeDimension && 
        flattenedIndex < outputBufferSize) {

        fragments[flattenedIndex].worldPosition = fragPos;
        fragments[flattenedIndex].albedo = vec4(1); // TOOD
        fragments[flattenedIndex].normal = fragNor;
        color = vec3(1);
    }
    else {
        color = vec3(1,0,0);
    }
}