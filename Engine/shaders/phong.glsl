
vec3 getPhong(vec3 V, vec3 N, vec3 L, vec3 ambientColor, vec3 albedo,  vec3 specularColor, float shine, vec3 lightCol, float attFactor) {

	vec3 H = normalize(L + V);
	float lambert = clamp(dot(L, N), 0.0, 1.0);
	vec3 diffuseContrib = lightCol * lambert / attFactor;
	vec3 specularContrib = lightCol * pow(clamp(dot(H, N), 0.0, 1.0), shine) / attFactor;

	return
		albedo.rgb * ambientColor
		+ albedo.rgb * diffuseContrib
		+ specularColor * specularContrib;
}
