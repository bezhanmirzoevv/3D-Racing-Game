#version 400 core

// Final output colour of the fragment
out vec4 vOutputColour;

// Uniform colour applied to the entire primitive
uniform vec4 vColour;

void main()
{
    // Output a constant colour (used for simple HUD elements or shapes)
    vOutputColour = vColour;
}