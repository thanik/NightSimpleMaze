#pragma once
using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;
class Player
{
public:
	Player();
	~Player();

	void							Update();
	DirectX::SimpleMath::Matrix		getCameraMatrix();
	void							setPosition(DirectX::SimpleMath::Vector3 newPosition);
	DirectX::SimpleMath::Vector3	getPosition();
	DirectX::SimpleMath::Vector3	getForward();
	void							setRotation(DirectX::SimpleMath::Vector3 newRotation);
	DirectX::SimpleMath::Vector3	getRotation();
	float							getMoveSpeed();
	float							getOverviewMoveSpeed();
	float							getRotationSpeed();
	DirectX::SimpleMath::Vector3	getMoveVector();
	float							m_camRotRate;
	void							RenderPlayerModel(ID3D11DeviceContext* deviceContext, BasicEffect* basicEffect, ID3D11InputLayout* inputLayout, CommonStates* states, Matrix view, Matrix projection);
	void							UpdateModelEffect(ID3D11Device* device, std::shared_ptr<DirectX::BasicEffect> pEffect);

private:
	DirectX::SimpleMath::Matrix		m_cameraMatrix;			//camera matrix to be passed out and used to set camera position and angle for wrestling
	DirectX::SimpleMath::Vector3	m_lookat;
	DirectX::SimpleMath::Vector3	m_position;
	DirectX::SimpleMath::Vector3	m_forward;
	DirectX::SimpleMath::Vector3	m_up;
	DirectX::SimpleMath::Vector3    m_move;
	DirectX::SimpleMath::Vector3	m_orientation;			//vector storing pitch yaw and roll. 
	std::unique_ptr<DirectX::Model> m_playerModel;
	float							m_modelScale;

	float							m_moveSpeed;
	float							m_overviewMoveSpeed;
};

