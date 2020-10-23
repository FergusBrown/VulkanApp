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

// - Lights
layout(set = 0, binding = 1) uniform Lights 
{
	PointLight pointLights[POINT_LIGHT_COUNT];
	SpotLight flashLight;	
} lights;

// - Descriptor set 1 ( texture samplers)
layout(set = 1, binding = 0) uniform sampler2D diffuseSampler;
layout(set = 1, binding = 1) uniform sampler2D normalSampler;

// Function prototypes
vec3 calcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 fragPos_tangentSpace, vec3 lightPos_tangentSpace, vec3 diffuseColour, vec3 specularColour);
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 viewDir, vec3 lightDir_tangentSpace, vec3 fragPos_tangentSpace, vec3 lightPos_tangentSpace, vec3 diffuseColour, vec3 specularColour);
float calcAttenuation(vec4 intensityAndAttenuation, float distance);

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
								fragPos_tangentSpace,
								lightPos_tangentSpace[i], 
								diffuseColour, 
								specularColour);
	}

	colour += calcSpotLight(lights.flashLight,
							normal,
							fragToViewDir,
							viewDir_tangentSpace,
							fragPos_tangentSpace,
							viewPos_tangentSpace,
							diffuseColour,
							specularColour);

	// Set alpha channel to 1 default for all opaque
	outColour = vec4(colour, 1.0);
}

// Calculate a point light's contribution to fragment colour
vec3 calcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 fragPos_tangentSpace, vec3 lightPos_tangentSpace, vec3 diffuseColour, vec3 specularColour)
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
	float attenuation = calcAttenuation(light.intensityAndAttenuation, distance);

	// Factor in light intensity, colour and attenuation
	return colour * light.colour.rgb * attenuation;

}

vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 viewDir, vec3 lightDir_tangentSpace, vec3 fragPos_tangentSpace, vec3 lightPos_tangentSpace, vec3 diffuseColour, vec3 specularColour)
{
	// Direction of the light (tangent space)
	vec3 fragToLightDir = normalize(lightPos_tangentSpace - fragPos_tangentSpace);

	// Get angle between spotDir (direction spot light is pointing) and lightDir (fragment to light )
	float cosTheta = dot(lightDir_tangentSpace, normalize(-fragToLightDir));

	// if cosTheta greater than cosPhi (light cutoff) then the fragment is lit by the spot light
	if (cosTheta > light.outerCutOff)
	{
		// Calculate edge intesity, this constant is smaller the further from the centre the fragment is
		float cosEpsilon = light.innerCutOff - light.outerCutOff;
		float edgeIntensity = clamp((cosTheta - light.outerCutOff)/ cosEpsilon, 0.0, 1.0);

		// Standard light calculations
		float diffuseFactor = max(dot(normal, fragToLightDir), 0.0);
		vec3 reflectDir = reflect(-fragToLightDir, normal);
		vec3 halfwayDir = normalize(fragToLightDir + viewDir);
		float specFactor = max(dot(viewDir, halfwayDir), 0.0);
		vec3 colour =
		diffuseColour * diffuseFactor +
		specularColour * pow(specFactor,5);
	
		// Distance to light (calculate in world space)
		float distance = length(light.position.xyz - fragPos_worldSpace);

		// Calculate attenuation factor
		float attenuation = calcAttenuation(light.intensityAndAttenuation, distance);

		// Factor in light intensity, colour and attenuation
		return colour * light.colour.rgb * attenuation * edgeIntensity;
	}
	else
	{
		return vec3(0.0, 0.0, 0.0);
	}

}

float calcAttenuation(vec4 intensityAndAttenuation, float distance)
{
	float denominator = intensityAndAttenuation.w + intensityAndAttenuation.z * distance + intensityAndAttenuation.y * distance * distance;

	return intensityAndAttenuation.x / denominator;
}
