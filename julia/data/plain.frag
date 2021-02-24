in  vec3 fragNorm;

out vec4 fragColor;

uniform PerInstance
{
	mat4 projection;
	mat4 view;
	mat4 model;
	mat3 modelIT;

	vec3 sunDirection;
	vec3 sunColor;
	vec4 ambientColor;

	vec4 ambientMaterial;
	vec3 diffuseMaterial;
};


void main()
{
	vec3 N = normalize(fragNorm);
	float NdotL = max(0.0, dot(N, sunDirection));
	vec3 diffuse = NdotL * sunColor * diffuseMaterial;
	fragColor = ambientColor * ambientMaterial + vec4(diffuse, 0);
}
