#version 450

// Note that out and in are separate so location = 0 is mapping to different "wires" in this case
layout(location = 0) in vec3 fragCol;
layout(location = 0) out vec4 outColour; // Final output colour (must also have location)

void main () {
	outColour = vec4(fragCol, 1.0); // 4th value (1.0) is the alpha value so 1.0 means opaque
}