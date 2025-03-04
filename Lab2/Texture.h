#pragma once

#include "include/gl/glew.h"
#include <gl/gl.h>


// Class that provides a texture for texture mapping in OpenGL
class CTexture
{
public:
	void CreateFromData(BYTE* bData, int iWidth, int iHeight, int iBPP, GLenum format, bool bGenerateMipMaps = false);
	bool Load(string sPath, bool bGenerateMipMaps = true);
	void Bind(int iTextureUnit = 0);

	void SetSamplerParameter(GLenum parameter, GLenum value);

	int GetWidth();
	int GetHeight();
	int GetBPP();

	void Release();

	CTexture();
private:
	int m_iWidth, m_iHeight, m_iBPP; // Texture width, height, and bytes per pixel
	UINT m_uiTexture; // Texture name
	UINT m_uiSampler; // Sampler name
	bool m_bMipMapsGenerated;

	string m_sPath;
};

