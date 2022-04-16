layout (points) in;
layout (triangle_strip, max_vertices = 14) out;

uniform mat4 mainP, mainV;
uniform mat4 persP, persV;
uniform mat4 orthoP, orthoV;

uniform vec3 persCamPos;
uniform vec3 persLookDir;
uniform float persCamNear;
uniform float persCamFar;

uniform vec3 dims;
uniform vec3 voxelSize;

in vec4 gPos[];
in vec4 gCol[];

out vec3 fragColor;

// Creates a unit cube triangle strip from just vertex ID (14 vertices)
vec3 vertexID_create_cube(in uint vertexID)
{
	uint b = 1u << vertexID;
	return vec3((0x287a & b) != 0, (0x02af & b) != 0, (0x31e3 & b) != 0);
}

void main() {
if (gCol[0].a > 0.05) {
        fragColor = gCol[0].rgb;

            // Ortho camera is dicated by light's lookdir lmao
		for (uint i = 0; i < 14; i++) {
            vec3 pos = gPos[0].xyz; // [0, dims]
            pos = pos.xyz / dims * 2.0 - 1.0; // [0, dims] / dims = [0, 1] * 2 - 1 = [-1, 1]
            pos *= dims; // [-dims, dims]

            // TODO - I think voxelsize is busted
            pos += (vertexID_create_cube(i) * 2.0 - 1.0) ;// * voxelSize * 2.0;
            pos *= voxelSize;

            // offset
            vec3 look = normalize(persLookDir);
            pos += persCamPos;
            pos += look * persCamNear;
            pos += look * (persCamFar - persCamNear) / 2.0;

            gl_Position = mainP * mainV * vec4(pos.xyz, 1.0);
            EmitVertex();
        }

        EndPrimitive();
}
}  
