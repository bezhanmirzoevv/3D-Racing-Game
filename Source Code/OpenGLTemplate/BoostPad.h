#pragma once

#include "Common.h"
#include "Texture.h"
#include "VertexBufferObject.h"

// CBoostPad
// ---------
// Renders a gameplay boost pad built entirely from generated triangle geometry.
// The object is split into two renderable parts:
// 1. A textured base panel.
// 2. A set of raised arrow prisms rendered separately so they can use a different material/shader setup.
class CBoostPad
{
public:
	CBoostPad();
	~CBoostPad();

	// Builds the boost pad geometry and loads the texture used by the base panel.
	void Create(const string& panelTexture);

	// Renders the textured base panel.
	void RenderPanel();

	// Renders the raised arrow geometry on top of the panel.
	void RenderArrows();

	// Releases GPU resources associated with the boost pad.
	void Release();

private:
	GLuint m_vao;
	CVertexBufferObject m_vbo;

	// Texture applied to the boost pad base panel.
	CTexture m_panelTexture;

	// Vertex range for the textured base panel.
	int m_panelStart;
	int m_panelCount;

	// Vertex range for the raised arrow geometry.
	int m_arrowStart;
	int m_arrowCount;
};