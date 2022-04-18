in vec2 fragTex;

out vec4 color;

uniform vec3  lookDir;
uniform vec3  upDir;
uniform vec3  rightDir;
uniform float near;
uniform float far;
uniform float fov;
uniform float ar;

uniform int lod;
uniform vec3 dims;
uniform sampler3D volume;

void main() {
    // [-1, 1]
    vec2 pcoords = fragTex * 2.0 - 1.0;

    float hFar = 2 * tan(fov / 2) * far;
    float wFar = hFar * ar;
    vec2 far = vec2(0);
    far += vec2(0,pcoords.y) * (hFar / 2);
    far += vec2(pcoords.x,0) * (wFar / 2);

    vec3 start = vec3(pcoords, 0.0);
    vec3 end = vec3(normalize(far), 1.0);
    vec3 dir = normalize(end - start);

    color = vec4(0.0);
    uint numberOfSteps = 16;
    float stepLength = length(end - start) / numberOfSteps;
    for(uint i = 0; i < numberOfSteps && color.a < 0.99f; ++i) {
        // [-1, 1]
       vec3 currentPoint = start + stepLength * i * dir;
        // [0, 1]
       vec3 coordinate = currentPoint * 0.5 + 0.5;
       vec4 currentSample = textureLod(volume, currentPoint, lod);
       color += (1.0f - color.a) * currentSample;
    }
	// color = vec4(color, 1.0);
}