#version 400 core

// Transformation matrices provided by the application.
uniform struct Matrices
{
    mat4 projMatrix;        // Projection matrix
    mat4 modelViewMatrix;   // Model-view matrix
    mat3 normalMatrix;      // Normal matrix for correct normal transformation
} matrices;

// Standard point light (scene light).
struct LightInfo
{
    vec4 position;  // Light position in eye space
    vec3 La;        // Ambient intensity
    vec3 Ld;        // Diffuse intensity
    vec3 Ls;        // Specular intensity
};

// Spotlight used for headlights.
struct SpotlightInfo
{
    vec4 position;      // Light position in eye space
    vec3 direction;     // Spotlight direction
    vec3 La;            // Ambient intensity
    vec3 Ld;            // Diffuse intensity
    vec3 Ls;            // Specular intensity
    float cutoff;       // Cutoff angle in degrees
    float exponent;     // Spotlight falloff exponent
};

// Material properties of the car.
struct MaterialInfo
{
    vec3 Ma;        // Ambient reflectivity
    vec3 Md;        // Diffuse reflectivity
    vec3 Ms;        // Specular reflectivity
    float shininess;// Shininess exponent
};

uniform LightInfo light1;
uniform SpotlightInfo headlightLeft;
uniform SpotlightInfo headlightRight;
uniform MaterialInfo material1;

// Vertex attributes from the mesh.
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inCoord;
layout (location = 2) in vec3 inNormal;

// Outputs passed to the fragment shader.
out vec2 vTexCoord;
out vec3 vColour;

// Standard Phong lighting model for the main scene light.
vec3 PhongModel(vec4 eyePosition, vec3 eyeNorm)
{
    vec3 s = normalize(vec3(light1.position - eyePosition));
    vec3 v = normalize(-eyePosition.xyz);
    vec3 r = reflect(-s, eyeNorm);

    vec3 ambient = light1.La * material1.Ma;

    float sDotN = max(dot(s, eyeNorm), 0.0);
    vec3 diffuse = light1.Ld * material1.Md * sDotN;

    vec3 specular = vec3(0.0);
    if (sDotN > 0.0)
    {
        float eps = 0.000001; // small value to avoid instability
        specular = light1.Ls * material1.Ms *
            pow(max(dot(r, v), 0.0), material1.shininess + eps);
    }

    return ambient + diffuse + specular;
}

// Spotlight lighting model used for headlights.
vec3 SpotlightModel(SpotlightInfo light, vec4 p, vec3 n)
{
    vec3 s = normalize(vec3(light.position - p));

    // Ambient contribution
    vec3 ambient = light.La * material1.Ma;

    // Compute spotlight angle and compare with cutoff
    float angle = acos(dot(-s, normalize(light.direction)));
    float cutoff = radians(clamp(light.cutoff, 0.0, 90.0));

    // Outside spotlight cone only ambient
    if (angle >= cutoff)
        return ambient;

    // Spotlight falloff factor
    float spotFactor = pow(max(dot(-s, normalize(light.direction)), 0.0), light.exponent);

    vec3 v = normalize(-p.xyz);
    vec3 r = reflect(-s, n);

    float sDotN = max(dot(s, n), 0.0);
    vec3 diffuse = light.Ld * material1.Md * sDotN;

    vec3 specular = vec3(0.0);
    if (sDotN > 0.0)
    {
        specular = light.Ls * material1.Ms *
            pow(max(dot(r, v), 0.0), material1.shininess);
    }

    return ambient + spotFactor * (diffuse + specular);
}

void main()
{
    // Transform position into eye space.
    vec4 eyePosition = matrices.modelViewMatrix * vec4(inPosition, 1.0);

    // Transform and normalise normal vector.
    vec3 eyeNormal = normalize(matrices.normalMatrix * inNormal);

    // Compute final clip-space position.
    gl_Position = matrices.projMatrix * eyePosition;

    // Pass texture coordinates to fragment shader.
    vTexCoord = inCoord;

    // Compute lighting contributions:
    // - main scene light
    // - left headlight
    // - right headlight
    vec3 baseColour = PhongModel(eyePosition, eyeNormal);
    vec3 leftLight = SpotlightModel(headlightLeft, eyePosition, eyeNormal);
    vec3 rightLight = SpotlightModel(headlightRight, eyePosition, eyeNormal);

    // Final per-vertex colour (interpolated across the surface).
    vColour = baseColour + leftLight + rightLight;
}