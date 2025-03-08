/* 
Basic OpenGL sample program for INM376 / IN3005
City University London, School of Informatics
Source code drawn from a number of sources and examples, including contributions from
 - Drs. Greg Slabaugh, Chris Child, Dean Mohamedally
 - Ben Humphrey (gametutorials.com), Michal Bubner (mbsoftworks.sk), Christophe Riccio (glm.g-truc.net)
 - Christy Quinn, Sam Kellett
 and others

 For educational use by Department of Computer Science, City, University of London UK.

 Dr. Greg Slabaugh (gregory.slabaugh.1@city.ac.uk) 
*/

// Setup includes
#include "game.h"
#include "GameWindow.h"

// Game includes
#include "Shaders.h"
#include "MatrixStack.h"
#include "Sphere.h"
#include "HighResolutionTimer.h"

Game::Game() : 
	m_pShaderProgram(NULL), m_uiVAO(0), 

	//setup objects
	rGameWindow(GameWindow::GetInstance())
{
	m_rotY = 0.0f;
}

Game::~Game() 
{ 
	//game objects
	delete m_pShaderProgram;
	delete m_pSphere;
	delete m_pTimer;
}



void Game::Init() 
{
	m_pShaderProgram = new CShaderProgram;
	m_pSphere = new CSphere;
	m_pTimer = new CHighResolutionTimer;

	// This sets the position, viewpoint, and up vector of the synthetic camera
	glm::vec3 vEye(0, 0, 20);
	glm::vec3 vView(0, 0, 0);
	glm::vec3 vUp(0, 1, 0);
	glm::mat4 mViewMatrix = glm::lookAt(vEye, vView, vUp);

	// This creates a view frustum
	glm::mat4 mProjectionMatrix = glm::perspective(45.0f, 1.333f, 1.0f, 150.0f);

	// This sets the background colour
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0);


	// Load and compile shaders 
	CShader shVertex, shFragment;	
	shVertex.LoadShader("data\\shaders\\shader.vert", GL_VERTEX_SHADER);
	shFragment.LoadShader("data\\shaders\\shader.frag", GL_FRAGMENT_SHADER);

	// Create shader program and add shaders
	m_pShaderProgram->CreateProgram();
	m_pShaderProgram->AddShaderToProgram(&shVertex);
	m_pShaderProgram->AddShaderToProgram(&shFragment);

	// Link and use the program
	m_pShaderProgram->LinkProgram();
	m_pShaderProgram->UseProgram();

	// Set the modeling, viewing, and projection matrices in the shader
	m_pShaderProgram->SetUniform("viewMatrix", mViewMatrix);
	m_pShaderProgram->SetUniform("projectionMatrix", mProjectionMatrix);
	m_pShaderProgram->SetUniform("vlightDirection", glm::normalize(glm::vec3(0.5f, 0.5f, 0.5f)));
	m_pShaderProgram->SetUniform("sampler0", 0);

	m_pSphere->Create("data\\textures\\", "dirtpile01.jpg", 25, 25);  // Texture downloaded from http://www.psionicgames.com/?page_id=26 on 24 Jan 2013

	
	m_pTimer->Start();
}




void Game::Render() 
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	m_pShaderProgram->SetUniform("t", (float) m_pTimer->Elapsed());

	// Bind the VAO
	glBindVertexArray(m_uiVAO);
	
	// Set the modeling matrix
	// glm::mat4 mModelMatrix = glm::mat4(1);
	glutil::MatrixStack modelMatrixStack;
	float s = sin(m_rotY / 100.0f);

	// Scale all both sets of spheres
	// modelMatrixStack.Scale(s);
	// First set of roating spheres
	modelMatrixStack.Push(); {
		modelMatrixStack.Translate(glm::vec3(-5, 0, 0));

		// Scale only one set of spheres
		// modelMatrixStack.Scale(s);

		modelMatrixStack.Rotate(glm::vec3(0, -1, 0), m_rotY);
		modelMatrixStack.Push(); {
			// Scale only a single sphere
			modelMatrixStack.Scale(s);
			m_pShaderProgram->SetUniform("modelMatrix", modelMatrixStack.Top());
			m_pSphere->Render();
		} modelMatrixStack.Pop();

		modelMatrixStack.Push(); {
			modelMatrixStack.Translate(glm::vec3(-1, 1, 0));
			m_pShaderProgram->SetUniform("modelMatrix", modelMatrixStack.Top());
			m_pSphere->Render();
		} modelMatrixStack.Pop();

		modelMatrixStack.Push(); {
			modelMatrixStack.Translate(glm::vec3(1, 1, 0));
			m_pShaderProgram->SetUniform("modelMatrix", modelMatrixStack.Top());
			m_pSphere->Render();
		} modelMatrixStack.Pop();
	} modelMatrixStack.Pop();

	// Second set of rotating spheres
	modelMatrixStack.Push(); {
		modelMatrixStack.Translate(glm::vec3(5, 0, 0));

		modelMatrixStack.Rotate(glm::vec3(0, -1, 0), m_rotY);
		modelMatrixStack.Push(); {
			m_pShaderProgram->SetUniform("modelMatrix", modelMatrixStack.Top());
			m_pSphere->Render();
		} modelMatrixStack.Pop();

		modelMatrixStack.Push(); {
			modelMatrixStack.Translate(glm::vec3(-1, 1, 0));
			m_pShaderProgram->SetUniform("modelMatrix", modelMatrixStack.Top());
			m_pSphere->Render();
		} modelMatrixStack.Pop();

		modelMatrixStack.Push(); {
			modelMatrixStack.Translate(glm::vec3(1, 1, 0));
			m_pShaderProgram->SetUniform("modelMatrix", modelMatrixStack.Top());
			m_pSphere->Render();
		} modelMatrixStack.Pop();
	} modelMatrixStack.Pop();
	
	
	// Swap buffers to show the rendered image
	SwapBuffers(rGameWindow.Hdc());		
}

 

// Update method runs repeatedly with the Render method
void Game::Update()
{
	m_rotY += 100.0f * m_dt;
}
		

// The game loop runs repeatedly until game over
void Game::GameLoop()
{
	// Variable timer
	m_pGameTimer->Start();
	Update();
	Render();
	m_dt = (float) m_pGameTimer->Elapsed();
}


WPARAM Game::Exec() 
{
	m_pGameTimer = new CHighResolutionTimer;
	rGameWindow.Init(hHinstance);

	if(!rGameWindow.Hdc()) {
		return 1;
	}

	Init();

	MSG msg;

	while(1) {													
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { 
			if(msg.message == WM_QUIT) {
				break;
			}

			TranslateMessage(&msg);	
			DispatchMessage(&msg);
		} else if (bAppActive) {
				GameLoop();
		} 
		else Sleep(200); // Do not consume processor power if application isn't active
	}

	rGameWindow.Deinit();

	return(msg.wParam);
}

LRESULT Game::ProcessEvents(HWND window,UINT message, WPARAM w_param, LPARAM l_param) 
{
	LRESULT result = 0;

	switch (message) {


	case WM_ACTIVATE:
	{
		switch(LOWORD(w_param))
		{
			case WA_ACTIVE:
			case WA_CLICKACTIVE:
				bAppActive = true;
				break;
			case WA_INACTIVE:
				bAppActive = false;
				break;
		}
		break;
		}

	case WM_SIZE:
			RECT dimensions;
			GetClientRect(window, &dimensions);
			rGameWindow.SetDimensions(dimensions);
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

	return game.Exec();
}
