#include "Car.h"
#include <cstdlib>
#include <cmath>

CCar::CCar()
{
	m_pBodyMesh = NULL;
	m_pWheelFLMesh = NULL;
	m_pWheelFRMesh = NULL;
	m_pWheelBLMesh = NULL;
	m_pWheelBRMesh = NULL;

	m_position = glm::vec3(0.0f, 0.0f, 0.0f);
	m_scale = 1.0f;
	m_yaw = 0.0f;
	m_pitch = 0.0f;
	m_roll = 0.0f;

	m_speed = 0.0f;
	m_steeringAngle = 0.0f;
	m_wheelSpinAngle = 0.0f;
	m_bodyYawOffset = 0.0f;

	// Local wheel offsets chosen to match the imported car mesh.
	m_wheelOffsetFL = glm::vec3(1.8f, -1.0f, 0.8f);
	m_wheelOffsetFR = glm::vec3(-1.8f, -1.0f, 0.8f);
	m_wheelOffsetBL = glm::vec3(1.8f, -1.0f, -3.7f);
	m_wheelOffsetBR = glm::vec3(-1.8f, -1.0f, -3.7f);

	// Initial track frame vectors, will be updated by the gameplay code to match the track orientation.
	m_trackTangent = glm::vec3(0.0f, 0.0f, 1.0f);
	m_trackNormal = glm::vec3(1.0f, 0.0f, 0.0f);
	m_trackBinormal = glm::vec3(0.0f, 1.0f, 0.0f);

	m_particleVAO = 0;
	m_particleVBO = 0;
}

CCar::~CCar()
{
	Release();
}

bool CCar::Initialise(const std::string& folder)
{
	// Load the main body and the four wheel meshes separately so that the wheels
	// can be animated independently using matrix-stack transforms.
	m_pBodyMesh = new COpenAssetImportMesh;
	m_pWheelFLMesh = new COpenAssetImportMesh;
	m_pWheelFRMesh = new COpenAssetImportMesh;
	m_pWheelBLMesh = new COpenAssetImportMesh;
	m_pWheelBRMesh = new COpenAssetImportMesh;

	bool ok1 = m_pBodyMesh->Load(folder + "CarBody.obj");
	bool ok2 = m_pWheelFLMesh->Load(folder + "Wheel_FL.obj");
	bool ok3 = m_pWheelFRMesh->Load(folder + "Wheel_FR.obj");
	bool ok4 = m_pWheelBLMesh->Load(folder + "Wheel_BL.obj");
	bool ok5 = m_pWheelBRMesh->Load(folder + "Wheel_BR.obj");

	if (!(ok1 && ok2 && ok3 && ok4 && ok5))
		return false;

	InitialiseParticleBuffers();
	return true;
}

void CCar::Release()
{
	if (m_pBodyMesh) {
		delete m_pBodyMesh;
		m_pBodyMesh = NULL;
	}

	if (m_pWheelFLMesh) {
		delete m_pWheelFLMesh;
		m_pWheelFLMesh = NULL;
	}

	if (m_pWheelFRMesh) {
		delete m_pWheelFRMesh;
		m_pWheelFRMesh = NULL;
	}

	if (m_pWheelBLMesh) {
		delete m_pWheelBLMesh;
		m_pWheelBLMesh = NULL;
	}

	if (m_pWheelBRMesh) {
		delete m_pWheelBRMesh;
		m_pWheelBRMesh = NULL;
	}

	ReleaseParticleBuffers();
	m_tireParticles.clear();
}

void CCar::SetPosition(const glm::vec3& position)
{
	m_position = position;
}

void CCar::SetScale(float scale)
{
	m_scale = scale;
}

void CCar::SetYaw(float yawDegrees)
{
	m_yaw = yawDegrees;
}

void CCar::SetSpeed(float speed)
{
	m_speed = speed;
}

void CCar::SetSteeringAngle(float steeringAngleDegrees)
{
	m_steeringAngle = steeringAngleDegrees;
}

void CCar::SetBodyYawOffset(float offsetDegrees)
{
	m_bodyYawOffset = offsetDegrees;
}

void CCar::SetPitch(float pitchDegrees)
{
	m_pitch = pitchDegrees;
}

void CCar::SetRoll(float rollDegrees)
{
	m_roll = rollDegrees;
}

void CCar::SetTrackFrame(const glm::vec3& tangent, const glm::vec3& normal, const glm::vec3& binormal)
{
	m_trackTangent = glm::normalize(tangent);
	m_trackNormal = glm::normalize(normal);
	m_trackBinormal = glm::normalize(binormal);
}

float CCar::RandomFloat(float minValue, float maxValue) const
{
	float t = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	return minValue + t * (maxValue - minValue);
}

glm::vec3 CCar::GetVisualForward() const
{
	float yawRadians = glm::radians(m_yaw + m_bodyYawOffset);
	return glm::normalize(glm::vec3(sinf(yawRadians), 0.0f, cosf(yawRadians)));
}

glm::vec3 CCar::GetHeadlightLeftWorldPosition() const
{
	glm::vec3 forward = GetVisualForward();
	glm::vec3 right = glm::normalize(glm::vec3(forward.z, 0.0f, -forward.x));
	glm::vec3 up(0.0f, 1.0f, 0.0f);

	// Local offset tuned to align the headlight with the imported mesh.
	glm::vec3 local(0.9f, 0.2f, 1.8f);
	local *= m_scale;

	return m_position +
		right * local.x +
		up * local.y +
		forward * local.z;
}

glm::vec3 CCar::GetHeadlightRightWorldPosition() const
{
	glm::vec3 forward = GetVisualForward();
	glm::vec3 right = glm::normalize(glm::vec3(forward.z, 0.0f, -forward.x));
	glm::vec3 up(0.0f, 1.0f, 0.0f);

	glm::vec3 local(-0.9f, 0.2f, 1.8f);
	local *= m_scale;

	return m_position +
		right * local.x +
		up * local.y +
		forward * local.z;
}

glm::vec3 CCar::GetRearLeftWheelWorldPosition() const
{
	float yawRadians = glm::radians(m_yaw + m_bodyYawOffset);

	glm::vec3 forward = glm::normalize(glm::vec3(sinf(yawRadians), 0.0f, cosf(yawRadians)));
	glm::vec3 up = glm::normalize(m_trackBinormal);

	forward = glm::normalize(forward - glm::dot(forward, up) * up);

	glm::vec3 right = glm::normalize(glm::cross(up, forward));
	forward = glm::normalize(glm::cross(right, up));

	glm::vec3 local = m_wheelOffsetBL * m_scale;
	local.x += 0.1f;
	local.y -= 0.5f;

	return m_position + right * local.x + up * local.y + forward * local.z;
}

glm::vec3 CCar::GetRearRightWheelWorldPosition() const
{
	float yawRadians = glm::radians(m_yaw + m_bodyYawOffset);

	glm::vec3 forward = glm::normalize(glm::vec3(sinf(yawRadians), 0.0f, cosf(yawRadians)));
	glm::vec3 up = glm::normalize(m_trackBinormal);

	forward = glm::normalize(forward - glm::dot(forward, up) * up);

	glm::vec3 right = glm::normalize(glm::cross(up, forward));
	forward = glm::normalize(glm::cross(right, up));

	glm::vec3 local = m_wheelOffsetBR * m_scale;
	local.x -= 0.1f;
	local.y -= 0.5f;

	return m_position + right * local.x + up * local.y + forward * local.z;
}

void CCar::InitialiseParticleBuffers()
{
	glGenVertexArrays(1, &m_particleVAO);
	glGenBuffers(1, &m_particleVBO);

	glBindVertexArray(m_particleVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_particleVBO);

	// Each particle stores position.xyz and colour.rgba.
	glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void CCar::ReleaseParticleBuffers()
{
	if (m_particleVBO != 0)
	{
		glDeleteBuffers(1, &m_particleVBO);
		m_particleVBO = 0;
	}

	if (m_particleVAO != 0)
	{
		glDeleteVertexArrays(1, &m_particleVAO);
		m_particleVAO = 0;
	}
}

void CCar::EmitTireParticles()
{
	// Emit particles only when the player is turning enough to suggest skidding.
	if (fabs(m_steeringAngle) <= 30.0f)
		return;

	const int particlesPerFrame = 5;

	glm::vec3 rearLeft = GetRearLeftWheelWorldPosition();
	glm::vec3 rearRight = GetRearRightWheelWorldPosition();

	float yawRadians = glm::radians(m_yaw + m_bodyYawOffset);
	glm::vec3 forward = glm::normalize(glm::vec3(sinf(yawRadians), 0.0f, cosf(yawRadians)));

	for (int i = 0; i < particlesPerFrame; i++)
	{
		STireParticle p;

		// Emit from either rear wheel to create a spread of tyre smoke.
		glm::vec3 basePos = (rand() % 2 == 0) ? rearLeft : rearRight;

		glm::vec3 up = glm::normalize(m_trackBinormal);
		glm::vec3 right = glm::normalize(glm::cross(up, forward));

		p.position = basePos + up * RandomFloat(0.0f, 0.15f);

		p.velocity = -forward * 2.0f + right * RandomFloat(-6.0f, 6.0f) + up * RandomFloat(8.0f, 10.0f);

		p.life = 0.3f;
		p.maxLife = 0.3f;
		p.colour = glm::vec4(0.5f, 0.5f, 0.5f, 0.7f);

		m_tireParticles.push_back(p);
	}
}

void CCar::UpdateParticles(float dtSeconds)
{
	for (std::vector<STireParticle>::iterator it = m_tireParticles.begin(); it != m_tireParticles.end(); )
	{
		it->life -= dtSeconds;

		if (it->life <= 0.0f)
		{
			it = m_tireParticles.erase(it);
			continue;
		}

		it->position += it->velocity * dtSeconds;

		// Fade the particle as it approaches the end of its lifetime.
		float t = it->life / it->maxLife;
		it->colour.a = 0.7f * t;

		++it;
	}
}

void CCar::Update(float dt)
{
	float dtSeconds = dt / 1000.0f;

	// Animate wheel spin based on current speed.
	m_wheelSpinAngle += m_speed * 80.0f * dtSeconds;

	if (m_wheelSpinAngle > 360.0f)
		m_wheelSpinAngle -= 360.0f;
	if (m_wheelSpinAngle < -360.0f)
		m_wheelSpinAngle += 360.0f;

	EmitTireParticles();
	UpdateParticles(dtSeconds);
}

void CCar::RenderMesh(COpenAssetImportMesh* pMesh,
	glutil::MatrixStack& modelViewMatrixStack,
	CShaderProgram* pShaderProgram,
	CCamera* pCamera)
{
	pShaderProgram->SetUniform("bUseTexture", true);
	pShaderProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
	pShaderProgram->SetUniform("matrices.normalMatrix",
		pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));

	pMesh->Render();
}

void CCar::RenderParticles(CShaderProgram* pParticleProgram, CCamera* pCamera)
{
	if (m_tireParticles.empty() || m_particleVAO == 0 || m_particleVBO == 0)
		return;

	std::vector<float> particleData;
	particleData.reserve(m_tireParticles.size() * 7);

	for (size_t i = 0; i < m_tireParticles.size(); i++)
	{
		const STireParticle& p = m_tireParticles[i];

		particleData.push_back(p.position.x);
		particleData.push_back(p.position.y);
		particleData.push_back(p.position.z);
		particleData.push_back(p.colour.r);
		particleData.push_back(p.colour.g);
		particleData.push_back(p.colour.b);
		particleData.push_back(p.colour.a);
	}

	pParticleProgram->UseProgram();
	pParticleProgram->SetUniform("matrices.projMatrix", pCamera->GetPerspectiveProjectionMatrix());
	pParticleProgram->SetUniform("matrices.modelViewMatrix", pCamera->GetViewMatrix());

	glEnable(GL_BLEND);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);

	glBindVertexArray(m_particleVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_particleVBO);
	glBufferData(GL_ARRAY_BUFFER,
		particleData.size() * sizeof(float),
		particleData.data(),
		GL_DYNAMIC_DRAW);

	glDrawArrays(GL_POINTS, 0, (GLsizei)m_tireParticles.size());

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}

void CCar::Render(glutil::MatrixStack& modelViewMatrixStack,
	CShaderProgram* pCarProgram,
	CShaderProgram* pParticleProgram,
	CCamera* pCamera)
{
	pCarProgram->UseProgram();

	// Root transform for the whole car
	modelViewMatrixStack.Push();
	{
		modelViewMatrixStack.Translate(m_position);

		float yawRadians = glm::radians(m_yaw + m_bodyYawOffset);

		glm::vec3 forward = glm::normalize(glm::vec3(
			sinf(yawRadians),
			0.0f,
			cosf(yawRadians)
		));

		glm::vec3 up = glm::normalize(m_trackBinormal);

		// Project car forward onto the banked road plane
		forward = glm::normalize(forward - glm::dot(forward, up) * up);

		glm::vec3 right = glm::normalize(glm::cross(up, forward));
		forward = glm::normalize(glm::cross(right, up));

		glm::mat4 carOrientation = glm::mat4(glm::mat3(
			right,
			up,
			forward
		));

		modelViewMatrixStack *= carOrientation;
		modelViewMatrixStack.Scale(m_scale);

		// Car body
		modelViewMatrixStack.Push();
		{
			RenderMesh(m_pBodyMesh, modelViewMatrixStack, pCarProgram, pCamera);
		}
		modelViewMatrixStack.Pop();

		// Front-left wheel: steering + wheel spin
		modelViewMatrixStack.Push();
		{
			modelViewMatrixStack.Translate(m_wheelOffsetFL);
			modelViewMatrixStack.Rotate(glm::vec3(0.0f, 1.0f, 0.0f), glm::radians(m_steeringAngle));
			modelViewMatrixStack.Rotate(glm::vec3(1.0f, 0.0f, 0.0f), glm::radians(m_wheelSpinAngle));
			RenderMesh(m_pWheelFLMesh, modelViewMatrixStack, pCarProgram, pCamera);
		}
		modelViewMatrixStack.Pop();

		// Front-right wheel: steering + wheel spin
		modelViewMatrixStack.Push();
		{
			modelViewMatrixStack.Translate(m_wheelOffsetFR);
			modelViewMatrixStack.Rotate(glm::vec3(0.0f, 1.0f, 0.0f), glm::radians(m_steeringAngle));
			modelViewMatrixStack.Rotate(glm::vec3(1.0f, 0.0f, 0.0f), glm::radians(m_wheelSpinAngle));
			RenderMesh(m_pWheelFRMesh, modelViewMatrixStack, pCarProgram, pCamera);
		}
		modelViewMatrixStack.Pop();

		// Rear-left wheel: wheel spin only
		modelViewMatrixStack.Push();
		{
			modelViewMatrixStack.Translate(m_wheelOffsetBL);
			modelViewMatrixStack.Rotate(glm::vec3(1.0f, 0.0f, 0.0f), glm::radians(m_wheelSpinAngle));
			RenderMesh(m_pWheelBLMesh, modelViewMatrixStack, pCarProgram, pCamera);
		}
		modelViewMatrixStack.Pop();

		// Rear-right wheel: wheel spin only
		modelViewMatrixStack.Push();
		{
			modelViewMatrixStack.Translate(m_wheelOffsetBR);
			modelViewMatrixStack.Rotate(glm::vec3(1.0f, 0.0f, 0.0f), glm::radians(m_wheelSpinAngle));
			RenderMesh(m_pWheelBRMesh, modelViewMatrixStack, pCarProgram, pCamera);
		}
		modelViewMatrixStack.Pop();
	}
	modelViewMatrixStack.Pop();

	// Render tyre smoke after the solid car geometry.
	RenderParticles(pParticleProgram, pCamera);
}