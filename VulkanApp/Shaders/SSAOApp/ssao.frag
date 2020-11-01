#version 450

// INPUTS
// - UV
layout(location = 0) in vec2 UV;

// Attachments
//layout(input_attachment_index = 0, binding = 0) uniform subpassInput inputPosition;
//layout(input_attachment_index = 1, binding = 1) uniform subpassInput inputNoise;

// OUTPUTS
//layout(location = 0) out float occlusionOut;
layout(location = 0) out vec4 occlusionOut;

// - Descriptor set 1 (texture samplers)
layout(set = 0, binding = 0) uniform sampler2D positionSampler;
layout(set = 0, binding = 1) uniform sampler2D normalSampler;
layout(set = 0, binding = 2) uniform sampler2D noiseSampler;

layout(set = 0, binding = 3) uniform viewProjection 
{
	mat4 P;
	mat4 V;
};

#define SAMPLE_COUNT 64
layout(set = 0, binding = 4) uniform uboSSAO 
{
	vec4 ssaoKernel[SAMPLE_COUNT];		// Positions to sample
};

// PARAMETERS
// tile noise texture over screen, based on screen dimensions divided by noise size (noise should be a 4x4 texture)
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
const vec2 noiseScale = vec2(SCREEN_WIDTH/4.0, SCREEN_HEIGHT/4.0); 

// TODO : modify these to be passed in as uniforms
float radius = 0.5;	// Affects radius sampled for SSAO
float bias = 0.025;	// Bias introduced to reduce shadow acne

void main () {
	
	// Get sample values - recall that position and normal are in view space
	vec3 fragPos = texture(positionSampler, UV).xyz;
	vec3 normal = normalize(texture(normalSampler, UV).rgb * 2 - 1);
	vec3 randomRotationVector = texture(noiseSampler, UV * noiseScale).xyz;

	// Create TBN: Tangent -> View space
	vec3 N = normal;
	// Use Gramm-Schmidt to create orthogonal basis with normal and rotation vector
	vec3 T = randomRotationVector;
	T = normalize(T - N * dot(N,T));
	vec3 B = cross(T, N);
	mat3 TBN = mat3(T, B, N);

	// Iterate over sample and build occlusion factor
	float occlusionFactor = 0.0;
	for(int i = 0; i < SAMPLE_COUNT; ++i)
	{
		// Get sample position in View space
		vec3 samplePos = TBN * ssaoKernel[i].xyz;
		// Multiply by radius to increase/decrease sample radius of ssaoKernel
		// Add frag position
		samplePos = samplePos * radius + fragPos;

		// Transform to clip space
		vec4 offset = vec4(samplePos, 1.0);
		offset = P * offset;	// Transform to clip space
		offset.xyz /= offset.w;			// perspective divide
		offset.xyz = offset.xyz * 0.5 + 0.5;		// transform to range [0,1] to sample texture correctly

		// Get sample depth
		float sampleDepth = texture(positionSampler, offset.xy).z;

		// Range check
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
		// Accumulate occlusion
		occlusionFactor += (sampleDepth >= samplePos.z + bias ? 1.0 : 0) * rangeCheck;
	}

	// Take 1 - normalized occlusion factor to find contribution to ambient lighting
	occlusionOut = vec4(vec3(1. - (occlusionFactor / SAMPLE_COUNT)), 1.0);	
}