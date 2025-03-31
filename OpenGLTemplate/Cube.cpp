#include "Cube.h"

CCube::CCube() : m_vao(0), m_vbo(0), m_numVertices(0) {}

CCube::~CCube() {
	Release();
}

void CCube::Create(const std::string& texturePath) {
	m_texture.Load(texturePath);
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Put into a different class soon
	float cubeVertices[] = {
		// Front face (z = -1) ccw
		//  Position              Normal             Tex
		-1.0f, -1.0f, -1.0f,      0.0f,  0.0f, -1.0f,  0.0f, 0.0f, // bottom left
		 1.0f,  1.0f, -1.0f,      0.0f,  0.0f, -1.0f,  1.0f, 1.0f, // top right
		 1.0f, -1.0f, -1.0f,      0.0f,  0.0f, -1.0f,  1.0f, 0.0f, // bottom right
		-1.0f, -1.0f, -1.0f,      0.0f,  0.0f, -1.0f,  0.0f, 0.0f, // bottom left
		-1.0f,  1.0f, -1.0f,      0.0f,  0.0f, -1.0f,  0.0f, 1.0f, // top left
		 1.0f,  1.0f, -1.0f,      0.0f,  0.0f, -1.0f,  1.0f, 1.0f, // top right

		 // Back face (z = +1) 
		-1.0f, -1.0f,  1.0f,      0.0f,  0.0f,  1.0f,  0.0f, 0.0f, // bottom left
		 1.0f, -1.0f,  1.0f,      0.0f,  0.0f,  1.0f,  1.0f, 0.0f, // bottom right
		-1.0f,  1.0f,  1.0f,      0.0f,  0.0f,  1.0f,  0.0f, 1.0f, // top left
		-1.0f,  1.0f,  1.0f,      0.0f,  0.0f,  1.0f,  0.0f, 1.0f, // top left
		 1.0f, -1.0f,  1.0f,      0.0f,  0.0f,  1.0f,  1.0f, 0.0f, // bottom right
		 1.0f,  1.0f,  1.0f,      0.0f,  0.0f,  1.0f,  1.0f, 1.0f, // top right

		 // Right face (x = +1) 
		 1.0f, -1.0f, -1.0f,      1.0f,  0.0f,  0.0f,  0.0f, 0.0f, // bottom front
		 1.0f,  1.0f,  1.0f,      1.0f,  0.0f,  0.0f,  1.0f, 1.0f, // top back
		 1.0f, -1.0f,  1.0f,      1.0f,  0.0f,  0.0f,  1.0f, 0.0f, // bottom back
		 1.0f, -1.0f, -1.0f,      1.0f,  0.0f,  0.0f,  0.0f, 0.0f, // bottom front
		 1.0f,  1.0f, -1.0f,      1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // top front
		 1.0f,  1.0f,  1.0f,      1.0f,  0.0f,  0.0f,  1.0f, 1.0f, // top back

		 // Left face (x = -1) 
		-1.0f, -1.0f,  1.0f,     -1.0f,  0.0f,  0.0f,  0.0f, 0.0f, // bottom back
		-1.0f,  1.0f, -1.0f,     -1.0f,  0.0f,  0.0f,  1.0f, 1.0f, // top front
		-1.0f, -1.0f, -1.0f,     -1.0f,  0.0f,  0.0f,  1.0f, 0.0f, // bottom front
		-1.0f, -1.0f,  1.0f,     -1.0f,  0.0f,  0.0f,  0.0f, 0.0f, // bottom back
		-1.0f,  1.0f,  1.0f,     -1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // top back
		-1.0f,  1.0f, -1.0f,     -1.0f,  0.0f,  0.0f,  1.0f, 1.0f, // top front

		// Top face (y = +1)
		-1.0f,  1.0f, -1.0f,      0.0f,  1.0f,  0.0f,  0.0f, 0.0f, // front left
		 1.0f,  1.0f,  1.0f,      0.0f,  1.0f,  0.0f,  1.0f, 1.0f, // back right
		 1.0f,  1.0f, -1.0f,      0.0f,  1.0f,  0.0f,  1.0f, 0.0f, // front right
		-1.0f,  1.0f, -1.0f,      0.0f,  1.0f,  0.0f,  0.0f, 0.0f, // front left
		-1.0f,  1.0f,  1.0f,      0.0f,  1.0f,  0.0f,  0.0f, 1.0f, // back left
		 1.0f,  1.0f,  1.0f,      0.0f,  1.0f,  0.0f,  1.0f, 1.0f, // back right

		 // Bottom face (y = -1)
		 -1.0f, -1.0f, -1.0f,      0.0f, -1.0f,  0.0f,  0.0f, 1.0f, // front left
		  1.0f, -1.0f, -1.0f,      0.0f, -1.0f,  0.0f,  1.0f, 1.0f, // front right
		 -1.0f, -1.0f,  1.0f,      0.0f, -1.0f,  0.0f,  0.0f, 0.0f, // back left
		  1.0f, -1.0f, -1.0f,      0.0f, -1.0f,  0.0f,  1.0f, 1.0f, // front right
		  1.0f, -1.0f,  1.0f,      0.0f, -1.0f,  0.0f,  1.0f, 0.0f, // back right
		 -1.0f, -1.0f,  1.0f,      0.0f, -1.0f,  0.0f,  0.0f, 0.0f  // back left
	};

	m_numVertices = sizeof(cubeVertices) / (8 * sizeof(float));

	// Generate and bind vao for cube
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);
	// Generate and bind vbo and then upload vertices
	glGenBuffers(1, &m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vao);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	GLsizei stride = 8 * sizeof(float);
	// Attrib ptrs
	// Pos
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);

	// Normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float))); //12 byte offset from stride
	glEnableVertexAttribArray(1);

	// Tex
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float))); //24 byte offset from stride
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Unbind
}

void CCube::Render() {}

void CCube::Release() {}