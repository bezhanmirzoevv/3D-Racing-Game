#pragma once

#include "Common.h"
#include "Texture.h"
#include <vector>
#include <string>

// CCameraPath
// -----------
// Generates and renders the coursework route as a spline-based track.
// The centreline is defined by Catmull-Rom control points, then sampled to
// build triangle geometry for the top, sides, and underside of the track.
class CCameraPath
{
public:
	CCameraPath();
	~CCameraPath();

	// Creates the track geometry and loads the textures used for the
	// top surface and the sides/underside.
	void CreateTrack(const std::string& directory,
		const std::string& topFilename,
		const std::string& sideFilename,
		float trackHalfWidth,
		int samplesPerSegment);

	// Returns a point on the spline for a normalised parameter t in [0,1).
	glm::vec3 Interpolate(float t) const;

	// Returns the normal direction of the track at a point based on the tilt angles
	glm::vec3 InterpolatedControlNormal(float t) const;

	// Returns the unit tangent direction of the spline for a normalised parameter t.
	glm::vec3 Tangent(float t) const;

	// Computes a local frame on the track made from:
	// position  - spline point
	// tangent   - forward direction along the track
	// normal    - upward direction of the local frame
	// binormal  - sideways direction across the width of the track
	void GetTrackFrame(float t,
		glm::vec3& position,
		glm::vec3& tangent,
		glm::vec3& normal,
		glm::vec3& binormal) const;

	// Renders the generated track geometry.
	void RenderTrack();

	// Releases textures and GPU buffers associated with the track.
	void Release();

	// Sets the world position of the route object.
	void SetPosition(const glm::vec3& position);

	// Returns the model transform applied to the route.
	glm::mat4 GetTransform() const;

	// Returns half the width of the generated road surface.
	float GetTrackHalfWidth() const;

private:
	// Single interleaved vertex used for route rendering.
	struct TrackVertex
	{
		glm::vec3 position;
		glm::vec2 texCoord;
		glm::vec3 normal;
	};

	// Stores one sampled cross-section of the track.
	// Each section contains the top and bottom edge positions used to build quads
	// between neighbouring samples.
	struct TrackSection
	{
		glm::vec3 topLeft;
		glm::vec3 topRight;
		glm::vec3 bottomLeft;
		glm::vec3 bottomRight;
		float vTex;
	};

	// Stores the position and normal of a point sampled on the track centreline.
	struct TrackPoint
	{
		glm::vec3 position;
		glm::vec3 normal;
	};

private:
	// Catmull-Rom interpolation for the region between p1 and p2.
	glm::vec3 CatmullRom(const glm::vec3& p0, const glm::vec3& p1,
		const glm::vec3& p2, const glm::vec3& p3, float t) const;

	// Derivative of the Catmull-Rom spline used to compute the tangent direction.
	glm::vec3 CatmullRomTangent(const glm::vec3& p0, const glm::vec3& p1,
		const glm::vec3& p2, const glm::vec3& p3, float t) const;

	// Creates the spline control points that define the overall route layout.
	void BuildControlPoints();

	// Computes the normal direction of the track at a point based on the tilt angles
	glm::vec3 ControlNormal(float xTilt, float yTilt, float zTilt) const;

	// Samples the spline and builds the top, side, and bottom triangle geometry.
	void BuildTrackGeometry();

	// Adds a quad as two triangles to a vertex list.
	void AddQuad(std::vector<TrackVertex>& verts,
		const glm::vec3& p0, const glm::vec3& p1,
		const glm::vec3& p2, const glm::vec3& p3,
		const glm::vec3& normal,
		const glm::vec2& uv0, const glm::vec2& uv1,
		const glm::vec2& uv2, const glm::vec2& uv3);

	// Uploads a generated mesh to a VAO/VBO pair.
	void UploadMesh(GLuint& vao, GLuint& vbo, int& vertexCount,
		const std::vector<TrackVertex>& verts);

	// Rebuilds the model matrix after a transform change.
	void UpdateModelMatrix();

private:
	// Track textures
	CTexture m_trackTopTexture;
	CTexture m_trackSideTexture;

	std::string m_directory;
	std::string m_topFilename;
	std::string m_sideFilename;

	// GPU data for the top surface of the road
	GLuint m_topVAO = 0;
	GLuint m_topVBO = 0;
	int m_topVertexCount = 0;

	// GPU data for the sides and underside of the road
	GLuint m_sideVAO = 0;
	GLuint m_sideVBO = 0;
	int m_sideVertexCount = 0;

	// Track dimensions and sampling resolution
	float m_trackHalfWidth = 10.0f;
	float m_trackThickness = 1.0f;
	int m_samplesPerSegment = 40;

	// Catmull-Rom spline control points
	std::vector<TrackPoint> m_controlPoints;

	// World transform of the route object
	glm::vec3 m_position = glm::vec3(0.0f);
	glm::mat4 m_modelMatrix = glm::mat4(1.0f);
};