#include "CatmullRom.h"
#define _USE_MATH_DEFINES
#include <math.h>



CCatmullRom::CCatmullRom()
{
	m_vertexCount = 0;
}

CCatmullRom::~CCatmullRom()
{}

// Perform Catmull Rom spline interpolation between four points, interpolating the space between p1 and p2
glm::vec3 CCatmullRom::Interpolate(glm::vec3 &p0, glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3, float t)
{
    float t2 = t * t;
    float t3 = t2 * t;

	glm::vec3 a = p1;
	glm::vec3 b = 0.5f * (-p0 + p2);
	glm::vec3 c = 0.5f * (2.0f*p0 - 5.0f*p1 + 4.0f*p2 - p3);
	glm::vec3 d = 0.5f * (-p0 + 3.0f*p1 - 3.0f*p2 + p3);

	return a + b*t + c*t2 + d*t3;

}


void CCatmullRom::SetControlPoints()
{
	// Set control points (m_controlPoints) here, or load from disk

	// Optionally, set upvectors (m_controlUpVectors, one for each control point as well)
	m_controlPoints.push_back(glm::vec3(100, 5, 0));
	m_controlPoints.push_back(glm::vec3(71, 5, 71));
	m_controlPoints.push_back(glm::vec3(0, 5, 100));
	m_controlPoints.push_back(glm::vec3(-71, 5, 71));
	m_controlPoints.push_back(glm::vec3(-100, 5, 0));
	m_controlPoints.push_back(glm::vec3(-71, 5, -71));
	m_controlPoints.push_back(glm::vec3(0, 5, -100));
	m_controlPoints.push_back(glm::vec3(71, 5, -71));
}


// Determine lengths along the control points, which is the set of control points forming the closed curve
void CCatmullRom::ComputeLengthsAlongControlPoints()
{
	int M = (int) m_controlPoints.size();

	float fAccumulatedLength = 0.0f;
	m_distances.push_back(fAccumulatedLength);
	for (int i = 1; i < M; i++) {
		fAccumulatedLength += glm::distance(m_controlPoints[i-1], m_controlPoints[i]);
		m_distances.push_back(fAccumulatedLength);
	}

	// Get the distance from the last point to the first
	fAccumulatedLength += glm::distance(m_controlPoints[M-1], m_controlPoints[0]);
	m_distances.push_back(fAccumulatedLength);
}


// Return the point (and upvector, if control upvectors provided) based on a distance d along the control polygon
bool CCatmullRom::Sample(float d, glm::vec3 &p, glm::vec3 &up)
{
	if (d < 0)
		return false;

	int M = (int) m_controlPoints.size();
	if (M == 0)
		return false;


	float fTotalLength = m_distances[m_distances.size() - 1];

	// The the current length along the control polygon; handle the case where we've looped around the track
	float fLength = d - (int) (d / fTotalLength) * fTotalLength;

	// Find the current segment
	int j = -1;
	for (int i = 0; i < (int)m_distances.size()-1; i++) {
		if (fLength >= m_distances[i] && fLength < m_distances[i + 1]) {
			j = i; // found it!
			break;
		}
	}

	if (j == -1)
		return false;

	// Interpolate on current segment -- get t
	float fSegmentLength = m_distances[j + 1] - m_distances[j];
	float t = (fLength - m_distances[j]) / fSegmentLength;
	
	// Get the indices of the four points along the control polygon for the current segment
	int iPrev = ((j-1) + M) % M;
	int iCur = j;
	int iNext = (j + 1) % M;
	int iNextNext = (j + 2) % M;

	// Interpolate to get the point (and upvector)
	p = Interpolate(m_controlPoints[iPrev], m_controlPoints[iCur], m_controlPoints[iNext], m_controlPoints[iNextNext], t);
	if (m_controlUpVectors.size() == m_controlPoints.size())
		up = glm::normalize(Interpolate(m_controlUpVectors[iPrev], m_controlUpVectors[iCur], m_controlUpVectors[iNext], m_controlUpVectors[iNextNext], t));

	return true;
}



// Sample a set of control points using an open Catmull-Rom spline, to produce a set of iNumSamples that are (roughly) equally spaced
void CCatmullRom::UniformlySampleControlPoints(int numSamples)
{
	glm::vec3 p, up;

	// Compute the lengths of each segment along the control polygon, and the total length
	ComputeLengthsAlongControlPoints();
	float fTotalLength = m_distances[m_distances.size() - 1];

	// The spacing will be based on the control polygon
	float fSpacing = fTotalLength / numSamples;

	// Call PointAt to sample the spline, to generate the points
	for (int i = 0; i < numSamples; i++) {
		Sample(i * fSpacing, p, up);
		m_centrelinePoints.push_back(p);
		if (m_controlUpVectors.size() > 0)
			m_centrelineUpVectors.push_back(up);

	}


	// Repeat once more for truly equidistant points
	m_controlPoints = m_centrelinePoints;
	m_controlUpVectors = m_centrelineUpVectors;
	m_centrelinePoints.clear();
	m_centrelineUpVectors.clear();
	m_distances.clear();
	ComputeLengthsAlongControlPoints();
	fTotalLength = m_distances[m_distances.size() - 1];
	fSpacing = fTotalLength / numSamples;
	for (int i = 0; i < numSamples; i++) {
		Sample(i * fSpacing, p, up);
		m_centrelinePoints.push_back(p);
		if (m_controlUpVectors.size() > 0)
			m_centrelineUpVectors.push_back(up);
	}


}



void CCatmullRom::CreateCentreline()
{
	// Call Set Control Points
	SetControlPoints();

	// Call UniformlySampleControlPoints with the number of samples required
	UniformlySampleControlPoints(500);

	// Create a VAO called m_vaoCentreline and a VBO to get the points onto the graphics card
	glGenVertexArrays(1, &m_vaoCentreline);
	glBindVertexArray(m_vaoCentreline);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	std::vector<float> vertexData;


	glm::vec2 texCoord(0.0f, 0.0f);  // Simple texture coordinate for now
	glm::vec3 normal(0.0f, 1.0f, 0.0f);  // Up-facing normal for now

	for (unsigned int i = 0; i < m_centrelinePoints.size(); i++) {
		vertexData.push_back(m_centrelinePoints[i].x);
		vertexData.push_back(m_centrelinePoints[i].y);
		vertexData.push_back(m_centrelinePoints[i].z);

		vertexData.push_back(texCoord.x);
		vertexData.push_back(texCoord.y);
		
		vertexData.push_back(normal.x);
		vertexData.push_back(normal.y);
		vertexData.push_back(normal.z);
	}

	// Upload VBO
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);


	// Set vertex attribute locations
	GLsizei stride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2);

	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);

	// Texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));

	// Normal vectors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,
		(void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));

	// Unbind the VAO
	glBindVertexArray(0);

}


void CCatmullRom::CreateOffsetCurves()
{
	// Compute the offset curves, one left, and one right.  Store the points in m_leftOffsetPoints and m_rightOffsetPoints respectively
	float w = 20.0f;

	m_leftOffsetPoints.clear();
	m_rightOffsetPoints.clear();

	for (int i = 0; i < m_centrelinePoints.size(); ++i) {
		glm::vec3 p = m_centrelinePoints[i];

		glm::vec3 pNext = m_centrelinePoints[(i + 1) % m_centrelinePoints.size()]; // Wraparound

		// TNB regen
		glm::vec3 T = glm::normalize(pNext - p);
		glm::vec3 N = glm::normalize(glm::cross(T, glm::vec3(0, 1, 0)));
		glm::vec3 B = glm::normalize(glm::cross(N, T));

		glm::vec3 l = p - (w / 2) * N; // l offset
		glm::vec3 r = p + (w / 2) * N; // r offset

		m_leftOffsetPoints.push_back(l);
		m_rightOffsetPoints.push_back(r);
	}

	// Generate two VAOs called m_vaoLeftOffsetCurve and m_vaoRightOffsetCurve, each with a VBO, and get the offset curve points on the graphics card
	// Note it is possible to only use one VAO / VBO with all the points instead.

	// Left offset
	// Generate and bind vao
	glGenVertexArrays(1, &m_vaoLeftOffsetCurve);
	glBindVertexArray(m_vaoLeftOffsetCurve);

	// Generate and bind vbo and then upload vertices
	GLuint leftVBO;
	glGenBuffers(1, &leftVBO);
	glBindBuffer(GL_ARRAY_BUFFER, leftVBO);

	std::vector<float> leftdata;
	glm::vec2 tex(0.0f, 0.0f);
	glm::vec3 normal(0.0f, 1.0f, 0.0f);

	// Making the buffer content packed
	for (const glm::vec3& point : m_leftOffsetPoints) {
		// Pos
		leftdata.push_back(point.x);
		leftdata.push_back(point.y);
		leftdata.push_back(point.z);

		// Tex
		leftdata.push_back(tex.x);
		leftdata.push_back(tex.y);

		// Normal
		leftdata.push_back(normal.x);
		leftdata.push_back(normal.y);
		leftdata.push_back(normal.z);
	}

	glBufferData(GL_ARRAY_BUFFER, leftdata.size() * sizeof(float), leftdata.data(), GL_STATIC_DRAW);
	GLuint stride = 8 * sizeof(float);

	// Attrib ptrs
	// Pos
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0); //void* 0 no offset

	// Tex
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3*sizeof(float))); //12 byte offset from stride

	// Normal
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(6*sizeof(float))); //24 byte offset from stride


	// Right offset
	// Generate and bind vao
	glGenVertexArrays(1, &m_vaoRightOffsetCurve);
	glBindVertexArray(m_vaoRightOffsetCurve);

	// Generate and bind vbo and then upload vertices
	GLuint rightVBO;
	glGenBuffers(1, &rightVBO);
	glBindBuffer(GL_ARRAY_BUFFER, rightVBO);

	std::vector<float> rightdata;

	// Making the buffer content packed
	for (const glm::vec3& point : m_rightOffsetPoints) {
		// Pos
		rightdata.push_back(point.x);
		rightdata.push_back(point.y);
		rightdata.push_back(point.z);

		// Tex
		rightdata.push_back(tex.x);
		rightdata.push_back(tex.y);

		// Normal
		rightdata.push_back(normal.x);
		rightdata.push_back(normal.y);
		rightdata.push_back(normal.z);
	}

	glBufferData(GL_ARRAY_BUFFER, rightdata.size() * sizeof(float), rightdata.data(), GL_STATIC_DRAW);

	// Attrib ptrs
	// Pos
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0); //void* 0 no offset

	// Tex
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float))); //12 byte offset from stride

	// Normal
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float))); //24 byte offset from stride
	
	glBindVertexArray(0); // Unbind

}


void CCatmullRom::CreateTrack()
{
	
	// Generate a VAO called m_vaoTrack and a VBO to get the offset curve points and indices on the graphics card
	glGenVertexArrays(1, &m_vaoTrack);
	glBindVertexArray(m_vaoTrack);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	std::vector<float> trackdata;
	glm::vec2 tex(0.0f, 0.0f);
	glm::vec3 normal(0.0f, 1.0f, 0.0f);

	for (int i = 0; i < m_leftOffsetPoints.size(); ++i) {
		glm::vec3 leftPoint = m_leftOffsetPoints[i];

		trackdata.push_back(leftPoint.x);
		trackdata.push_back(leftPoint.y);
		trackdata.push_back(leftPoint.z);
		trackdata.push_back(0.0f);  //u
		trackdata.push_back(float(i) / m_leftOffsetPoints.size());  //v
		trackdata.push_back(normal.x);
		trackdata.push_back(normal.y);
		trackdata.push_back(normal.z);

		glm::vec3 rightPoint = m_rightOffsetPoints[i];

		trackdata.push_back(rightPoint.x);
		trackdata.push_back(rightPoint.y);
		trackdata.push_back(rightPoint.z);
		trackdata.push_back(1.0f);  //u
		trackdata.push_back(float(i) / m_rightOffsetPoints.size());  //v
		trackdata.push_back(rightPoint.x);
		trackdata.push_back(rightPoint.y);
		trackdata.push_back(rightPoint.z);
	}

	// First two points to close the loop
	glm::vec3 leftPoint = m_leftOffsetPoints[0];

	trackdata.push_back(leftPoint.x);
	trackdata.push_back(leftPoint.y);
	trackdata.push_back(leftPoint.z);
	trackdata.push_back(0.0f);  //u
	trackdata.push_back(0.0f);  //v
	trackdata.push_back(normal.x);
	trackdata.push_back(normal.y);
	trackdata.push_back(normal.z);

	glm::vec3 rightPoint = m_rightOffsetPoints[0];

	trackdata.push_back(rightPoint.x);
	trackdata.push_back(rightPoint.y);
	trackdata.push_back(rightPoint.z);
	trackdata.push_back(1.0f);  //u
	trackdata.push_back(0.0f);  //v
	trackdata.push_back(rightPoint.x);
	trackdata.push_back(rightPoint.y);
	trackdata.push_back(rightPoint.z);

	glBufferData(GL_ARRAY_BUFFER, trackdata.size() * sizeof(float), trackdata.data(), GL_STATIC_DRAW);
	GLuint stride = 8 * sizeof(float);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));

	m_vertexCount = (m_leftOffsetPoints.size() + 1) * 2;
	glBindVertexArray(0);

}


void CCatmullRom::RenderCentreline()
{
	// Bind the VAO m_vaoCentreline and render it
	glBindVertexArray(m_vaoCentreline);
	// Render the centreline as points
	glPointSize(5.0f);
	glDrawArrays(GL_POINTS, 0, m_centrelinePoints.size());
	// Render the centreline as a line loop
	glLineWidth(2.0f);
	glDrawArrays(GL_LINE_STRIP, 0, m_centrelinePoints.size());

	glBindVertexArray(0);

}

void CCatmullRom::RenderOffsetCurves()
{
	// Bind the VAO m_vaoLeftOffsetCurve and render it
	glBindVertexArray(m_vaoLeftOffsetCurve);
	glPointSize(3.0f);
	glDrawArrays(GL_POINTS, 0, m_leftOffsetPoints.size());

	glLineWidth(1.5f);
	glDrawArrays(GL_LINE_LOOP, 0, m_leftOffsetPoints.size());

	// Bind the VAO m_vaoRightOffsetCurve and render it
	glBindVertexArray(m_vaoRightOffsetCurve);
	glPointSize(3.0f);
	glDrawArrays(GL_POINTS, 0, m_rightOffsetPoints.size());

	glLineWidth(1.5f);
	glDrawArrays(GL_LINE_LOOP, 0, m_rightOffsetPoints.size());

	glBindVertexArray(0);
}


void CCatmullRom::RenderTrack()
{
	// Bind the VAO m_vaoTrack and render it
	glBindVertexArray(m_vaoTrack);

	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, m_vertexCount);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);

	glBindVertexArray(0);
}

int CCatmullRom::CurrentLap(float d)
{

	return (int)(d / m_distances.back());

}

void CCatmullRom::CreatePath(glm::vec3& p0, glm::vec3& p1, glm::vec3& p2, glm::vec3& p3)
{
	// Use VAO to store state associated with vertices
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);
	// Create a VBO
	CVertexBufferObject vbo;
	vbo.Create();
	vbo.Bind();
	glm::vec2 texCoord(0.0f, 0.0f);
	glm::vec3 normal(0.0f, 1.0f, 0.0f);
	for (unsigned int i = 0; i < 100; i++) {
		float t = (float)i / 100.0f;
		glm::vec3 v = Interpolate(p0, p1, p2, p3, t);
		vbo.AddData(&v, sizeof(glm::vec3));
		vbo.AddData(&texCoord, sizeof(glm::vec2));
		vbo.AddData(&normal, sizeof(glm::vec3));
	}
	// Upload the VBO to the GPU
	vbo.UploadDataToGPU(GL_STATIC_DRAW);
	// Set the vertex attribute locations
	GLsizei stride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2);
	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
	// Texture coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));
	// Normal vectors
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(glm::vec3)
		+ sizeof(glm::vec2)));
}

void CCatmullRom::RenderPath()
{
	glBindVertexArray(m_vao);
	glDrawArrays(GL_LINE_STRIP, 0, 100);
	glLineWidth(5.0f);
	glBindVertexArray(0);

}

glm::vec3 CCatmullRom::_dummy_vector(0.0f, 0.0f, 0.0f);