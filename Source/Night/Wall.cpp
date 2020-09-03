#include "pch.h"
#include "Wall.h"

Wall::Wall() noexcept(false)
{
	m_initialized = false;
}

Wall::~Wall()
{
	if (m_initialized)
	{
		m_model.reset();
	}
}

void Wall::InitializeWall(ID3D11DeviceContext* deviceContext, Vector3 position, Vector3 size)
{
	m_model = GeometricPrimitive::CreateBox(deviceContext, size);
	m_position = position;
	m_initialized = true;
	m_size = size;
}

void Wall::InitializeWall(ID3D11DeviceContext* deviceContext, Vector3 position, float size = 1)
{
	m_model = GeometricPrimitive::CreateCube(deviceContext, size);
	m_position = position;
	m_initialized = true;
	m_size = Vector3(size, size, size);
}

void Wall::MoveWall(Vector3 position)
{
	m_position = position;
}

void Wall::Render(ID3D11DeviceContext* deviceContext, BasicEffect* basicEffect, ID3D11InputLayout* inputLayout, CommonStates* states, Matrix view, Matrix projection)
{
	if (m_initialized)
	{
		Matrix m_world;
		m_world = Matrix::Identity;
		basicEffect->SetWorld(m_world);
		Matrix newPosition = SimpleMath::Matrix::CreateTranslation(m_position);
		m_world = m_world * newPosition;
		basicEffect->SetWorld(m_world);
		m_model->Draw(basicEffect, inputLayout, false, false);
	}
}

void Wall::Free()
{
	if (m_initialized)
	{
		m_model.reset();
	}
}

Vector3 Wall::GetPosition()
{
	return m_position;
}

Vector3 Wall::GetSize()
{
	return m_size;
}
