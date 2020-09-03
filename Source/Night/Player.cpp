#include "pch.h"
#include "Player.h"
using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;
Player::Player()
{
	//initalise values. 
	//Orientation and Position are how we control the camera. 
	m_orientation.x = 0.0f;		//rotation around x - pitch
	m_orientation.y = 0.0f;		//rotation around y - yaw
	m_orientation.z = 0.0f;		//rotation around z - roll	//we tend to not use roll a lot in first person

	m_position.x = 0.0f;		//camera position in space. 
	m_position.y = 0.0f;
	m_position.z = 0.0f;

	//These variables are used for internal calculations and not set.  but we may want to queary what they 
	//externally at points
	m_lookat.x = 0.0f;		//Look target point
	m_lookat.y = 0.0f;
	m_lookat.z = 0.0f;

	m_forward.x = 0.0f;		//forward/look direction
	m_forward.y = 0.0f;
	m_forward.z = 0.0f;

	//
	m_moveSpeed = 0.25f;
	m_overviewMoveSpeed = 1.f;
	m_camRotRate = 40.0;
	m_modelScale = 0.1f;

	//force update with initial values to generate other camera data correctly for first update. 
	Update();
}


Player::~Player()
{
	m_playerModel.reset();
}

void Player::UpdateModelEffect(ID3D11Device* device, std::shared_ptr<DirectX::BasicEffect> pEffect)
{
	for (auto it = m_playerModel->meshes.cbegin(); it != m_playerModel->meshes.cend(); ++it)
	{
		auto mesh = it->get();
		assert(mesh != nullptr);

		for (auto it = mesh->meshParts.cbegin(); it != mesh->meshParts.cend(); ++it)
		{
			if ((*it)->effect)
			{
				shared_ptr<IEffect> effect = std::dynamic_pointer_cast<IEffect>(pEffect);
				(*it)->ModifyEffect(device, effect, false);
			}
		}
		//current_wall.Render(context, m_batchEffect.get(), m_batchInputLayout.Get(), m_states.get(), m_view, m_projection);
	}
	m_playerModel->Modified();
}

void Player::RenderPlayerModel(ID3D11DeviceContext* deviceContext, BasicEffect* basicEffect, ID3D11InputLayout* inputLayout, CommonStates* states, Matrix view, Matrix projection)
{
	Matrix m_world;
	m_world = Matrix::Identity;
	SimpleMath::Matrix newScale = SimpleMath::Matrix::CreateScale(m_modelScale);
	SimpleMath::Matrix newPosition = SimpleMath::Matrix::CreateTranslation(m_position);
	m_world = m_world * newScale * newPosition;
	basicEffect->SetWorld(m_world);

	//m_playerModel->UpdateEffects();
	//m_playerModel->UpdateEffects([&](IEffect* effect)
	//	{
	//		auto lights = dynamic_cast<IEffectLights*>(effect);
	//		if (lights)
	//		{
	//			lights->SetLightingEnabled(true);
	//			lights->SetPerPixelLighting(true);
	//			lights->SetLightEnabled(0, true);
	//			lights->SetLightDiffuseColor(0, lightColor);
	//			lights->SetLightDirection(0, lightDirection);
	//			lights->SetLightEnabled(1, false);
	//			lights->SetLightEnabled(2, false);
	//		}
	//	});
	m_playerModel->Draw(deviceContext, *states, m_world, view, projection);
}

void Player::Update()
{
	//rotation in yaw - using the paramateric equation of a circle
	//m_forward.x = sin((m_orientation.y)*3.1415f / 180.0f);
	//m_forward.z = cos((m_orientation.y)*3.1415f / 180.0f);
	//m_forward.Normalize();

	m_forward.x = cos((m_orientation.y) * 3.1415f / 180.0f) * sin((m_orientation.x) * 3.1415f / 180.0f);
	m_forward.z = sin((m_orientation.y) * 3.1415f / 180.0f) * sin((m_orientation.x) * 3.1415f / 180.0f);
	m_forward.y = cos((m_orientation.x) * 3.1415f / 180.0f);
	m_forward.Normalize();

	m_move.x = m_forward.x;
	m_move.z = m_forward.z;

	//update lookat point
	m_lookat = m_position + m_forward;

	//apply camera vectors and create camera matrix
	m_cameraMatrix = (DirectX::SimpleMath::Matrix::CreateLookAt(m_position, m_lookat, DirectX::SimpleMath::Vector3::UnitY));


}

DirectX::SimpleMath::Matrix Player::getCameraMatrix()
{
	return m_cameraMatrix;
}

void Player::setPosition(DirectX::SimpleMath::Vector3 newPosition)
{
	m_position = newPosition;
}

DirectX::SimpleMath::Vector3 Player::getPosition()
{
	return m_position;
}

DirectX::SimpleMath::Vector3 Player::getForward()
{
	return m_forward;
}

DirectX::SimpleMath::Vector3 Player::getMoveVector()
{
	return m_move;
}

void Player::setRotation(DirectX::SimpleMath::Vector3 newRotation)
{
	m_orientation = newRotation;
}

DirectX::SimpleMath::Vector3 Player::getRotation()
{
	return m_orientation;
}

float Player::getMoveSpeed()
{
	return m_moveSpeed;
}

float Player::getOverviewMoveSpeed()
{
	return m_overviewMoveSpeed;
}

float Player::getRotationSpeed()
{
	return m_camRotRate;
}