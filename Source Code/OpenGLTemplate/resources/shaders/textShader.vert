#version 400 core

// Structure for matrices
uniform struct Matrices
{
	mat4 projMatrix;        // Projection matrix
	mat4 modelViewMatrix;   // Model-view matrix
} matrices;

// Layout of vertex attributes in VBO
layout (location = 0) in vec2 inPosition;
layout (location = 1) in vec2 inCoord;

out vec2 vTexCoord; // Texture coordinates passed to fragment shader

void main()
{
	// Transform the point into clip space.
	gl_Position = matrices.projMatrix * matrices.modelViewMatrix * vec4(inPosition, 0.0, 1.0);

	// Pass through the texture coordinates.
	vTexCoord = inCoord;
}