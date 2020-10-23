#version 450		// Use GLSL 4.5

// OUTPUTS
// Vertex Input Bindings (bound in pipeline creation)
layout(location = 0) in vec3 vertexPos;		// Locations represent locations in some bound data. ALSO note that binding = 0 is implied when not stated
layout(location = 1) in vec3 normal;	// binding = 0 has no relation between inputs, outputs and uniforms
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 UV;		
	
// OUTPUTS
layout(location = 0) out vec2 vertexUV;

// - worldSpace outputs
layout(location = 1) out vec3 vertexPos_worldSpace;

// - tangentSpace outputs
#define POINT_LIGHT_COUNT 3
layout(location = 2) out vec3 viewPos_tangentSpace;
layout(location = 3) out vec3 vertexPos_tangentSpace;
layout(location = 4) out vec3 viewDir_tangentSpace;
layout(location = 5) out vec3 lightPos_tangentSpace[POINT_LIGHT_COUNT]; // Place last as this consumes several locations

// UNIFORM DATA

// - PointLight struct
struct PointLight
{
	vec4 colour;
	vec4 position;
	vec4 intensityAndAttenuation;
};

// - SpotLight struct
struct SpotLight
{
	vec4 colour;
	vec4 position;
	vec4 direction;
	vec4 intensityAndAttenuation;
	float innerCutOff;
	float outerCutOff;
};

// - Descriptor set data
// - Matrices
layout(set = 0, binding = 0) uniform viewProjection 
{
	mat4 P;
	mat4 V;
} matrices;

// - Lights
layout(set = 0, binding = 1) uniform Lights 
{
	PointLight pointLights[POINT_LIGHT_COUNT];
	SpotLight flashLight;	
} lights;

// Push constant data
// - Model matrix
layout(push_constant) uniform PushModel 
{
	mat4 M;
} push;

// Function prototypes
mat3 calculateInverseTBN(mat3 MV);

void main() {
	// Shortcuts
	mat4 MV = matrices.V * push.M;

	// Vertex UV
	vertexUV = UV;

	// Vertex position (world space)
	vertexPos_worldSpace = (push.M * vec4(vertexPos, 1.0)).xyz;
	
	// Get positions in view space
	vec3 vertexPos_viewSpace = (matrices.V * vec4(vertexPos_worldSpace, 1.0)).xyz;
	vec3 viewPos_viewSpace = vec3(0.0, 0.0, 0.0);

	// View direction in view space
	vec3 viewDir_viewSpace = vec3(0.0, 0.0, -1.0);

	// Get inverse TBN matrix
	mat3 invTBN = calculateInverseTBN(mat3(MV));
	
	// Convert relevant vectors to tangent space
	viewPos_tangentSpace = invTBN * viewPos_viewSpace;
	vertexPos_tangentSpace = invTBN * vertexPos_viewSpace;
	viewDir_tangentSpace = invTBN * viewDir_viewSpace;

	for (int i = 0; i < POINT_LIGHT_COUNT; ++i)
	{
		vec3 lightPos_viewSpace = (matrices.V * lights.pointLights[i].position).xyz;
		lightPos_tangentSpace[i] = invTBN * lightPos_viewSpace;
	}

	// Vertex position (clip space)
	gl_Position = matrices.P * vec4(vertexPos_viewSpace, 1.0); 
}

// Use this matrix to transform from view space to tangent space
mat3 calculateInverseTBN(mat3 MV)
{
	// Create normal matrix from MV matrix
	// Must take inverse transpose to correct any scaling
	// Consider perforiming this operation outside of shaders as inverse is costly
	mat3 normalMatrix = transpose(inverse(mat3(MV)));

	vec3 tangent_viewSpace = normalize(normalMatrix * tangent);
	vec3 bitangent_viewSpace = normalize(normalMatrix * bitangent);
	vec3 normal_viewSpace = normalize(normalMatrix * normal);

	// Create TBN matrix (transform tangent to camera space)
	// Get inverse of TBN matrix to transfrom camera to tangent space
	// (components are othogonal so can take transpose)
	mat3 invTBN = transpose(mat3(
		tangent_viewSpace,
		bitangent_viewSpace,
		normal_viewSpace));

	return invTBN;
}