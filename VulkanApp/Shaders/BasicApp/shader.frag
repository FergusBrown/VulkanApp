#version 450

// INPUTS
// - UV
layout(location = 0) in vec2 UV;

// - worldSpace inputs
layout(location = 1) in vec3 vertexPos_worldSpace;

// - tangentSpace inputs
layout(location = 2) in vec3 lightPos_tangentSpace;
layout(location = 3) in vec3 viewPos_tangentSpace;
layout(location = 4) in vec3 fragPos_tangentSpace;

// OUTPUTS
layout(location = 0) out vec4 outColour; // Final output colour (must also have location)

// UNIFORM DATA
// - Light Struct
struct Light
{
	vec3 colour;
	float intensity;
	vec3 position;
};

// - Descriptor set 0 (transforms + light positions)
layout(set = 0, binding = 0) uniform Uniforms 
{
	mat4 P;
	mat4 V;
	Light light;
} uniforms;

// - Descriptor set 1 ( texture samplers)
layout(set = 1, binding = 0) uniform sampler2D diffuseSampler;
layout(set = 1, binding = 1) uniform sampler2D normalSampler;



void main () {
	// Material Properties
	vec3 materialDiffuseColour = texture(diffuseSampler, UV).rgb;
	vec3 materialAmbientColour = vec3(0.1, 0.1, 0.1) * materialDiffuseColour;
	vec3 materialSpecularColour = vec3(0.3, 0.3, 0.3);

	// Local normal (tangent space)
	// Normal from normal map = 2*colour - 1 -> convert from range of [0,1] to [-1,1]
	vec3 normal = normalize(texture(normalSampler, UV).rgb * 2 - 1);

	// Direction of the light (tangent space)
	vec3 lightDir = normalize(lightPos_tangentSpace - fragPos_tangentSpace);

	
	// Lambert cosine - Light intensity directly proportional to n.l
	// Clamped between 1 and 0
	// Max is 1 since when incident light is parallel to normal theta = 0 so cosTheta is 1
	// Min is 0 since when theta >= 90 the light is either perpendicular to the surface or not incident
	float diffIntensity = max(dot(normal, lightDir), 0.0);

	// view direction towards camera
	vec3 viewDir = normalize(viewPos_tangentSpace - fragPos_tangentSpace);
	


	// Direction in which triangle reflects light
	// Note that -l is the direction of the incident light on the triangle
	vec3 reflectDir = reflect(-lightDir, normal);

	vec3 halfwayDir = normalize(lightDir + viewDir);

	// Use lambert to determine intensity of light directed towards the eye by the reflection
	float specIntensity = max(dot(viewDir, halfwayDir), 0.0);

	// Distance to light
	float distance = length(uniforms.light.position - vertexPos_worldSpace);

	// Light parameters
	vec3 lightColour = uniforms.light.colour;
	float lightIntensity = uniforms.light.intensity;

	vec3 colour =
		// Ambient : "faked" indirect light
		materialAmbientColour +
		// Diffuse colour :  Proportional to angle between normal and light
		// Intensity decays in intensity by inverse square law -> divided by d^2
		(materialDiffuseColour * lightColour * lightIntensity * diffIntensity) / (distance * distance) +
		// Specular colour : proportional to cos of angle between eye and reflection direction
		// Has similar light dependencies as diffue colour
		// Raise power to decrease strength of this effect
		(materialSpecularColour * lightColour * lightIntensity * pow(specIntensity,5)) / (distance * distance);

	// Set alpha channel to 1 default for all opaque
	outColour = vec4(colour, 1.0);
	//outColour = vec4(viewDir, 1.0);
}