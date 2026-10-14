#version 400 core

in vec3 vColour;			// Interpolated colour using colour calculated in the vertex shader
in vec2 vTexCoord;			// Interpolated texture coordinate using texture coordinate from the vertex shader

out vec4 vOutputColour;		// The output colour

uniform sampler2D sampler0;  // The texture sampler
uniform samplerCube CubeMapTex; // Cubemap sampler used for skybox rendering
uniform bool bUseTexture;    // A flag indicating if texture-mapping should be applied
uniform bool renderSkybox;   // Switch between normal object rendering and skybox rendering
uniform float skyboxBrightness; // Brightness multiplier for the skybox
in vec3 worldPosition;       // Direction used to sample the cubemap

void main()
{
	if (renderSkybox) {
		// Sample the cubemap using the vertex position as a direction.
		vec4 skyColour = texture(CubeMapTex, worldPosition);

		// Apply a simple brightness scale to the skybox.
		skyColour.rgb *= skyboxBrightness;
		vOutputColour = skyColour;

	} else {

		// Get the texel colour from the texture sampler
		vec4 vTexColour = texture(sampler0, vTexCoord);	

		if (bUseTexture)
			vOutputColour = vTexColour * vec4(vColour, 1.0f);	// Combine object colour and texture 
		else
			vOutputColour = vec4(vColour, 1.0f);	// Just use the colour instead
	}
}