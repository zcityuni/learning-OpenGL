#pragma once

#include "Texture.h"
#include "VertexBufferObject.h"

// Class for generating a unit sphere
class CSphere
{
public:
	void Create(string a_sDirectory, string a_sFront, int slicesIn, int stacksIn);
	void Render();
	void Release();
private:
	UINT m_uiVAO;
	CVertexBufferObject m_vboData;
	CTexture m_tTexture;
	string m_sDirectory;
	string m_sFilename;
	int m_iNumTriangles;
};