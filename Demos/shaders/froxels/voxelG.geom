layout (points) in;
layout (triangle_strip, max_vertices = 14) out;

uniform mat4 mainP, mainV;
uniform mat4 persP, persV;
uniform mat4 orthoP, orthoV;

uniform vec3 dims;
uniform vec3 voxelSize;

in vec4 pos[];
in vec4 col[];

out vec3 fragColor;

// Creates a unit cube triangle strip from just vertex ID (14 vertices)
vec3 vertexID_create_cube(in uint vertexID)
{
	uint b = 1u << vertexID;
	return vec3((0x287a & b) != 0, (0x02af & b) != 0, (0x31e3 & b) != 0);
}

void main() {
    if(col[0].a > 0.5) {
        fragColor = col[0].rgb;

            // Sizzle is a half fix -- there's another multiplication needed
            // Ortho camera is dicated by light's lookdir lmao
		for (uint i = 0; i < 14; i++) {
            vec3 endpos = pos[0].xyz; // [0, dims]
            // endpos -= dims; endpos.z *= -1;
            endpos = endpos.xyz / dims * 2 - 1; // [0, dims] / dims = [0, 1] * 2 - 1 = [-1, 1]
            endpos *= dims; // [-dims, dims]
            endpos += (vertexID_create_cube(i) - vec3(0, 1, 0)) * 2; // [-dims + cubeVertPos, dims + cubeVertPos]
            endpos *= dims * voxelSize / dims; // TODO
            gl_Position = persP * persV * vec4(endpos.xyz, 1.0);
            // gl_Position = orthoP * orthoV * vec4(endpos.xyz, 1.0);
            EmitVertex();
        }

        EndPrimitive();
    }
}  
