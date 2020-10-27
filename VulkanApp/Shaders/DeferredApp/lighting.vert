#version 450

// Array for triangle that fills screen
vec2 positions[3] = vec2[](
	vec2(3.0, -1.0),
	vec2(-1.0, -1.0),
	vec2(-1.0, 3.0)
);

// TODO : pass UV coord via indexes here
void main()
{
	gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}