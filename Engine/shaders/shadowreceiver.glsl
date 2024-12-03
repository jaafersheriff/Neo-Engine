bool validCascade(vec4 _shadowCoord) {
	return true
		&& _shadowCoord.x > 0.0 && _shadowCoord.x < 1.0
		&& _shadowCoord.y > 0.0 && _shadowCoord.y < 1.0
		&& _shadowCoord.z > 0.0 && _shadowCoord.z < 1.0
	;
}

// TODO - PCF
float getSingleShadow(vec4 _shadowCoord, sampler2D _shadowMap, int _lod) {
	float bias = 0.0001;
	if (validCascade(_shadowCoord)) {
		return saturate(_shadowCoord.z) + bias > textureLod(_shadowMap, saturate(_shadowCoord.xy), _lod).r ? 1.0 : 0.0;
	}
	return 0.f;
}

float getCSMShadowVisibility(float _sceneDepth, vec4 _csmDepths, vec4 _shadowCoord[4], sampler2D _shadowMap) {
	int lod = 0;
	if (_sceneDepth < _csmDepths.x) {
		lod = 0;
	}
	else if (_sceneDepth < _csmDepths.y) {
		lod = 1;
	}
	else if (_sceneDepth < _csmDepths.z) {
		lod = 2;
	}
	else if (_sceneDepth < _csmDepths.w) {
		lod = 3;
	}

	float shadow = getSingleShadow(_shadowCoord[lod], _shadowMap, lod);
	return 1.0 - saturate(shadow);
}

float getShadowVisibility(int pcfSize, samplerCube _shadowMap, vec3 _shadowCoord, float _shadowMapResolution, float shadowRange, float bias) {
	float worldDepth = length(_shadowCoord) / shadowRange;

	float visibility = 1.0;
	if (pcfSize > 0) {
		float shadow = 0.0;
		float texelSize = 6.0 / _shadowMapResolution;
		for (int x = -pcfSize; x <= pcfSize; x++) {
			for (int y = -pcfSize; y <= pcfSize; y++) {
				for (int z = -pcfSize; z <= pcfSize; z++) {
					float pcfDepth = texture(_shadowMap, _shadowCoord + vec3(x, y, z) * texelSize).r;
					shadow += worldDepth - bias < pcfDepth ? 1.0 : 0.0;
				}
			}
		}
		shadow /= (2 * pcfSize + 1) * (2 * pcfSize + 1);
		visibility = shadow;
	}
	else if (worldDepth - bias > texture(_shadowMap, _shadowCoord).r) {
		visibility = 0.0;
	}

	return saturate(visibility);
}