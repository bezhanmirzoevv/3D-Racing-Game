#include "CameraPath.h"
#include <cstddef>
#include <cmath>
#include <algorithm>

CCameraPath::CCameraPath()
{
	UpdateModelMatrix();
}

CCameraPath::~CCameraPath()
{
	Release();
}

glm::vec3 CCameraPath::CatmullRom(const glm::vec3& p0, const glm::vec3& p1,
	const glm::vec3& p2, const glm::vec3& p3, float t) const
{
	// Catmull-Rom interpolation
	float t2 = t * t;
	float t3 = t2 * t;

	glm::vec3 a = p1;
	glm::vec3 b = 0.5f * (-p0 + p2);
	glm::vec3 c = 0.5f * (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3);
	glm::vec3 d = 0.5f * (-p0 + 3.0f * p1 - 3.0f * p2 + p3);

	return a + b * t + c * t2 + d * t3;
}

glm::vec3 CCameraPath::CatmullRomTangent(const glm::vec3& p0, const glm::vec3& p1,
	const glm::vec3& p2, const glm::vec3& p3, float t) const
{
	// Derivative of the Catmull-Rom spline
	float t2 = t * t;

	glm::vec3 b = 0.5f * (-p0 + p2);
	glm::vec3 c = 0.5f * (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3);
	glm::vec3 d = 0.5f * (-p0 + 3.0f * p1 - 3.0f * p2 + p3);

	return b + 2.0f * c * t + 3.0f * d * t2;
}

void CCameraPath::BuildControlPoints()
{
	m_controlPoints.clear();

	float sx = 1.0f;
	float sz = 1.0f;
	float sy = 1.0f;

	// Control points defining the overall route shape.
	// The path is closed and sampled cyclically to form a continuous loop.
	// Start section
	m_controlPoints.push_back({ glm::vec3(65.0f * sx, 0.0f * sy, 15.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(105.0f * sx, 0.0f * sy, -10.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(140.0f * sx, 0.0f * sy, -35.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });

	// First big loop
	m_controlPoints.push_back({ glm::vec3(170.0f * sx, 0.0f * sy, -20.0f * sz), ControlNormal(0.5f, 1.0f, -0.5f) });
	m_controlPoints.push_back({ glm::vec3(205.0f * sx, 0.0f * sy, -30.0f * sz), ControlNormal(-0.5f, 1.0f, -0.5f) });
	m_controlPoints.push_back({ glm::vec3(220.0f * sx, 0.0f * sy, -65.0f * sz), ControlNormal(-0.7f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(210.0f * sx, 0.0f * sy, -100.0f * sz), ControlNormal(-0.7f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(180.0f * sx, 0.0f * sy, -110.0f * sz), ControlNormal(-0.9f, 1.0f, 0.9f) });

	// Straight parralel to the start
	m_controlPoints.push_back({ glm::vec3(145.0f * sx, 2.0f * sy, -90.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(115.0f * sx, 6.0f * sy, -55.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(80.0f * sx, 6.0f * sy, -20.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(45.0f * sx, 8.0f * sy, 20.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });

	// Loop under itself
	m_controlPoints.push_back({ glm::vec3(18.0f * sx, 8.0f * sy, 82.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(42.0f * sx, 8.0f * sy, 88.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(62.0f * sx, 0.0f * sy, 65.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(60.0f * sx, -6.0f * sy, 35.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(35.0f * sx, -8.0f * sy, 10.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(15.0f * sx, -6.0f * sy, -20.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(5.0f * sx, -6.0f * sy, -55.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });

	// Second big loop
	m_controlPoints.push_back({ glm::vec3(-5.0f * sx, 0.0f * sy, -80.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(-25.0f * sx, 0.0f * sy, -105.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(-70.0f * sx, 0.0f * sy, -100.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(-95.0f * sx, 0.0f * sy, -70.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(-95.0f * sx, 0.0f * sy, -25.0f * sz), ControlNormal(0.5f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(-74.0f * sx, 0.0f * sy, -6.0f * sz), ControlNormal(0.5f, 1.0f, -0.5f) });
	m_controlPoints.push_back({ glm::vec3(-42.0f * sx, 0.0f * sy, 0.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(-20.0f * sx, 0.0f * sy, 12.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });

	// Loop under itself around earth
	m_controlPoints.push_back({ glm::vec3(-22.0f * sx, -2.0f * sy, 34.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(-18.0f * sx, -4.0f * sy, 58.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(-8.0f * sx, -6.5f * sy, 70.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(12.0f * sx, -8.5f * sy, 84.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(34.0f * sx, -8.0f * sy, 92.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(52.0f * sx, -6.0f * sy, 104.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(62.0f * sx, -4.0f * sy, 124.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(58.0f * sx, -2.0f * sy, 148.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(42.0f * sx, 0.0f * sy, 166.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(18.0f * sx, 0.0f * sy, 172.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(-5.0f * sx, 0.0f * sy, 156.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(-20.0f * sx, 0.0f * sy, 132.0f * sz), ControlNormal(0.5f, 1.0f, 0.0f) });

	// Finishing straight
	m_controlPoints.push_back({ glm::vec3(-15.0f * sx, 0.0f * sy, 112.0f * sz), ControlNormal(0.5f, 1.0f, 0.0f) });
	m_controlPoints.push_back({ glm::vec3(25.0f * sx, 0.0f * sy, 35.0f * sz), ControlNormal(0.0f, 1.0f, 0.0f) });
}

glm::vec3 CCameraPath::Interpolate(float t) const
{
	if (m_controlPoints.size() < 4)
		return glm::vec3(0.0f);

	// Wrap t so the route behaves as a closed loop.
	t = t - floor(t);
	if (t < 0.0f)
		t += 1.0f;

	int numSegments = (int)m_controlPoints.size();
	float ft = t * (float)numSegments;

	int seg = (int)floor(ft);
	float localT = ft - (float)seg;

	int i0 = (seg - 1 + numSegments) % numSegments;
	int i1 = seg % numSegments;
	int i2 = (seg + 1) % numSegments;
	int i3 = (seg + 2) % numSegments;

	return CatmullRom(m_controlPoints[i0].position, m_controlPoints[i1].position, m_controlPoints[i2].position, m_controlPoints[i3].position, localT);
}

glm::vec3 CCameraPath::InterpolatedControlNormal(float t) const
{
	if (m_controlPoints.size() < 4)
		return glm::vec3(0.0f, 1.0f, 0.0f);

	t = t - floor(t);
	if (t < 0.0f)
		t += 1.0f;

	int numSegments = (int)m_controlPoints.size();
	float ft = t * (float)numSegments;

	int seg = (int)floor(ft);
	float localT = ft - (float)seg;

	int i0 = (seg - 1 + numSegments) % numSegments;
	int i1 = seg % numSegments;
	int i2 = (seg + 1) % numSegments;
	int i3 = (seg + 2) % numSegments;

	glm::vec3 b = CatmullRom(
		m_controlPoints[i0].normal,
		m_controlPoints[i1].normal,
		m_controlPoints[i2].normal,
		m_controlPoints[i3].normal,
		localT);

	return glm::normalize(b);
}

glm::vec3 CCameraPath::Tangent(float t) const
{
	if (m_controlPoints.size() < 4)
		return glm::vec3(1.0f, 0.0f, 0.0f);

	// Wrap t so tangent lookup is also cyclic.
	t = t - floor(t);
	if (t < 0.0f)
		t += 1.0f;

	int numSegments = (int)m_controlPoints.size();
	float ft = t * (float)numSegments;

	int seg = (int)floor(ft);
	float localT = ft - (float)seg;

	int i0 = (seg - 1 + numSegments) % numSegments;
	int i1 = seg % numSegments;
	int i2 = (seg + 1) % numSegments;
	int i3 = (seg + 2) % numSegments;

	return glm::normalize(CatmullRomTangent(m_controlPoints[i0].position, m_controlPoints[i1].position, m_controlPoints[i2].position, m_controlPoints[i3].position, localT));
}

void CCameraPath::GetTrackFrame(float t,
	glm::vec3& position,
	glm::vec3& tangent,
	glm::vec3& normal,
	glm::vec3& binormal) const
{
	position = Interpolate(t);
	tangent = Tangent(t);

	// This is the interpolated banking control vector.
	binormal = InterpolatedControlNormal(t);

	// Make sure binormal is not accidentally parallel to tangent.
	if (glm::length(glm::cross(tangent, binormal)) < 0.0001f)
	{
		binormal = glm::vec3(0.0f, 1.0f, 0.0f);
	}

	// Lecture-aligned TNB frame:
	// tangent = direction of travel
	// binormal = local up / banking vector
	// normal = sideways vector
	normal = glm::normalize(glm::cross(tangent, binormal));

	// Rebuild binormal so the frame is clean and orthogonal.
	binormal = glm::normalize(glm::cross(normal, tangent));
}

void CCameraPath::AddQuad(std::vector<TrackVertex>& verts,
	const glm::vec3& p0, const glm::vec3& p1,
	const glm::vec3& p2, const glm::vec3& p3,
	const glm::vec3& normal,
	const glm::vec2& uv0, const glm::vec2& uv1,
	const glm::vec2& uv2, const glm::vec2& uv3)
{
	// First triangle
	verts.push_back({ p0, uv0, normal });
	verts.push_back({ p1, uv1, normal });
	verts.push_back({ p2, uv2, normal });

	// Second triangle
	verts.push_back({ p0, uv0, normal });
	verts.push_back({ p2, uv2, normal });
	verts.push_back({ p3, uv3, normal });
}

void CCameraPath::UploadMesh(GLuint& vao, GLuint& vbo, int& vertexCount,
	const std::vector<TrackVertex>& verts)
{
	// Delete old mesh data before uploading a rebuilt version.
	if (vao)
	{
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}
	if (vbo)
	{
		glDeleteBuffers(1, &vbo);
		vbo = 0;
	}

	vertexCount = (int)verts.size();
	if (vertexCount == 0)
		return;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER,
		verts.size() * sizeof(TrackVertex),
		verts.data(),
		GL_STATIC_DRAW);

	GLsizei stride = sizeof(TrackVertex);

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
		(void*)offsetof(TrackVertex, position));

	// Texture coordinate attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride,
		(void*)offsetof(TrackVertex, texCoord));

	// Normal attribute
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,
		(void*)offsetof(TrackVertex, normal));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CCameraPath::BuildTrackGeometry()
{
	std::vector<TrackVertex> topVerts;
	std::vector<TrackVertex> sideVerts;
	std::vector<TrackSection> sections;

	// The route is sampled uniformly across every Catmull-Rom segment.
	const int totalSamples = (int)m_controlPoints.size() * m_samplesPerSegment;
	sections.reserve(totalSamples + 1);

	float vTex = 0.0f;
	glm::vec3 prevCentre = Interpolate(0.0f);

	for (int i = 0; i <= totalSamples; ++i)
	{
		float t = (float)i / (float)totalSamples;

		glm::vec3 p, T, N, B;
		GetTrackFrame(t, p, T, N, B);

		// Offset left and right from the centreline using the local binormal.
		glm::vec3 left = p - m_trackHalfWidth * N;
		glm::vec3 right = p + m_trackHalfWidth * N;

		// Accumulate distance-based texture coordinates so the road texture tiles
		// along the direction of travel.
		if (i > 0)
			vTex += glm::length(p - prevCentre) * 0.08f;

		prevCentre = p;

		TrackSection s;
		s.topLeft = left;
		s.topRight = right;
		s.bottomLeft = left - m_trackThickness * B;
		s.bottomRight = right - m_trackThickness * B;
		s.vTex = vTex;
		sections.push_back(s);
	}

	float sideV = 0.58f;

	for (int i = 0; i < totalSamples; ++i)
	{
		const TrackSection& a = sections[i];
		const TrackSection& b = sections[i + 1];

		glm::vec2 uvTL0(0.0f, a.vTex);
		glm::vec2 uvTR0(1.0f, a.vTex);
		glm::vec2 uvTL1(0.0f, b.vTex);
		glm::vec2 uvTR1(1.0f, b.vTex);

		// Top surface of the road
		glm::vec3 nTop = glm::normalize(glm::cross(b.topLeft - a.topLeft, a.topRight - a.topLeft));
		AddQuad(topVerts,
			a.topLeft, a.topRight, b.topRight, b.topLeft,
			nTop,
			uvTL0, uvTR0, uvTR1, uvTL1);

		// Left side wall
		glm::vec3 nLeft = glm::normalize(glm::cross(a.bottomLeft - a.topLeft, b.topLeft - a.topLeft));
		AddQuad(sideVerts,
			a.topLeft, b.topLeft, b.bottomLeft, a.bottomLeft,
			nLeft,
			glm::vec2(0.0f, a.vTex),
			glm::vec2(0.0f, b.vTex),
			glm::vec2(sideV, b.vTex),
			glm::vec2(sideV, a.vTex));

		// Right side wall
		glm::vec3 nRight = glm::normalize(glm::cross(b.topRight - a.topRight, a.bottomRight - a.topRight));
		AddQuad(sideVerts,
			a.topRight, a.bottomRight, b.bottomRight, b.topRight,
			nRight,
			glm::vec2(0.0f, a.vTex),
			glm::vec2(sideV, a.vTex),
			glm::vec2(sideV, b.vTex),
			glm::vec2(0.0f, b.vTex));

		// Underside of the road
		glm::vec3 nBottom = -nTop;
		AddQuad(sideVerts,
			a.bottomLeft, b.bottomLeft, b.bottomRight, a.bottomRight,
			nBottom,
			uvTL0, uvTL1, uvTR1, uvTR0);
	}

	UploadMesh(m_topVAO, m_topVBO, m_topVertexCount, topVerts);
	UploadMesh(m_sideVAO, m_sideVBO, m_sideVertexCount, sideVerts);
}

void CCameraPath::CreateTrack(const std::string& directory,
	const std::string& topFilename,
	const std::string& sideFilename,
	float trackHalfWidth,
	int samplesPerSegment)
{
	Release();

	m_directory = directory;
	m_topFilename = topFilename;
	m_sideFilename = sideFilename;
	m_trackHalfWidth = trackHalfWidth;
	m_samplesPerSegment = std::max(4, samplesPerSegment);

	// Build the spline route first, then load textures and generate mesh data.
	BuildControlPoints();

	m_trackTopTexture.Load(m_directory + m_topFilename);
	m_trackTopTexture.SetSamplerObjectParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	m_trackTopTexture.SetSamplerObjectParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_trackTopTexture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_trackTopTexture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

	m_trackSideTexture.Load(m_directory + m_sideFilename);
	m_trackSideTexture.SetSamplerObjectParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	m_trackSideTexture.SetSamplerObjectParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_trackSideTexture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_trackSideTexture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

	BuildTrackGeometry();
}

void CCameraPath::RenderTrack()
{
	// Render top road surface
	if (m_topVAO != 0 && m_topVertexCount > 0)
	{
		m_trackTopTexture.Bind();
		glBindVertexArray(m_topVAO);
		glDrawArrays(GL_TRIANGLES, 0, m_topVertexCount);
		glBindVertexArray(0);
	}

	// Render side walls and underside
	if (m_sideVAO != 0 && m_sideVertexCount > 0)
	{
		m_trackSideTexture.Bind();
		glBindVertexArray(m_sideVAO);
		glDrawArrays(GL_TRIANGLES, 0, m_sideVertexCount);
		glBindVertexArray(0);
	}
}

void CCameraPath::Release()
{
	// Release top mesh
	if (m_topVBO)
	{
		glDeleteBuffers(1, &m_topVBO);
		m_topVBO = 0;
	}
	if (m_topVAO)
	{
		glDeleteVertexArrays(1, &m_topVAO);
		m_topVAO = 0;
	}

	// Release side mesh
	if (m_sideVBO)
	{
		glDeleteBuffers(1, &m_sideVBO);
		m_sideVBO = 0;
	}
	if (m_sideVAO)
	{
		glDeleteVertexArrays(1, &m_sideVAO);
		m_sideVAO = 0;
	}

	m_topVertexCount = 0;
	m_sideVertexCount = 0;
}

void CCameraPath::UpdateModelMatrix()
{
	m_modelMatrix = glm::mat4(1.0f);
	m_modelMatrix = glm::translate(m_modelMatrix, m_position);
}

void CCameraPath::SetPosition(const glm::vec3& position)
{
	m_position = position;
	UpdateModelMatrix();
}

glm::mat4 CCameraPath::GetTransform() const
{
	return m_modelMatrix;
}

float CCameraPath::GetTrackHalfWidth() const
{
	return m_trackHalfWidth;
}
glm::vec3 CCameraPath::ControlNormal(float xTilt, float yTilt, float zTilt) const
{
	return glm::normalize(glm::vec3(xTilt, yTilt, zTilt));
}