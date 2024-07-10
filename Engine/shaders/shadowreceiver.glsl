
float getShadowVisibility(int pcfSize, sampler2D shadowMap, vec2 shadowMapResolution, vec4 shadowCoord, float bias) {
	if (shadowCoord.z < 0.0 || shadowCoord.z > 1.0) {
		return 1.0;
	}
	if (shadowCoord.x < 0.0 || shadowCoord.x > 1.0) {
		return 1.0;
	}
	if (shadowCoord.y < 0.0 || shadowCoord.y > 1.0) {
		return 1.0;
	}

	float visibility = 1.0;
	if (pcfSize > 0) {
		float shadow = 0.0;
		vec2 texelSize = 1.0 / shadowMapResolution;
		for (int x = -pcfSize; x <= pcfSize; x++) {
			for (int y = -pcfSize; y <= pcfSize; y++) {
				float pcfDepth = texture(shadowMap, shadowCoord.xy + vec2(x, y) * texelSize).r;
				shadow += shadowCoord.z - bias > pcfDepth ? 1.0 : 0.0;
			}
		}
		shadow /= (2 * pcfSize + 1) * (2 * pcfSize + 1);
		visibility = 1.0 - shadow;
	}
	else if (texture(shadowMap, shadowCoord.xy).r < shadowCoord.z - bias) {
		visibility = 0.0;
	}

	return saturate(visibility);
}

float getShadowVisibility(int pcfSize, samplerCube shadowMap, vec3 shadowCoord, float shadowMapResolution, float shadowRange, float bias) {
	if (abs(shadowCoord.x) > abs(shadowCoord.y) && abs(shadowCoord.x) > abs(shadowCoord.z)) {
		shadowCoord.y *= -1.0;
		shadowCoord.z *= -1.0;
	}
	else if (abs(shadowCoord.z) > abs(shadowCoord.y) && abs(shadowCoord.z) > abs(shadowCoord.x)) {
		shadowCoord.y *= -1.0;
		shadowCoord.x *= -1.0;
	}
	else {
		shadowCoord.x *= -1.0;
		shadowCoord.z *= -1.0;
	}
	float worldDepth = length(shadowCoord);

	float texelSize = 8.0 / shadowMapResolution;

	float visibility = 1.0;
	if (pcfSize > 0) {
		float shadow = 0.0;
		for (int x = -pcfSize; x <= pcfSize; x++) {
			for (int y = -pcfSize; y <= pcfSize; y++) {
				for (int z = -pcfSize; z <= pcfSize; z++) {
					float pcfDepth = texture(shadowMap, shadowCoord + vec3(x, y, z) * texelSize).r * shadowRange;
					shadow += worldDepth - bias < pcfDepth ? 1.0 : 0.0;
				}
			}
		}
		shadow /= (2 * pcfSize + 1) * (2 * pcfSize + 1);
		visibility = shadow;
	}
	else if (worldDepth - bias > texture(shadowMap, shadowCoord).r * shadowRange) {
		visibility = 0.0;
	}

	return saturate(visibility);
}