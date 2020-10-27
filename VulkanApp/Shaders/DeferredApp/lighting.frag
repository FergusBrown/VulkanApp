#version 450

// INPUT ATTACHMENTS
layout(input_attachment_index = 0, binding = 0) uniform subpassInput inputPos;			// Position output from subpass 0
layout(input_attachment_index = 1, binding = 1) uniform subpassInput inputNormal;		// Normal output from subpass 0
layout(input_attachment_index = 2, binding = 2) uniform subpassInput inputAlbedo;	
layout(input_attachment_index = 3, binding = 3) uniform subpassInput inputSpecular;	
layout(input_attachment_index = 4, binding = 4) uniform subpassInput inputDepth;		// Depth output from subpass 0

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

// - Lights ubo
#define POINT_LIGHT_COUNT 3
layout(set = 0, binding = 5) uniform lights 
{
	PointLight pointLights[POINT_LIGHT_COUNT];
	SpotLight flashLight;	
};

layout(location = 0) out vec4 outColour;

// Function prototypes
vec3 calcPointLight(PointLight light, vec3 fragPos, vec3 normal, vec3 viewDir, vec4 albedoSpecColour);
vec3 calcSpotLight(SpotLight light, vec3 fragPos, vec3 normal, vec3 viewDir, vec4 albedoSpecColour);
float calcAttenuation(vec4 intensityAndAttenuation, float distance);

void main()
{
	// Get g-buffer data
	vec3 fragPos_worldSpace = subpassLoad(inputPos).rgb;

	vec3 fragNormal_worldSpace = subpassLoad(inputNormal).rgb;

	vec4 albedoSpec = vec4(subpassLoad(inputAlbedo).rgb, subpassLoad(inputSpecular).r);

	// Calculate Lighting

	// view direction towards camera
	vec3 viewPos_worldSpace = flashLight.position.xyz;
	vec3 fragToViewDir = normalize(viewPos_worldSpace - fragPos_worldSpace);

	vec3 colour = vec3(0.1, 0.1, 0.1); // default colour is ambient light

	for (int i = 0; i < POINT_LIGHT_COUNT; ++i)
	{
		colour += calcPointLight(pointLights[i], 
								fragPos_worldSpace,
								fragNormal_worldSpace, 
								fragToViewDir, 
								albedoSpec);
	}

	colour += calcSpotLight(flashLight,
							fragPos_worldSpace,
							fragNormal_worldSpace, 
							fragToViewDir, 
							albedoSpec);

	outColour = vec4(colour, 1.0);

}

// Calculate a point light's contribution to fragment colour
vec3 calcPointLight(PointLight light, vec3 fragPos, vec3 normal, vec3 viewDir, vec4 albedoSpecColour)
{
	vec3 lightPos = light.position.xyz;

	// Direction of the light (tangent space)
	vec3 lightDir = normalize(lightPos - fragPos);

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
		albedoSpecColour.rgb * diffuseFactor +
		vec3(albedoSpecColour.a) * pow(specFactor,32);
	
	// Distance to light (calculate in world space)
	float distance = length(lightPos - fragPos);

	// Calculate attenuation factor
	float attenuation = calcAttenuation(light.intensityAndAttenuation, distance);

	// Factor in light intensity, colour and attenuation
	return colour * light.colour.rgb * attenuation;

}

vec3 calcSpotLight(SpotLight light, vec3 fragPos, vec3 normal, vec3 viewDir, vec4 albedoSpecColour)
{
	vec3 lightPos = light.position.xyz;
	vec3 lightDir = light.direction.xyz;

	// Direction of the light (tangent space)
	vec3 fragToLightDir = normalize(lightPos - fragPos);

	// Get angle between spotDir (direction spot light is pointing) and lightDir (fragment to light )
	float cosTheta = dot(lightDir, normalize(-fragToLightDir));

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
		albedoSpecColour.rgb * diffuseFactor +
		vec3(albedoSpecColour.a) * pow(specFactor,32);
	
		// Distance to light (calculate in world space)
		float distance = length(lightPos - fragPos);

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