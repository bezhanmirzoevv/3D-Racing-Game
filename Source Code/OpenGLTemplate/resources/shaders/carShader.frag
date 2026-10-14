#version 400 core

// Interpolated texture coordinates from vertex shader.
in vec2 vTexCoord;

// Interpolated lighting colour from vertex shader.
in vec3 vColour;

// Final output colour.
out vec4 vOutputColour;

// Diffuse texture for the car.
uniform sampler2D sampler0;

// Controls whether to use the texture or just lighting colour.
uniform bool bUseTexture;

void main()
{
    // Sample the texture colour.
    vec4 texColour = texture(sampler0, vTexCoord);

    // If texturing is enabled, combine texture with lighting.
    if (bUseTexture)
        vOutputColour = texColour * vec4(vColour, 1.0);
    else
        // Otherwise, just output the lighting colour.
        vOutputColour = vec4(vColour, 1.0);
}