#include "pch.h"
#include "Input.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

Input::Input()
{
}

Input::~Input()
{
}

void Input::Initialise(HWND window)
{
	m_keyboard = std::make_unique<DirectX::Keyboard>();
	m_mouse = std::make_unique<DirectX::Mouse>();
	m_mouse->SetWindow(window);
	m_quitApp = false;

	m_GameInput.forward = false;
	m_GameInput.back = false;
	m_GameInput.right = false;
	m_GameInput.left = false;
	m_GameInput.rotRight = false;
	m_GameInput.rotLeft = false;
	m_GameInput.rotUp = false;
	m_GameInput.rotDown = false;
	m_GameInput.mouseDelta = DirectX::SimpleMath::Vector3(0.f, 0.f, 0.f);
	m_GameInput.cameraSwitch = false;
}

void Input::Update()
{
	auto kb = m_keyboard->GetState();	//updates the basic keyboard state
	m_KeyboardTracker.Update(kb);		//updates the more feature filled state. Press / release etc. 
	auto mouse = m_mouse->GetState();   //updates the basic mouse state
	m_MouseTracker.Update(mouse);		//updates the more advanced mouse state. 

	if (kb.Escape)// check has escape been pressed.  if so, quit out. 
	{
		m_quitApp = true;
	}

	//A key
	if (kb.A)	m_GameInput.left = true;
	else		m_GameInput.left = false;

	//D key
	if (kb.D)	m_GameInput.right = true;
	else		m_GameInput.right = false;

	//W key
	if (kb.W)	m_GameInput.forward = true;
	else		m_GameInput.forward = false;

	//S key
	if (kb.S)	m_GameInput.back = true;
	else		m_GameInput.back = false;

	if (kb.Up)	m_GameInput.rotUp = true;
	else        m_GameInput.rotUp = false;

	if (kb.Down)	m_GameInput.rotDown = true;
	else        m_GameInput.rotDown = false;

	if (kb.Left)	m_GameInput.rotLeft = true;
	else        m_GameInput.rotLeft = false;

	if (kb.Right)	m_GameInput.rotRight = true;
	else        m_GameInput.rotRight = false;

	if (m_KeyboardTracker.pressed.M)	m_GameInput.cameraSwitch = true;
	else        m_GameInput.cameraSwitch = false;

	m_GameInput.mouseDelta = DirectX::SimpleMath::Vector3(float(mouse.x), float(mouse.y), 0.f);

}

bool Input::Quit()
{
	return m_quitApp;
}

void Input::SetMouseMode(DirectX::Mouse::Mode mode)
{
	m_mouse->SetMode(mode);
	m_GameInput.mouseDelta = Vector3(0.f, 0.f, 0.f);
}

InputCommands Input::getGameInput()
{
	return m_GameInput;
}