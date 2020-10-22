#version 450		// Use GLSL 4.5

// OUTPUTS
// Vertex Input Bindings (bound in pipeline creation)
layout(location = 0) in vec3 vertexPos;		// Locations represent locations in some bound data. ALSO note that binding = 0 is implied when not stated
layout(location = 1) in vec3 vertexNormal;	// binding = 0 has no relation between inputs, outputs and uniforms
layout(location = 2) in vec3 vertexTangent;
layout(location = 3) in vec3 vertexBitangent;
layout(location = 4) in vec2 vertexUV;		
	
// OUTPUTS
// Vert shader outputs which will be interpolated for each fragment
layout(location = 0) out vec2 fragUV;

// - worldSpace outputs
layout(location = 1) out vec3 vertexPos_worldSpace;

// - cameraSpace outputs
layout(location = 2) out vec3 eyeDirection_cameraSpace;
layout(location = 3) out vec3 lightDirection_cameraSpace;

// - tangentSpace outputs
layout(location = 4) out vec3 lightDirection_tangentSpace;
layout(location = 5) out vec3 eyeDirection_tangentSpace;

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
	// Vertex UV
	fragUV = vertexUV;

	// Matrix shortcuts
	mat4 MV = uniforms.V * push.M;
	mat4 MVP = uniforms.P * MV;

	// Vertex position (clip space)
	gl_Position = MVP * vec4(vertexPos, 1.0);  // Projected to model location, camera view then projection
	
	// Vertex position (world space)
	vertexPos_worldSpace = (push.M * vec4(vertexPos, 1.0)).xyz;

	// Vector from vertex position to the camera (camera space)
	vec3 vertexPos_cameraSpace = (MV * vec4(vertexPos, 1.0)).xyz;
	vec3 eyePos_cameraSpace = vec3(0, 0, 0);
	eyeDirection_cameraSpace = eyePos_cameraSpace - vertexPos_cameraSpace;

	// Vector from vertex position to the light (camera space)
	vec3 lightPos_worldSpace = uniforms.light.position;
	vec3 lightPos_cameraSpace = (uniforms.V * vec4(lightPos_worldSpace, 1.0)).xyz;
	lightDirection_cameraSpace = lightPos_cameraSpace + eyeDirection_cameraSpace;
	//lightDirection_cameraSpace = lightPos_cameraSpace - vertexPos_cameraSpace;
	 

	// Create model to camera space transform matrix
	//mat4 invTM = inverse(transpose(push.M));
	//mat3 MV3x3 = mat3(uniforms.V * invTM);
	mat3 MV3x3 = mat3(MV);

	vec3 vertexTangent_cameraSpace = MV3x3 * vertexTangent;
	vec3 vertexBitangent_cameraSpace = MV3x3 * vertexBitangent;
	vec3 vertexNormal_cameraSpace = MV3x3 * vertexNormal;

	mat3 TBN = mat3(
		vertexTangent_cameraSpace,
		vertexBitangent_cameraSpace,
		vertexNormal_cameraSpace);

	// Get inverse to create camera to model space transform matrixCompMult
	// Note this matrix is orthogonal so inverse = transpose 
	// Use transpose since it is a cheaper operation
	mat3 invTBN = transpose(TBN);
	
	lightDirection_tangentSpace = invTBN * lightDirection_cameraSpace;
	eyeDirection_tangentSpace = invTBN * eyeDirection_cameraSpace;
}