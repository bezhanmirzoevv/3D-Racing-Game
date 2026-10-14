#version 400

// Optional texture used by the boost pad base panel.
uniform sampler2D sampler0;

// Controls whether the fragment colour comes from the texture or vertex colour.
uniform bool bUseTexture;

// Enables a simple emissive-style boost for the arrow geometry.
uniform bool bEmissiveArrows;

uniform struct LightInfo
{
    vec4 position;   // Light position in eye coordinates
    vec3 La;         // Ambient light intensity
    vec3 Ld;         // Diffuse light intensity
    vec3 Ls;         // Specular light intensity
} light1;

uniform struct MaterialInfo
{
    vec3 Ma;         // Ambient material
    vec3 Md;         // Diffuse material
    vec3 Ms;         // Specular material
    float shininess; // Phong shininess exponent
} material1;

in vec2 vTexCoord;
in vec3 vNormal;
in vec4 vEyePosition;
in vec3 vColour;

out vec4 vOutputColour;

// Standard Phong lighting model in eye coordinates.
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
    vec3 n = normalize(vNormal);
    vec3 lighting = PhongModel(vEyePosition, n);

    // Use either the sampled texture colour or the interpolated vertex colour.
    vec3 baseColour;
    if (bUseTexture)
        baseColour = texture(sampler0, vTexCoord).rgb;
    else
        baseColour = vColour;

    vec3 finalColour = baseColour * lighting;

    // Add a small emissive boost for the raised arrows so they stand out visually.
    if (!bUseTexture && bEmissiveArrows)
        finalColour += 0.35 * vColour;

    vOutputColour = vec4(finalColour, 1.0);
}