#pragma once

#include "Common.h"
#include "OpenAssetImportMesh.h"
#include "MatrixStack.h"
#include "Shaders.h"
#include "Camera.h"
#include <vector>

// CCar
// ----
// Renders the player car as a multi-part mesh object consisting of a body and four wheels.
// The class also manages a simple tyre particle effect used to visualise skidding/steering.
class CCar
{
public:
	CCar();
	~CCar();

	// Loads the car body and wheel meshes from the given folder and initialises
	// the particle rendering buffers.
	bool Initialise(const std::string& folder);

	// Updates wheel animation and particle simulation.
	void Update(float dt);

	// Renders the car using hierarchical transforms and then renders the tyre particles.
	void Render(glutil::MatrixStack& modelViewMatrixStack,
		CShaderProgram* pCarProgram,
		CShaderProgram* pParticleProgram,
		CCamera* pCamera);

	// Releases loaded meshes and particle GPU resources.
	void Release();

	// Setters used by the gameplay/controller code to drive the visual state of the car.
	void SetPosition(const glm::vec3& position);
	void SetScale(float scale);
	void SetYaw(float yawDegrees);
	void SetSpeed(float speed);
	void SetSteeringAngle(float steeringAngleDegrees);
	void SetBodyYawOffset(float offsetDegrees);
	void SetPitch(float pitchDegrees);
	void SetRoll(float rollDegrees);
	void SetTrackFrame(const glm::vec3& tangent, const glm::vec3& normal, const glm::vec3& binormal);

	// Returns world-space positions used for dynamic headlights.
	glm::vec3 GetHeadlightLeftWorldPosition() const;
	glm::vec3 GetHeadlightRightWorldPosition() const;

	// Returns the current forward direction of the visually rendered car.
	glm::vec3 GetVisualForward() const;

private:
	// A single particle used for tyre smoke.
	struct STireParticle
	{
		glm::vec3 position;
		glm::vec3 velocity;
		float life;
		float maxLife;
		glm::vec4 colour;
	};

	// Renders one mesh using the currently built model-view matrix.
	void RenderMesh(COpenAssetImportMesh* pMesh,
		glutil::MatrixStack& modelViewMatrixStack,
		CShaderProgram* pShaderProgram,
		CCamera* pCamera);

	// Particle system helpers
	void InitialiseParticleBuffers();
	void ReleaseParticleBuffers();
	void EmitTireParticles();
	void UpdateParticles(float dtSeconds);
	void RenderParticles(CShaderProgram* pParticleProgram, CCamera* pCamera);

	// Helper positions used for tyre particle emission.
	glm::vec3 GetRearLeftWheelWorldPosition() const;
	glm::vec3 GetRearRightWheelWorldPosition() const;

	// Returns a random float in the given interval.
	float RandomFloat(float minValue, float maxValue) const;

private:
	// Car meshes
	COpenAssetImportMesh* m_pBodyMesh;
	COpenAssetImportMesh* m_pWheelFLMesh;
	COpenAssetImportMesh* m_pWheelFRMesh;
	COpenAssetImportMesh* m_pWheelBLMesh;
	COpenAssetImportMesh* m_pWheelBRMesh;

	// Visual transform and motion state
	glm::vec3 m_position;
	float m_scale;
	float m_yaw;
	float m_pitch;
	float m_roll;

	float m_speed;
	float m_steeringAngle;
	float m_wheelSpinAngle;
	float m_bodyYawOffset;

	// Wheel offsets in the car's local coordinate system
	glm::vec3 m_wheelOffsetFL;
	glm::vec3 m_wheelOffsetFR;
	glm::vec3 m_wheelOffsetBL;
	glm::vec3 m_wheelOffsetBR;

	// Track frame vectors used for orienting the car to the track and for particle emission.
	glm::vec3 m_trackTangent;
	glm::vec3 m_trackNormal;
	glm::vec3 m_trackBinormal;

	// Tyre smoke particles
	std::vector<STireParticle> m_tireParticles;

	// GPU resources for particle rendering
	GLuint m_particleVAO;
	GLuint m_particleVBO;
};