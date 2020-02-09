
#include "postprocess.glsl"

uniform vec3 focalPoints; 

out vec4 outColor;

// DofDown
void main() {
    float depth = texture(inputDepth, fragTex).r;
    // remap depth from [0, 1] to [-1 ,1]
    depth = depth * 2.0 - 1.0;

    float f = 0.0;
    if (depth < focalPoints.y) {
        // scale depth to [-1, 0]
        f = (depth - focalPoints.y) / (focalPoints.y - focalPoints.x);
    }
    else {
        // scale depth to [0, 1]
        f = (depth - focalPoints.y) / (focalPoints.z - focalPoints.y);

        // TODO - clamp to max depth blur?
    }

    // scale f to [0, 1]
    f = f * 0.5 + 0.5;

    outColor = vec4(depth, f, 0.0, 1.0);
}