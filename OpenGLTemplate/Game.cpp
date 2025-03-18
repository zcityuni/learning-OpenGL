/* 
OpenGL Template for INM376 / IN3005
City University London, School of Mathematics, Computer Science and Engineering
Source code drawn from a number of sources and examples, including contributions from
 - Ben Humphrey (gametutorials.com), Michal Bubner (mbsoftworks.sk), Christophe Riccio (glm.g-truc.net)
 - Christy Quinn, Sam Kellett and others

 For educational use by Department of Computer Science, City University London UK.

 This template contains a skybox, simple terrain, camera, lighting, shaders, texturing

 Potential ways to modify the code:  Add new geometry types, shaders, change the terrain, load new meshes, change the lighting, 
 different camera controls, different shaders, etc.
 
 Template version 5.0a 29/01/2017
 Dr Greg Slabaugh (gregory.slabaugh.1@city.ac.uk) 

 version 6.0a 29/01/2019
 Dr Eddie Edwards (Philip.Edwards@city.ac.uk)

 version 6.1a 13/02/2022 - Sorted out Release mode and a few small compiler warnings
 Dr Eddie Edwards (Philip.Edwards@city.ac.uk)

*/


#include "game.h"


// Setup includes
#include "HighResolutionTimer.h"
#include "GameWindow.h"

// Game includes
#include "Camera.h"
#include "Skybox.h"
#include "Plane.h"
#include "Shaders.h"
#include "FreeTypeFont.h"
#include "Sphere.h"
#include "MatrixStack.h"
#include "OpenAssetImportMesh.h"
#include "Audio.h"
#include "CatmullRom.h"

GLuint cubeVAO, cubeVBO, cubeEBO;

// Constructor
Game::Game()
{
	m_pSkybox = NULL;
	m_pCamera = NULL;
	m_pShaderPrograms = NULL;
	m_pPlanarTerrain = NULL;
	m_pFtFont = NULL;
	m_pHighResolutionTimer = NULL;
	m_pCatmullRom = NULL;

	m_dt = 0.0;
	m_framesPerSecond = 0;
	m_frameCount = 0;
	m_elapsedTime = 0.0f;
}

// Destructor
Game::~Game() 
{ 
	//game objects
	delete m_pCamera;
	delete m_pSkybox;
	delete m_pPlanarTerrain;
	delete m_pFtFont;
	delete m_pCatmullRom;

	if (m_pShaderPrograms != NULL) {
		for (unsigned int i = 0; i < m_pShaderPrograms->size(); i++)
			delete (*m_pShaderPrograms)[i];
	}
	delete m_pShaderPrograms;

	//setup objects
	delete m_pHighResolutionTimer;
}

// Initialisation:  This method only runs once at startup
void Game::Initialise() 
{
	// Set the clear colour and depth
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f);

	/// Create objects
	m_pCamera = new CCamera;
	m_pSkybox = new CSkybox;
	m_pShaderPrograms = new vector <CShaderProgram *>;
	m_pPlanarTerrain = new CPlane;
	m_pFtFont = new CFreeTypeFont;
	m_pCatmullRom = new CCatmullRom();


	RECT dimensions = m_gameWindow.GetDimensions();

	int width = dimensions.right - dimensions.left;
	int height = dimensions.bottom - dimensions.top;

	// Set the orthographic and perspective projection matrices based on the image size
	m_pCamera->SetOrthographicProjectionMatrix(width, height); 
	m_pCamera->SetPerspectiveProjectionMatrix(45.0f, (float) width / (float) height, 0.5f, 5000.0f);

	// Load shaders
	vector<CShader> shShaders;
	vector<string> sShaderFileNames;
	sShaderFileNames.push_back("mainShader.vert");
	sShaderFileNames.push_back("mainShader.frag");
	sShaderFileNames.push_back("textShader.vert");
	sShaderFileNames.push_back("textShader.frag");

	for (int i = 0; i < (int) sShaderFileNames.size(); i++) {
		string sExt = sShaderFileNames[i].substr((int) sShaderFileNames[i].size()-4, 4);
		int iShaderType;
		if (sExt == "vert") iShaderType = GL_VERTEX_SHADER;
		else if (sExt == "frag") iShaderType = GL_FRAGMENT_SHADER;
		else if (sExt == "geom") iShaderType = GL_GEOMETRY_SHADER;
		else if (sExt == "tcnl") iShaderType = GL_TESS_CONTROL_SHADER;
		else iShaderType = GL_TESS_EVALUATION_SHADER;
		CShader shader;
		shader.LoadShader("resources\\shaders\\"+sShaderFileNames[i], iShaderType);
		shShaders.push_back(shader);
	}

	// Create the main shader program
	CShaderProgram *pMainProgram = new CShaderProgram;
	pMainProgram->CreateProgram();
	pMainProgram->AddShaderToProgram(&shShaders[0]);
	pMainProgram->AddShaderToProgram(&shShaders[1]);
	pMainProgram->LinkProgram();
	m_pShaderPrograms->push_back(pMainProgram);

	// Create a shader program for fonts
	CShaderProgram *pFontProgram = new CShaderProgram;
	pFontProgram->CreateProgram();
	pFontProgram->AddShaderToProgram(&shShaders[2]);
	pFontProgram->AddShaderToProgram(&shShaders[3]);
	pFontProgram->LinkProgram();
	m_pShaderPrograms->push_back(pFontProgram);

	// You can follow this pattern to load additional shaders

	// Create the skybox
	// Skybox downloaded from http://www.akimbo.in/forum/viewtopic.php?f=10&t=9
	m_pSkybox->Create(2500.0f);
	
	// Create the planar terrain
	m_pPlanarTerrain->Create("resources\\textures\\", "grassfloor01.jpg", 2000.0f, 2000.0f, 50.0f); // Texture downloaded from http://www.psionicgames.com/?page_id=26 on 24 Jan 2013

	m_pFtFont->LoadSystemFont("arial.ttf", 32);
	m_pFtFont->SetShaderProgram(pFontProgram);


	// Spline path creation (to visualise not to move along, that would be done in Game::Update())
	glm::vec3 p1 = glm::vec3(-50.0f, 10.0f, 150.0f);  // Start of the arc
	glm::vec3 p2 = glm::vec3(120.0f, 10.0f, -20.0f);  // End of the arc

	// Control points
	glm::vec3 p0 = glm::vec3(100.0f, 10.0f, 250.0f);  
	glm::vec3 p3 = glm::vec3(1150.0f, 10.0f, 550.0f);
	//m_pCatmullRom->CreatePath(p0, p1, p2, p3);
	m_pCatmullRom->CreateCentreline();
	
	float cubeVertices[] = {
		// Front face (z = -1) ccw
		//  Position              Normal             Texutre
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

		   -1.0f, -1.0f, -1.0f,      0.0f, -1.0f,  0.0f,  0.0f, 1.0f, // front left
		    1.0f, -1.0f, -1.0f,      0.0f, -1.0f,  0.0f,  1.0f, 1.0f, // front right
		   -1.0f, -1.0f,  1.0f,      0.0f, -1.0f,  0.0f,  0.0f, 0.0f, // back left

			1.0f, -1.0f, -1.0f,      0.0f, -1.0f,  0.0f,  1.0f, 1.0f, // front right
			1.0f, -1.0f,  1.0f,      0.0f, -1.0f,  0.0f,  1.0f, 0.0f, // back right
		   -1.0f, -1.0f,  1.0f,      0.0f, -1.0f,  0.0f,  0.0f, 0.0f  // back left
	};


	// Generate and bind vao for cube
	glGenVertexArrays(1, &cubeVAO);
	glBindVertexArray(cubeVAO);
	// Generate and bind vbo and then upload vertices
	glGenBuffers(1, &cubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	// Define stride
	GLsizei stride = 8 * sizeof(float); 
	// Position: 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	// Normal: 1
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// Texture coordinate: 2
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	// Unbind vao
	glBindVertexArray(0);


}

// Render method runs repeatedly in a loop
void Game::Render() 
{
	// Clear the buffers and enable depth testing (z-buffering)
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//glDisable(GL_CULL_FACE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Set up a matrix stack
	glutil::MatrixStack modelViewMatrixStack;
	modelViewMatrixStack.SetIdentity();

	// Use the main shader program 
	CShaderProgram *pMainProgram = (*m_pShaderPrograms)[0];
	pMainProgram->UseProgram();
	pMainProgram->SetUniform("bUseTexture", true);
	pMainProgram->SetUniform("sampler0", 0);
	// Note: cubemap and non-cubemap textures should not be mixed in the same texture unit.  Setting unit 10 to be a cubemap texture.
	int cubeMapTextureUnit = 10; 
	pMainProgram->SetUniform("CubeMapTex", cubeMapTextureUnit);
	

	// Set the projection matrix
	pMainProgram->SetUniform("matrices.projMatrix", m_pCamera->GetPerspectiveProjectionMatrix());

	// Call LookAt to create the view matrix and put this on the modelViewMatrix stack. 
	// Store the view matrix and the normal matrix associated with the view matrix for later (they're useful for lighting -- since lighting is done in eye coordinates)
	modelViewMatrixStack.LookAt(m_pCamera->GetPosition(), m_pCamera->GetView(), m_pCamera->GetUpVector());
	glm::mat4 viewMatrix = modelViewMatrixStack.Top();
	glm::mat3 viewNormalMatrix = m_pCamera->ComputeNormalMatrix(viewMatrix);

	
	// Set light and materials in main shader program
	glm::vec4 lightPosition1 = glm::vec4(-100, 100, -100, 1); // Position of light source *in world coordinates*
	pMainProgram->SetUniform("light1.position", viewMatrix*lightPosition1); // Position of light source *in eye coordinates*
	pMainProgram->SetUniform("light1.La", glm::vec3(1.0f));		// Ambient colour of light
	pMainProgram->SetUniform("light1.Ld", glm::vec3(1.0f));		// Diffuse colour of light
	pMainProgram->SetUniform("light1.Ls", glm::vec3(1.0f));		// Specular colour of light
	pMainProgram->SetUniform("material1.Ma", glm::vec3(1.0f));	// Ambient material reflectance
	pMainProgram->SetUniform("material1.Md", glm::vec3(0.0f));	// Diffuse material reflectance
	pMainProgram->SetUniform("material1.Ms", glm::vec3(0.0f));	// Specular material reflectance
	pMainProgram->SetUniform("material1.shininess", 15.0f);		// Shininess material property
		

	// Render the skybox and terrain with full ambient reflectance 
	modelViewMatrixStack.Push();
		pMainProgram->SetUniform("renderSkybox", true);
		// Translate the modelview matrix to the camera eye point so skybox stays centred around camera
		glm::vec3 vEye = m_pCamera->GetPosition();
		modelViewMatrixStack.Translate(vEye);
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		m_pSkybox->Render(cubeMapTextureUnit);
		pMainProgram->SetUniform("renderSkybox", false);
	modelViewMatrixStack.Pop();

	// Render the planar terrain
	modelViewMatrixStack.Push();
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		m_pPlanarTerrain->Render();
	modelViewMatrixStack.Pop();


	// Turn on diffuse + specular materials
	pMainProgram->SetUniform("material1.Ma", glm::vec3(0.5f));	// Ambient material reflectance
	pMainProgram->SetUniform("material1.Md", glm::vec3(0.5f));	// Diffuse material reflectance
	pMainProgram->SetUniform("material1.Ms", glm::vec3(1.0f));	// Specular material reflectance	

	// Render spline path
	modelViewMatrixStack.Push();
		pMainProgram->SetUniform("bUseTexture", false);
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		m_pCatmullRom->RenderCentreline();
	modelViewMatrixStack.Pop();



	// Set up your transformation matrix for the cube
	modelViewMatrixStack.Push();
	// For example, translate the cube  to (x, y, z)
	modelViewMatrixStack.Scale(5.0f);
	modelViewMatrixStack.Translate(glm::vec3(0.0f, 10.0f, 0.0f));
	pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
	pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));

	// Bind the cube VAO and render it
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vertices cube
	glBindVertexArray(0);
	modelViewMatrixStack.Pop();

	// Draw the 2D graphics after the 3D graphics
	DisplayFrameRate();

	// Swap buffers to show the rendered image
	SwapBuffers(m_gameWindow.Hdc());		

}

// Update method runs repeatedly with the Render method
void Game::Update() 
{
	//m_pCamera->Set(m_pCamera->GetPosition(), glm::vec3(0, 0, 0), m_pCamera->GetUpVector());
	//m_pCamera->Set(glm::vec3(0, 500, 0.1), glm::vec3(0, 0, 0), m_pCamera->GetUpVector());
	// Update the camera using the amount of time that has elapsed to avoid framerate dependent motion
	m_pCamera->Update(m_dt);
	/*
	// Moving and interpolating along the spline 
	// Spline control points
	glm::vec3 p1 = glm::vec3(-50.0f, 20.0f, 150.0f);  // Start of the arc
	glm::vec3 p2 = glm::vec3(120.0f, 20.0f, -20.0f);  // End of the arc
	glm::vec3 p0 = glm::vec3(100.0f, 20.0f, 250.0f);
	glm::vec3 p3 = glm::vec3(1150.0f, 20.0f, 550.0f);

	static float t = 0.0f;
	t += 0.0001f * m_dt; 
	if (t > 1.0f) {
		t = 0.0f;
	}

	// New camera pos
	glm::vec3 cameraPos = m_pCatmullRom->Interpolate(p0, p1, p2, p3, t);
	m_pCamera->Set(cameraPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	*/	

	//m_pAudio->Update();
}



void Game::DisplayFrameRate()
{


	CShaderProgram *fontProgram = (*m_pShaderPrograms)[1];

	RECT dimensions = m_gameWindow.GetDimensions();
	int height = dimensions.bottom - dimensions.top;

	// Increase the elapsed time and frame counter
	m_elapsedTime += m_dt;
	m_frameCount++;

	// Now we want to subtract the current time by the last time that was stored
	// to see if the time elapsed has been over a second, which means we found our FPS.
	if (m_elapsedTime > 1000)
    {
		m_elapsedTime = 0;
		m_framesPerSecond = m_frameCount;

		// Reset the frames per second
		m_frameCount = 0;
    }

	if (m_framesPerSecond > 0) {
		// Use the font shader program and render the text
		fontProgram->UseProgram();
		glDisable(GL_DEPTH_TEST);
		fontProgram->SetUniform("matrices.modelViewMatrix", glm::mat4(1));
		fontProgram->SetUniform("matrices.projMatrix", m_pCamera->GetOrthographicProjectionMatrix());
		fontProgram->SetUniform("vColour", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		m_pFtFont->Render(20, height - 20, 20, "FPS: %d", m_framesPerSecond);
	}
}

// The game loop runs repeatedly until game over
void Game::GameLoop()
{
	/*
	// Fixed timer
	dDt = pHighResolutionTimer->Elapsed();
	if (dDt > 1000.0 / (double) Game::FPS) {
		pHighResolutionTimer->Start();
		Update();
		Render();
	}
	*/
	
	// Variable timer
	m_pHighResolutionTimer->Start();
	Update();
	Render();
	m_dt = m_pHighResolutionTimer->Elapsed();
}


WPARAM Game::Execute() 
{
	m_pHighResolutionTimer = new CHighResolutionTimer;
	m_gameWindow.Init(m_hInstance);

	if(!m_gameWindow.Hdc()) {
		return 1;
	}

	Initialise();

	m_pHighResolutionTimer->Start();

	
	MSG msg;

	while(1) {													
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { 
			if(msg.message == WM_QUIT) {
				break;
			}

			TranslateMessage(&msg);	
			DispatchMessage(&msg);
		} else if (m_appActive) {
			GameLoop();
		} 
		else Sleep(200); // Do not consume processor power if application isn't active
	}

	m_gameWindow.Deinit();

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
				m_appActive = true;
				m_pHighResolutionTimer->Start();
				break;
			case WA_INACTIVE:
				m_appActive = false;
				break;
		}
		break;
		}

	case WM_SIZE:
			RECT dimensions;
			GetClientRect(window, &dimensions);
			m_gameWindow.SetDimensions(dimensions);
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
		case '1':
			m_pAudio->PlayEventSound();
			break;
		case VK_F1:
			m_pAudio->PlayEventSound();
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
	m_hInstance = hinstance;
}

LRESULT CALLBACK WinProc(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
	return Game::GetInstance().ProcessEvents(window, message, w_param, l_param);
}

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE, PSTR, int) 
{
	Game &game = Game::GetInstance();
	game.SetHinstance(hinstance);

	return int(game.Execute());
}
