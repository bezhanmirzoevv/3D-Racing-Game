#version 400

// Matrices provided by the client program.
uniform struct Matrices
{
	mat4 projMatrix;        // Projection matrix
	mat4 modelViewMatrix;   // Model-view matrix
	mat3 normalMatrix;      // Normal matrix for transforming normals
} matrices;

// Vertex attributes from the generated route mesh.
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inCoord;
layout (location = 2) in vec3 inNormal;

// Outputs passed to the fragment shader.
out vec2 vTexCoord;      // Texture coordinates
out vec3 vNormal;        // Transformed normal in eye space
out vec4 vEyePosition;   // Vertex position in eye space

void main()
{
	// Pass texture coordinate through to fragment shader.
	vTexCoord = inCoord;

	// Transform the normal into eye space.
	vNormal = normalize(matrices.normalMatrix * inNormal);

	// Transform the vertex position into eye space.
	vEyePosition = matrices.modelViewMatrix * vec4(inPosition, 1.0f);

	// Transform to clip space.
	gl_Position = matrices.projMatrix * vEyePosition;
}