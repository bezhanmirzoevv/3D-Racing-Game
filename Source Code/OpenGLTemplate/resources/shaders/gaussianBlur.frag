#version 400

// Input texture (usually the bright-pass texture for bloom).
uniform sampler2D image;

// Determines blur direction:
// true  = horizontal blur
// false = vertical blur
uniform bool horizontal;

in vec2 vTexCoord;
out vec4 fragColour;

void main()
{
    // Gaussian weights (precomputed for performance).
    float weight[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

    // Texture coordinate offset based on texture resolution.
    vec2 texOffset = 1.0 / textureSize(image, 0);

    // Start with centre sample.
    vec3 result = texture(image, vTexCoord).rgb * weight[0];

    // Apply blur in one direction (horizontal or vertical).
    for (int i = 1; i < 5; ++i)
    {
        if (horizontal)
        {
            // Sample left and right neighbours.
            result += texture(image, vTexCoord + vec2(texOffset.x * i, 0.0)).rgb * weight[i];
            result += texture(image, vTexCoord - vec2(texOffset.x * i, 0.0)).rgb * weight[i];
        }
        else
        {
            // Sample up and down neighbours.
            result += texture(image, vTexCoord + vec2(0.0, texOffset.y * i)).rgb * weight[i];
            result += texture(image, vTexCoord - vec2(0.0, texOffset.y * i)).rgb * weight[i];
        }
    }

    // Output blurred colour.
    fragColour = vec4(result, 1.0);
}