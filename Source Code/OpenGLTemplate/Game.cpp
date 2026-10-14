#include "game.h"

// Setup includes
#include "HighResolutionTimer.h"
#include "GameWindow.h"

// Game includes
#include "Camera.h"
#include "Skybox.h"
#include "Shaders.h"
#include "FreeTypeFont.h"
#include "MatrixStack.h"
#include "OpenAssetImportMesh.h"
#include "Audio.h"
#include "CameraPath.h"
#include "Car.h"
#include "PlayerCar.h"
#include "StartLine.h"
#include "BoostPad.h"

// Constructor
Game::Game()
{
	m_pSkybox = NULL;
	m_pCamera = NULL;
	m_pShaderPrograms = NULL;
	m_pFtFont = NULL;
	m_pSpaceStationMesh = NULL;
	m_pEarthMesh = NULL;
	m_pAsteroidsMesh = NULL;
	m_pHighResolutionTimer = NULL;
	m_pAudio = NULL;
	m_pCameraPath = NULL;
	m_pCar = NULL;
	m_pPlayerCar = NULL;
	m_pStartLine = NULL;
	m_pBoostPad = NULL;

	m_cameraMode = START_VIEW;
	m_prevCameraMode = THIRD_PERSON;

	m_sceneFBO = 0;
	m_sceneColourTex = 0;
	m_sceneDepthRBO = 0;

	m_brightFBO = 0;
	m_brightTex = 0;

	m_pingpongFBO[0] = 0;
	m_pingpongFBO[1] = 0;
	m_pingpongTex[0] = 0;
	m_pingpongTex[1] = 0;

	m_quadVAO = 0;
	m_quadVBO = 0;

	m_dt = 0.0;
	m_framesPerSecond = 0;
	m_appActive = false;
	m_hInstance = NULL;
	m_frameCount = 0;
	m_elapsedTime = 0.0f;


	m_windowWidth = 0;
	m_windowHeight = 0;
}

// Destructor
Game::~Game()
{
	ReleaseBloom();

	delete m_pCamera;
	delete m_pSkybox;
	delete m_pFtFont;
	delete m_pSpaceStationMesh;
	delete m_pEarthMesh;
	delete m_pAsteroidsMesh;
	delete m_pAudio;
	delete m_pCameraPath;
	delete m_pCar;
	delete m_pPlayerCar;
	delete m_pStartLine;
	delete m_pBoostPad;

	if (m_pShaderPrograms != NULL)
	{
		for (unsigned int i = 0; i < m_pShaderPrograms->size(); i++)
			delete (*m_pShaderPrograms)[i];
	}
	delete m_pShaderPrograms;

	delete m_pHighResolutionTimer;
}

void Game::CreateCoreObjects()
{
	m_pCamera = new CCamera;
	m_pSkybox = new CSkybox;
	m_pShaderPrograms = new vector<CShaderProgram*>;
	m_pFtFont = new CFreeTypeFont;
	m_pSpaceStationMesh = new COpenAssetImportMesh;
	m_pEarthMesh = new COpenAssetImportMesh;
	m_pAsteroidsMesh = new COpenAssetImportMesh;
	m_pAudio = new CAudio;
	m_pCameraPath = new CCameraPath;
	m_pCar = new CCar;
	m_pPlayerCar = new CPlayerCar;
	m_pStartLine = new CStartLine;
	m_pBoostPad = new CBoostPad;
}

// Updates cached window dimensions and rebuilds the orthographic and perspective projections
void Game::SetupProjectionMatrices(int width, int height)
{
	m_windowWidth = width;
	m_windowHeight = height;

	m_pCamera->SetOrthographicProjectionMatrix(width, height);
	m_pCamera->SetPerspectiveProjectionMatrix(45.0f, (float)width / (float)height, 0.5f, 5000.0f);
}

void Game::LoadShaderPrograms()
{
	vector<CShader> shShaders;
	vector<string> sShaderFileNames;

	sShaderFileNames.push_back("mainShader.vert");
	sShaderFileNames.push_back("mainShader.frag");
	sShaderFileNames.push_back("textShader.vert");
	sShaderFileNames.push_back("textShader.frag");
	sShaderFileNames.push_back("carShader.vert");
	sShaderFileNames.push_back("carShader.frag");
	sShaderFileNames.push_back("routeShader.vert");
	sShaderFileNames.push_back("routeShader.frag");
	sShaderFileNames.push_back("screenQuad.vert");
	sShaderFileNames.push_back("brightPass.frag");
	sShaderFileNames.push_back("gaussianBlur.frag");
	sShaderFileNames.push_back("bloomCombine.frag");
	sShaderFileNames.push_back("boostpadShader.vert");
	sShaderFileNames.push_back("boostpadShader.frag");
	sShaderFileNames.push_back("particleShader.vert");
	sShaderFileNames.push_back("particleShader.frag");
	sShaderFileNames.push_back("hudShader.vert");
	sShaderFileNames.push_back("hudShader.frag");

	for (int i = 0; i < (int)sShaderFileNames.size(); i++)
	{
		string sExt = sShaderFileNames[i].substr((int)sShaderFileNames[i].size() - 4, 4);
		int iShaderType;

		if (sExt == "vert") iShaderType = GL_VERTEX_SHADER;
		else if (sExt == "frag") iShaderType = GL_FRAGMENT_SHADER;
		else if (sExt == "geom") iShaderType = GL_GEOMETRY_SHADER;
		else if (sExt == "tcnl") iShaderType = GL_TESS_CONTROL_SHADER;
		else iShaderType = GL_TESS_EVALUATION_SHADER;

		CShader shader;
		shader.LoadShader("resources\\shaders\\" + sShaderFileNames[i], iShaderType);
		shShaders.push_back(shader);
	}

	CShaderProgram* pMainProgram = new CShaderProgram;
	pMainProgram->CreateProgram();
	pMainProgram->AddShaderToProgram(&shShaders[0]);
	pMainProgram->AddShaderToProgram(&shShaders[1]);
	pMainProgram->LinkProgram();
	m_pShaderPrograms->push_back(pMainProgram);

	CShaderProgram* pFontProgram = new CShaderProgram;
	pFontProgram->CreateProgram();
	pFontProgram->AddShaderToProgram(&shShaders[2]);
	pFontProgram->AddShaderToProgram(&shShaders[3]);
	pFontProgram->LinkProgram();
	m_pShaderPrograms->push_back(pFontProgram);

	CShaderProgram* pCarProgram = new CShaderProgram;
	pCarProgram->CreateProgram();
	pCarProgram->AddShaderToProgram(&shShaders[4]);
	pCarProgram->AddShaderToProgram(&shShaders[5]);
	pCarProgram->LinkProgram();
	m_pShaderPrograms->push_back(pCarProgram);

	CShaderProgram* pRouteProgram = new CShaderProgram;
	pRouteProgram->CreateProgram();
	pRouteProgram->AddShaderToProgram(&shShaders[6]);
	pRouteProgram->AddShaderToProgram(&shShaders[7]);
	pRouteProgram->LinkProgram();
	m_pShaderPrograms->push_back(pRouteProgram);

	CShaderProgram* pBrightProgram = new CShaderProgram;
	pBrightProgram->CreateProgram();
	pBrightProgram->AddShaderToProgram(&shShaders[8]);
	pBrightProgram->AddShaderToProgram(&shShaders[9]);
	pBrightProgram->LinkProgram();
	m_pShaderPrograms->push_back(pBrightProgram);

	CShaderProgram* pBlurProgram = new CShaderProgram;
	pBlurProgram->CreateProgram();
	pBlurProgram->AddShaderToProgram(&shShaders[8]);
	pBlurProgram->AddShaderToProgram(&shShaders[10]);
	pBlurProgram->LinkProgram();
	m_pShaderPrograms->push_back(pBlurProgram);

	CShaderProgram* pCombineProgram = new CShaderProgram;
	pCombineProgram->CreateProgram();
	pCombineProgram->AddShaderToProgram(&shShaders[8]);
	pCombineProgram->AddShaderToProgram(&shShaders[11]);
	pCombineProgram->LinkProgram();
	m_pShaderPrograms->push_back(pCombineProgram);

	CShaderProgram* pBoostPadProgram = new CShaderProgram;
	pBoostPadProgram->CreateProgram();
	pBoostPadProgram->AddShaderToProgram(&shShaders[12]);
	pBoostPadProgram->AddShaderToProgram(&shShaders[13]);
	pBoostPadProgram->LinkProgram();
	m_pShaderPrograms->push_back(pBoostPadProgram);

	CShaderProgram* pParticleProgram = new CShaderProgram;
	pParticleProgram->CreateProgram();
	pParticleProgram->AddShaderToProgram(&shShaders[14]);
	pParticleProgram->AddShaderToProgram(&shShaders[15]);
	pParticleProgram->LinkProgram();
	m_pShaderPrograms->push_back(pParticleProgram);

	CShaderProgram* pHUDProgram = new CShaderProgram;
	pHUDProgram->CreateProgram();
	pHUDProgram->AddShaderToProgram(&shShaders[16]);
	pHUDProgram->AddShaderToProgram(&shShaders[17]);
	pHUDProgram->LinkProgram();
	m_pShaderPrograms->push_back(pHUDProgram);
}

void Game::CreateSceneObjects()
{
	m_pCameraPath->CreateTrack("resources\\textures\\", "Road.jpg", "Road.jpg", 6.0f, 40);

	m_pSkybox->Create(2500.0f);

	m_pFtFont->LoadSystemFont("arial.ttf", 32);
	m_pFtFont->SetShaderProgram((*m_pShaderPrograms)[FONT_PROGRAM]);

	m_pSpaceStationMesh->Load("resources\\models\\SpaceStation\\SpaceStation.obj");
	m_pEarthMesh->Load("resources\\models\\Earth\\Earth.obj");
	m_pAsteroidsMesh->Load("resources\\models\\Asteroids\\Asteroids.obj");

	glEnable(GL_CULL_FACE);
}

void Game::CreateGameplayObjects()
{
	m_pCar->Initialise("resources\\models\\Car\\");
	m_pCar->SetScale(0.5f);

	m_pPlayerCar->Initialise(m_pCar, m_pCameraPath);

	m_pStartLine->Create(
		"resources\\textures\\OuterRing.jpg", // Inner Ring
		"resources\\textures\\OuterRing.jpg",	// Outer Ring
		"resources\\textures\\StartBanner.jpg",	// Sign front
		"resources\\textures\\Wood.jpg",	// Sign side
		"resources\\textures\\FinishLine.jpg");	// Crossing

	m_boostPads.push_back({ 0.04f, 2.0f, glm::vec3(0.0f) });
	m_boostPads.push_back({ 0.1f, -2.0f, glm::vec3(0.0f) });
	m_boostPads.push_back({ 0.15f, 2.0f, glm::vec3(0.0f) });
	m_boostPads.push_back({ 0.2f, 0.0f, glm::vec3(0.0f) });
	m_boostPads.push_back({ 0.23f, -2.5f, glm::vec3(0.0f) });
	m_boostPads.push_back({ 0.27f, 3.0f, glm::vec3(0.0f) });
	m_boostPads.push_back({ 0.34f, 0.0f, glm::vec3(0.0f) });
	m_boostPads.push_back({ 0.42f, 2.0f, glm::vec3(0.0f) });
	m_boostPads.push_back({ 0.50f, 0.0f, glm::vec3(0.0f) });
	m_boostPads.push_back({ 0.56f, -1.0f, glm::vec3(0.0f) });
	m_boostPads.push_back({ 0.65f, 1.0f, glm::vec3(0.0f) });
	m_boostPads.push_back({ 0.74f, 0.0f, glm::vec3(0.0f) });
	m_boostPads.push_back({ 0.82f, 0.0f, glm::vec3(0.0f) });
	m_boostPads.push_back({ 0.92f, 0.0f, glm::vec3(0.0f) });
	m_boostPads.push_back({ 0.97f, 2.0f, glm::vec3(0.0f) });
	for (size_t i = 0; i < m_boostPads.size(); i++)
	{
		glm::vec3 centre, T, N, B;
		m_pCameraPath->GetTrackFrame(m_boostPads[i].routeT, centre, T, N, B);

		m_boostPads[i].position =
			centre +
			m_boostPads[i].lateralOffset * N +
			0.08f * B;
	}


	m_pBoostPad->Create("resources\\textures\\BoostPad.jpg");
}

void Game::InitialiseAudio()
{
	m_pAudio->Initialise();
	m_pAudio->LoadEventSound("resources\\Audio\\Boost.mp3");
	m_pAudio->LoadMusicStream("resources\\Audio\\Game-Track.mp3");
}

// Initialisation
void Game::Initialise()
{
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f);

	CreateCoreObjects();

	RECT dimensions = m_gameWindow.GetDimensions();
	int width = dimensions.right - dimensions.left;
	int height = dimensions.bottom - dimensions.top;

	SetupProjectionMatrices(width, height);
	LoadShaderPrograms();
	InitialiseBloom(width, height);
	CreateSceneObjects();
	CreateGameplayObjects();
	InitialiseAudio();

	m_currentLap = 0;
	m_timeGivenForLap = 60.0f;
	m_countdownTime = m_timeGivenForLap;
	m_raceStarted = false;
	m_raceFinished = false;
	m_playingGame = true;

	// Line A (start of finish line)
	m_lineA1 = glm::vec2(82.0f, 1.0f);
	m_lineA2 = glm::vec2(87.0f, 8.0f);

	// Second finish-line gate used to confirm forward crossing direction.
	m_lineB1 = glm::vec2(85.5f, -2.9f);
	m_lineB2 = glm::vec2(90.0f, 6.5f);

	// Store initial car position
	glm::vec3 startPos = m_pPlayerCar->GetPosition();
	m_prevCarXZ = glm::vec2(startPos.x, startPos.z);

	m_crossedA = false;
}

// Checks the player car against each boost pad using simple distance-based collision.
void Game::CheckBoostPadCollisions()
{
	if (!m_pPlayerCar)
		return;

	glm::vec3 carPos = m_pPlayerCar->GetPosition();

	for (size_t i = 0; i < m_boostPads.size(); i++)
	{
		float dist = glm::length(carPos - m_boostPads[i].position);
		if (dist < 2.5f) {
			m_pPlayerCar->ActivateBoost();
			m_pAudio->PlayEventSound();
		}
	}
}

void Game::UpdatePlayerAndCamera()
{
	glm::vec3 up(0.0f, 1.0f, 0.0f);

	switch (m_cameraMode) {
	// Free debug camera from the template.
	case FREE_VIEW:
		m_pCamera->Update(m_dt);
		break;

	// Fixed pre-race overview camera.
	case START_VIEW:
	{
		glm::vec3 eye(27.3f, 37.9f, 138.6f);
		glm::vec3 look(30.4f, 29.4f, 107.8f);
		m_pCamera->Set(eye, look, up);
		break;
	}

	// Gameplay cameras that follow the player car.
	case THIRD_PERSON:
	case TOP_VIEW:
	{
		if (!m_pPlayerCar || m_raceFinished) { break; }

		m_pPlayerCar->Update((float)m_dt);

		glm::vec3 carPos = m_pPlayerCar->GetPosition();
		glm::vec3 forward = m_pPlayerCar->GetForward();

		glm::vec3 eye, look, camUp;

		if (m_cameraMode == THIRD_PERSON) {
			eye = carPos + 4.0f * up - 16.0f * forward;
			look = carPos + 2.0f * up + 16.0f * forward;
			camUp = up;
		}
		else {
			eye = carPos + 40.0f * up;
			look = carPos;
			camUp = forward;
		}

		m_pCamera->Set(eye, look, camUp);
		CheckBoostPadCollisions();
		break;
	}
	}
}

namespace
{
	// Returns the signed side of point p relative to the directed line a->b.
	float SideOfLine(const glm::vec2& p, const glm::vec2& a, const glm::vec2& b)
	{
		return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
	}

	// Returns true if the segment prev->curr crosses the line a->b.
	bool CrossedLine(const glm::vec2& prev, const glm::vec2& curr, const glm::vec2& a, const glm::vec2& b)
	{
		float prevSide = SideOfLine(prev, a, b);
		float currSide = SideOfLine(curr, a, b);

		return (prevSide < 0.0f && currSide >= 0.0f) ||
			(prevSide > 0.0f && currSide <= 0.0f);
	}
}

// Uses a two-line gate so laps are only counted when the player crosses the finish
// region in the intended forward direction.
void Game::CheckFinishLineCrossing()
{
	if (!m_pPlayerCar){return;}

	glm::vec3 pos = m_pPlayerCar->GetPosition();
	glm::vec2 currXZ(pos.x, pos.z);

	if (!m_crossedA)
	{
		m_crossedA = CrossedLine(m_prevCarXZ, currXZ, m_lineA1, m_lineA2);
	}
	else if (CrossedLine(m_prevCarXZ, currXZ, m_lineB1, m_lineB2))
	{
		if (m_currentLap == 0){m_raceStarted = true;}

		m_currentLap++;

		m_timeGivenForLap -= 10.0f;
		if (m_timeGivenForLap < 20.0f)
			m_timeGivenForLap = 20.0f;

		m_countdownTime = m_timeGivenForLap;
		m_crossedA = false;
	}

	m_prevCarXZ = currXZ;
}

void Game::UpdateRaceLogic()
{
	if (m_raceFinished || !m_pPlayerCar) {
		return;

	}else if (m_raceStarted && m_playingGame) {
		m_countdownTime -= (float)m_dt * 0.001f;

		if (m_countdownTime <= 0.0f)
		{
			m_countdownTime = 0.0f;
			m_raceFinished = true;
			return;
		}
	}

	CheckFinishLineCrossing();
}

void Game::Update()
{
	m_pAudio->Update();
	UpdatePlayerAndCamera();
	UpdateRaceLogic();
}

void Game::RenderHUD()
{
	CShaderProgram* fontProgram = (*m_pShaderPrograms)[FONT_PROGRAM];
	fontProgram->UseProgram();
	glDisable(GL_DEPTH_TEST);
	fontProgram->SetUniform("matrices.modelViewMatrix", glm::mat4(1));
	fontProgram->SetUniform("matrices.projMatrix", m_pCamera->GetOrthographicProjectionMatrix());
	fontProgram->SetUniform("vColour", glm::vec4(1.0f));
	fontProgram->SetUniform("bBlink", false);
	fontProgram->SetUniform("t", (float)m_elapsedTime * 0.001f);
	fontProgram->SetUniform("vColour", glm::vec4(1, 1, 1, 1));

	glm::vec3 eye = m_pCamera->GetPosition();
	glm::vec3 look = m_pCamera->GetView();

	m_elapsedTime += m_dt;
	m_frameCount++;

	if (m_elapsedTime > 1000)
	{
		m_elapsedTime = 0;
		m_framesPerSecond = m_frameCount;
		m_frameCount = 0;
	}

	if (m_framesPerSecond > 0)
	{
		switch (m_cameraMode)
		{
		case FREE_VIEW:
			m_pFtFont->Render(20, m_windowHeight - 20, 20, "FPS: %d", m_framesPerSecond);
			m_pFtFont->Render(20, m_windowHeight - 80, 20, "Car: %.1f %.1f %.1f", m_pPlayerCar->GetPosition().x, m_pPlayerCar->GetPosition().y, m_pPlayerCar->GetPosition().z);

			// Debug info for camera vectors
			m_pFtFont->Render(20, m_windowHeight - 140, 20, "Eye: %.1f %.1f %.1f", eye.x, eye.y, eye.z);
			m_pFtFont->Render(20, m_windowHeight - 160, 20, "Look: %.1f %.1f %.1f", look.x, look.y, look.z);
			break;
		case START_VIEW:
		{
			DrawHUDBox(m_windowWidth / 4 - 40.0f, m_windowHeight - 460.0f, 480.0f, 300.0f, glm::vec4(0.0f, 0.0f, 0.0f, 0.45f));
			fontProgram->UseProgram();

			m_pFtFont->Render(m_windowWidth / 4 + 80, m_windowHeight - 190, 23, "W / S    :    Accelerate / Brake");
			m_pFtFont->Render(m_windowWidth / 4 + 80, m_windowHeight - 220, 23, "A / D     :    Move Left / Right");
			m_pFtFont->Render(m_windowWidth / 4 + 100, m_windowHeight - 280, 23, "R       :     Camera Toggle");
			m_pFtFont->Render(m_windowWidth / 4 + 86, m_windowHeight - 340, 23, "TAB     :       Pause Game");
			m_pFtFont->Render(m_windowWidth / 4 + 86, m_windowHeight - 370, 23, "ESC     :            Quit");
			m_pFtFont->Render(m_windowWidth / 4 + 60, m_windowHeight - 440, 28, "Press SPACE to continue");
			break;
		}
		case THIRD_PERSON:
		case TOP_VIEW:
			m_pFtFont->Render(m_windowWidth - 120, m_windowHeight - 40, 30, "Laps: %d", m_currentLap);
			m_pFtFont->Render(20, m_windowHeight - 40, 20, "TAB to Pause");
			m_pFtFont->Render(20, m_windowHeight - 60, 20, "R to Toggle Camera");
			if (m_raceFinished) {
				DrawHUDBox(m_windowWidth / 4 + 40.0f, m_windowHeight / 2 - 55.0f, 300.0f, 100.0f, glm::vec4(0.0f, 0.0f, 0.0f, 0.45f));
				fontProgram->UseProgram();
				m_pFtFont->Render(m_windowWidth / 4 + 60, m_windowHeight / 2, 40, "You completed");
				m_pFtFont->Render(m_windowWidth / 4 + 140, m_windowHeight / 2 - 40, 40, "%d laps!", m_currentLap);
			}
			// Use DISCARD in the shader to create a flashing effect.
			if (m_countdownTime < 10.0f) {
				fontProgram->SetUniform("bBlink", true);
				fontProgram->SetUniform("vColour", glm::vec4(1.0f, 0.f, 0.f, 1.0f));
			}
			if (m_raceStarted) { m_pFtFont->Render(m_windowWidth - 460, m_windowHeight - 50, 50, "%.2fs", m_countdownTime); }
			else{ 
				fontProgram->SetUniform("bBlink", true);
				m_pFtFont->Render(m_windowWidth / 4 + 20, m_windowHeight - 50, 25, "Drive through the start line to begin"); 
			}
			break;
		}
		
	}
}

void Game::GameLoop()
{
	m_pHighResolutionTimer->Start();
	Update();
	Render();
	m_dt = m_pHighResolutionTimer->Elapsed();
}

WPARAM Game::Execute()
{
	m_pHighResolutionTimer = new CHighResolutionTimer;
	m_gameWindow.Init(m_hInstance);

	if (!m_gameWindow.Hdc())
		return 1;

	Initialise();
	m_pHighResolutionTimer->Start();

	MSG msg;

	while (1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else if (m_appActive)
		{
			GameLoop();
		}
		else
		{
			Sleep(200);
		}
	}

	m_gameWindow.Deinit();
	return msg.wParam;
}

LRESULT Game::ProcessEvents(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
	LRESULT result = 0;

	switch (message)
	{
	case WM_ACTIVATE:
	{
		switch (LOWORD(w_param))
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
	{
		RECT dimensions;
		GetClientRect(window, &dimensions);
		m_gameWindow.SetDimensions(dimensions);

		int width = dimensions.right - dimensions.left;
		int height = dimensions.bottom - dimensions.top;

		m_windowWidth = width;
		m_windowHeight = height;

		if (m_pCamera && width > 0 && height > 0)
		{
			SetupProjectionMatrices(width, height);
		}

		if (width > 0 && height > 0)
		{
			ReleaseBloom();
			InitialiseBloom(width, height);
		}
		break;
	}

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(window, &ps);
		EndPaint(window, &ps);
		break;
	}

	case WM_KEYDOWN:
		switch (w_param)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		case VK_SPACE:
			m_cameraMode = m_prevCameraMode;
			m_playingGame = true;
			m_pAudio->PlayMusicStream();
			break;
		case VK_TAB:
			if (m_cameraMode == THIRD_PERSON || m_cameraMode == TOP_VIEW)
				m_prevCameraMode = m_cameraMode;
			m_cameraMode = START_VIEW;
			m_playingGame = false;
			m_pAudio->StopMusicStream();
			break;
		case '1':
			m_cameraMode = FREE_VIEW;
			m_playingGame = false;
			m_pAudio->StopMusicStream();
			break;
		case '2':
			if (m_pPlayerCar)
				m_pPlayerCar->ActivateBoost();
			break;
		case 'R':
			if (m_cameraMode == THIRD_PERSON || m_cameraMode == TOP_VIEW)
				m_cameraMode = (m_cameraMode == THIRD_PERSON) ? TOP_VIEW : THIRD_PERSON;
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

// Creates the fullscreen quad and framebuffer chain used for bloom:
// 1. render full scene
// 2. extract bright regions
// 3. blur using ping-pong framebuffers
// 4. combine blurred bloom with the original scene
void Game::InitialiseBloom(int width, int height)
{
	float quadVertices[] =
	{
		-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,  0.0f, 1.0f,

		-1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
		 1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,  1.0f, 1.0f
	};

	glGenVertexArrays(1, &m_quadVAO);
	glGenBuffers(1, &m_quadVBO);

	glBindVertexArray(m_quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	GLenum drawBuffer = GL_COLOR_ATTACHMENT0;

	// Scene FBO
	glGenFramebuffers(1, &m_sceneFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_sceneFBO);

	glGenTextures(1, &m_sceneColourTex);
	glBindTexture(GL_TEXTURE_2D, m_sceneColourTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_sceneColourTex, 0);
	glDrawBuffers(1, &drawBuffer);
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	glGenRenderbuffers(1, &m_sceneDepthRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_sceneDepthRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_sceneDepthRBO);

	// Bright-pass FBO
	glGenFramebuffers(1, &m_brightFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_brightFBO);

	glGenTextures(1, &m_brightTex);
	glBindTexture(GL_TEXTURE_2D, m_brightTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_brightTex, 0);
	glDrawBuffers(1, &drawBuffer);
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	// Ping-pong FBOs
	glGenFramebuffers(2, m_pingpongFBO);
	glGenTextures(2, m_pingpongTex);

	for (int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_pingpongFBO[i]);

		glBindTexture(GL_TEXTURE_2D, m_pingpongTex[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pingpongTex[i], 0);
		glDrawBuffers(1, &drawBuffer);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Deletes all bloom-related OpenGL objects and resets their IDs to 0.
void Game::ReleaseBloom()
{
	if (m_quadVBO) glDeleteBuffers(1, &m_quadVBO);
	if (m_quadVAO) glDeleteVertexArrays(1, &m_quadVAO);

	if (m_sceneDepthRBO) glDeleteRenderbuffers(1, &m_sceneDepthRBO);

	if (m_sceneColourTex) glDeleteTextures(1, &m_sceneColourTex);
	if (m_brightTex) glDeleteTextures(1, &m_brightTex);

	if (m_pingpongTex[0] || m_pingpongTex[1])
		glDeleteTextures(2, m_pingpongTex);

	if (m_sceneFBO) glDeleteFramebuffers(1, &m_sceneFBO);
	if (m_brightFBO) glDeleteFramebuffers(1, &m_brightFBO);

	if (m_pingpongFBO[0] || m_pingpongFBO[1])
		glDeleteFramebuffers(2, m_pingpongFBO);

	m_sceneFBO = 0;
	m_sceneColourTex = 0;
	m_sceneDepthRBO = 0;
	m_brightFBO = 0;
	m_brightTex = 0;
	m_pingpongFBO[0] = 0;
	m_pingpongFBO[1] = 0;
	m_pingpongTex[0] = 0;
	m_pingpongTex[1] = 0;
	m_quadVAO = 0;
	m_quadVBO = 0;
}

// Renders the fullscreen quad used for post-processing effects like bloom.
void Game::RenderFullscreenQuad()
{
	glBindVertexArray(m_quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

// Sets all the uniform values for the player car's headlights, including transforming their world positions and directions into eye space.
void Game::ApplyHeadlightUniforms(CShaderProgram* pProgram,
	const glm::mat4& viewMatrix,
	const glm::vec3& La,
	const glm::vec3& Ld,
	const glm::vec3& Ls,
	float cutoff,
	float exponent,
	float attenuation)
{
	glm::vec3 leftHeadlightWorld = m_pCar->GetHeadlightLeftWorldPosition();
	glm::vec3 rightHeadlightWorld = m_pCar->GetHeadlightRightWorldPosition();
	glm::vec3 headlightForwardWorld = m_pCar->GetVisualForward();

	glm::mat3 viewRotation = glm::mat3(viewMatrix);

	glm::vec4 leftHeadlightEye = viewMatrix * glm::vec4(leftHeadlightWorld, 1.0f);
	glm::vec4 rightHeadlightEye = viewMatrix * glm::vec4(rightHeadlightWorld, 1.0f);
	glm::vec3 headlightDirEye = glm::normalize(viewRotation * headlightForwardWorld);

	pProgram->SetUniform("headlightLeft.position", leftHeadlightEye);
	pProgram->SetUniform("headlightLeft.direction", headlightDirEye);
	pProgram->SetUniform("headlightLeft.La", La);
	pProgram->SetUniform("headlightLeft.Ld", Ld);
	pProgram->SetUniform("headlightLeft.Ls", Ls);
	pProgram->SetUniform("headlightLeft.cutoff", cutoff);
	pProgram->SetUniform("headlightLeft.exponent", exponent);
	pProgram->SetUniform("headlightLeft.attenuation", attenuation);

	pProgram->SetUniform("headlightRight.position", rightHeadlightEye);
	pProgram->SetUniform("headlightRight.direction", headlightDirEye);
	pProgram->SetUniform("headlightRight.La", La);
	pProgram->SetUniform("headlightRight.Ld", Ld);
	pProgram->SetUniform("headlightRight.Ls", Ls);
	pProgram->SetUniform("headlightRight.cutoff", cutoff);
	pProgram->SetUniform("headlightRight.exponent", exponent);
	pProgram->SetUniform("headlightRight.attenuation", attenuation);
}

void Game::RenderBoostPads(glutil::MatrixStack& modelViewMatrixStack,
	const glm::mat4& viewMatrix,
	const glm::vec4& lightPosition1)
{
	CShaderProgram* pBoostPadProgram = (*m_pShaderPrograms)[BOOSTPAD_PROGRAM];

	for (size_t i = 0; i < m_boostPads.size(); i++)
	{
		RenderBoostPad(pBoostPadProgram,
			modelViewMatrixStack,
			m_boostPads[i].routeT,
			m_boostPads[i].lateralOffset,
			0.5f,
			viewMatrix,
			lightPosition1);
	}
}

// Renders the complete 3D scene into whichever framebuffer is currently bound.
void Game::RenderSceneToCurrentFramebuffer()
{
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glutil::MatrixStack modelViewMatrixStack;
	modelViewMatrixStack.SetIdentity();

	CShaderProgram* pMainProgram = (*m_pShaderPrograms)[MAIN_PROGRAM];
	pMainProgram->UseProgram();
	pMainProgram->SetUniform("bUseTexture", true);
	pMainProgram->SetUniform("sampler0", 0);

	int cubeMapTextureUnit = 10;
	pMainProgram->SetUniform("CubeMapTex", cubeMapTextureUnit);
	pMainProgram->SetUniform("matrices.projMatrix", m_pCamera->GetPerspectiveProjectionMatrix());

	modelViewMatrixStack.LookAt(m_pCamera->GetPosition(), m_pCamera->GetView(), m_pCamera->GetUpVector());
	glm::mat4 viewMatrix = modelViewMatrixStack.Top();

	glm::vec4 lightPosition1 = glm::vec4(-100, 100, -100, 1);

	pMainProgram->SetUniform("light1.position", viewMatrix * lightPosition1);
	pMainProgram->SetUniform("light1.La", glm::vec3(0.4f));
	pMainProgram->SetUniform("light1.Ld", glm::vec3(0.8f));
	pMainProgram->SetUniform("light1.Ls", glm::vec3(0.8f));
	pMainProgram->SetUniform("material1.Ma", glm::vec3(1.0f));
	pMainProgram->SetUniform("material1.Md", glm::vec3(1.0f));
	pMainProgram->SetUniform("material1.Ms", glm::vec3(0.4f));
	pMainProgram->SetUniform("material1.shininess", 1.0f);
	pMainProgram->SetUniform("skyboxBrightness", 0.5f);

	// Set headlight uniforms for the main shader
	ApplyHeadlightUniforms(pMainProgram,
		viewMatrix,
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(2000.0f),
		glm::vec3(10.0f),
		35.0f,
		2.0f,
		1.0f);

	// Skybox
	modelViewMatrixStack.Push();
	{
		pMainProgram->SetUniform("renderSkybox", true);
		glm::vec3 vEye = m_pCamera->GetPosition();
		modelViewMatrixStack.Translate(vEye);
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		m_pSkybox->Render(cubeMapTextureUnit);
		pMainProgram->SetUniform("renderSkybox", false);
	}
	modelViewMatrixStack.Pop();

	// Route
	CShaderProgram* pRouteProgram = (*m_pShaderPrograms)[ROUTE_PROGRAM];
	pRouteProgram->UseProgram();
	pRouteProgram->SetUniform("sampler0", 0);
	pRouteProgram->SetUniform("bUseTexture", true);
	pRouteProgram->SetUniform("matrices.projMatrix", m_pCamera->GetPerspectiveProjectionMatrix());

	pRouteProgram->SetUniform("light1.position", viewMatrix * lightPosition1);
	pRouteProgram->SetUniform("light1.La", glm::vec3(1.0f));
	pRouteProgram->SetUniform("light1.Ld", glm::vec3(1.0f));
	pRouteProgram->SetUniform("light1.Ls", glm::vec3(1.0f));
	pRouteProgram->SetUniform("material1.Ma", glm::vec3(0.5f));
	pRouteProgram->SetUniform("material1.Md", glm::vec3(0.0f));
	pRouteProgram->SetUniform("material1.Ms", glm::vec3(1.0f));
	pRouteProgram->SetUniform("material1.shininess", 32.0f);

	modelViewMatrixStack.Push();
	{
		modelViewMatrixStack *= m_pCameraPath->GetTransform();
		pRouteProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pRouteProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		m_pCameraPath->RenderTrack();
	}
	modelViewMatrixStack.Pop();

	// Earth mesh
	pMainProgram->UseProgram();
	pMainProgram->SetUniform("bUseTexture", true);
	pMainProgram->SetUniform("sampler0", 0);

	modelViewMatrixStack.Push();
	{
		modelViewMatrixStack.Translate(glm::vec3(20.0f, 0.0f, 130.0f));   // position
		modelViewMatrixStack.Rotate(glm::vec3(0.0f, 1.0f, 0.0f), 45.0f);   // rotation
		modelViewMatrixStack.Scale(25.0f);                                  // scale

		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));

		m_pEarthMesh->Render();
	}
	modelViewMatrixStack.Pop();

	// Space station
	modelViewMatrixStack.Push();
	{
		modelViewMatrixStack.Translate(glm::vec3(44.0f, 10.0f, 61.0f));   // position
		modelViewMatrixStack.Rotate(glm::vec3(0.0f, 1.0f, 0.0f), 45.0f);   // rotation
		modelViewMatrixStack.Scale(3.5f);                                  // scale

		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));

		m_pSpaceStationMesh->Render();
	}
	modelViewMatrixStack.Pop();

	// Asteroid
	glm::vec3 asteroidPositions[] =
	{
		glm::vec3(184.0f, 10.0f, -68.0f),
		glm::vec3(184.0f, 10.0f, 13.0f),
		glm::vec3(260.0f, 10.0f, -34.0f),
		glm::vec3(240.f, 10.0f, -130.0f),
		glm::vec3(160.0f, 10.0f, -140.0f),
		glm::vec3(110.0f, 10.0f, -100.0f),
		glm::vec3(58.0f, 10.0f, -34.0f),
		glm::vec3(-50.0f, 10.0f, -110.0f),
		glm::vec3(-5.0f, 8.0f, -80.0f),
		glm::vec3(-25.0f, 8.0f, -105.0f),
		glm::vec3(-77.0f, 8.0f, -134.0f),
		glm::vec3(-62.0f, 8.0f, -60.0f),
		glm::vec3(-134.0f, 8.0f, -80.0f),
		glm::vec3(-116.0f, 8.0f, 2.8f),
		glm::vec3(-60.0f, 8.0f, 37.8f),
	};

	const int asteroidCount = sizeof(asteroidPositions) / sizeof(asteroidPositions[0]);
	for (int i = 0; i < asteroidCount; i++)
	{
		modelViewMatrixStack.Push();
		{
			modelViewMatrixStack.Translate(asteroidPositions[i]);
			modelViewMatrixStack.Scale(2.0f);

			pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
			pMainProgram->SetUniform("matrices.normalMatrix",
				m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));

			m_pAsteroidsMesh->Render();
		}
		modelViewMatrixStack.Pop();
	}

	// Start line

	modelViewMatrixStack.Push();
	{
		modelViewMatrixStack.Translate(glm::vec3(85.0f, 1.0f, 3.0f));
		modelViewMatrixStack.Rotate(glm::vec3(0.0f, 1.0f, 0.0f), 90.1f);
		modelViewMatrixStack.Scale(0.8f);

		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));

		m_pStartLine->Render();
	}
	modelViewMatrixStack.Pop();

	// Car
	RenderCar(modelViewMatrixStack,
		(*m_pShaderPrograms)[CAR_PROGRAM],
		(*m_pShaderPrograms)[PARTICLE_PROGRAM],
		m_pCamera);

	// Boost pads
	RenderBoostPads(modelViewMatrixStack, viewMatrix, lightPosition1);
}

void Game::Render()
{
	// Pass 1: render scene to texture
	glBindFramebuffer(GL_FRAMEBUFFER, m_sceneFBO);
	glViewport(0, 0, m_windowWidth, m_windowHeight);
	RenderSceneToCurrentFramebuffer();

	// Pass 2: extract bright fragments for bloom
	glBindFramebuffer(GL_FRAMEBUFFER, m_brightFBO);
	glViewport(0, 0, m_windowWidth, m_windowHeight);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glClear(GL_COLOR_BUFFER_BIT);

	CShaderProgram* pBrightProgram = (*m_pShaderPrograms)[BRIGHT_PROGRAM];
	pBrightProgram->UseProgram();
	pBrightProgram->SetUniform("sceneTex", 0);
	pBrightProgram->SetUniform("threshold", 1.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_sceneColourTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	RenderFullscreenQuad();

	// Pass 3: apply separable Gaussian blur using ping-pong framebuffers
	CShaderProgram* pBlurProgram = (*m_pShaderPrograms)[BLUR_PROGRAM];
	pBlurProgram->UseProgram();
	pBlurProgram->SetUniform("imageTex", 0);

	bool horizontal = true;
	bool firstPass = true;
	const int blurPasses = 40;

	for (int i = 0; i < blurPasses; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_pingpongFBO[horizontal ? 1 : 0]);
		glViewport(0, 0, m_windowWidth, m_windowHeight);
		glClear(GL_COLOR_BUFFER_BIT);

		pBlurProgram->SetUniform("horizontal", horizontal ? 1.0f : 0.0f);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, firstPass ? m_brightTex : m_pingpongTex[horizontal ? 0 : 1]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

		RenderFullscreenQuad();

		horizontal = !horizontal;
		if (firstPass)
			firstPass = false;
	}

	// Pass 4: combine the blurred bloom texture with the original scene
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, m_windowWidth, m_windowHeight);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glClear(GL_COLOR_BUFFER_BIT);

	CShaderProgram* pCombineProgram = (*m_pShaderPrograms)[COMBINE_PROGRAM];
	pCombineProgram->UseProgram();
	pCombineProgram->SetUniform("sceneTex", 0);
	pCombineProgram->SetUniform("bloomTex", 1);
	pCombineProgram->SetUniform("bloomStrength", 1.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_sceneColourTex);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_pingpongTex[horizontal ? 0 : 1]);

	RenderFullscreenQuad();

	RenderHUD();
	SwapBuffers(m_gameWindow.Hdc());
}

void Game::RenderBoostPad(CShaderProgram* pBoostPadProgram,
	glutil::MatrixStack& modelViewMatrixStack,
	float routeT,
	float lateralOffset,
	float scale,
	const glm::mat4& viewMatrix,
	const glm::vec4& lightPosition1)
{
	pBoostPadProgram->UseProgram();

	pBoostPadProgram->SetUniform("matrices.projMatrix", m_pCamera->GetPerspectiveProjectionMatrix());
	pBoostPadProgram->SetUniform("light1.position", viewMatrix * lightPosition1);
	pBoostPadProgram->SetUniform("light1.La", glm::vec3(1.0f));
	pBoostPadProgram->SetUniform("light1.Ld", glm::vec3(1.0f));
	pBoostPadProgram->SetUniform("light1.Ls", glm::vec3(1.0f));

	modelViewMatrixStack.Push();
	{
		glm::vec3 centre, T, N, B;
		m_pCameraPath->GetTrackFrame(routeT, centre, T, N, B);

		glm::vec3 padPosition = centre + lateralOffset * N + 0.08f * B;

		glm::mat4 padOrientation = glm::mat4(glm::mat3(T, B, -N));

		modelViewMatrixStack.Translate(padPosition);
		modelViewMatrixStack *= padOrientation;
		modelViewMatrixStack.Scale(scale);

		pBoostPadProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pBoostPadProgram->SetUniform("matrices.normalMatrix",
			m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));

		// Panel
		pBoostPadProgram->SetUniform("bUseTexture", true);
		pBoostPadProgram->SetUniform("bEmissiveArrows", false);
		pBoostPadProgram->SetUniform("sampler0", 0);
		pBoostPadProgram->SetUniform("material1.Ma", glm::vec3(0.8f));
		pBoostPadProgram->SetUniform("material1.Md", glm::vec3(0.8f));
		pBoostPadProgram->SetUniform("material1.Ms", glm::vec3(0.15f));
		pBoostPadProgram->SetUniform("material1.shininess", 8.0f);
		m_pBoostPad->RenderPanel();

		// Arrows
		pBoostPadProgram->SetUniform("bUseTexture", false);
		pBoostPadProgram->SetUniform("bEmissiveArrows", true);
		pBoostPadProgram->SetUniform("material1.Ma", glm::vec3(1.0f));
		pBoostPadProgram->SetUniform("material1.Md", glm::vec3(0.6f));
		pBoostPadProgram->SetUniform("material1.Ms", glm::vec3(0.0f));
		pBoostPadProgram->SetUniform("material1.shininess", 1.0f);
		m_pBoostPad->RenderArrows();
	}
	modelViewMatrixStack.Pop();
}

void Game::RenderCar(glutil::MatrixStack& modelViewMatrixStack,
	CShaderProgram* pCarProgram,
	CShaderProgram* pParticleProgram,
	CCamera* pCamera)
{
	pCarProgram->UseProgram();
	pCarProgram->SetUniform("matrices.projMatrix", pCamera->GetPerspectiveProjectionMatrix());
	pCarProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
	pCarProgram->SetUniform("matrices.normalMatrix", pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));

	// Main light
	pCarProgram->SetUniform("light1.position", glm::vec4(0.0f, 100.0f, 200.0f, 1.0f));
	pCarProgram->SetUniform("light1.La", glm::vec3(0.3f, 0.3f, 0.3f));
	pCarProgram->SetUniform("light1.Ld", glm::vec3(1.0f, 1.0f, 1.0f));
	pCarProgram->SetUniform("light1.Ls", glm::vec3(1.0f, 1.0f, 1.0f));

	// Material
	pCarProgram->SetUniform("material1.Ma", glm::vec3(1.0f, 1.0f, 1.0f));
	pCarProgram->SetUniform("material1.Md", glm::vec3(1.0f, 1.0f, 1.0f));
	pCarProgram->SetUniform("material1.Ms", glm::vec3(0.8f, 0.8f, 0.8f));
	pCarProgram->SetUniform("material1.shininess", 64.0f);

	pCarProgram->SetUniform("sampler0", 0);
	pCarProgram->SetUniform("bUseTexture", true);

	glm::mat4 viewMatrix = pCamera->GetViewMatrix();

	ApplyHeadlightUniforms(pCarProgram,
		viewMatrix,
		glm::vec3(0.0f),
		glm::vec3(10.0f),
		glm::vec3(0.0f),
		60.0f,
		10.0f,
		1.0f);

	m_pCar->Render(modelViewMatrixStack, pCarProgram, pParticleProgram, pCamera);
}

// Draws a simple translucent screen-space rectangle used behind HUD text.
void Game::DrawHUDBox(float x, float y, float w, float h, const glm::vec4& colour)
{
	static GLuint vao = 0;
	static GLuint vbo = 0;

	if (vao == 0)
	{
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

		glBindVertexArray(0);
	}

	CShaderProgram* pHUD = (*m_pShaderPrograms)[HUD_PROGRAM];
	pHUD->UseProgram();

	// Direct screen-space quad
	float vertices[] =
	{
		x,     y,
		x + w, y,
		x + w, y + h,

		x,     y,
		x + w, y + h,
		x,     y + h
	};

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

	// Identity matrices (we are already in screen space)
	pHUD->SetUniform("projMatrix", m_pCamera->GetOrthographicProjectionMatrix());
	pHUD->SetUniform("modelViewMatrix", glm::mat4(1.0f));
	pHUD->SetUniform("vColour", colour);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glDisable(GL_BLEND);
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
	Game& game = Game::GetInstance();
	game.SetHinstance(hinstance);
	return int(game.Execute());
}