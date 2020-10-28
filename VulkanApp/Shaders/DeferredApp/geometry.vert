#version 450		// Use GLSL 4.5

// Vertex Input Bindings 
layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec3 normal;	
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 UV;		
	
// OUTPUTS
layout(location = 0) out vec2 vertexUV;

// - worldSpace outputs
layout(location = 1) out vec3 vertexPos_worldSpace;
layout(location = 2) out vec3 normal_worldSpace;	// Use tangent and normal to construct TBN matrix in fragment shader
layout(location = 3) out vec3 tangent_worldSpace;

// UNIFORM DATA
// - Descriptor set data
// - Matrices
layout(set = 0, binding = 0) uniform viewProjection 
{
	mat4 P;
	mat4 V;
};

// Push constant data
// - Model matrix
layout(push_constant) uniform PushModel 
{
	mat4 M;
};

// Function prototypes
mat3 calculateTBN(mat3 M);

void main() {
	// Vertex UV
	vertexUV = UV;

	// Vertex position (world space)
	vertexPos_worldSpace = (M * vec4(vertexPos, 1.0)).xyz;

	// Create normal matrix from M matrix
	// Must take inverse transpose to correct any scaling
	// Consider performing this operation outside of shaders as inverse is costly
	mat3 normalMatrix = transpose(inverse(mat3(M)));

	// Calculate T and N for TBN matrix
	tangent_worldSpace = normalMatrix * normalize(tangent);
	normal_worldSpace = normalMatrix * normalize(normal);

	// Vertex position (clip space)
	gl_Position = P * V * vec4(vertexPos_worldSpace, 1.0); 
}