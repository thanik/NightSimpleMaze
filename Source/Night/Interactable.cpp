#include "pch.h"
#include "Interactable.h"

Interactable::Interactable(InteractableType type) noexcept(false)
{
	m_initialized = false;
	m_enabled = false;
	m_type = type;
	m_rotatingTime = 5.f;
	m_rotateTime = 0.f;
}

Interactable::~Interactable()
{
	if (m_initialized)
	{
		m_model.reset();
	}
}

void Interactable::InitializeModel(ID3D11Device* device, IEffectFactory& fxFactory, Vector3 position)
{
	wstring filename = L"";
	if (m_type == InteractableType::GOAL)
	{
		filename = L"Assets/Models/flag_oga.cmo";
		m_modelScale = Vector3(0.035f, 0.035f, 0.035f);
		m_collisionSize = Vector3(0.1f, 0.1f, 0.1f);
	}
	else if (m_type == InteractableType::ITEM)
	{
		filename = L"Assets/Models/item.cmo";
		m_modelScale = Vector3(0.01f, 0.01f, 0.01f);
		m_collisionSize = Vector3(0.1f, 0.1f, 0.1f);
	}
	m_model = Model::CreateFromCMO(device, filename.c_str(), fxFactory);
	m_enabled = true;
	m_position = position;
	m_initialized = true;
}

void Interactable::SetPosition(Vector3 position)
{
	m_position = position;
}

void Interactable::SetRotation(Vector3 rotation)
{
	m_rotation = rotation;
}

void Interactable::SetEnabled(bool enabled)
{
	m_enabled = enabled;
}

void Interactable::UpdateEffect(ID3D11Device* device, std::shared_ptr<DirectX::BasicEffect> pEffect)
{
	for (auto it = m_model->meshes.cbegin(); it != m_model->meshes.cend(); ++it)
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
	m_model->Modified();
}

void Interactable::RotateBackAndForth(float deltaTime)
{
	if (m_enabled && m_initialized)
	{
		m_rotateTime = m_rotateTime + deltaTime;
		float animation_percentage = 0.f;
		if (m_rotateTime > m_rotatingTime * 2)
		{
			m_rotateTime = 0.f;
		}
		else if (m_rotateTime > m_rotatingTime)
		{
			animation_percentage = (m_rotateTime - m_rotatingTime) / (m_rotatingTime);
			float flipped = 1 - animation_percentage;
			flipped = flipped * flipped;
			flipped = 1 - flipped;
			float square = animation_percentage * animation_percentage;
			animation_percentage = 1 - (square + (flipped - square) * animation_percentage);
		}
		else
		{
			animation_percentage = (m_rotateTime) / (m_rotatingTime);
			float flipped = 1 - animation_percentage;
			flipped = flipped * flipped;
			flipped = 1 - flipped;
			float square = animation_percentage * animation_percentage;
			animation_percentage = square + (flipped - square) * animation_percentage;
		}
		
		m_rotation = Vector3::Lerp(Vector3(1.5f, 45.f, 0.f), Vector3(-1.5f, -45.f, 0.f), animation_percentage);

	}
}

void Interactable::Render(ID3D11DeviceContext* deviceContext, BasicEffect* basicEffect, ID3D11InputLayout* inputLayout, CommonStates* states, Matrix view, Matrix projection)
{
	if (m_initialized && m_enabled)
	{
		Matrix m_world;
		m_world = Matrix::Identity;
		SimpleMath::Matrix newRotation = SimpleMath::Matrix::CreateFromYawPitchRoll(m_rotation.y * XM_PI / 180, m_rotation.x * XM_PI / 180, m_rotation.z * XM_PI / 180);
		SimpleMath::Matrix newScale = SimpleMath::Matrix::CreateScale(m_modelScale);
		SimpleMath::Matrix newPosition = SimpleMath::Matrix::CreateTranslation(m_position);
		m_world = m_world * newScale * newRotation * newPosition;
		basicEffect->SetWorld(m_world);
		m_model->Draw(deviceContext, *states, m_world, view, projection);
	}
}

Vector3 Interactable::GetPosition()
{
	return m_position;
}

Vector3 Interactable::GetModelScale()
{
	return m_modelScale;
}

Vector3 Interactable::GetCollisionSize()
{
	return m_collisionSize;
}

InteractableType Interactable::GetType()
{
	return m_type;
}

bool Interactable::GetEnabled()
{
	return m_enabled;
}