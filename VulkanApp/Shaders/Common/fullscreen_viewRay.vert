#version 450

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec4 viewRay;

layout(set = 0, binding = 0) uniform viewProjection 
{
	mat4 P;
	mat4 V;
};

// Use vertex indices to draw fullscreen triangle. 
// See the below link for how it works.
// https://www.saschawillems.de/blog/2016/08/13/vulkan-tutorial-on-rendering-a-fullscreen-quad-without-buffers/
void main()
{
	outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);

	vec4 vertex_clipSpace = vec4(outUV * 2.0f + -1.0f, 0.0f, 1.0f);

    gl_Position = vertex_clipSpace;

	mat4 invP = inverse(P);
	// Output view to vertex vector
    viewRay = invP * vertex_clipSpace; // This is equivalent to vertex_viewSpace - viewPos_viewSpace
}