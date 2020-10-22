#version 450		// Use GLSL 4.5

// OUTPUTS
// Vertex Input Bindings (bound in pipeline creation)
layout(location = 0) in vec3 vertexPos;		// Locations represent locations in some bound data. ALSO note that binding = 0 is implied when not stated
layout(location = 1) in vec3 normal;	// binding = 0 has no relation between inputs, outputs and uniforms
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 UV;		
	
// OUTPUTS
layout(location = 0) out vec2 fragUV;

// - worldSpace outputs
layout(location = 1) out vec3 vertexPos_worldSpace;

// - tangentSpace outputs
layout(location = 2) out vec3 lightPos_tangentSpace;
layout(location = 3) out vec3 viewPos_tangentSpace;
layout(location = 4) out vec3 fragPos_tangentSpace;

// UNIFORM DATA
// - Light Struct
struct Light
{
	vec3 colour;
	float intensity;
	vec3 position;
};

// - Descriptor set data
layout(set = 0, binding = 0) uniform Uniforms 
{
	mat4 P;
	mat4 V;
	Light light;
} uniforms;

// - Push constant data
layout(push_constant) uniform PushModel {
	mat4 M;
} push;

void main() {
	// Shortcuts
	mat4 MV = uniforms.V * push.M;

	// Vertex UV
	fragUV = UV;

	// Vertex position (world space)
	vertexPos_worldSpace = (push.M * vec4(vertexPos, 1.0)).xyz;
	
	// Get positions in camera space
	vec3 vertexPos_cameraSpace = (uniforms.V * vec4(vertexPos_worldSpace, 1.0)).xyz;
	vec3 viewPos_cameraSpace = vec3(0, 0, 0);
	vec3 lightPos_cameraSpace = (uniforms.V * vec4(uniforms.light.position, 1.0)).xyz;
	 
	// Create normal matrix from MV matrix
	// Must take inverse transpose to correct any scaling
	// Consider perforiming this operation outside of shaders as inverse is costly
	mat3 normalMatrix = transpose(inverse(mat3(MV)));

	vec3 tangent_cameraSpace = normalize(normalMatrix * tangent);
	vec3 bitangent_cameraSpace = normalize(normalMatrix * bitangent);
	vec3 normal_cameraSpace = normalize(normalMatrix * normal);

	// Create TBN matrix (transform tangent to camera space)
	// Get inverse of TBN matrix to transfrom camera to tangent space
	// (components are othogonal so can take transpose)
	mat3 invTBN = transpose(mat3(
		tangent_cameraSpace,
		bitangent_cameraSpace,
		normal_cameraSpace));
	
	lightPos_tangentSpace = invTBN * lightPos_cameraSpace;
	viewPos_tangentSpace = invTBN * viewPos_cameraSpace;
	fragPos_tangentSpace = invTBN * vertexPos_cameraSpace;

	// Vertex position (clip space)
	gl_Position = uniforms.P * vec4(vertexPos_cameraSpace, 1.0); 

	
}