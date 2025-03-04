#pragma once

#include "include/gl/glew.h"
#include <gl/gl.h>

// This class provides a wrapper around an OpenGL Vertex Buffer Object
class CVertexBufferObject
{
public:
	CVertexBufferObject();

	void Create(int iSize = 0);						// Creates a VBO
	void Bind(int iBufferType = GL_ARRAY_BUFFER);	// Binds the VBO
	void Release();									// Releases the VBO

	void AddData(void* ptrData, UINT uiDataSize);	// Add data to the VBO
	void UploadDataToGPU(int iUsageHint);			// Upload the VBO to the GPU

	void *MapBufferToMemory(int iUsageHint);
	void *MapSubBufferToMemory(int iUsageHint, UINT uiOffset, UINT uiLength);
	void UnmapBuffer();

	void* GetDataPointer();
	UINT GetBuffer();
	
private:
	UINT m_uiBuffer;
	int m_iSize;
	int m_iBufferType;
	vector<BYTE> m_data;

	bool m_bDataUploaded;
};