#version 330 core

in vec4 position;
in vec3 normal;
in vec3 color;
in vec4 shadow;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec3 normalColor;

uniform vec3 to_light;
uniform sampler2DShadow shadowmap;

vec2 poisson_disk[16] = vec2[](vec2(-0.94201624, -0.39906216), 
							   vec2(0.94558609, -0.76890725), 
							   vec2(-0.094184101, -0.92938870), 
							   vec2(0.34495938, 0.29387760), 
							   vec2(-0.91588581, 0.45771432), 
							   vec2(-0.81544232, -0.87912464), 
							   vec2(-0.38277543, 0.27676845), 
							   vec2(0.97484398, 0.75648379), 
							   vec2(0.44323325, -0.97511554), 
							   vec2(0.53742981, -0.47373420), 
							   vec2(-0.26496911, -0.41893023), 
							   vec2(0.79197514, 0.19090188), 
							   vec2(-0.24188840, 0.99706507), 
							   vec2(-0.81409955, 0.91437590), 
							   vec2(0.19984126, 0.78641367), 
							   vec2(0.14383161, -0.14100790));

float random(vec3 seed, int i)
{
	vec4 seed4 = vec4(seed, i);
	float dot_product = dot(seed4, vec4(12.9898, 78.233, 45.164, 94.673));
	return fract(sin(dot_product) * 43758.5453);
}

// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/
void main()
{
	vec3 norm = normalize(normal);
	float nl = dot(norm, to_light);
	float cosTheta = clamp(nl, 0.0f, 1.0f);

	float visibility = 1.0f;
	float bias = clamp(0.005f * tan(acos(cosTheta)), 0.0f, 1.0f);

	for (int i = 0; i < 16; i++)
	{
		//int index = i;
		//int index = int(16.0f * random(gl_FragCoord.xyy, i)) % 16;
		int index = int(16.0f * random(floor(position.xyz * 1000.0f), i)) % 16;
		visibility -= 0.05f * (1.0f - texture(shadowmap, vec3(shadow.xy + poisson_disk[index] / 1200.0, (shadow.z - bias) / shadow.w)));
	}

	//float intensity = nl; // max(0.0, nl * visibility)
	//float intensity = max(0.0, nl * visibility);
	float intensity = (visibility * smoothstep(0.0, 0.1, nl) * 0.7 + 0.3);
	float coeff;
    if (intensity > 0.95)      coeff = 1.0; // can raise above 1.0
    else if (intensity > 0.75) coeff = 0.95;
    else if (intensity > 0.50) coeff = 0.75;
    else if (intensity > 0.25) coeff = 0.50;
    else                       coeff = 0.25;
	vec3 ambient = 0.7 * color;
	//fragColor = vec4(ambient + 0.5f * visibility * color * (smoothstep(0.0, 0.1, nl) * 0.6 + 0.4), 1.0);
	//fragColor = vec4(ambient + 0.5f * visibility * color * color2, 1.0);
	fragColor = vec4(ambient + 0.5 * coeff * color, 1.0);
	//fragColor = vec4(ambient + 0.5f * visibility * color * (smoothstep(0.0, 0.1, nl) * 0.6 + 0.4), 1.0);

	normalColor = (norm + vec3(1.0, 1.0, 1.0)) * 0.5;
}
