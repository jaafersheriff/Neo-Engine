layout(location = 0) in vec3 vertPos;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out vec4 fragPos;

#ifdef BOUNDING_BOX
uniform vec3 bbMin;
uniform vec3 bbMax;
layout(binding = 0) uniform sampler2D hiZ;
uniform int hiZMips;
uniform vec2 hiZDimension;
#endif

out int isVisible;
out vec3 debugColor;

void main() {
	fragPos = M * vec4(vertPos, 1.0);
	gl_Position = P * V * fragPos;

	isVisible = 1;
#ifdef BOUNDING_BOX
	// Clip space
	vec4 csMin = P * V * M * vec4(bbMin, 1.0); 
	vec4 csMax = P * V * M * vec4(bbMax, 1.0);

	// NDC space
	vec4 ndcMin = csMin;
	ndcMin.xyz /= ndcMin.w;
	vec4 ndcMax = csMax;
	ndcMax.xyz /= ndcMax.w;
	// xform so it can sample a texture
	ndcMin.xy = ndcMin.xy * 0.5 + 0.5;
	ndcMax.xy = ndcMax.xy * 0.5 + 0.5;

	float viewSizeX = (ndcMax.x - ndcMin.x) * (hiZDimension.x);
	float viewSizeY = (ndcMax.y - ndcMin.y) * (hiZDimension.y);
	float LOD = ceil(log2(max(viewSizeX, viewSizeY) / 2.0));

	float nearestBoxDepth = max(ndcMin.z, ndcMax.z);

	vec2 sampleCoords = vec2(ndcMin.x + (ndcMax.x - ndcMin.x) / 2.0, ndcMin.y + (ndcMax.y - ndcMin.y) / 2.0);
	debugColor = vec3(sampleCoords.x);
	float hiZSample = textureLod(hiZ, sampleCoords, LOD).r;
	isVisible = nearestBoxDepth <= hiZSample ? 1 : 0;
#endif
}
