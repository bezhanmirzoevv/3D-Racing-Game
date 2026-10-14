#include "StartLine.h"

namespace
{
	// Adds one interleaved vertex to the VBO.
	// Vertex layout: position, texture coordinate, normal.
	void AddVertex(CVertexBufferObject& vbo, int& vertexCount,
		const glm::vec3& p, const glm::vec2& t, const glm::vec3& n)
	{
		vbo.AddData((void*)&p, sizeof(glm::vec3));
		vbo.AddData((void*)&t, sizeof(glm::vec2));
		vbo.AddData((void*)&n, sizeof(glm::vec3));
		vertexCount++;
	}

	// Adds a triangle using per-vertex normals for smooth shading.
	void AddTriangleSmooth(CVertexBufferObject& vbo, int& vertexCount,
		const glm::vec3& p0, const glm::vec2& t0, const glm::vec3& n0,
		const glm::vec3& p1, const glm::vec2& t1, const glm::vec3& n1,
		const glm::vec3& p2, const glm::vec2& t2, const glm::vec3& n2)
	{
		AddVertex(vbo, vertexCount, p0, t0, n0);
		AddVertex(vbo, vertexCount, p1, t1, n1);
		AddVertex(vbo, vertexCount, p2, t2, n2);
	}

	// Adds a quad as two triangles using per-vertex normals.
	void AddQuadSmooth(CVertexBufferObject& vbo, int& vertexCount,
		const glm::vec3& p0, const glm::vec2& t0, const glm::vec3& n0,
		const glm::vec3& p1, const glm::vec2& t1, const glm::vec3& n1,
		const glm::vec3& p2, const glm::vec2& t2, const glm::vec3& n2,
		const glm::vec3& p3, const glm::vec2& t3, const glm::vec3& n3)
	{
		AddTriangleSmooth(vbo, vertexCount, p0, t0, n0, p1, t1, n1, p2, t2, n2);
		AddTriangleSmooth(vbo, vertexCount, p0, t0, n0, p2, t2, n2, p3, t3, n3);
	}

	// Loads a texture and sets sampler parameters suitable for mipmapped rendering.
	void LoadTexture(CTexture& tex, const string& filename)
	{
		tex.Load(filename, true);
		tex.SetSamplerObjectParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		tex.SetSamplerObjectParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		tex.SetSamplerObjectParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
		tex.SetSamplerObjectParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	// Generates a ring surface by sweeping a circular tube around a larger circular path.
	// This is used for both the thick inner ring and thin outer ring.
	void AddRing(CVertexBufferObject& vbo, int& vertexCount,
		float majorRadius,
		float tubeRadius,
		int majorSegments,
		int minorSegments,
		float texRepeatU,
		float texRepeatV)
	{
		for (int i = 0; i < majorSegments; i++)
		{
			float u0 = 2.0f * glm::pi<float>() * (float)i / (float)majorSegments;
			float u1 = 2.0f * glm::pi<float>() * (float)(i + 1) / (float)majorSegments;

			for (int j = 0; j < minorSegments; j++)
			{
				float v0 = 2.0f * glm::pi<float>() * (float)j / (float)minorSegments;
				float v1 = 2.0f * glm::pi<float>() * (float)(j + 1) / (float)minorSegments;

				auto MakePoint = [&](float u, float v) -> glm::vec3
					{
						float cu = cos(u);
						float su = sin(u);
						float cv = cos(v);
						float sv = sin(v);

						float r = majorRadius + tubeRadius * cv;

						return glm::vec3(
							r * cu,
							r * su,
							tubeRadius * sv
						);
					};

				auto MakeNormal = [&](float u, float v) -> glm::vec3
					{
						float cu = cos(u);
						float su = sin(u);
						float cv = cos(v);
						float sv = sin(v);

						return glm::normalize(glm::vec3(
							cv * cu,
							cv * su,
							sv
						));
					};

				glm::vec3 p00 = MakePoint(u0, v0);
				glm::vec3 p10 = MakePoint(u1, v0);
				glm::vec3 p11 = MakePoint(u1, v1);
				glm::vec3 p01 = MakePoint(u0, v1);

				glm::vec3 n00 = MakeNormal(u0, v0);
				glm::vec3 n10 = MakeNormal(u1, v0);
				glm::vec3 n11 = MakeNormal(u1, v1);
				glm::vec3 n01 = MakeNormal(u0, v1);

				glm::vec2 t00(texRepeatU * (float)i / (float)majorSegments,
					texRepeatV * (float)j / (float)minorSegments);
				glm::vec2 t10(texRepeatU * (float)(i + 1) / (float)majorSegments,
					texRepeatV * (float)j / (float)minorSegments);
				glm::vec2 t11(texRepeatU * (float)(i + 1) / (float)majorSegments,
					texRepeatV * (float)(j + 1) / (float)minorSegments);
				glm::vec2 t01(texRepeatU * (float)i / (float)majorSegments,
					texRepeatV * (float)(j + 1) / (float)minorSegments);

				AddQuadSmooth(vbo, vertexCount,
					p00, t00, n00,
					p10, t10, n10,
					p11, t11, n11,
					p01, t01, n01);
			}
		}
	}

	// Adds a sign box and separates its draw ranges into:
	// - front/back faces
	// - side faces
	// so that different textures can be applied to each group.
	void AddBoxSplit(CVertexBufferObject& vbo, int& vertexCount,
		float width, float height, float depth,
		float yCentre, float zCentre,
		int& frontStart, int& frontCount,
		int& sideStart, int& sideCount)
	{
		float x0 = -0.5f * width;
		float x1 = 0.5f * width;
		float y0 = yCentre - 0.5f * height;
		float y1 = yCentre + 0.5f * height;
		float z0 = zCentre - 0.5f * depth;
		float z1 = zCentre + 0.5f * depth;

		glm::vec3 p000(x0, y0, z0);
		glm::vec3 p001(x0, y0, z1);
		glm::vec3 p010(x0, y1, z0);
		glm::vec3 p011(x0, y1, z1);
		glm::vec3 p100(x1, y0, z0);
		glm::vec3 p101(x1, y0, z1);
		glm::vec3 p110(x1, y1, z0);
		glm::vec3 p111(x1, y1, z1);

		frontStart = vertexCount;

		// Front face
		{
			glm::vec3 n(0.0f, 0.0f, 1.0f);
			AddQuadSmooth(vbo, vertexCount,
				p001, glm::vec2(0.0f, 0.0f), n,
				p101, glm::vec2(1.0f, 0.0f), n,
				p111, glm::vec2(1.0f, 1.0f), n,
				p011, glm::vec2(0.0f, 1.0f), n);
		}

		// Back face
		{
			glm::vec3 n(0.0f, 0.0f, -1.0f);
			AddQuadSmooth(vbo, vertexCount,
				p100, glm::vec2(0.0f, 0.0f), n,
				p000, glm::vec2(1.0f, 0.0f), n,
				p010, glm::vec2(1.0f, 1.0f), n,
				p110, glm::vec2(0.0f, 1.0f), n);
		}

		frontCount = vertexCount - frontStart;

		sideStart = vertexCount;

		// Left face
		{
			glm::vec3 n(-1.0f, 0.0f, 0.0f);
			AddQuadSmooth(vbo, vertexCount,
				p000, glm::vec2(0.0f, 0.0f), n,
				p001, glm::vec2(1.0f, 0.0f), n,
				p011, glm::vec2(1.0f, 1.0f), n,
				p010, glm::vec2(0.0f, 1.0f), n);
		}

		// Right face
		{
			glm::vec3 n(1.0f, 0.0f, 0.0f);
			AddQuadSmooth(vbo, vertexCount,
				p101, glm::vec2(0.0f, 0.0f), n,
				p100, glm::vec2(1.0f, 0.0f), n,
				p110, glm::vec2(1.0f, 1.0f), n,
				p111, glm::vec2(0.0f, 1.0f), n);
		}

		// Top face
		{
			glm::vec3 n(0.0f, 1.0f, 0.0f);
			AddQuadSmooth(vbo, vertexCount,
				p011, glm::vec2(0.0f, 0.0f), n,
				p111, glm::vec2(1.0f, 0.0f), n,
				p110, glm::vec2(1.0f, 1.0f), n,
				p010, glm::vec2(0.0f, 1.0f), n);
		}

		// Bottom face
		{
			glm::vec3 n(0.0f, -1.0f, 0.0f);
			AddQuadSmooth(vbo, vertexCount,
				p000, glm::vec2(0.0f, 0.0f), n,
				p100, glm::vec2(1.0f, 0.0f), n,
				p101, glm::vec2(1.0f, 1.0f), n,
				p001, glm::vec2(0.0f, 1.0f), n);
		}

		sideCount = vertexCount - sideStart;
	}

	// Adds a thin box used as the visible checkered finish-line crossing.
	void AddCrossingBox(CVertexBufferObject& vbo, int& vertexCount,
		float width, float height, float depth,
		float yCentre, float zCentre)
	{
		float x0 = -0.5f * width;
		float x1 = 0.5f * width;
		float y0 = yCentre - 0.5f * height;
		float y1 = yCentre + 0.5f * height;
		float z0 = zCentre - 0.5f * depth;
		float z1 = zCentre + 0.5f * depth;

		glm::vec3 p000(x0, y0, z0);
		glm::vec3 p001(x0, y0, z1);
		glm::vec3 p010(x0, y1, z0);
		glm::vec3 p011(x0, y1, z1);
		glm::vec3 p100(x1, y0, z0);
		glm::vec3 p101(x1, y0, z1);
		glm::vec3 p110(x1, y1, z0);
		glm::vec3 p111(x1, y1, z1);

		// Top face: visible checkered strip
		{
			float repeatU = width * 0.5f;
			float repeatV = depth * 0.5f;
			glm::vec3 n(0.0f, 1.0f, 0.0f);

			AddQuadSmooth(vbo, vertexCount,
				p011, glm::vec2(0.0f, 0.0f), n,
				p111, glm::vec2(repeatU, 0.0f), n,
				p110, glm::vec2(repeatU, repeatV), n,
				p010, glm::vec2(0.0f, repeatV), n);
		}

		// Bottom face
		{
			glm::vec3 n(0.0f, -1.0f, 0.0f);
			AddQuadSmooth(vbo, vertexCount,
				p000, glm::vec2(0.0f, 0.0f), n,
				p100, glm::vec2(1.0f, 0.0f), n,
				p101, glm::vec2(1.0f, 1.0f), n,
				p001, glm::vec2(0.0f, 1.0f), n);
		}

		// Front face
		{
			glm::vec3 n(0.0f, 0.0f, 1.0f);
			AddQuadSmooth(vbo, vertexCount,
				p001, glm::vec2(0.0f, 0.0f), n,
				p101, glm::vec2(1.0f, 0.0f), n,
				p111, glm::vec2(1.0f, 1.0f), n,
				p011, glm::vec2(0.0f, 1.0f), n);
		}

		// Back face
		{
			glm::vec3 n(0.0f, 0.0f, -1.0f);
			AddQuadSmooth(vbo, vertexCount,
				p100, glm::vec2(0.0f, 0.0f), n,
				p000, glm::vec2(1.0f, 0.0f), n,
				p010, glm::vec2(1.0f, 1.0f), n,
				p110, glm::vec2(0.0f, 1.0f), n);
		}

		// Left face
		{
			glm::vec3 n(-1.0f, 0.0f, 0.0f);
			AddQuadSmooth(vbo, vertexCount,
				p000, glm::vec2(0.0f, 0.0f), n,
				p001, glm::vec2(1.0f, 0.0f), n,
				p011, glm::vec2(1.0f, 1.0f), n,
				p010, glm::vec2(0.0f, 1.0f), n);
		}

		// Right face
		{
			glm::vec3 n(1.0f, 0.0f, 0.0f);
			AddQuadSmooth(vbo, vertexCount,
				p101, glm::vec2(0.0f, 0.0f), n,
				p100, glm::vec2(1.0f, 0.0f), n,
				p110, glm::vec2(1.0f, 1.0f), n,
				p111, glm::vec2(0.0f, 1.0f), n);
		}
	}
}

CStartLine::CStartLine()
{
	m_vao = 0;

	m_thickRingStart = 0;
	m_thickRingCount = 0;

	m_outerRingStart = 0;
	m_outerRingCount = 0;

	m_signFrontStart = 0;
	m_signFrontCount = 0;

	m_signSideStart = 0;
	m_signSideCount = 0;

	m_crossingStart = 0;
	m_crossingCount = 0;
}

CStartLine::~CStartLine()
{
	Release();
}

void CStartLine::Create(const string& thickRingTexture,
	const string& outerRingTexture,
	const string& signFrontTexture,
	const string& signSideTexture,
	const string& crossingTexture)
{
	Release();

	// Load textures used by the different start-line parts.
	LoadTexture(m_thickRingTexture, thickRingTexture);
	LoadTexture(m_outerRingTexture, outerRingTexture);
	LoadTexture(m_signFrontTexture, signFrontTexture);
	LoadTexture(m_signSideTexture, signSideTexture);
	LoadTexture(m_crossingTexture, crossingTexture);

	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	m_vbo.Create();
	m_vbo.Bind();

	int vertexCount = 0;

	// Thick inner ring
	m_thickRingStart = vertexCount;
	AddRing(m_vbo, vertexCount,
		12.0f,
		2.2f,
		64,
		24,
		8.0f,
		2.0f);
	m_thickRingCount = vertexCount - m_thickRingStart;

	// Thin outer ring
	m_outerRingStart = vertexCount;
	AddRing(m_vbo, vertexCount,
		15.0f,
		0.45f,
		64,
		16,
		70.0f,
		1.0f);
	m_outerRingCount = vertexCount - m_outerRingStart;

	// Sign box split into front/back and side-face draw ranges
	AddBoxSplit(m_vbo, vertexCount,
		11.0f,
		2.8f,
		0.5f,
		8.0f,
		-2.0f,
		m_signFrontStart, m_signFrontCount,
		m_signSideStart, m_signSideCount);

	// Checkered crossing strip placed slightly above the track to avoid z-fighting
	m_crossingStart = vertexCount;
	AddCrossingBox(m_vbo, vertexCount,
		14.9f,
		0.5f,
		2.8f,
		-1.4f,
		0.0f);
	m_crossingCount = vertexCount - m_crossingStart;

	m_vbo.UploadDataToGPU(GL_STATIC_DRAW);

	GLsizei stride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2);

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);

	// Texture coordinate attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));

	// Normal attribute
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));

	glBindVertexArray(0);
}

void CStartLine::Render()
{
	glBindVertexArray(m_vao);

	// Thick ring
	m_thickRingTexture.Bind();
	glDrawArrays(GL_TRIANGLES, m_thickRingStart, m_thickRingCount);

	// Outer ring
	m_outerRingTexture.Bind();
	glDrawArrays(GL_TRIANGLES, m_outerRingStart, m_outerRingCount);

	// Sign front and back
	m_signFrontTexture.Bind();
	glDrawArrays(GL_TRIANGLES, m_signFrontStart, m_signFrontCount);

	// Sign sides
	m_signSideTexture.Bind();
	glDrawArrays(GL_TRIANGLES, m_signSideStart, m_signSideCount);

	// Crossing strip
	m_crossingTexture.Bind();
	glDrawArrays(GL_TRIANGLES, m_crossingStart, m_crossingCount);

	glBindVertexArray(0);
}

void CStartLine::Release()
{
	// Release all textures used by the start line.
	m_thickRingTexture.Release();
	m_outerRingTexture.Release();
	m_signFrontTexture.Release();
	m_signSideTexture.Release();
	m_crossingTexture.Release();

	if (m_vao != 0)
	{
		glDeleteVertexArrays(1, &m_vao);
		m_vao = 0;
	}

	m_vbo.Release();

	// Reset draw ranges.
	m_thickRingStart = 0;
	m_thickRingCount = 0;
	m_outerRingStart = 0;
	m_outerRingCount = 0;
	m_signFrontStart = 0;
	m_signFrontCount = 0;
	m_signSideStart = 0;
	m_signSideCount = 0;
	m_crossingStart = 0;
	m_crossingCount = 0;
}