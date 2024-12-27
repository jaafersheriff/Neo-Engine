bool validCascade(vec4 _shadowCoord) {
	return true
		&& _shadowCoord.x >= 0.0 && _shadowCoord.x <= 1.0
		&& _shadowCoord.y >= 0.0 && _shadowCoord.y <= 1.0
		&& _shadowCoord.z >= 0.0 && _shadowCoord.z <= 1.0
	;
}

float getSingleShadow(float _pcfSize, vec4 _shadowCoord, sampler2D _shadowMap, float _shadowMapResolution, int _lod, float _bias) {
	if (_pcfSize > 0.0) {
		float shadow = 0.0;
		float texelSize = 1.0 / _shadowMapResolution;
		for (float x = -_pcfSize; x <= _pcfSize; x++) {
			for (float y = -_pcfSize; y <= _pcfSize; y++) {
				vec2 sampleCoords = saturate(_shadowCoord.xy + vec2(x, y) * texelSize);
				float shadowSample = textureLod(_shadowMap, sampleCoords, _lod).r * 0.5 + 0.5;
				shadow += saturate(_shadowCoord.z * 0.5 + 0.5) + _bias < shadowSample ? 1.0 : 0.0;
			}
		}
		shadow /= (2 * _pcfSize + 1) * (2 * _pcfSize + 1);
		return shadow;
	}
	else {
		float shadowSample = textureLod(_shadowMap, saturate(_shadowCoord.xy), _lod).r * 0.5 + 0.5;
		return saturate(_shadowCoord.z * 0.5 + 0.5) + _bias < shadowSample ? 1.0 : 0.0;
	}
}

float getCSMShadowVisibility(int _pcfSize, vec4 _shadowCoord[3], sampler2D _shadowMap, float _shadowMapResolution, float _bias) {
	int lod = 0;
	if (validCascade(_shadowCoord[0])) {
		lod = 0;
	}
	else if (validCascade(_shadowCoord[1])) {
		lod = 1;
	}
	else if (validCascade(_shadowCoord[2])) {
		lod = 2;
	}
	else {
		return 1.0;
	}

	float shadow = getSingleShadow(float(_pcfSize), _shadowCoord[lod], _shadowMap, _shadowMapResolution, lod, _bias);
	return saturate(shadow);
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
		shadow /= (2 * pcfSize + 1) * (2 * pcfSize + 1) * (2 * pcfSize + 1);
		visibility = shadow;
	}
	else if (worldDepth - bias > texture(_shadowMap, _shadowCoord).r) {
		visibility = 0.0;
	}

	return saturate(visibility);
}