
layout(location = 0) in vec3 vertPos;

uniform int lod;
uniform sampler3D volume;

out vec4 gPos;
out vec4 gCol;

uint flatten3D(uvec3 coord, uvec3 dim) {
	return (coord.z * dim.x * dim.y) + (coord.y * dim.x) + coord.x;
}
// flattened array index to 3D array index
uvec3 unflatten3D(uint idx, uvec3 dim)
{
	uint z = idx / (dim.x * dim.y);
	idx -= (z * dim.x * dim.y);
	uint y = idx / dim.x;
	uint x = idx % dim.x;
	return  uvec3(x, y, z);
}

void main() {
	// uvec3 coord = unflatten3D(gl_VertexID, dims);
    gPos = vec4(vertPos, 1.0);
    gCol = texelFetch(volume, ivec3(vertPos), 0);
}