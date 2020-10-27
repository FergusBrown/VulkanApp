#version 450

layout(input_attachment_index = 0, binding = 0) uniform subpassInput inputColour;	// Colour output from lighting

layout(location = 0) out vec4 colour;

void main()
{
	colour = subpassLoad(inputColour).rgba;
}