layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec4 geomPos[];
in vec3 geomNor[];
in vec2 geomTex[];

uniform int volumeDimension;

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

	// 3. Project vertices into standard [-1, 1] NDC space based on dominant axis
    vec4 projPos[3];
    for (int i = 0; i < 3; ++i) {
        if (dominantAxis == 0) {
            projPos[i] = vec4(gl_in[i].gl_Position.y, gl_in[i].gl_Position.z, -1.0, 1.0);
        } else if (dominantAxis == 1) {
            projPos[i] = vec4(gl_in[i].gl_Position.x, gl_in[i].gl_Position.z, -1.0, 1.0);
        } else {
            projPos[i] = vec4(gl_in[i].gl_Position.x, gl_in[i].gl_Position.y, -1.0, 1.0);
        }
    }

    // 4. Calculate outward edge normals for Conservative Rasterization
    // Half a voxel width in NDC is 1.0 / volumeDimension. We bloat by this amount to catch pixel centers.
    float halfVoxelNDC = 1.0 / float(volumeDimension); 

    vec2 e0 = projPos[1].xy - projPos[0].xy;
    vec2 e1 = projPos[2].xy - projPos[1].xy;
    vec2 e2 = projPos[0].xy - projPos[2].xy;

    // FIXED: Changed to (e.y, -e.x) so it correctly points OUTWARD by default
    vec2 n0 = normalize(vec2(e0.y, -e0.x));
    vec2 n1 = normalize(vec2(e1.y, -e1.x));
    vec2 n2 = normalize(vec2(e2.y, -e2.x));

    // Correct for winding order to ensure normals always face outward
    if ((e0.x * e1.y - e0.y * e1.x) < 0.0) {
        n0 = -n0; n1 = -n1; n2 = -n2;
    }

    // 5. Emit the expanded vertices
    for (int i = 0; i < 3; ++i) {
        fragPos = geomPos[i].xyz;
        fragNor = geomNor[i];
        fragTex = geomTex[i];

        // Safely convert the 3D volume position from [-1,1] to [0,1] 
        // without altering gl_Position and corrupting the fixed-function pipeline.
        volumePos = gl_in[i].gl_Position.xyz / gl_in[i].gl_Position.w * 0.5 + vec3(0.5);

        // Apply conservative bloat to the 2D raster position
        gl_Position = projPos[i];
        if (i == 0) gl_Position.xy += normalize(n0 + n2) * halfVoxelNDC;
        if (i == 1) gl_Position.xy += normalize(n1 + n0) * halfVoxelNDC;
        if (i == 2) gl_Position.xy += normalize(n2 + n1) * halfVoxelNDC;

        EmitVertex();
    }
	EndPrimitive();
}  
