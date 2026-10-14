#version 400

// Diffuse texture applied to the route surface.
uniform sampler2D sampler0;

// Enables or disables texture mapping.
uniform bool bUseTexture;

// Main scene light in eye coordinates.
uniform struct LightInfo
{
	vec4 position;   // eye coordinates
	vec3 La;         // Ambient light intensity
	vec3 Ld;         // Diffuse light intensity
	vec3 Ls;         // Specular light intensity
} light1;

// Material properties for the route surface.
uniform struct MaterialInfo
{
	vec3 Ma;         // Ambient reflectivity
	vec3 Md;         // Diffuse reflectivity
	vec3 Ms;         // Specular reflectivity
	float shininess; // Phong shininess exponent
} material1;

in vec2 vTexCoord;     // Interpolated texture coordinates
in vec3 vNormal;       // Interpolated surface normal in eye space
in vec4 vEyePosition;  // Fragment position in eye space

out vec4 vOutputColour; // Final fragment colour

// Standard Phong lighting model evaluated per fragment.
vec3 PhongModel(vec4 p, vec3 n)
{
	vec3 s = normalize(vec3(light1.position - p));
	vec3 v = normalize(-p.xyz);
	vec3 r = reflect(-s, n);

	vec3 ambient = light1.La * material1.Ma;

	float sDotN = max(dot(s, n), 0.0);
	vec3 diffuse = light1.Ld * material1.Md * sDotN;

	vec3 specular = vec3(0.0);
	if (sDotN > 0.0)
	{
		specular = light1.Ls * material1.Ms *
			pow(max(dot(r, v), 0.0), material1.shininess);
	}

	return ambient + diffuse + specular;
}

void main()
{
    // Normalise the interpolated normal before lighting.
    vec3 n = normalize(vNormal);

    // Use white if texturing is disabled, otherwise sample the route texture.
    vec3 baseColour = vec3(1.0);
    if (bUseTexture)
        baseColour = texture(sampler0, vTexCoord).rgb;

    // Compute standard Phong lighting contribution.
    vec3 lighting = PhongModel(vEyePosition, n);
    vec3 litColour = baseColour * lighting;

    // Boost the base texture slightly so the route colour appears more vivid.
    vec3 boostedBase = baseColour * 1.5;
    vec3 boostedLitColour = boostedBase * lighting;

    // Blend between the standard lit result and the boosted version.
    float glowBlend = 0.6;
    vec3 surfaceColour = mix(litColour, boostedLitColour, glowBlend);

    // Add a small emissive-style contribution to help the route bloom more strongly.
    vec3 glowColour = baseColour * 0.6;
    float emissiveStrength = 0.8;
    vec3 emissive = glowColour * emissiveStrength;

    vec3 finalColour = surfaceColour + emissive;

    vOutputColour = vec4(finalColour, 1.0);
}