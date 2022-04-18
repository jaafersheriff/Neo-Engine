out vec4 color;

uniform mat4 P, V;
uniform vec2 bufferSize;

uniform vec3  lookDir;
uniform vec3  upDir;
uniform vec3  rightDir;
uniform float near;
uniform float far;
uniform float fov;
uniform float ar;



void main() {
	// [0, 1]
	vec2 pcoords = vec2(gl_FragCoord.xy / bufferSize);
    // [-1, 1]
    pcoords = pcoords * 2.0 - 1.0;

    float hFar = 2 * tan(fov / 2) * far;
    float wFar = hFar * ar;
    vec2 far = vec2(0);
    far += vec2(0,pcoords.y) * (hFar / 2);
    far += vec2(pcoords.x,0) * (wFar / 2);

    vec3 start = vec3(pcoords, 0.0);
    vec3 end = vec3(normalize(far), 1.0);
    vec3 dir = normalize(end - start);
	color = vec4(dir, 1.0);
}