#pragma once

#include "Common.h"
#include "GameWindow.h"
#include "MatrixStack.h"

class CCamera;
class CSkybox;
class CShader;
class CShaderProgram;
class CFreeTypeFont;
class CHighResolutionTimer;
class COpenAssetImportMesh;
class CAudio;
class CCameraPath;
class CCar;
class CPlayerCar;
class CStartLine;
class CBoostPad;

// Available camera modes used by the game and debug workflow.
enum CameraMode
{
	FREE_VIEW = 0,
	START_VIEW,
	THIRD_PERSON,
	TOP_VIEW
};

// Indices used to access shader programs after loading and linking.
enum ShaderProgramIndex
{
	MAIN_PROGRAM = 0,
	FONT_PROGRAM,
	CAR_PROGRAM,
	ROUTE_PROGRAM,
	BRIGHT_PROGRAM,
	BLUR_PROGRAM,
	COMBINE_PROGRAM,
	BOOSTPAD_PROGRAM,
	PARTICLE_PROGRAM,
	HUD_PROGRAM
};

// World-space placement for each boost pad instance.
struct BoostPadInstance
{
	float routeT;
	float lateralOffset;
	glm::vec3 position;
};

// Game
// ----
// Main application class responsible for initialisation, update, rendering,
// input handling, gameplay state, and post-processing.
class Game
{
private:
	// Core game loop methods
	void Initialise();
	void Update();
	void Render();
	void GameLoop();
	void RenderHUD();

	// Initialisation helpers
	void CreateCoreObjects();
	void SetupProjectionMatrices(int width, int height);
	void LoadShaderPrograms();
	void CreateSceneObjects();
	void CreateGameplayObjects();
	void InitialiseAudio();

	// Update helpers
	void UpdatePlayerAndCamera();
	void CheckBoostPadCollisions();

	// Rendering helpers
	void RenderSceneToCurrentFramebuffer();
	void RenderFullscreenQuad();
	void RenderBoostPads(glutil::MatrixStack& modelViewMatrixStack,
		const glm::mat4& viewMatrix,
		const glm::vec4& lightPosition1);
	void RenderBoostPad(CShaderProgram* pBoostPadProgram,
		glutil::MatrixStack& modelViewMatrixStack,
		float routeT,
		float lateralOffset,
		float scale,
		const glm::mat4& viewMatrix,
		const glm::vec4& lightPosition1);
	void RenderCar(glutil::MatrixStack& modelViewMatrixStack,
		CShaderProgram* pCarProgram,
		CShaderProgram* pParticleProgram,
		CCamera* pCamera);
	void DrawHUDBox(float x, float y, float w, float h, const glm::vec4& colour);

	// Gameplay helpers
	void UpdateRaceLogic();
	void CheckFinishLineCrossing();

	// Bloom / post-processing helpers
	void InitialiseBloom(int width, int height);
	void ReleaseBloom();
	void ApplyHeadlightUniforms(CShaderProgram* pProgram,
		const glm::mat4& viewMatrix,
		const glm::vec3& La,
		const glm::vec3& Ld,
		const glm::vec3& Ls,
		float cutoff,
		float exponent,
		float attenuation);

public:
	Game();
	~Game();

	static Game& GetInstance();

	LRESULT ProcessEvents(HWND window, UINT message, WPARAM w_param, LPARAM l_param);
	void SetHinstance(HINSTANCE hinstance);
	WPARAM Execute();

private:
	static const int FPS = 60;

	GameWindow m_gameWindow;
	HINSTANCE m_hInstance;

	// Core scene objects and systems
	CSkybox* m_pSkybox;
	CCamera* m_pCamera;
	std::vector<CShaderProgram*>* m_pShaderPrograms;
	CFreeTypeFont* m_pFtFont;
	COpenAssetImportMesh* m_pSpaceStationMesh;
	COpenAssetImportMesh* m_pEarthMesh;
	COpenAssetImportMesh* m_pAsteroidsMesh;
	CHighResolutionTimer* m_pHighResolutionTimer;
	CAudio* m_pAudio;
	CCameraPath* m_pCameraPath;
	CCar* m_pCar;
	CPlayerCar* m_pPlayerCar;
	CStartLine* m_pStartLine;
	CBoostPad* m_pBoostPad;

	// Current and previous camera states used for gameplay and pause/start screens
	CameraMode m_cameraMode;
	CameraMode m_prevCameraMode;

	// Off-screen framebuffer resources used for bloom post-processing
	GLuint m_sceneFBO;
	GLuint m_sceneColourTex;
	GLuint m_sceneDepthRBO;

	GLuint m_brightFBO;
	GLuint m_brightTex;

	GLuint m_pingpongFBO[2];
	GLuint m_pingpongTex[2];

	GLuint m_quadVAO;
	GLuint m_quadVBO;

	// Frame timing and window activity state
	double m_dt;
	int m_framesPerSecond;
	bool m_appActive;
	int m_frameCount;
	double m_elapsedTime;

	// Race and gameplay state
	int m_currentLap;
	float m_countdownTime;
	float m_timeGivenForLap;
	bool m_raceStarted;
	bool m_raceFinished;
	bool m_playingGame;

	// Two-line finish gate used to detect forward lap completion
	glm::vec2 m_lineA1, m_lineA2;
	glm::vec2 m_lineB1, m_lineB2;
	glm::vec2 m_prevCarXZ;
	bool m_crossedA;

	// Cached window dimensions used for projection and post-processing
	int m_windowWidth;
	int m_windowHeight;

	// Placed gameplay objects
	std::vector<BoostPadInstance> m_boostPads;
};