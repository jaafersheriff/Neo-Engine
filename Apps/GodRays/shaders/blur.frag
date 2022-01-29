
in vec2 fragTex;

uniform sampler2D godray;
uniform float blurSteps;

uniform mat4 P;
uniform mat4 V;

uniform float contribution;
uniform float decay;
uniform float density;
uniform float weight;
uniform vec2 sunPos;

out vec4 outcolor;

void main() {
   
    vec2 texCoords = fragTex;
    vec2 deltaTexCoords = density * (texCoords - sunPos) / blurSteps;
    float totalDecay = 1.0;
    vec4 sum = texture(godray, texCoords) * contribution;
    for(int i = 0; i < blurSteps; i++) {
        texCoords -= deltaTexCoords;
        vec4 currentSample = texture(godray, clamp(texCoords, 0.0, 1.0));
        sum += currentSample * contribution * totalDecay * weight;
        totalDecay *= decay;
    }
    outcolor = sum;

}

