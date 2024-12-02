bool validCascade(vec4 shadowCoord) {
	return true
		&& shadowCoord.x > 0.0 && shadowCoord.x < 1.0
		&& shadowCoord.y > 0.0 && shadowCoord.y < 1.0
		&& shadowCoord.z > 0.0 && shadowCoord.z < 1.0
	;
}

// TODO - PCF
float getSingleShadow(vec4 shadowCoord, sampler2D shadowMap, int lod) {
	float bias = 0.002;
	if (validCascade(shadowCoord)) {
		return saturate(shadowCoord.z) - bias > textureLod(_shadowMap, saturate(shadowCoord.xy), lod).r ? 1.0 : 0.0;
	}
	return 0.f;
}

float getShadowVisibility(float sceneDepth, vec4 csmDepths, vec4 shadowCoord[4], sampler2D shadowMap) {
	int lod = 0;
	if (sceneDepth < csmDepths.x) {
		lod = 0;
	}
	else if (sceneDepth < csmDepths.y) {
		lod = 1;
	}
	else if (sceneDepth < csmDepths.z) {
		lod = 2;
	}
	else if (sceneDepth < csmDepths.w) {
		lod = 3;
	}

	float shadow = getSingleShadow(shadowCoord[lod], shadowMap, lod);
	return 1.0 - saturate(shadow);
}

float getShadowVisibility(int pcfSize, samplerCube shadowMap, vec3 shadowCoord, float shadowMapResolution, float shadowRange, float bias) {
	float worldDepth = length(shadowCoord) / shadowRange;

	float visibility = 1.0;
	if (pcfSize > 0) {
		float shadow = 0.0;
		float texelSize = 6.0 / shadowMapResolution;
		for (int x = -pcfSize; x <= pcfSize; x++) {
			for (int y = -pcfSize; y <= pcfSize; y++) {
				for (int z = -pcfSize; z <= pcfSize; z++) {
					float pcfDepth = texture(shadowMap, shadowCoord + vec3(x, y, z) * texelSize).r;
					shadow += worldDepth - bias < pcfDepth ? 1.0 : 0.0;
				}
			}
		}
		shadow /= (2 * pcfSize + 1) * (2 * pcfSize + 1);
		visibility = shadow;
	}
	else if (worldDepth - bias > texture(shadowMap, shadowCoord).r) {
		visibility = 0.0;
	}

	return saturate(visibility);
}