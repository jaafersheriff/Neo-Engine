in vec3 fragPos;
in vec3 fragNor;

out vec4 color;

uniform bool wf;
uniform vec3 lightPos;
uniform float t;

void main() {
    if (wf) {
        color = vec4(1);
    }
    else {
        vec3 c = vec3(1,0,1);
        vec3 N = normalize(fragNor);
        vec3 lightDir = lightPos - fragPos;
        vec3 L = normalize(lightDir);
        float lambert = clamp(dot(L, N), 0.0, 1.0);
        color = vec4(
                c * 0.2 + 
                c * lambert
                , 1.0);
    }
}