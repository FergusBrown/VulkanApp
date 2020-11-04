#version 450

// INPUTS
// - UV
layout(location = 0) in vec2 UV;

// - worldSpace inputs
layout(location = 1) in vec3 fragPos_worldSpace;
layout(location = 2) in vec3 normal_worldSpace;
layout(location = 3) in vec3 tangent_worldSpace;

// OUTPUTS
layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal; 
layout(location = 2) out vec4 gAlbedo; 
layout(location = 3) out vec4 gSpecular; 


// - Descriptor set 1 (texture samplers)
layout(set = 1, binding = 0) uniform sampler2D albedoSampler;
layout(set = 1, binding = 1) uniform sampler2D normalSampler;
layout(set = 1, binding = 2) uniform sampler2D specularSampler;


void main () {
	// Position map
	gPosition = vec4(fragPos_worldSpace, 1.0);

	// Calculate TBN matrix
	vec3 T = normalize(tangent_worldSpace);
	vec3 N = normalize(normal_worldSpace);
	vec3 B = cross(N, T);

	mat3 TBN = mat3(T, B, N);

	// Normal map in worldspace - convert from range of [0,1] to [-1,1] when sampling
	vec3 normal_worldSpace = TBN * (texture(normalSampler, UV).rgb * 2 - 1.0);
	// Convert back to range of [0,1] as values will be clamped at 0 when stored in RGB texture
	normal_worldSpace = normalize(normal_worldSpace) * 0.5 + 0.5;
	gNormal = vec4(normal_worldSpace, 1.0);

	

	// Albedo map
	gAlbedo = texture(albedoSampler, UV);

	// Specular map
	gSpecular = texture(specularSampler, UV).rgba;
}