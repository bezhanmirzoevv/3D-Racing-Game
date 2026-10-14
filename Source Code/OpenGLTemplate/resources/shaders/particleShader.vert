#version 400 core

layout(location = 0) in vec3 inPosition; // Particle position in world/object space
layout(location = 1) in vec4 inColour;   // Particle colour including alpha

uniform struct Matrices
{
    mat4 projMatrix;        // Projection matrix
    mat4 modelViewMatrix;   // Model-view matrix
} matrices;

out vec4 vColour; // Colour passed to fragment shader

void main()
{
    // Transform particle position into clip space.
    gl_Position = matrices.projMatrix * matrices.modelViewMatrix * vec4(inPosition, 1.0f);

    // Set rendered point size for GL_POINTS particle rendering.
    gl_PointSize = 10.0f;

    // Pass particle colour to the fragment shader.
    vColour = inColour;
}