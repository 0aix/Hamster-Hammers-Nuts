#version 330 core

in vec2 uv;

out vec4 fragColor;

uniform sampler2D diffusemap;
uniform sampler2D normalmap;
uniform sampler2D depthmap;

float dx = 1.0 / 800.0;
float dy = 1.0 / 600.0;
float dxN = 0.5 / 800.0;
float dyN = 0.5 / 600.0;
float A = 1.0;
float B = 2.0;
float C = 1.0;
float D = 2.0;

vec3 textureNormal(sampler2D sampler, vec2 uv)
{
	return 2.0 * texture(sampler, uv).xyz - vec3(1.0, 1.0, 1.0);
}

// http://stylized.realtimerendering.com/
void main()
{
	float centerDepth = texture(depthmap, uv).r;
	vec4 diagDepths = vec4(texture(depthmap, uv + vec2(-dx, dy)).r,
						   texture(depthmap, uv + vec2(dx, dy)).r,
						   texture(depthmap, uv + vec2(-dx, -dy)).r,
						   texture(depthmap, uv + vec2(dx, -dy)).r);
	vec4 axisDepths = vec4(texture(depthmap, uv + vec2(0.0, dy)).r,
						   texture(depthmap, uv + vec2(-dx, 0.0)).r,
						   texture(depthmap, uv + vec2(dx, 0.0)).r,
						   texture(depthmap, uv + vec2(0.0, -dy)).r);
	if (diagDepths.x <= centerDepth) diagDepths.x = centerDepth;
	if (diagDepths.y <= centerDepth) diagDepths.y = centerDepth;
	if (diagDepths.z <= centerDepth) diagDepths.z = centerDepth;
	if (diagDepths.w <= centerDepth) diagDepths.w = centerDepth;
	if (axisDepths.x <= centerDepth) axisDepths.x = centerDepth;
	if (axisDepths.y <= centerDepth) axisDepths.y = centerDepth;
	if (axisDepths.z <= centerDepth) axisDepths.z = centerDepth;
	if (axisDepths.w <= centerDepth) axisDepths.w = centerDepth;
	diagDepths -= centerDepth;
	axisDepths -= centerDepth;
	vec4 sobelH = diagDepths * vec4(C, -C, C, -C) +
				  axisDepths * vec4(0.0, D, -D, 0.0);
	vec4 sobelV = diagDepths * vec4(A, A, -A, -A) +
				  axisDepths * vec4(B, 0.0, 0.0, -B);
	float sobelX = dot(sobelH, vec4(1.0, 1.0, 1.0, 1.0));
	float sobelY = dot(sobelV, vec4(1.0, 1.0, 1.0, 1.0));
	float sobel = sqrt(sobelX * sobelX + sobelY * sobelY) * 10.0 / centerDepth;
	float coeff = clamp(1.0 - pow(clamp(sobel, 0.0, 1.0), 1.0), 0.0, 1.0);

	// Normal
	vec3 center = textureNormal(normalmap, uv);
	vec3 samples[8];

	// Diags
	samples[0] = textureNormal(normalmap, uv + vec2(-dxN, dyN));
	samples[1] = textureNormal(normalmap, uv + vec2(dxN, dyN));
	samples[2] = textureNormal(normalmap, uv + vec2(-dxN, -dyN));
	samples[3] = textureNormal(normalmap, uv + vec2(dxN, -dyN));
	// Axes
	samples[4] = textureNormal(normalmap, uv + vec2(0.0, dyN));
	samples[5] = textureNormal(normalmap, uv + vec2(-dxN, 0.0));
	samples[6] = textureNormal(normalmap, uv + vec2(dxN, 0.0));
	samples[7] = textureNormal(normalmap, uv + vec2(0.0, -dyN));

	vec3 diffs[8];
	for (int i = 0; i < 8; i++)
		diffs[i] = abs(samples[i] - center);
	float n = 0.0;
	for (int i = 0; i < 8; i++)
		n = max(diffs[i].x, max(diffs[i].y, diffs[i].z));
	float coeffN = clamp(1.0 - pow(clamp(n * 0.8, 0.0, 1.0), 2.0), 0.0, 1.0);

	vec4 color = texture(diffusemap, uv);
	float intensity = min(coeff, coeffN);
	fragColor = vec4(color.xyz * intensity, color.w);
}
