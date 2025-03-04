#pragma once

#include "Common.h"
#include "include/gl/glew.h"
#include <gl/gl.h>

// Classes used in game
class CShader;
class CShaderProgram;
class GameWindow;
class CSphere;
class CHighResolutionTimer;

class Game {
public:

private:
	void Init();
	void Update();
	void Render();
	void GameLoop();

	CShaderProgram *m_pShaderProgram;
	CHighResolutionTimer *m_pGameTimer;
	CHighResolutionTimer *m_pTimer;
	CSphere *m_pSphere;

	GLuint m_uiVAO;	
	float m_rotY;
	float m_dt;

public:

	static Game& GetInstance();

	~Game();

	WPARAM Exec();

	void SetHinstance(HINSTANCE hinstance);
	LRESULT ProcessEvents(HWND window,UINT message, WPARAM w_param, LPARAM l_param);

private:
	Game();
	Game(const Game&);

	GameWindow &rGameWindow;
	HINSTANCE hHinstance;

	bool bAppActive;
};
