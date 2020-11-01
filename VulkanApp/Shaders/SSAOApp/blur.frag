#version 450

// INPUTS
// - UV
layout(location = 0) in vec2 UV;

// OUTPUTS
layout(location = 0) out vec4 blurOut;

// - Descriptor set 1 (texture samplers)
layout(set = 0, binding = 0) uniform sampler2D ssaoSampler;

// Average a 4x4 rectangle around each pixel to create a blur 
// (box blur kernel)
void main () {
	vec2 texelSize = 1. / vec2(textureSize(ssaoSampler, 0)); // TextureSize retrieves texture dimensions

	float result = 0.0;
	for (int x = -2; x < 2; ++x)
	{
		for (int y = -2; y < 2; ++y)
		{
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(ssaoSampler, UV + offset).r;
		}
	}
	// Normalize and output result
	blurOut = vec4(vec3(result / (4. * 4.)), 1.0);
}