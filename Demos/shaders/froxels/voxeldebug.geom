layout (points) in;
layout (triangle_strip, max_vertices = 14) out;

uniform mat4 mainP, mainV;
uniform mat4 persP, persV;
uniform mat4 orthoP, orthoV;

uniform vec3  camPos;
uniform vec3  lookDir;
uniform vec3  upDir;
uniform vec3  rightDir;
uniform float camNear;
uniform float camFar;
uniform float fov;
uniform float ar;

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

mat4 scaleMat(vec3 c) {
	return mat4(c.x, 0, 0, 0,
	            0, c.y, 0, 0,
	            0, 0, c.z, 0,
	            0, 0, 0, 1);
}

void main() {
if (gCol[0].a > 0.05) {
        fragColor = gCol[0].rgb;

            // Ortho camera is dicated by light's lookdir lmao
		for (uint i = 0; i < 14; i++) {
            // vec3 pos = gPos[0].xyz; // [0, dims]
            // pos = pos.xyz / dims * 2.0 - 1.0; // [0, dims] / dims = [0, 1] * 2 - 1 = [-1, 1]
            // pos *= dims; // [-dims, dims]

            // // TODO - I think voxelsize is busted
            // pos += (vertexID_create_cube(i) * 2.0 - 1.0) ;// * voxelSize * 2.0;
            // pos *= voxelSize;

            // // offset
            // vec3 look = normalize(persLookDir);
            // pos += persCamPos;
            // pos += look * persCamNear;
            // pos += look * (persCamFar - persCamNear) / 2.0;

            // gl_Position = mainP * mainV * vec4(pos.xyz, 1.0);
            // EmitVertex();

            vec3 index = gPos[0].xyz;
            // vec3 cNear = camPos + lookDir * camNear;
            // vec3 cFar = camPos + lookDir * camFar;
            // vec3 cCenter = camPos + lookDir * camNear + lookDir * (camFar - camNear) / 2.0;
            // float hNear = 2 * tan(fov / 2) * camNear;
            // float hFar = 2 * tan(fov / 2) * camFar;
            // float wNear = hNear * ar;
            // float wFar = hFar * ar;
            // vec3 nearBottomLeft = cNear - (upDir * (hNear / 2)) - (rightDir * (wNear / 2));
            // vec3 farRightTop = cFar + (upDir * (hFar / 2)) + (rightDir * (wFar / 2));

            float _camNear = camNear + (camFar - camNear) * (index.z) / dims.z;
            float _camFar = _camNear + (camFar - camNear) * (index.z + 1) / dims.z;
            float _hNear = 2 * tan(fov / 2) * _camNear;
            float _hFar = 2 * tan(fov / 2) * _camFar;
            float _wNear = _hNear * ar;
            float _wFar = _hFar * ar;
            vec3 _cNear = camPos + lookDir * _camNear;
            vec3 _cFar = camPos + lookDir * _camFar;
            vec3 _nearBottomLeft = _cNear - (upDir * (_hNear / 2)) - (rightDir * (_wNear / 2));
            vec3 _nearTopLeft = _cNear + (upDir * (_hNear / 2)) - (rightDir * (_wNear / 2));
            vec3 _nearRightBottom = _cNear - (upDir * (_hNear / 2)) + (rightDir * (_wNear / 2));
            vec3 _farLeftBottom = _cFar - (upDir * (_hFar / 2)) - (rightDir * (_wFar / 2));
            vec3 _farRightTop = _cFar + (upDir * (_hFar / 2)) + (rightDir * (_wFar / 2));
            vec3 scale = vec3(0.1);
            scale.x = distance(_nearBottomLeft, _nearRightBottom) / dims.x;
            scale.y = distance(_nearBottomLeft, _nearTopLeft) / dims.y;
            scale.z = distance(_farLeftBottom, _nearBottomLeft) / dims.z;
            vec3 endPos = _nearBottomLeft;
            endPos += rightDir * index.x * scale.x;
            endPos += upDir * index.y * scale.y;
            // dont do thisendPos += lookDir * index.z * scale.z;
            vec3 cubeVert = (vertexID_create_cube(i)) * scale; // TODO - voxelsize

        // fragColor = gPos[0].rgb / dims;
            gl_Position = mainP * mainV * vec4(endPos + cubeVert, 1.0);
            EmitVertex();
        }

        EndPrimitive();
}
}  
