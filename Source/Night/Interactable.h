#pragma once
using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

enum class InteractableType {
	ITEM,
	GOAL
};

class Interactable
{
private:
	std::shared_ptr<Model> m_model;
	Vector3 m_position;
	Vector3 m_modelScale;
	Vector3 m_collisionSize;
	Vector3 m_rotation;
	bool m_initialized;
	bool m_enabled;
	InteractableType m_type;

	float m_rotatingTime;
	float m_rotateTime;


public:
	Interactable(InteractableType type) noexcept(false);
	~Interactable();

	void SetPosition(Vector3 position);
	void SetRotation(Vector3 rotation);
	void Render(ID3D11DeviceContext* deviceContext, BasicEffect* basicEffect, ID3D11InputLayout* inputLayout, CommonStates* states, Matrix view, Matrix projection);
	void UpdateEffect(ID3D11Device* device, std::shared_ptr<DirectX::BasicEffect> basicEffect);
	Vector3 GetPosition();
	Vector3 GetModelScale();
	Vector3 GetCollisionSize();
	InteractableType GetType();
	void InitializeModel(ID3D11Device* device, IEffectFactory& fxFactory, Vector3 position);
	void RotateBackAndForth(float deltaTime);
	void SetEnabled(bool enabled);
	bool GetEnabled();

	int gridPosX;
	int gridPosY;
};

