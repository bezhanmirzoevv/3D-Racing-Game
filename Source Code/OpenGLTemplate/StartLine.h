#pragma once

#include "Common.h"
#include "Texture.h"
#include "VertexBufferObject.h"

// CStartLine
// ----------
// Custom start-line arch built entirely from generated triangle geometry.
// The object contains:
// 1. A thick inner ring
// 2. A thin outer ring
// 3. A rectangular sign box
// 4. A checkered crossing box placed on the track
class CStartLine
{
public:
	CStartLine();
	~CStartLine();

	// Builds the full start-line geometry and loads the textures used by each part.
	void Create(const string& thickRingTexture,
		const string& outerRingTexture,
		const string& signFrontTexture,
		const string& signSideTexture,
		const string& crossingTexture);

	// Renders all parts of the start line from the shared VBO.
	void Render();

	// Releases textures and GPU resources associated with the start line.
	void Release();

private:
	GLuint m_vao;
	CVertexBufferObject m_vbo;

	// Textures used by the different parts of the start line.
	CTexture m_thickRingTexture;
	CTexture m_outerRingTexture;
	CTexture m_signFrontTexture;
	CTexture m_signSideTexture;
	CTexture m_crossingTexture;

	// Draw ranges inside the shared VBO.
	int m_thickRingStart;
	int m_thickRingCount;

	int m_outerRingStart;
	int m_outerRingCount;

	int m_signFrontStart;
	int m_signFrontCount;

	int m_signSideStart;
	int m_signSideCount;

	int m_crossingStart;
	int m_crossingCount;
};