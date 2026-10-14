#version 400

// Original rendered scene colour buffer.
uniform sampler2D sceneTex;

// Blurred bright-pass texture used for bloom.
uniform sampler2D bloomTex;

// Scalar controlling how strongly the bloom texture is added back into the scene.
uniform float bloomStrength;

in vec2 vTexCoord;
out vec4 fragColour;

void main()
{
    // Sample the original scene and the blurred bloom contribution.
	vec3 sceneColour = texture(sceneTex, vTexCoord).rgb;
	vec3 bloomColour = texture(bloomTex, vTexCoord).rgb;

    // Combine the two textures to produce the final bloom-enhanced image.
	vec3 finalColour = sceneColour + bloomStrength * bloomColour;
	fragColour = vec4(finalColour, 1.0);
}