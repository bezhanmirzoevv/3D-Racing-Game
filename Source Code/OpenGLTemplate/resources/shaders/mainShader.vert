#version 400 core

// Structure for matrices
uniform struct Matrices
{
	mat4 projMatrix;
	mat4 modelViewMatrix; 
	mat3 normalMatrix;
} matrices;

// Structure holding light information:  its position as well as ambient, diffuse, and specular colours
struct LightInfo
{
	vec4 position;
	vec3 La;
	vec3 Ld;
	vec3 Ls;
};

// Structure holding material information:  its ambient, diffuse, and specular colours, and shininess
struct MaterialInfo
{
	vec3 Ma;
	vec3 Md;
	vec3 Ms;
	float shininess;
};

struct SpotlightInfo
{
	vec4 position;
	vec3 direction;
	vec3 La;
	vec3 Ld;
	vec3 Ls;
	float cutoff;
	float exponent;
	float attenuation;
};

// Lights and materials passed in as uniform variables from client programme
uniform LightInfo light1; 
uniform MaterialInfo material1; 
uniform SpotlightInfo headlightLeft;
uniform SpotlightInfo headlightRight;

// Layout of vertex attributes in VBO
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inCoord;
layout (location = 2) in vec3 inNormal;

// Vertex colour output to fragment shader -- using Gouraud (interpolated) shading
out vec3 vColour;	// Colour computed using reflectance model
out vec2 vTexCoord;	// Texture coordinate

out vec3 worldPosition;	// used for skybox

// This function implements the Phong shading model
vec3 PhongModel(vec4 eyePosition, vec3 eyeNorm)
{
	// Direction from fragment to main light in eye space.
	vec3 s = normalize(vec3(light1.position - eyePosition));

	// Direction from fragment to viewer in eye space.
	vec3 v = normalize(-eyePosition.xyz);

	// Reflection direction for specular term.
	vec3 r = reflect(-s, eyeNorm);

	vec3 n = eyeNorm;

	// Ambient lighting contribution.
	vec3 ambient = light1.La * material1.Ma;

	// Diffuse lighting contribution.
	float sDotN = max(dot(s, n), 0.0f);
	vec3 diffuse = light1.Ld * material1.Md * sDotN;

	// Specular lighting contribution.
	vec3 specular = vec3(0.0f);
	float eps = 0.000001f; // add eps to shininess below -- pow not defined if second argument is 0 (as described in GLSL documentation)
	if (sDotN > 0.0f) 
		specular = light1.Ls * material1.Ms * pow(max(dot(r, v), 0.0f), material1.shininess + eps);
	

	return ambient + diffuse + specular;

}

// Spotlight model used for the car headlights.
vec3 SpotlightModel(SpotlightInfo light, vec4 eyePosition, vec3 eyeNorm)
{
	// Spotlight direction in eye space.
	vec3 lightDir = normalize(light.direction);

	// Direction from fragment to spotlight position.
	vec3 s = normalize(vec3(light.position - eyePosition));

	// View and reflection directions for specular term.
	vec3 v = normalize(-eyePosition.xyz);
	vec3 r = reflect(-s, eyeNorm);
	vec3 n = eyeNorm;

	// Ambient contribution from spotlight.
	vec3 ambient = light.La * material1.Ma;

	// Compare fragment direction with spotlight cutoff cone.
	float cosAngle = dot(-s, lightDir);
	float cosCutoff = cos(radians(clamp(light.cutoff, 0.0f, 90.0f)));

	if (cosAngle < cosCutoff)
		return ambient;

	// Spotlight falloff inside the cone.
	float spotFactor = pow(max(cosAngle, 0.0f), light.exponent);

	// Diffuse term.
	float sDotN = max(dot(s, n), 0.0f);
	vec3 diffuse = light.Ld * material1.Md * sDotN;

	// Distance attenuation for spotlight intensity.
	float distance = length(light.position.xyz - eyePosition.xyz);
	float attenuation = 1.0 / (1.0 + light.attenuation * distance * distance);

	// Specular term.
	vec3 specular = vec3(0.0f);
	float eps = 0.000001f;
	if (sDotN > 0.0f)
		specular = light.Ls * material1.Ms * pow(max(dot(r, v), 0.0f), material1.shininess + eps);

	vec3 result = ambient + attenuation * spotFactor * (diffuse + specular);

	// Clamp spotlight brightness to keep it visually controlled.
	return min(result, vec3(0.8f));
}

// This is the entry point into the vertex shader
void main()
{	

	// Save the world position for rendering the skybox
	worldPosition = inPosition;

	// Transform the vertex spatial position using 
	gl_Position = matrices.projMatrix * matrices.modelViewMatrix * vec4(inPosition, 1.0f);
	
	// Get the vertex normal and vertex position in eye coordinates
	vec3 vEyeNorm = normalize(matrices.normalMatrix * inNormal);
	vec4 vEyePosition = matrices.modelViewMatrix * vec4(inPosition, 1.0f);
		
	// Apply the Phong model to compute the vertex colour
	vec3 baseColour = PhongModel(vEyePosition, vEyeNorm);
	vec3 leftHeadlightColour = SpotlightModel(headlightLeft, vEyePosition, vEyeNorm);
	vec3 rightHeadlightColour = SpotlightModel(headlightRight, vEyePosition, vEyeNorm);
	vColour = baseColour + leftHeadlightColour + rightHeadlightColour;
	
	// Pass through the texture coordinate
	vTexCoord = inCoord;
} 