#version 450

// INPUTS
// - UV
layout(location = 0) in vec2 UV;

// - worldSpace inputs
layout(location = 1) in vec3 fragPos_worldSpace;

// - tangentSpace inputs
#define POINT_LIGHT_COUNT 3
layout(location = 2) in vec3 viewPos_tangentSpace;
layout(location = 3) in vec3 fragPos_tangentSpace;
layout(location = 4) in vec3 viewDir_tangentSpace;
layout(location = 5) in vec3 lightPos_tangentSpace[POINT_LIGHT_COUNT];

// OUTPUTS
layout(location = 0) out vec4 outColour; // Final output colour (must also have location)

// UNIFORM DATA
// - PointLight struct
struct PointLight
{
	vec4 colour;
	vec4 position;

	float intensity;
	float kq;
	float kl;
	float kc;
};

// - SpotLight struct
struct SpotLight
{
	vec4 colour;
	vec4 position;
	vec4 direction;
	float intensity;
	float cutOff;
};

// - Descriptor set data

// - Lights
layout(set = 0, binding = 1) uniform Lights 
{
	PointLight pointLights[POINT_LIGHT_COUNT];
	//SpotLight flashLight;	
} lights;

// - Descriptor set 1 ( texture samplers)
layout(set = 1, binding = 0) uniform sampler2D diffuseSampler;
layout(set = 1, binding = 1) uniform sampler2D normalSampler;

// Function prototypes
vec3 calcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 fragPos_worldSpace, vec3 fragPos_tangentSpace, vec3 lightPos_tangentSpace, vec3 diffuseColour, vec3 specularColour);
float calcAttenuation(PointLight light, float distance);

void main () {
	// Material Properties
	vec3 diffuseColour = texture(diffuseSampler, UV).rgb;
	vec3 ambientColour = vec3(0.1, 0.1, 0.1) * diffuseColour;
	vec3 specularColour = vec3(0.3, 0.3, 0.3);

	// Local normal (tangent space)
	// Normal from normal map = 2*colour - 1 -> convert from range of [0,1] to [-1,1]
	vec3 normal = normalize(texture(normalSampler, UV).rgb * 2 - 1);

	// view direction towards camera
	vec3 fragToViewDir = normalize(viewPos_tangentSpace - fragPos_tangentSpace);

	vec3 colour = ambientColour;

	for (int i = 0; i < POINT_LIGHT_COUNT; ++i)
	{
		colour += calcPointLight(lights.pointLights[i], 
								normal, 
								fragToViewDir, 
								fragPos_worldSpace,
								fragPos_tangentSpace,
								lightPos_tangentSpace[i], 
								diffuseColour, 
								specularColour);
	}

	// Set alpha channel to 1 default for all opaque
	outColour = vec4(colour, 1.0);
}

// Calculate a point light's contribution to fragment colour
vec3 calcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 vertexPos_worldSpace, vec3 fragPos_tangentSpace, vec3 lightPos_tangentSpace, vec3 diffuseColour, vec3 specularColour)
{
	// Direction of the light (tangent space)
	vec3 lightDir = normalize(lightPos_tangentSpace - fragPos_tangentSpace);

	// Lambert cosine - Light intensity directly proportional to n.l
	// Clamped between 1 and 0
	// Max is 1 since when incident light is parallel to normal theta = 0 so cosTheta is 1
	// Min is 0 since when theta >= 90 the light is either perpendicular to the surface or not incident
	float diffuseFactor = max(dot(normal, lightDir), 0.0);

	// Direction in which triangle reflects light
	// Note that -l is the direction of the incident light on the triangle
	vec3 reflectDir = reflect(-lightDir, normal);

	// Half way vector for Blinn-Phong model of specular component
	// This is the average between the lightDir and viewDir
	vec3 halfwayDir = normalize(lightDir + viewDir);

	// Use lambert to determine intensity of light directed towards the eye by the reflection
	float specFactor = max(dot(viewDir, halfwayDir), 0.0);

	

	// Colour calculations based on lambert cosines
	vec3 colour =
		diffuseColour * diffuseFactor +
		specularColour * pow(specFactor,5);
	
	// Distance to light (calculate in world space)
	float distance = length(light.position.xyz - fragPos_worldSpace);

	// Calculate attenuation factor
	float attenuation = calcAttenuation(light, distance);

	// Factor in light intensity, colour and attenuation
	return colour * light.colour.rgb * light.intensity * attenuation;

}

float calcAttenuation(PointLight light, float distance)
{
	float denominator = light.kc + light.kl * distance + light.kq * distance * distance;

	return 1 / denominator;
}
