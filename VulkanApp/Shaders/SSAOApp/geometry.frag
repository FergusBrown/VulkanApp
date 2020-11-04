#version 450

// INPUTS
// - UV
layout(location = 0) in vec2 UV;

// - worldSpace inputs
layout(location = 1) in vec3 fragPos_viewSpace;
layout(location = 2) in vec3 normal_viewSpace;
layout(location = 3) in vec3 tangent_viewSpace;

// OUTPUTS
layout(location = 0) out vec4 gNormal; 
layout(location = 1) out vec4 gAlbedo; 
layout(location = 2) out vec4 gSpecular; 


// - Descriptor set 1 (texture samplers)
layout(set = 1, binding = 0) uniform sampler2D albedoSampler;
layout(set = 1, binding = 1) uniform sampler2D normalSampler;
layout(set = 1, binding = 2) uniform sampler2D specularSampler;


void main () {
	// Albedo map
	gAlbedo = texture(albedoSampler, UV);

	// Specular map
	gSpecular = texture(specularSampler, UV).rgba;

	// Calculate TBN matrix
	vec3 T = normalize(tangent_viewSpace);
	vec3 N = normalize(normal_viewSpace);
	vec3 B = cross(N, T);

	mat3 TBN = mat3(T, B, N);

	// Normal map in viewspace - convert from range of [0,1] to [-1,1] when sampling
	vec3 normal_viewSpace = TBN * (texture(normalSampler, UV).rgb * 2 - 1.0);
	// Convert back to range of [0,1] as values will be clamped at 0 when stored in RGB texture
	normal_viewSpace = normalize(normal_viewSpace) * 0.5 + 0.5;
	gNormal = vec4(normal_viewSpace, 1.0);



	
}