
layout(location = 0) in vec3 vertPos;

uniform int lod;
uniform sampler3D volume;

out vec4 pos;
out vec4 col;

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
	uvec3 dims = uvec3(textureSize(volume, 0));
	// uvec3 coord = unflatten3D(gl_VertexID, dims);
    pos = vec4(vertPos, 1.0);
    col = texture(volume, vertPos / vec3(dims), 0);
}