#pragma once

#include "Common.h"
#include "GameWindow.h"

// Classes used in game
class CShader;
class CShaderProgram;
class CHighResolutionTimer;

class Game {
private:
	void Initialise();
	void Update();
	void Render();

	CShaderProgram *m_pShaderProgram;
	CHighResolutionTimer *m_pTimer;
	glm::mat4 *m_pModelMatrix;
	glm::mat4 *m_pViewMatrix;
	glm::mat4 *m_pProjectionMatrix;
	GLuint m_uiVAO;	// A vertex array object (to wrap VBOs)

public:
	Game();
	~Game();
	static Game& GetInstance();
	LRESULT ProcessEvents(HWND window,UINT message, WPARAM w_param, LPARAM l_param);
	void SetHinstance(HINSTANCE hinstance);
	WPARAM Execute();

private:
	void GameLoop();
	GameWindow gameWindow;
	HINSTANCE hHinstance;

};
