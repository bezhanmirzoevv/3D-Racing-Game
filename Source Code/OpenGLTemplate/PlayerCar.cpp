#include "PlayerCar.h"
#include <algorithm>
#include <cfloat>
#include "CameraPath.h"

CPlayerCar::CPlayerCar()
{
	m_pCar = NULL;
	m_pRoute = NULL;

	m_position = glm::vec3(0.0f, 0.0f, 0.0f);
	m_velocity = glm::vec3(0.0f, 0.0f, 0.0f);

	m_yawDegrees = 120.0f;
	m_pitchDegrees = 0.0f;
	m_rollDegrees = 0.0f;

	m_throttleInput = 0.0f;
	m_steerInput = 0.0f;

	// Visual steering for wheel animation only
	m_visualSteerAngle = 0.0f;
	m_maxVisualSteerAngle = 40.0f;
	m_visualSteerSpeed = 40.0f;

	// Visual body yaw offset used to exaggerate steering
	m_visualBodyYawOffset = 0.0f;
	m_maxVisualBodyYawOffset = 30.0f;
	m_visualBodyYawSpeed = 10.0f;

	// Movement tuning
	m_engineForce = 75.0f;
	m_brakeForce = 75.0f;
	m_drag = 2.5f;
	m_maxSpeed = 150.0f;
	m_turnSpeed = 120.0f;
	m_gravity = 35.0f;

	m_carBaseOffset = 1.0f;
	m_grounded = false;

	// Boost state
	m_boostActive = false;
	m_boostTimer = 0.0f;
	m_originalMaxSpeed = m_maxSpeed;
	m_originalEngineForce = m_engineForce;

	// Off-track / respawn state
	m_offTrackTimer = 0.0f;
	m_respawnDelay = 1.5f;

	m_hasLastSafeFrame = false;
	m_lastSafeCentre = glm::vec3(0.0f);
	m_lastSafeTangent = glm::vec3(0.0f, 0.0f, 1.0f);
	m_lastSafeNormal = glm::vec3(0.0f, 1.0f, 0.0f);
	m_lastSafeBinormal = glm::vec3(1.0f, 0.0f, 0.0f);
	m_currentTangent = glm::vec3(0.0f, 0.0f, 1.0f);
	m_currentNormal = glm::vec3(1.0f, 0.0f, 0.0f);
	m_currentBinormal = glm::vec3(0.0f, 1.0f, 0.0f);
}

CPlayerCar::~CPlayerCar()
{}

void CPlayerCar::Initialise(CCar* pCar, CCameraPath* pRoute)
{
	m_pCar = pCar;
	m_pRoute = pRoute;

	// Initial position near the start of the route.
	m_position = glm::vec3(65.f, 1.0f, 15.0f);
	m_velocity = glm::vec3(0.0f, 0.0f, 0.0f);

	m_yawDegrees = 120.0f;
	m_pitchDegrees = 0.0f;
	m_rollDegrees = 0.0f;

	m_throttleInput = 0.0f;
	m_steerInput = 0.0f;
	m_visualSteerAngle = 0.0f;

	m_grounded = true;
	m_offTrackTimer = 0.0f;
	m_hasLastSafeFrame = false;

	// Save an initial safe frame from the route if one is available.
	glm::vec3 centre, tangent, normal, binormal;
	if (FindClosestTrackFrame(centre, tangent, normal, binormal))
	{
		m_lastSafeCentre = centre;
		m_lastSafeTangent = tangent;
		m_lastSafeNormal = normal;
		m_lastSafeBinormal = binormal;

		m_currentTangent = glm::normalize(tangent);
		m_currentNormal = glm::normalize(normal);
		m_currentBinormal = glm::normalize(binormal);

		m_hasLastSafeFrame = true;
	}

	UpdateCarVisuals();
}

void CPlayerCar::Update(float dt)
{
	if (!m_pCar)
		return;

	float dtSeconds = dt / 1000.0f;

	// Update temporary boost duration and restore original movement values when it ends.
	if (m_boostActive)
	{
		m_boostTimer -= dtSeconds;

		if (m_boostTimer <= 0.0f)
		{
			m_boostActive = false;
			m_maxSpeed = m_originalMaxSpeed;
			m_engineForce = m_originalEngineForce;
		}
	}

	ReadInput(dtSeconds);
	Integrate(dtSeconds);
	ResolveGroundContact(dtSeconds);
	UpdateCarVisuals();

	m_pCar->Update(dt);
}

void CPlayerCar::ReadInput(float dtSeconds)
{
	m_throttleInput = 0.0f;
	m_steerInput = 0.0f;

	if ((GetKeyState('W') & 0x80) || (GetKeyState(VK_UP) & 0x80))
		m_throttleInput = 1.0f;

	if ((GetKeyState('S') & 0x80) || (GetKeyState(VK_DOWN) & 0x80))
		m_throttleInput = -1.0f;

	if ((GetKeyState('A') & 0x80) || (GetKeyState(VK_LEFT) & 0x80))
		m_steerInput = 1.0f;

	if ((GetKeyState('D') & 0x80) || (GetKeyState(VK_RIGHT) & 0x80))
		m_steerInput = -1.0f;

	// Smooth the wheel steering angle for a more natural visual animation.
	float targetVisualSteer = m_steerInput * m_maxVisualSteerAngle;
	m_visualSteerAngle += (targetVisualSteer - m_visualSteerAngle) * m_visualSteerSpeed * dtSeconds;
	m_visualSteerAngle = glm::clamp(m_visualSteerAngle, -m_maxVisualSteerAngle, m_maxVisualSteerAngle);

	// Smooth the body yaw offset as a purely visual turning effect.
	float targetBodyYawOffset = m_steerInput * m_maxVisualBodyYawOffset;
	m_visualBodyYawOffset += (targetBodyYawOffset - m_visualBodyYawOffset) * m_visualBodyYawSpeed * dtSeconds;
}

void CPlayerCar::Integrate(float dtSeconds)
{
	float yawRadians = glm::radians(m_yawDegrees);

	glm::vec3 forward = glm::normalize(glm::vec3(sinf(yawRadians), 0.0f, cosf(yawRadians)));
	glm::vec3 right = glm::normalize(glm::vec3(forward.z, 0.0f, -forward.x));

	float forwardSpeed = glm::dot(glm::vec3(m_velocity.x, 0.0f, m_velocity.z), forward);

	// Only allow steering when there is meaningful forward/backward motion.
	if (fabs(forwardSpeed) > 1.0f)
	{
		m_yawDegrees += m_steerInput * m_turnSpeed * dtSeconds;
	}

	yawRadians = glm::radians(m_yawDegrees);
	forward = glm::normalize(glm::vec3(sinf(yawRadians), 0.0f, cosf(yawRadians)));
	right = glm::normalize(glm::vec3(forward.z, 0.0f, -forward.x));

	glm::vec3 acceleration(0.0f);

	if (m_throttleInput > 0.0f)
	{
		acceleration += m_engineForce * forward;
	}
	else if (m_throttleInput < 0.0f)
	{
		glm::vec3 horizontalVelocity(m_velocity.x, 0.0f, m_velocity.z);
		float horizontalSpeed = glm::length(horizontalVelocity);

		if (horizontalSpeed > 0.01f)
			acceleration += -m_brakeForce * glm::normalize(horizontalVelocity);
	}

	// Apply drag and gravity.
	acceleration += -m_drag * glm::vec3(m_velocity.x, 0.0f, m_velocity.z);
	acceleration += glm::vec3(0.0f, -m_gravity, 0.0f);

	m_velocity += acceleration * dtSeconds;

	// Reduce lateral drift relative to the car's facing direction.
	glm::vec3 horizontalVelocity(m_velocity.x, 0.0f, m_velocity.z);
	glm::vec3 lateral = right * glm::dot(horizontalVelocity, right);
	horizontalVelocity -= 6.0f * lateral * dtSeconds;

	m_velocity.x = horizontalVelocity.x;
	m_velocity.z = horizontalVelocity.z;

	// Clamp horizontal speed.
	float horizontalSpeed = glm::length(glm::vec2(m_velocity.x, m_velocity.z));
	if (horizontalSpeed > m_maxSpeed)
	{
		glm::vec2 horiz = glm::normalize(glm::vec2(m_velocity.x, m_velocity.z)) * m_maxSpeed;
		m_velocity.x = horiz.x;
		m_velocity.z = horiz.y;
	}

	m_position += m_velocity * dtSeconds;
}

void CPlayerCar::ResolveGroundContact(float dtSeconds)
{
	// Once the car has left the supported road region, do not attach it to a new
	// road sample until it has been respawned.
	if (!m_grounded)
	{
		m_offTrackTimer += dtSeconds;

		if (m_position.y < -50.0f || m_offTrackTimer >= m_respawnDelay)
		{
			RespawnToLastSafeFrame();
		}

		return;
	}

	glm::vec3 centre, tangent, normal, binormal;

	if (!FindClosestTrackFrame(centre, tangent, normal, binormal))
	{
		m_grounded = false;
		m_offTrackTimer = 0.0f;
		return;
	}

	glm::vec3 d = m_position - centre;

	float lateral = glm::dot(d, normal);
	float vertical = glm::dot(d, binormal);

	float trackHalfWidth = m_pRoute->GetTrackHalfWidth();
	float supportedHeight = m_carBaseOffset;

	float lateralTolerance = 2.0f;
	float maxSnapAbove = 3.0f;
	float maxSnapBelow = 1.0f;

	bool withinWidth = fabs(lateral) <= (trackHalfWidth + lateralTolerance);
	bool nearSurface = (vertical >= supportedHeight - maxSnapBelow &&
		vertical <= supportedHeight + maxSnapAbove);

	// Leaving the supported region means the player is now off-track.
	if (!withinWidth || !nearSurface)
	{
		m_grounded = false;
		m_offTrackTimer = 0.0f;
		return;
	}

	// Snap the player onto the route surface.
	m_position += binormal * (supportedHeight - vertical);

	// Remove downward motion into the route surface.
	float vn = glm::dot(m_velocity, binormal);
	if (vn < 0.0f)
		m_velocity -= vn * binormal;

	m_grounded = true;
	m_offTrackTimer = 0.0f;

	// Save the current valid frame so it can be used for respawn.
	m_lastSafeCentre = centre;
	m_lastSafeTangent = glm::normalize(tangent);
	m_lastSafeNormal = glm::normalize(normal);
	m_lastSafeBinormal = glm::normalize(binormal);
	m_hasLastSafeFrame = true;

	m_currentTangent = glm::normalize(tangent);
	m_currentNormal = glm::normalize(normal);
	m_currentBinormal = glm::normalize(binormal);

	// Estimate pitch from the route tangent so the car matches the slope visually.
	glm::vec3 flatTangent (tangent.x, 0.0f, tangent.z);
	float slopePitch = 0.0f;

	if (glm::length(flatTangent) > 0.0001f)
	{
		float y = glm::clamp(glm::normalize(tangent).y, -1.0f, 1.0f);
		slopePitch = glm::degrees(asinf(y));
	}
	m_pitchDegrees = slopePitch;

	// Estimate roll from the banked up vector.
	float yawRadians = glm::radians(m_yawDegrees);

	glm::vec3 carForward = glm::normalize(glm::vec3(
		sinf(yawRadians),
		0.0f,
		cosf(yawRadians)
	));

	glm::vec3 carRight = glm::normalize(glm::vec3(
		carForward.z,
		0.0f,
		-carForward.x
	));

	float targetRoll = -glm::degrees(asinf(glm::clamp(glm::dot(binormal, carRight), -1.0f, 1.0f)));

	float rollSmoothSpeed = 8.0f;
	m_rollDegrees += (targetRoll - m_rollDegrees) * rollSmoothSpeed * dtSeconds;
}

bool CPlayerCar::FindClosestTrackFrame(glm::vec3& centre,
	glm::vec3& tangent,
	glm::vec3& normal,
	glm::vec3& binormal) const
{
	if (!m_pRoute)
		return false;

	const int samples = 800;
	float bestDist2 = FLT_MAX;

	// Sample the spline route densely and keep the closest frame to the player.
	for (int i = 0; i < samples; i++)
	{
		float t = (float)i / (float)samples;

		glm::vec3 p, T, N, B;
		m_pRoute->GetTrackFrame(t, p, T, N, B);

		glm::vec3 diff = m_position - p;
		float d2 = glm::dot(diff, diff);

		if (d2 < bestDist2)
		{
			bestDist2 = d2;
			centre = p;
			tangent = glm::normalize(T);
			normal = glm::normalize(N);
			binormal = glm::normalize(B);
		}
	}

	return true;
}

void CPlayerCar::RespawnToLastSafeFrame()
{
	if (!m_hasLastSafeFrame)
		return;

	// Restore the player to the last known safe route frame.
	m_position = m_lastSafeCentre + m_lastSafeBinormal * m_carBaseOffset;
	m_velocity = glm::vec3(0.0f);

	// Restore the local frame to match the saved route frame orientation.
	m_currentTangent = m_lastSafeTangent;
	m_currentNormal = m_lastSafeNormal;
	m_currentBinormal = m_lastSafeBinormal;

	glm::vec3 forward = glm::normalize(glm::vec3(m_lastSafeTangent.x, 0.0f, m_lastSafeTangent.z));
	float cosTheta = glm::clamp(glm::dot(forward, glm::vec3(0.0f, 0.0f, 1.0f)), -1.0f, 1.0f);
	float angle = glm::degrees(acos(cosTheta));

	if (forward.x < 0.0f)
		angle = -angle;

	m_yawDegrees = angle;

	if (m_yawDegrees >= 360.0f) m_yawDegrees -= 360.0f;
	if (m_yawDegrees < 0.0f)    m_yawDegrees += 360.0f;

	// Recompute pitch to match the saved route tangent.
	glm::vec3 flatTangent = glm::normalize(glm::vec3(m_lastSafeTangent.x, 0.0f, m_lastSafeTangent.z));
	float slopePitch = 0.0f;

	if (glm::length(flatTangent) > 0.0001f)
	{
		float y = glm::clamp(glm::normalize(m_lastSafeTangent).y, -1.0f, 1.0f);
		slopePitch = glm::degrees(asinf(y));
	}

	m_pitchDegrees = slopePitch;

	glm::vec3 up(0.0f, 1.0f, 0.0f);

	float cosRoll = glm::clamp(glm::dot(m_lastSafeBinormal, up), -1.0f, 1.0f);
	float roll = glm::degrees(acos(cosRoll));

	if (m_lastSafeBinormal.x < 0.0f)
		roll = -roll;
	m_rollDegrees = roll;

	m_grounded = true;
	m_offTrackTimer = 0.0f;
}

void CPlayerCar::UpdateCarVisuals()
{
	if (!m_pCar)
		return;

	m_pCar->SetPosition(m_position);
	m_pCar->SetYaw(m_yawDegrees);
	m_pCar->SetSteeringAngle(m_visualSteerAngle);
	m_pCar->SetSpeed(glm::length(glm::vec2(m_velocity.x, m_velocity.z)));
	m_pCar->SetBodyYawOffset(m_visualBodyYawOffset);
	m_pCar->SetPitch(m_pitchDegrees);
	m_pCar->SetRoll(m_rollDegrees);
	m_pCar->SetTrackFrame(m_currentTangent, m_currentNormal, m_currentBinormal);
}

void CPlayerCar::ActivateBoost()
{
	if (!m_boostActive)
	{
		m_boostActive = true;
		m_boostTimer = 2.0f;

		m_originalMaxSpeed = m_maxSpeed;
		m_originalEngineForce = m_engineForce;

		m_maxSpeed *= 4.0f;
		m_engineForce *= 2.0f;
	}else if (m_boostActive)
	{
		m_boostTimer = 2.0f;
	}
}

glm::vec3 CPlayerCar::GetPosition() const
{
	return m_position;
}

glm::vec3 CPlayerCar::GetForward() const
{
	float yawRadians = glm::radians(m_yawDegrees);
	return glm::normalize(glm::vec3(sinf(yawRadians), 0.0f, cosf(yawRadians)));
}