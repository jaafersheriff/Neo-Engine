
#define PI 3.141592653589
#define EP 1e-5
#define FP16_MAX 65504.0


#define saturate(_x) clamp(_x, 0.0, 1.0)
#define mul(_a, _b) ( (_a) * (_b) )

float linearizeDepth(float depth, float near, float far) {
	float z = depth * 2.0 - 1.0;
	float a = 2.0 * near * far / (far + near - z * (far - near));
	return (a - near) / (far - near);
}
