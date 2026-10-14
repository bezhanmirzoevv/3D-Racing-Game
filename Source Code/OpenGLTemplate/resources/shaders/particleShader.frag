#version 400 core

in vec4 vColour;   // Interpolated particle colour from the vertex shader
out vec4 fragColour; // Final fragment colour

void main()
{
    // Output the particle colour directly.
    fragColour = vColour;
}