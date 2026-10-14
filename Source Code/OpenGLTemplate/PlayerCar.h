#pragma once

#include "Common.h"
#include "Car.h"

class CCameraPath;

// CPlayerCar
// ----------
// Handles player-controlled motion, route attachment, boost behaviour,
// off-track respawning, and transfer of gameplay state to the visual car mesh.
class CPlayerCar
{
public:
	CPlayerCar();
	~CPlayerCar();

	// Initialises the player controller with the rendered car object and the route.
	void Initialise(CCar* pCar, CCameraPath* pRoute);

	// Updates input, movement, route contact, boost state, and visual car state.
	void Update(float dt);

	// Returns the current player world position.
	glm::vec3 GetPosition() const;

	// Returns the current forward direction based on the player's yaw.
	glm::vec3 GetForward() const;

	// Activates a temporary movement boost.
	void ActivateBoost();

	// Finds the closest sampled frame on the route to the player's current position.
	bool FindClosestTrackFrame(glm::vec3& centre,
		glm::vec3& tangent,
		glm::vec3& normal,
		glm::vec3& binormal) const;

private:
	// Reads keyboard input and converts it into throttle and steering values.
	void ReadInput(float dtSeconds);

	// Updates velocity, turning, drag, gravity, and position.
	void Integrate(float dtSeconds);

	// Resolves whether the player is supported by the route and snaps the car to the track when valid.
	void ResolveGroundContact(float dtSeconds);

	// Moves the player back to the most recent safe position on the route.
	void RespawnToLastSafeFrame();

	// Pushes the gameplay transform and animation state into the rendered car object.
	void UpdateCarVisuals();

private:
	CCar* m_pCar;
	CCameraPath* m_pRoute;

	// Physical state
	glm::vec3 m_position;
	glm::vec3 m_velocity;

	float m_yawDegrees;
	float m_pitchDegrees;
	float m_rollDegrees;

	// Current player input state
	float m_throttleInput;
	float m_steerInput;

	// Visual steering only, used for the front wheels
	float m_visualSteerAngle;
	float m_maxVisualSteerAngle;
	float m_visualSteerSpeed;

	// Visual body yaw offset used to exaggerate turning
	float m_visualBodyYawOffset;
	float m_maxVisualBodyYawOffset;
	float m_visualBodyYawSpeed;

	// Movement tuning values
	float m_engineForce;
	float m_brakeForce;
	float m_drag;
	float m_maxSpeed;
	float m_turnSpeed;
	float m_gravity;

	float m_carBaseOffset;
	bool m_grounded;

	// Temporary boost state
	bool m_boostActive;
	float m_boostTimer;
	float m_originalMaxSpeed;
	float m_originalEngineForce;

	// Off-track detection and delayed respawn
	float m_offTrackTimer;
	float m_respawnDelay;

	// Last safe route frame used for respawning
	bool m_hasLastSafeFrame;
	glm::vec3 m_lastSafeCentre;
	glm::vec3 m_lastSafeTangent;
	glm::vec3 m_lastSafeNormal;
	glm::vec3 m_lastSafeBinormal;
	glm::vec3 m_currentTangent;
	glm::vec3 m_currentNormal;
	glm::vec3 m_currentBinormal;
};