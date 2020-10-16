#version 450		// Use GLSL 4.5

// Vertex Input Bindings (bounf in pipeline creation)
layout(location = 0) in vec3 vertexPos;		// Locations represent locations in some bound data. ALSO note that binding = 0 is implied when not stated
layout(location = 1) in vec3 vertexNorm;	// binding = 0 has no relation between inputs, outputs and uniforms
layout(location = 2) in vec2 vertexUV;
	
// Vert shader outputs which will be interpolated for each fragment
layout(location = 0) out vec3 fragCol;
layout(location = 1) out vec2 fragUV;


// Uniform data
// - Descriptor set data
layout(set = 0, binding = 0) uniform ViewProjection {
	mat4 P;
	mat4 V;
} viewProjection;

// - Push constant data
layout(push_constant) uniform PushModel {
	mat4 M;
} push;


void main() {
	gl_Position = viewProjection.P * viewProjection.V * push.M * vec4(pos, 1.0);  // Projected to model location, camera view then projection
	
	faceNormal = col;


	// Vertex UV
	fragUV = vertexUV;
}