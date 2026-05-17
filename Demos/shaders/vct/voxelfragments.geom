layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec4 geomPos[];
in vec3 geomNor[];
in vec2 geomTex[];

out vec3 fragPos;
out vec3 volumePos;
out vec3 fragNor;
out vec2 fragTex;

void main() {
	// 1. Calculate the face normal using World Position for physical accuracy
	vec3 p1 = geomPos[1].xyz - geomPos[0].xyz;
	vec3 p2 = geomPos[2].xyz - geomPos[0].xyz;
	vec3 faceNormal = abs(cross(p1, p2));

	// 2. Find dominant axis
	uint dominantAxis = 0;
	if (faceNormal.y > faceNormal.x && faceNormal.y > faceNormal.z) {
		dominantAxis = 1;
	} else if (faceNormal.z > faceNormal.x && faceNormal.z > faceNormal.y) {
		dominantAxis = 2;
	}

	// 3. Project vertices using Volume Position so it fits the [-1, 1] NDC space
	for (int i = 0; i < 3; ++i) {
		fragPos = geomPos[i].xyz;
		fragNor = geomNor[i];
		fragTex = geomTex[i];

		// Swizzle the Volume Position (-1 to 1) into gl_Position
		if (dominantAxis == 0) {
			gl_Position = vec4(gl_in[i].gl_Position.y, gl_in[i].gl_Position.z, 0.0, 1.0);
		} else if (dominantAxis == 1) {
			gl_Position = vec4(gl_in[i].gl_Position.x, gl_in[i].gl_Position.z, 0.0, 1.0);
		} else {
			gl_Position = vec4(gl_in[i].gl_Position.x, gl_in[i].gl_Position.y, 0.0, 1.0);
		}

		volumePos = gl_Position.xyz / gl_Position.w;

		EmitVertex();
	}
	EndPrimitive();
}  
