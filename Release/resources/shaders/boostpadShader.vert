#version 400

// Matrices supplied by the client program.
uniform struct Matrices
{
    mat4 projMatrix;
    mat4 modelViewMatrix;
    mat3 normalMatrix;
} matrices;

// Interleaved vertex attributes from the boost pad VBO.
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inColour;

// Data passed to the fragment shader.
out vec2 vTexCoord;
out vec3 vNormal;
out vec4 vEyePosition;
out vec3 vColour;

void main()
{
    // Transform the vertex into eye coordinates, then into clip coordinates.
    vec4 p = matrices.modelViewMatrix * vec4(inPosition, 1.0);
    gl_Position = matrices.projMatrix * p;

    // Pass interpolated attributes to the fragment shader.
    vTexCoord = inTexCoord;
    vNormal = normalize(matrices.normalMatrix * inNormal);
    vEyePosition = p;
    vColour = inColour;
}