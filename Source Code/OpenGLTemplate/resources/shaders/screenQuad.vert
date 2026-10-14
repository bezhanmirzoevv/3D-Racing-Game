#version 400

// Full-screen quad vertex position in clip space.
layout (location = 0) in vec3 inPosition;

// Texture coordinates used for post-processing texture lookups.
layout (location = 1) in vec2 inTexCoord;

out vec2 vTexCoord;

void main()
{
	// Pass texture coordinates through to the fragment shader.
	vTexCoord = inTexCoord;

	// Full-screen quad is already provided in clip space.
	gl_Position = vec4(inPosition, 1.0);
}