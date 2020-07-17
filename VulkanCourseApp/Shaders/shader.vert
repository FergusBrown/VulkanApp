#version 450		// Use GLSL 4.5

layout(location = 0) in vec3 pos;		// Locations represent locations in some bound data. ALSO note that binding = 0 is implied when not stated
layout(location = 1) in vec3 col;		// binding = 0 has no relation between inputs, outputs and uniforms
	
layout(binding = 0) uniform UboViewProjection {
	mat4 projection;
	mat4 view;
} uboViewProjection;

// BELOW uboModel NOT IN USE AND REPLACED BY PUSH MODEL. LEFT FOR REFERENCE.
layout(binding = 1) uniform UboModel {
	mat4 model;
} uboModel;

layout(push_constant) uniform PushModel {
	mat4 model;
} pushModel;

layout(location = 0) out vec3 fragCol;


void main() {
	gl_Position = uboViewProjection.projection * uboViewProjection.view * pushModel.model * vec4(pos, 1.0);  // Projected to model location, camera view then projection
	
	fragCol = col;
}