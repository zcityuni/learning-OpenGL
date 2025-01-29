/* 
	Lab 1 
*/


#include "game.h"
#include "GameWindow.h"
#include "Shaders.h"
#include "HighResolutionTimer.h"


// Constructor
Game::Game()
{

	m_pShaderProgram = NULL;

}

// Destructor
Game::~Game() 
{ 
	delete m_pShaderProgram;
	delete m_pTimer;

}

// Initialisation:  This method only runs once at startup
void Game::Initialise() 
{
	m_pShaderProgram = new CShaderProgram;
	m_pTimer = new CHighResolutionTimer;
	m_pModelMatrix = new glm::mat4(1);
	m_pViewMatrix = new glm::mat4(1);
	m_pProjectionMatrix = new glm::mat4(1);


	GLuint uiVBO[2];	// Two vertex buffer objects

	float fTrianglePositions[9];	// An array to store triangle vertex positions
	float fTriangleColor[9];		// An array to store triangle vertex colours

	// This sets the position, viewpoint, and up vector of the camera
	glm::vec3 vEye(10, 10, 10);
	glm::vec3 vView(0, 0, 0);
	glm::vec3 vUp(0, 1, 0);
	*m_pViewMatrix = glm::lookAt(vEye, vView, vUp);

	// This creates a view frustum
	*m_pProjectionMatrix = glm::perspective(45.0f, 1.333f, 1.0f, 150.0f);

	// This sets the background colour
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Setup triangle vertex positions
	fTrianglePositions[0] = -1.0f; fTrianglePositions[1] = 0.0f; fTrianglePositions[2] = 0.0f;
	fTrianglePositions[3] = 1.0f; fTrianglePositions[4] = 0.0f; fTrianglePositions[5] = 0.0f;
	fTrianglePositions[6] = 0.0f; fTrianglePositions[7] = 1.0f; fTrianglePositions[8] = 0.0f;

	// Setup triangle vertex colours
	fTriangleColor[0] = 0.0f; fTriangleColor[1] = 1.0f; fTriangleColor[2] = 0.0f;
	fTriangleColor[3] = 0.0f; fTriangleColor[4] = 0.0f; fTriangleColor[5] = 1.0f;
	fTriangleColor[6] = 1.0f; fTriangleColor[7] = 0.0f; fTriangleColor[8] = 0.0f;

	// Generate a VAO and two VBOs
	glGenVertexArrays(1, &m_uiVAO); 
	glGenBuffers(2, &uiVBO[0]); 

	// Create the VAO for the triangle
	glBindVertexArray(m_uiVAO);

	// Create a VBO for the triangle vertices
	glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
	glBufferData(GL_ARRAY_BUFFER, 9*sizeof(float), fTrianglePositions, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Create a VBO for the triangle colours
	glBindBuffer(GL_ARRAY_BUFFER, uiVBO[1]);
	glBufferData(GL_ARRAY_BUFFER, 9*sizeof(float), fTriangleColor, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Load and compile shaders 
	CShader shVertex, shFragment;	
	shVertex.LoadShader("resources\\shaders\\shader.vert", GL_VERTEX_SHADER);
	shFragment.LoadShader("resources\\shaders\\shader.frag", GL_FRAGMENT_SHADER);

	// Create shader program and add shaders
	m_pShaderProgram->CreateProgram();
	m_pShaderProgram->AddShaderToProgram(&shVertex);
	m_pShaderProgram->AddShaderToProgram(&shFragment);

	// Link and use the program
	m_pShaderProgram->LinkProgram();
	m_pShaderProgram->UseProgram();

	// Set the modeling, viewing, and projection matrices in the shader
	m_pShaderProgram->SetUniform("viewMatrix", m_pViewMatrix);
	m_pShaderProgram->SetUniform("projectionMatrix", m_pProjectionMatrix);

	m_pTimer->Start();
	glEnable(GL_DEPTH_TEST);
}

// Render method runs repeatedly in a loop
void Game::Render() 
{

	// Clear the buffer for rendering a new frame
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Bind the VAO
	glBindVertexArray(m_uiVAO);
	
	// Set the modeling matrix
	*m_pModelMatrix = glm::translate(glm::mat4(1), glm::vec3(0, 0, 0));
	m_pShaderProgram->SetUniform("modelMatrix", m_pModelMatrix);
	m_pShaderProgram->SetUniform("t", (float) m_pTimer->Elapsed());

	// Render the triangle consisting of 3 vertices
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// Swap buffers to show the rendered image
	SwapBuffers(gameWindow.Hdc());		
}


// The game loop runs repeatedly until game over.  Later we'll add an update method here and use a timer as well to avoid framerate dependent motion.
void Game::GameLoop()
{
	Render();
}


WPARAM Game::Execute() 
{
	gameWindow.Init(hHinstance);

	if(!gameWindow.Hdc()) {
		return 1;
	}
	
	Initialise();
	
	MSG msg;

	while(1) {													
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { 
			if(msg.message == WM_QUIT) {
				break;
			}

			TranslateMessage(&msg);	
			DispatchMessage(&msg);
		} else {
			GameLoop();
		} 
	}

	gameWindow.Deinit();

	return(msg.wParam);
}

LRESULT Game::ProcessEvents(HWND window,UINT message, WPARAM w_param, LPARAM l_param) 
{
	LRESULT result = 0;

	switch (message) {
		
	case WM_SIZE:
			RECT dimensions;
			GetClientRect(window, &dimensions);
			gameWindow.SetDimensions(dimensions);
		break;

	case WM_PAINT:
		PAINTSTRUCT ps;
		BeginPaint(window, &ps);
		EndPaint(window, &ps);
		break;

	case WM_KEYDOWN:
		switch(w_param) {
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		result = DefWindowProc(window, message, w_param, l_param);
		break;
	}

	return result;
}

Game& Game::GetInstance() 
{
	static Game instance;

	return instance;
}

void Game::SetHinstance(HINSTANCE hinstance) 
{
	hHinstance = hinstance;
}

LRESULT CALLBACK WinProc(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
	return Game::GetInstance().ProcessEvents(window, message, w_param, l_param);
}

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE, PSTR, int) 
{
	Game &game = Game::GetInstance();
	game.SetHinstance(hinstance);

	return game.Execute();
}
