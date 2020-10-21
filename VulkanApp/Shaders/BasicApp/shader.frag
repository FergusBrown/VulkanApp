#version 450

// Note that out and in are separate so location = 0 is mapping to different "wires" in this case
layout(location = 0) in vec2 fragUV;

layout(location = 1) in vec3 vertexPos_worldSpace;
layout(location = 2) in vec3 eyeDirection_cameraSpace;
layout(location = 3) in vec3 lightDirection_cameraSpace;
layout(location = 4) in vec3 vertexNormal_cameraSpace;
layout(location = 5) in vec3 lightColour;
layout(location = 6) in float lightIntensity;
layout(location = 7) in vec3 lightPos_worldSpace;
//layout(location = 4) in vec3 lightDirection_tangentSpace;
//layout(location = 5) in vec3 eyeDirection_tangentSpace;

layout(set = 1, binding = 0) uniform sampler2D textureSampler;

layout(location = 0) out vec4 outColour; // Final output colour (must also have location)

void main () {

	

	// Material Properties
	vec3 materialDiffuseColour = texture(textureSampler, fragUV).rgb;
	vec3 materialAmbientColour = vec3(0.1, 0.1, 0.1) * materialDiffuseColour;
	vec3 materialSpecularColour = vec3(0.3, 0.3, 0.3);

	// Distance to light
	float distance = length(lightPos_worldSpace - vertexPos_worldSpace);

	// Normal of the computed fragment, in camera space
	vec3 n = normalize(vertexNormal_cameraSpace);

	// Direction of the light (from the fragment to the light)
	vec3 l = normalize(lightDirection_cameraSpace);

	// Lambert cosine - Light intensity directly proportional to n.l
	// Clamped between 1 and 0
	// Max is 1 since when incident light is parallel to normal theta = 0 so cosTheta is 1
	// Min is 0 since when theta >= 90 the light is either perpendicular to the surface or not incident
	float cosTheta = clamp(dot(n, l), 1, 1);

	// Eye vector towards camera
	vec3 E = normalize(eyeDirection_cameraSpace);

	// Direction in which triangle reflects light
	// Note that -l is the direction of the incident light on the triangle
	vec3 R = reflect(-l, n);

	// Use lambert to determine intensity of light directed towards the eye by the reflection
	float cosAlpha = clamp(dot(E, R), 0, 1);

	vec3 colour =
		// Ambient : "faked" indirect light
		materialAmbientColour +
		// Diffuse colour :  Proportional to angle between normal and light
		// Intensity decays in intensity by inverse square law -> divided by d^2
		materialDiffuseColour * lightColour * lightIntensity * cosTheta / (distance * distance) +
		// Specular colour : proportional to cos of angle between eye and reflection direction
		// Has similar light dependencies as diffue colour
		// Raise power to decrease strength of this effect
		materialSpecularColour * lightColour * lightIntensity * pow(cosAlpha,5) / (distance * distance);

	// Set alpha channel to 1 default for all opaque
	outColour = vec4(colour, 1.0);
	//outColour = vec4(lightDirection_cameraSpace, 1.0);
}