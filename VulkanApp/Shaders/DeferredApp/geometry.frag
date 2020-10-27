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
layout(location = 2) out uvec4 gAlbedoSpec; 


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
	vec3 B = cross(T, N);

	mat3 TBN = mat3(T, B, N);

	// Normal map in worldspace
	vec3 normal_worldSpace = TBN * normalize(texture(normalSampler, UV).rgb * 2 - 1);
	gNormal = vec4(normal_worldSpace, 1.0);

	

	// Albedo map
	vec4 albedo = texture(albedoSampler, UV);

	// Specular map
	float specular = texture(specularSampler, UV).r;

	gAlbedoSpec.r = packHalf2x16(vec2(albedo.rg));
	gAlbedoSpec.g = packHalf2x16(vec2(albedo.ba));
	gAlbedoSpec.b = packHalf2x16(vec2(specular, 0.0));
	
}