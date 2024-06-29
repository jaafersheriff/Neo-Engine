vec3 getNormal(vec3 modelNormal, vec3 texNormal, vec4 modelTangent) {
	mat3 TBN;

	vec3 tan = normalize(modelTangent.xyz);
	vec3 biTan = normalize(cross(modelNormal, tan)) * modelTangent.w;
	if (any(isnan(biTan))) {
		biTan = vec3(0, 1, 0); // uhhh
	}
	TBN = mat3(tan, biTan, modelNormal);

	vec3 tangentNormal = texNormal * 2.0 - 1.0;
	return normalize(TBN * normalize(tangentNormal));
}

vec3 getNormal(vec3 modelNormal, vec3 texNormal, vec3 worldPos, vec2 modelUV) {
	mat3 TBN;
	// http://www.thetenthplanet.de/archives/1180
	vec3 dp1 = dFdx( worldPos ); 
	vec3 dp2 = dFdy( worldPos ); 
	vec2 duv1 = dFdx( modelUV ); 
	vec2 duv2 = dFdy( modelUV );
	vec3 dp2perp = cross( dp2, modelNormal ); 
	vec3 dp1perp = cross( modelNormal, dp1 ); 
	vec3 T = dp2perp * duv1.x + dp1perp * duv2.x; 
	vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
	float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) ); 
	TBN = mat3( T * invmax, B * invmax, modelNormal );

	vec3 tangentNormal = texNormal * 2.0 - 1.0;
	return normalize(TBN * normalize(tangentNormal));
}


