#pragma once
using namespace DirectX;
using namespace DirectX::SimpleMath;
class Wall
{
private:
	std::shared_ptr<GeometricPrimitive> m_model;
	Vector3 m_position;
	Vector3 m_size;
	bool m_initialized;

public:
	Wall() noexcept(false);
	~Wall();

	void InitializeWall(ID3D11DeviceContext* deviceContext, Vector3 position, Vector3 size);
	void InitializeWall(ID3D11DeviceContext* deviceContext, Vector3 position, float size);
	void MoveWall(Vector3 position);
	void Render(ID3D11DeviceContext* deviceContext, BasicEffect* basicEffect, ID3D11InputLayout* inputLayout, CommonStates* states, Matrix view, Matrix projection);
	void Free();
	Vector3 GetPosition();
	Vector3 GetSize();
};

