#include "BoostPad.h"

namespace
{
	// Adds a single interleaved vertex to the VBO.
	// Vertex layout: position, texture coordinate, normal, colour.
	void AddVertex(CVertexBufferObject& vbo, int& vertexCount,
		const glm::vec3& p,
		const glm::vec2& t,
		const glm::vec3& n,
		const glm::vec3& c)
	{
		vbo.AddData((void*)&p, sizeof(glm::vec3));
		vbo.AddData((void*)&t, sizeof(glm::vec2));
		vbo.AddData((void*)&n, sizeof(glm::vec3));
		vbo.AddData((void*)&c, sizeof(glm::vec3));
		vertexCount++;
	}

	// Adds one triangle and computes a face normal from the winding order.
	void AddTriangle(CVertexBufferObject& vbo, int& vertexCount,
		const glm::vec3& p0, const glm::vec2& t0,
		const glm::vec3& p1, const glm::vec2& t1,
		const glm::vec3& p2, const glm::vec2& t2,
		const glm::vec3& colour)
	{
		glm::vec3 n = glm::normalize(glm::cross(p1 - p0, p2 - p0));

		AddVertex(vbo, vertexCount, p0, t0, n, colour);
		AddVertex(vbo, vertexCount, p1, t1, n, colour);
		AddVertex(vbo, vertexCount, p2, t2, n, colour);
	}

	// Adds a quad as two triangles.
	void AddQuad(CVertexBufferObject& vbo, int& vertexCount,
		const glm::vec3& p0, const glm::vec3& p1,
		const glm::vec3& p2, const glm::vec3& p3,
		const glm::vec2& t0, const glm::vec2& t1,
		const glm::vec2& t2, const glm::vec2& t3,
		const glm::vec3& colour)
	{
		AddTriangle(vbo, vertexCount, p0, t0, p1, t1, p2, t2, colour);
		AddTriangle(vbo, vertexCount, p0, t0, p2, t2, p3, t3, colour);
	}

	// Loads the texture for the boost pad panel and sets sampler parameters
	// suitable for a tiled, mipmapped surface.
	void LoadTexture(CTexture& tex, const string& filename)
	{
		tex.Load(filename, true);
		tex.SetSamplerObjectParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		tex.SetSamplerObjectParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		tex.SetSamplerObjectParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
		tex.SetSamplerObjectParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	// Adds a box centred at the supplied position.
	// This is used to create the base panel of the boost pad.
	void AddBox(CVertexBufferObject& vbo, int& vertexCount,
		float width, float height, float depth,
		const glm::vec3& centre,
		float texRepeatX,
		float texRepeatZ,
		const glm::vec3& colour)
	{
		float x0 = centre.x - 0.5f * width;
		float x1 = centre.x + 0.5f * width;
		float y0 = centre.y - 0.5f * height;
		float y1 = centre.y + 0.5f * height;
		float z0 = centre.z - 0.5f * depth;
		float z1 = centre.z + 0.5f * depth;

		glm::vec3 p000(x0, y0, z0);
		glm::vec3 p001(x0, y0, z1);
		glm::vec3 p010(x0, y1, z0);
		glm::vec3 p011(x0, y1, z1);
		glm::vec3 p100(x1, y0, z0);
		glm::vec3 p101(x1, y0, z1);
		glm::vec3 p110(x1, y1, z0);
		glm::vec3 p111(x1, y1, z1);

		// Front face
		AddQuad(vbo, vertexCount,
			p001, p101, p111, p011,
			glm::vec2(0.0f, 0.0f), glm::vec2(texRepeatX, 0.0f),
			glm::vec2(texRepeatX, 1.0f), glm::vec2(0.0f, 1.0f),
			colour);

		// Back face
		AddQuad(vbo, vertexCount,
			p100, p000, p010, p110,
			glm::vec2(0.0f, 0.0f), glm::vec2(texRepeatX, 0.0f),
			glm::vec2(texRepeatX, 1.0f), glm::vec2(0.0f, 1.0f),
			colour);

		// Left face
		AddQuad(vbo, vertexCount,
			p000, p001, p011, p010,
			glm::vec2(0.0f, 0.0f), glm::vec2(texRepeatZ, 0.0f),
			glm::vec2(texRepeatZ, 1.0f), glm::vec2(0.0f, 1.0f),
			colour);

		// Right face
		AddQuad(vbo, vertexCount,
			p101, p100, p110, p111,
			glm::vec2(0.0f, 0.0f), glm::vec2(texRepeatZ, 0.0f),
			glm::vec2(texRepeatZ, 1.0f), glm::vec2(0.0f, 1.0f),
			colour);

		// Top face
		AddQuad(vbo, vertexCount,
			p011, p111, p110, p010,
			glm::vec2(0.0f, 0.0f), glm::vec2(texRepeatX, 0.0f),
			glm::vec2(texRepeatX, texRepeatZ), glm::vec2(0.0f, texRepeatZ),
			colour);

		// Bottom face
		AddQuad(vbo, vertexCount,
			p000, p100, p101, p001,
			glm::vec2(0.0f, 0.0f), glm::vec2(texRepeatX, 0.0f),
			glm::vec2(texRepeatX, texRepeatZ), glm::vec2(0.0f, texRepeatZ),
			colour);
	}

	// Adds a prism shaped like a forward arrow.
	// This forms the raised green arrows placed on top of the boost pad.
	void AddArrowPrism(CVertexBufferObject& vbo, int& vertexCount,
		const glm::vec3& centre,
		float width,
		float length,
		float height,
		const glm::vec3& colour)
	{
		float xBack = centre.x - 0.5f * length;
		float xFront = centre.x + 0.5f * length;

		// Defines the shoulder and notch positions to shape the arrow head and inner cut.
		float xShoulder = centre.x - 0.05f * length;
		float xNotch = centre.x - 0.2f * length;

		float zOuter = 0.5f * width;
		float zInner = 0.28f * width;

		float y0 = centre.y - 0.5f * height;
		float y1 = centre.y + 0.5f * height;

		// Top polygon vertices
		glm::vec3 A(xBack, y1, -zOuter);
		glm::vec3 B(xShoulder, y1, -zInner);
		glm::vec3 C(xFront, y1, 0.0f);
		glm::vec3 D(xShoulder, y1, zInner);
		glm::vec3 E(xBack, y1, zOuter);
		glm::vec3 F(xNotch, y1, 0.0f);

		// Matching vertices for the bottom face
		glm::vec3 A0 = A; A0.y = y0;
		glm::vec3 B0 = B; B0.y = y0;
		glm::vec3 C0 = C; C0.y = y0;
		glm::vec3 D0 = D; D0.y = y0;
		glm::vec3 E0 = E; E0.y = y0;
		glm::vec3 F0 = F; F0.y = y0;

		// Top face
		AddTriangle(vbo, vertexCount, A, glm::vec2(0, 0), B, glm::vec2(0.4f, 0), F, glm::vec2(0.2f, 1), colour);
		AddTriangle(vbo, vertexCount, B, glm::vec2(0.4f, 0), C, glm::vec2(1, 0.5f), F, glm::vec2(0.2f, 1), colour);
		AddTriangle(vbo, vertexCount, C, glm::vec2(1, 0.5f), D, glm::vec2(0.4f, 1), F, glm::vec2(0.2f, 1), colour);
		AddTriangle(vbo, vertexCount, D, glm::vec2(0.4f, 1), E, glm::vec2(0, 1), F, glm::vec2(0.2f, 1), colour);

		// Bottom face
		AddTriangle(vbo, vertexCount, F0, glm::vec2(0.2f, 1), B0, glm::vec2(0.4f, 0), A0, glm::vec2(0, 0), colour);
		AddTriangle(vbo, vertexCount, F0, glm::vec2(0.2f, 1), C0, glm::vec2(1, 0.5f), B0, glm::vec2(0.4f, 0), colour);
		AddTriangle(vbo, vertexCount, F0, glm::vec2(0.2f, 1), D0, glm::vec2(0.4f, 1), C0, glm::vec2(1, 0.5f), colour);
		AddTriangle(vbo, vertexCount, F0, glm::vec2(0.2f, 1), E0, glm::vec2(0, 1), D0, glm::vec2(0.4f, 1), colour);

		// Side faces around the prism
		AddQuad(vbo, vertexCount, A0, A, B, B0,
			glm::vec2(0, 0), glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(1, 0), colour);

		AddQuad(vbo, vertexCount, B0, B, C, C0,
			glm::vec2(0, 0), glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(1, 0), colour);

		AddQuad(vbo, vertexCount, C0, C, D, D0,
			glm::vec2(0, 0), glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(1, 0), colour);

		AddQuad(vbo, vertexCount, D0, D, E, E0,
			glm::vec2(0, 0), glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(1, 0), colour);

		AddQuad(vbo, vertexCount, E0, E, F, F0,
			glm::vec2(0, 0), glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(1, 0), colour);

		AddQuad(vbo, vertexCount, F0, F, A, A0,
			glm::vec2(0, 0), glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(1, 0), colour);
	}
}

CBoostPad::CBoostPad()
{
	m_vao = 0;
	m_panelStart = 0;
	m_panelCount = 0;
	m_arrowStart = 0;
	m_arrowCount = 0;
}

CBoostPad::~CBoostPad()
{
	Release();
}

void CBoostPad::Create(const string& panelTexture)
{
	Release();

	// Load the texture used by the base panel.
	LoadTexture(m_panelTexture, panelTexture);

	// Create and bind the VAO/VBO used to store the boost pad geometry.
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	m_vbo.Create();
	m_vbo.Bind();

	int vertexCount = 0;

	glm::vec3 white(1.0f, 1.0f, 1.0f);
	glm::vec3 arrowGreen(0.3f, 1.0f, 0.3f);

	// Build the textured base panel.
	m_panelStart = vertexCount;
	AddBox(m_vbo, vertexCount,
		12.0f,
		0.25f,
		4.5f,
		glm::vec3(0, 0, 0),
		1.0f,
		1.0f,
		white);
	m_panelCount = vertexCount - m_panelStart;

	// Build the raised arrow prisms positioned along the top of the pad.
	m_arrowStart = vertexCount;

	float arrowY = 0.22f;
	float arrowW = 4.f;
	float arrowL = 1.6f;
	float arrowH = 0.18f;

	AddArrowPrism(m_vbo, vertexCount, glm::vec3(-4.2f, arrowY, 0.0f), arrowW, arrowL, arrowH, arrowGreen);
	AddArrowPrism(m_vbo, vertexCount, glm::vec3(-2.1f, arrowY, 0.0f), arrowW, arrowL, arrowH, arrowGreen);
	AddArrowPrism(m_vbo, vertexCount, glm::vec3(0.0f, arrowY, 0.0f), arrowW, arrowL, arrowH, arrowGreen);
	AddArrowPrism(m_vbo, vertexCount, glm::vec3(2.1f, arrowY, 0.0f), arrowW, arrowL, arrowH, arrowGreen);
	AddArrowPrism(m_vbo, vertexCount, glm::vec3(4.2f, arrowY, 0.0f), arrowW, arrowL, arrowH, arrowGreen);

	m_arrowCount = vertexCount - m_arrowStart;

	// Upload interleaved vertex data to the GPU.
	m_vbo.UploadDataToGPU(GL_STATIC_DRAW);

	GLsizei stride = sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(glm::vec3) + sizeof(glm::vec3);

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);

	// Texture coordinate attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));

	// Normal attribute
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));

	// Colour attribute
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(glm::vec3)));

	glBindVertexArray(0);
}

void CBoostPad::RenderPanel()
{
	glBindVertexArray(m_vao);
	m_panelTexture.Bind();
	glDrawArrays(GL_TRIANGLES, m_panelStart, m_panelCount);
	glBindVertexArray(0);
}

void CBoostPad::RenderArrows()
{
	glBindVertexArray(m_vao);
	glDrawArrays(GL_TRIANGLES, m_arrowStart, m_arrowCount);
	glBindVertexArray(0);
}

void CBoostPad::Release()
{
	// Release the panel texture first.
	m_panelTexture.Release();

	// Delete the VAO if it has been created.
	if (m_vao != 0)
	{
		glDeleteVertexArrays(1, &m_vao);
		m_vao = 0;
	}

	// Release the vertex buffer object.
	m_vbo.Release();

	// Reset draw ranges.
	m_panelStart = 0;
	m_panelCount = 0;
	m_arrowStart = 0;
	m_arrowCount = 0;
}