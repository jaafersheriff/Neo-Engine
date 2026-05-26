
// uv [0.0, 1.0] 
// layer [0, 5]
vec3 cubemapSamplingCoords(vec2 uv, int layer) {
    uv = 2.0 * uv - 1.0;
    uv.y *= -1.0;

    vec3 R;
    switch (layer) {
    case 0:
        R = vec3(1.0, uv.y, -uv.x);
        break;
    case 1:
        R = vec3(-1.0, uv.y, uv.x);
        break;
    case 2:
        R = vec3(uv.x, 1.0, -uv.y);
        break;
    case 3:
        R = vec3(uv.x, -1.0, uv.y);
        break;
    case 4:
        R = vec3(uv.x, uv.y, 1.0);
        break;
    case 5:
    default:
        R = vec3(-uv.x, uv.y, -1.0);
    }
    return normalize(R);
}