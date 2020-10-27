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
layout(location = 2) out vec4 gAlbedoSpec; 


// - Descriptor set 1 ( texture samplers)
layout(set = 1, binding = 0) uniform sampler2D albedoSampler;
layout(set = 1, binding = 1) uniform sampler2D normalSampler;
layout(set = 1, binding = 2) uniform sampler2D specularSampler;


void main () {
	// Position map
	gPosition = fragPos_worldSpace;

	// Calculate TBN matrix
	vec3 T = normalize(tangent_worldSpace);
	vec3 N = normalize(normal_worldSpace);
	vec3 B = cross(T, N);

	mat3 TBN = mat3(T, B, N);

	// Normal map in worldspace
	gNormal = TBN * (texture(normalSampler, UV).rgb * 2 - 1);

	// Albedo map
	gAlbedoSpec.rgb = texture(albedoSampler, UV).rgb;

	// Specular map
	gAlbedoSpec.a = texture(specularSampler, UV).a);

}