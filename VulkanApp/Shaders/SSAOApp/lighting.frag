#version 450

// INPUTS
// - UV
layout(location = 0) in vec2 UV;
layout(location = 1) in vec4 viewRay;

// INPUT ATTACHMENTS
layout(input_attachment_index = 0, binding = 1) uniform subpassInput inputDepth;	
layout(input_attachment_index = 1, binding = 2) uniform subpassInput inputNormal;		
layout(input_attachment_index = 2, binding = 3) uniform subpassInput inputAlbedo;	
layout(input_attachment_index = 3, binding = 4) uniform subpassInput inputSpecular;	
layout(input_attachment_index = 5, binding = 5) uniform subpassInput inputBlur;	

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
layout(set = 0, binding = 6) uniform lights 
{
	PointLight pointLights[POINT_LIGHT_COUNT];
	SpotLight flashLight;	
};

layout(location = 0) out vec4 outColour;

// Function prototypes
vec3 calcPointLight(PointLight light, vec3 fragPos, vec3 normal, vec4 albedoSpecColour);
vec3 calcSpotLight(SpotLight light, vec3 fragPos, vec3 normal, vec4 albedoSpecColour);
float calcAttenuation(vec4 intensityAndAttenuation, float distance);

float lineariseDepth(float depth);

// Clip plain near and far distance (view space)
const float zNear = 0.1;
const float zFar = 300.;

void main()
{
	// Get g-buffer data
	// Get position from depth buffer using viewray
	float depth = subpassLoad(inputDepth).x;
	float linearDepth = lineariseDepth(depth);

	vec3 fragPos_viewSpace = viewRay.xyz * linearDepth;

	vec3 fragNormal_viewSpace = normalize(subpassLoad(inputNormal).rgb * 2 - 1);

	vec4 albedoSpec = vec4(subpassLoad(inputAlbedo).rgb, subpassLoad(inputSpecular).r);

	// Get SSAO data
	float ambientOcclusion = subpassLoad(inputBlur).r;

	// Calculate Lighting
	vec3 colour = 0.3 * albedoSpec.rgb * ambientOcclusion; // default colour is ambient light * AO factor

	for (int i = 0; i < POINT_LIGHT_COUNT; ++i)
	{
		colour += calcPointLight(pointLights[i], 
								fragPos_viewSpace,
								fragNormal_viewSpace, 
								albedoSpec);
	}

	colour += calcSpotLight(flashLight,
							fragPos_viewSpace,
							fragNormal_viewSpace, 
							albedoSpec);

	outColour = vec4(colour, 1.0);
}

// Calculate a point light's contribution to fragment colour
vec3 calcPointLight(PointLight light, vec3 fragPos, vec3 normal, vec4 albedoSpecColour)
{
	vec3 lightPos = light.position.xyz;
	vec3 viewDir = normalize(-fragPos);

	// Distance to light
	float distance = length(lightPos - fragPos);

	// Direction of the light (view space)
	vec3 lightDir = normalize(lightPos - fragPos);

	// Lambert cosine - Light intensity directly proportional to n.l
	// Clamped between 1 and 0
	// Max is 1 since when incident light is parallel to normal theta = 0 so cosTheta is 1
	// Min is 0 since when theta >= 90 the light is either perpendicular to the surface or not incident
	float diffuseFactor = max(dot(normal, lightDir), 0.0);

	// Half way vector for Blinn-Phong model of specular component
	// This is the average between the lightDir and viewDir
	vec3 halfwayDir = normalize(lightDir + viewDir);

	// Use lambert to determine intensity of light directed towards the eye by the reflection
	float specFactor = pow(max(dot(normal, halfwayDir), 0.0),32);

	// Colour calculations based on lambert cosines
	vec3 colour =
		albedoSpecColour.rgb * diffuseFactor +
		vec3(albedoSpecColour.a) * specFactor;
	


	// Calculate attenuation factor
	float attenuation = calcAttenuation(light.intensityAndAttenuation, distance);

	// Factor in light intensity, colour and attenuation
	return colour * light.colour.rgb * attenuation;

}

vec3 calcSpotLight(SpotLight light, vec3 fragPos, vec3 normal, vec4 albedoSpecColour)
{
	vec3 lightPos = light.position.xyz;
	vec3 lightDir = light.direction.xyz;
	vec3 viewDir = normalize(-fragPos);

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
		vec3 halfwayDir = normalize(fragToLightDir + viewDir);
		float specFactor = pow(max(dot(normal, halfwayDir), 0.0), 32);
		vec3 colour =
		albedoSpecColour.rgb * diffuseFactor +
		vec3(albedoSpecColour.a) * specFactor;
	
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

float lineariseDepth(float depth)
{
	// Convert depth to NDC
	float z = depth * 2. - 1.;

	float linearDepth = (2. * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));

	return linearDepth;
}