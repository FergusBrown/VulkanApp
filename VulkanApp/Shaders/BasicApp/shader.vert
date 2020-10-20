#version 450		// Use GLSL 4.5

// Vertex Input Bindings (bounf in pipeline creation)
layout(location = 0) in vec3 vertexPos;		// Locations represent locations in some bound data. ALSO note that binding = 0 is implied when not stated
layout(location = 1) in vec3 vertexNorm;	// binding = 0 has no relation between inputs, outputs and uniforms
layout(location = 2) in vec2 vertexTangent;
layout(location = 3) in vec2 vertexBitangent;
layout(location = 4) in vec2 vertexUV;		
	
// Vert shader outputs which will be interpolated for each fragment
layout(location = 0) out vec2 fragUV;

layout(location = 1) out vec3 vertexPos_worldSpace;
layout(location = 2) out vec3 eyeDirection_cameraSpace;
layout(location = 3) out vec3 lightDirection_cameraSpace;
layout(location = 4) out vec3 vertexNormal_cameraSpace;
layout(location = 5) out vec3 lightColour;
layout(location = 6) out float lightIntensity;
layout(location = 7) out vec3 lightPos_worldSpace;
//layout(location = 4) out vec3 lightDirection_tangentSpace;
//layout(location = 5) out vec3 eyeDirection_tangentSpace;



// Uniform data
// - Descriptor set data
layout(set = 0, binding = 0) uniform ViewProjection 
{
	mat4 P;
	mat4 V;
} viewProjection;

struct Light
{
	vec3 colour;
	float intensity;
	vec3 position;
};

layout(set = 0, binding = 1) uniform Lights
{
	Light lights[2];
} lights;

// - Push constant data
layout(push_constant) uniform PushModel {
	mat4 M;
} push;

void main() {
	// Vertex UV
	fragUV = vertexUV;

	// Vertex position (clip space)
	gl_Position = viewProjection.P * viewProjection.V * push.M * vec4(vertexPos, 1.0);  // Projected to model location, camera view then projection
	
	// Vertex position (world space)
	vertexPos_worldSpace = (push.M * vec4(vertexPos, 1.0)).xyz;

	// Vector from vertex position to the camera (camera space)
	vec3 vertexPos_cameraSpace = (viewProjection.V * push.M * vec4(vertexPos, 1.0)).xyz;
	vec3 eyePos_cameraSpace = vec3(0, 0, 0);
	eyeDirection_cameraSpace = eyePos_cameraSpace - vertexPos_cameraSpace;

	// Vector from vertex position to the light (camera space)
	vec3 lightPos_cameraSpace = (viewProjection.V * vec4(lights.lights[0].position, 1.0)).xyz;
	lightDirection_cameraSpace = lightPos_cameraSpace + eyeDirection_cameraSpace;
	 
	// Vertex normal in camera space

	// If model matrix is scaled must take its inverse transpose
	mat4 invTransposeM = transpose(inverse(push.M));
	vertexNormal_cameraSpace = (viewProjection.V * invTransposeM * vec4(vertexNorm, 1.0)).xyz;

	// Light parameters
	lightColour = lights.lights[0].colour;
	lightIntensity = lights.lights[0].intensity;
	lightPos_worldSpace = lights.lights[0].position;
	// Get tangent space components
//	vec3 vertexTangent_cameraSpace = (viewProjection.V * push.M * vec4(vertexTangent, 1.0)).xyz;
//	vec3 vertexBitangent_cameraSpace = (viewProjection.V * push.M * vec4(vertexBitangent, 1.0)).xyz;
//	vec3 vertexNormal_cameraSpace = (viewProjection.V * push.M * vec4(vertexNormal, 1.0)).xyz;
//	
//	// Get transpose so we can transform from model space to tangent space
//	mat3 TBN = transpose(mat3(
//		vertexTangent_cameraSpace,
//		vertexBitangent_cameraSpace,
//		vertexNormal_cameraSpace	
//	)); 
//
//	lightDirection_tangentSpace = TBN * lightDirection_cameraSpace;
//	eyeDirection_tangentSpace =  TBN * eyeDirection_cameraSpace;
//
}