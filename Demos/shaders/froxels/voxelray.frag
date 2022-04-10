
in vec4 mock_gl_Position;

uniform sampler3D volume;
uniform vec3 camPos;
uniform float near;
uniform float far;

uniform float StepSize;
uniform int Iterations;

out vec4 color;

// ivec3 getVoxelIndex() {
// }

void main() {
    vec3 front = vec3(vec2(0.5), 0.0);
    vec3 back = normalize(mock_gl_Position.xyz / mock_gl_Position.w);
    vec3 dir = normalize(back - front);
    vec3 pos = vec3(front);
 
    vec4 dst = vec4(0, 0, 0, 0);
    vec4 src = vec4(0);
 
    vec4 value = vec4(0);
 
    vec3 Step = dir * StepSize;
 
    for(int i = 0; i < Iterations; i++) {
        value = texture(volume, pos.xyz);
        src = value;
        dst = src;

        //Front to back blending
        // // dst.rgb = dst.rgb + (1 - dst.a) * src.a * src.rgb
        // // dst.a   = dst.a   + (1 - dst.a) * src.a     
        // src.rgb *= src.a;
        // dst = (1.0f - dst.a)*src + dst;     
        //     //break from the loop when alpha gets high enough
        if(dst.a >= .95f)
        break; 
        //     //advance the current position
        pos.xyz += Step;
        //     //break if the position is greater than <1, 1, 1>
        if(pos.x > 1.0f || pos.y > 1.0f || pos.z > 1.0f)
        break;
    }
 
    color = dst;
}
