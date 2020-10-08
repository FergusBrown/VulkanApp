#version 450

// Note that out and in are separate so location = 0 is mapping to different "wires" in this case
layout(location = 0) in vec3 fragCol;
layout(location = 1) in vec2 fragTex;

layout(set = 1, binding = 0) uniform sampler2D textureSampler;

layout(location = 0) out vec4 outColour; // Final output colour (must also have location)

void main () {
	outColour = texture(textureSampler, fragTex);
}