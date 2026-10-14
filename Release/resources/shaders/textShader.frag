#version 400 core

in vec2 vTexCoord;     // Texture coordinates for sampling the glyph atlas
out vec4 vOutputColour; // Final fragment colour

uniform sampler2D sampler0; // Font texture
uniform vec4 vColour;       // Text colour

uniform bool bBlink; // Enables blinking text effect
uniform float t;     // Time value used for blinking

void main()
{
    // Sample the font texture.
    vec4 vTexColour = texture(sampler0, vTexCoord);

    // The glyph intensity is stored in the red channel.
    float alpha = vTexColour.r;

    // Optional blink logic: discard fragments for half of the cycle.
    if (bBlink) {
        if (mod(t, 1.0) < 0.5)
            discard;
    }

    // Multiply the sampled glyph value by the desired text colour.
    vOutputColour = vec4(vTexColour.r) * vColour; 
}