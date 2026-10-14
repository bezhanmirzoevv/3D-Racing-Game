#version 400

// Full rendered scene texture.
uniform sampler2D sceneTex;

// Brightness threshold used to isolate bright fragments.
uniform float threshold;

in vec2 vTexCoord;
out vec4 fragColour;

void main()
{
    // Sample the scene colour.
    vec3 colour = texture(sceneTex, vTexCoord).rgb;

    // Compute perceived brightness using standard luminance weights.
    float brightness = dot(colour, vec3(0.2126, 0.7152, 0.0722));

    // Keep only bright fragments above the threshold.
    // Everything else is discarded (set to black).
    if (brightness > threshold)
        fragColour = vec4(colour, 1.0);
    else
        fragColour = vec4(0.0, 0.0, 0.0, 1.0);
}