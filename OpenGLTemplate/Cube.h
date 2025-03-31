#pragma once

#include "Common.h"
#include "Texture.h"

class CCube{
public:
	CCube();
	~CCube();

	void Create(const std::string& texturePath);
	void Render();
	void Release();

private:
	GLuint m_vao, m_vbo;
	int m_numVertices;
	CTexture m_texture;
}