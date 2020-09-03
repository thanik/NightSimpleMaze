//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

extern void ExitGame();

using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_D32_FLOAT, 2, D3D_FEATURE_LEVEL_10_0);
    m_deviceResources->RegisterDeviceNotify(this);
	m_mazeWidth = 5;
	m_mazeHeight = 5;
	m_wallHeight = 0.1f;
	m_overviewSwitchingTime = 1.f;
	m_overviewSwitchTime = m_overviewSwitchingTime;
	m_win = false;
	m_gameTime = 0.f;
	m_itemsCollected = 0;
	m_numberOfItems = 2;
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
	m_isOverview = false;
	m_started = false;
	m_input.Initialise(window);
	m_input.SetMouseMode(Mouse::MODE_ABSOLUTE);
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

	// setup player
	m_player.setPosition(Vector3(0.0f, 0.1f, 0.0f));
	m_player.setRotation(Vector3(90.0f, 0.f, 0.0f));	//orientation is -90 becuase zero will be looking up at the sky straight up. 

	//setup ground
	auto context = m_deviceResources->GetD3DDeviceContext();
	m_ground.InitializeWall(context, Vector3(0.f, 0.f, 0.f), Vector3(1.f, 0.1f, 1.f));

	// setup light
	lightDiffuseColor = Vector4(0.2f, 0.2f, 0.2f, 1.f);
	lightDirection = Vector3(-1.f, 0.f, -1.f);

	UpdateCameraMovement(0);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.Fonts->AddFontFromFileTTF("Assets/Fonts/Barlow-Regular.ttf", 24.f);
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(window);		//tie to our window
	ImGui_ImplDX11_Init(m_deviceResources->GetD3DDevice(), m_deviceResources->GetD3DDeviceContext());	//tie to directx

}

void Game::GenerateMaze()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto device = m_deviceResources->GetD3DDevice();
	int width_blocks = m_mazeWidth * 2 - 1;
	int height_blocks = m_mazeHeight * 2 - 1;

	m_ground.InitializeWall(context, Vector3(0.f, 0.f, 0.f), Vector3(0.1f * (height_blocks + 2), 0.1f, 0.1f * (width_blocks + 2)));

	srand(time(0));
	vector<vector<int>> intMap = m_mazeGenerator.GenerateMap(m_mazeWidth, m_mazeHeight);
	
	for (size_t y = 0; y < intMap.size(); y++)
	{
		float yPos = ((0.1f * height_blocks) / 2) - 0.05f - (0.1f * y);
		for (size_t x = 0; x < intMap[y].size(); x++)
		{
			if (intMap[y][x] == 0)
			{
				Wall* newWall = new Wall();
				float xPos = -((0.1f * width_blocks) / 2) + 0.05f + (0.1f * x);
				newWall->InitializeWall(context, Vector3(yPos, m_wallHeight, xPos), Vector3(0.1f, m_wallHeight, 0.1f));
				m_walls.push_back(newWall);
			}
		}
	}

	Wall* leftCornerWall = new Wall();
	leftCornerWall->InitializeWall(context, Vector3(0.f, 0.1f, (-(0.1f * width_blocks) / 2) - 0.05f), Vector3(0.1f * height_blocks, m_wallHeight, 0.1f));
	m_walls.push_back(leftCornerWall);
	Wall* rightCornerWall = new Wall();
	rightCornerWall->InitializeWall(context, Vector3(0.f, 0.1f, (0.1f * width_blocks) / 2 + 0.05f), Vector3(0.1f * height_blocks, m_wallHeight, 0.1f));
	m_walls.push_back(rightCornerWall);
	Wall* topCornerWall = new Wall();
	topCornerWall->InitializeWall(context, Vector3((0.1f * height_blocks) / 2 + 0.05f, 0.1f, 0.f), Vector3(0.1f, m_wallHeight, 0.1f * (width_blocks + 2)));
	m_walls.push_back(topCornerWall);
	Wall* downCornerWall = new Wall();
	downCornerWall->InitializeWall(context, Vector3((-(0.1f * height_blocks) / 2) - 0.05f, 0.1f, 0.f), Vector3(0.1f, m_wallHeight, 0.1f * (width_blocks + 2)));
	m_walls.push_back(downCornerWall);

	//random player pos
	vector<int> playerPos;
	playerPos.push_back(rand() % (m_mazeWidth));
	playerPos.push_back(rand() % (m_mazeHeight));
	while (intMap[playerPos[1]][playerPos[0]] == 0)
	{
		playerPos[0] = rand() % (m_mazeWidth);
		playerPos[1] = rand() % (m_mazeHeight);
	}
	float yPos = ((0.1f * height_blocks) / 2) - 0.05f - (0.1f * ((playerPos[1] * 2)));
	float xPos = -((0.1f * width_blocks) / 2) + 0.05f + (0.1f * (playerPos[0] * 2));
	m_player.setPosition(Vector3(yPos, 0.1f, xPos));

	//spawn interactable
	vector<int> goalPos;
	goalPos.push_back(rand() % (m_mazeWidth));
	goalPos.push_back(rand() % (m_mazeHeight));
	while (intMap[goalPos[1]][goalPos[0]] == 0 || (goalPos[0] == playerPos[0] && goalPos[1] == playerPos[1]))
	{
		goalPos[0] = rand() % (m_mazeWidth);
		goalPos[1] = rand() % (m_mazeHeight);
	}
	yPos = ((0.1f * height_blocks) / 2) - 0.05f - (0.1f * ((goalPos[1] * 2)));
	xPos = -((0.1f * width_blocks) / 2) + 0.05f + (0.1f * (goalPos[0] * 2));
	Interactable* goal = new Interactable(InteractableType::GOAL);
	goal->InitializeModel(device, *m_fxFactory, Vector3(yPos, 0.05f, xPos));
	m_interactables.push_back(goal);

	for (int i = 0; i < m_numberOfItems; i++)
	{
		vector<int> itemPos;
		itemPos.push_back(rand() % (m_mazeWidth));
		itemPos.push_back(rand() % (m_mazeHeight));

		// check for position conflict
		bool check = false;
		do
		{
			check = false;
			if (intMap[itemPos[1]][itemPos[0]] == 0 || (itemPos[0] == playerPos[0] && itemPos[1] == playerPos[1]) || (itemPos[0] == goalPos[0] && itemPos[1] == goalPos[1]))
			{
				itemPos[0] = rand() % (m_mazeWidth);
				itemPos[1] = rand() % (m_mazeHeight);
				check = true;
			}
			else
			{
				for (vector<Interactable*>::iterator it = m_interactables.begin(); it != m_interactables.end(); ++it)
				{
					Interactable* current_interactable = *it;
					if (current_interactable->GetType() == InteractableType::ITEM && itemPos[0] == current_interactable->gridPosX && itemPos[1] == current_interactable->gridPosY)
					{
						itemPos[0] = rand() % (m_mazeWidth);
						itemPos[1] = rand() % (m_mazeHeight);
						check = true;
						break;
					}
				}
			}
		} while (check);
		yPos = ((0.1f * height_blocks) / 2) - 0.05f - (0.1f * ((itemPos[1] * 2)));
		xPos = -((0.1f * width_blocks) / 2) + 0.05f + (0.1f * (itemPos[0] * 2));
		Interactable* item = new Interactable(InteractableType::ITEM);
		item->InitializeModel(device, *m_fxFactory, Vector3(yPos, 0.06f, xPos));
		item->gridPosX = itemPos[0];
		item->gridPosY = itemPos[1];
		m_interactables.push_back(item);
	}
	StartGame();
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    //take in input
    m_input.Update();								//update the hardware
    m_gameInputCommands = m_input.getGameInput();	//retrieve the input for our game

    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

	if (m_started)
	{
		m_gameTime += elapsedTime;
		m_postProcess->SetEffect(BasicPostProcess::Copy);
		
		if (m_gameInputCommands.cameraSwitch)
		{
			m_isOverview = !m_isOverview;
			m_overviewSwitchTime = 0.f;
			
		}

		if (m_isOverview)
		{
			Vector3 overviewRotation = Vector3(180.f, 0.f, 0.f);
			Vector3 overviewPosition = Vector3(0.f, 1.5f, 0.f);
			if (m_overviewSwitchTime > m_overviewSwitchingTime)
			{
				UpdateOverview(elapsedTime);
				m_overviewRotation = Vector3(180.f, 0.f, 0.f);
			}
			else
			{
				m_overviewSwitchTime += elapsedTime;
				float keyframe_percentage = m_overviewSwitchTime / m_overviewSwitchingTime;
				float flipped = 1 - keyframe_percentage;
				flipped = flipped * flipped;
				flipped = 1 - flipped;
				float square = keyframe_percentage * keyframe_percentage;
				keyframe_percentage = square + (flipped - square) * keyframe_percentage;
				m_overviewPosition = Vector3::Lerp(m_player.getPosition(), overviewPosition, keyframe_percentage);
				m_overviewRotation = Vector3::Lerp(m_player.getRotation(), overviewRotation, keyframe_percentage);
				
			}
			Vector3 forward = Vector3();

			forward.x = cos((m_overviewRotation.y) * 3.1415f / 180.0f) * sin((m_overviewRotation.x) * 3.1415f / 180.0f);
			forward.z = sin((m_overviewRotation.y) * 3.1415f / 180.0f) * sin((m_overviewRotation.x) * 3.1415f / 180.0f);
			forward.y = cos((m_overviewRotation.x) * 3.1415f / 180.0f);
			forward.Normalize();

			//update lookat point
			Vector3 m_lookat = m_overviewPosition + forward;

			//apply camera vectors and create camera matrix
			m_view = (DirectX::SimpleMath::Matrix::CreateLookAt(m_overviewPosition, m_lookat, DirectX::SimpleMath::Vector3::UnitY));
			m_world = Matrix::Identity;

		}
		else
		{
			if (m_overviewSwitchTime > m_overviewSwitchingTime)
			{
				UpdateCameraMovement(elapsedTime);
				m_overviewRotation = Vector3(180.f, 0.f, 0.f);
			}
			else
			{
				m_overviewSwitchTime += elapsedTime;
				float keyframe_percentage = m_overviewSwitchTime / m_overviewSwitchingTime;
				float flipped = 1 - keyframe_percentage;
				flipped = flipped * flipped;
				flipped = 1 - flipped;
				float square = keyframe_percentage * keyframe_percentage;
				keyframe_percentage = square + (flipped - square) * keyframe_percentage;
				Vector3 transitionBackPosition = Vector3::Lerp(m_overviewPosition, m_player.getPosition(), keyframe_percentage);
				Vector3 overviewRotation = Vector3::Lerp(m_overviewRotation, m_player.getRotation(), keyframe_percentage);

				Vector3 forward = Vector3();

				forward.x = cos((overviewRotation.y) * 3.1415f / 180.0f) * sin((overviewRotation.x) * 3.1415f / 180.0f);
				forward.z = sin((overviewRotation.y) * 3.1415f / 180.0f) * sin((overviewRotation.x) * 3.1415f / 180.0f);
				forward.y = cos((overviewRotation.x) * 3.1415f / 180.0f);
				forward.Normalize();

				//update lookat point
				Vector3 m_lookat = transitionBackPosition + forward;

				//apply camera vectors and create camera matrix
				m_view = (DirectX::SimpleMath::Matrix::CreateLookAt(transitionBackPosition, m_lookat, DirectX::SimpleMath::Vector3::UnitY));
				m_world = Matrix::Identity;
			}
		}
		CheckInteractablesCollision(m_player.getPosition());

		if (m_itemsCollected < m_numberOfItems && m_gameTime < m_timeToHideTip)
		{
			ImGui::Begin("Need more items", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
			string text = "You need more items to complete the level. (" + std::to_string(m_itemsCollected) + " from total " + std::to_string(m_numberOfItems) + ").";
			ImGui::Text(text.c_str());
			ImGui::End();
		}
	}
	else
	{
		m_postProcess->SetEffect(BasicPostProcess::BloomBlur);
		m_postProcess->SetBloomBlurParameters(true, 5.f, 0.85f);
		
		if (m_win)
		{
			ImGui::Begin("Level Complete", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
			ImGui::Text("You win!");
			string timeText = "Time took: " + std::to_string(m_gameTime) + " seconds.";
			ImGui::Text(timeText.c_str());
			if (ImGui::Button("Exit"))
			{
				ExitGame();
			}
			ImGui::End();
		}
		else
		{
			ImGui::Begin("Main Menu", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
			ImGui::Text("Simple Maze");
			ImGui::Separator();
			ImGui::Text("How to play:\nCollect all items and get to the flag.");
			ImGui::Separator();
			ImGui::Text("Control:\nMouse - Look\nW,A,S,D - Player movement (in first person view), Move camera (in map view)\nM - show top-down map\nUp Arrow Key - Zoom in (in map view only)\nDown Arrow Key - Zoom out (in map view only)");
			ImGui::Separator();
			ImGui::Text("Map Configuration");
			ImGui::SliderInt("Maze Width", &m_mazeWidth, 5, 20);
			ImGui::SliderInt("Maze Height", &m_mazeHeight, 5, 20);
			ImGui::SliderInt("Number of required items", &m_numberOfItems, 0, 10);
			ImGui::SliderFloat("Mouse Sensitivity", &m_player.m_camRotRate, 40.f, 100.f);
			if (ImGui::Button("Generate Level & Start Game"))
			{
				GenerateMaze();
			}
			ImGui::End();
		}
	}

	if (m_input.Quit())
	{
		ExitGame();
	}
	
}

bool Game::CheckWallsCollision(Vector3 position)
{
	// return true for allowing movement
	float offset = 0.016f;
	for (vector<Wall*>::iterator it = m_walls.begin(); it != m_walls.end(); ++it)
	{
		Wall* current_wall = *it;
		Vector3 wall_position = current_wall->GetPosition();
		Vector3 wall_size = current_wall->GetSize();
		if (position.x >= (wall_position.x - (wall_size.x / 2) - offset) && position.x <= (wall_position.x + (wall_size.x / 2) + offset) &&
			position.y >= (wall_position.y - (wall_size.y / 2)) && position.y <= (wall_position.y + (wall_size.y / 2)) &&
			position.z >= (wall_position.z - (wall_size.z / 2) - offset) && position.z <= (wall_position.z + (wall_size.z / 2) + offset))
		{
			return false;
		}
	}
	return true;
}

void Game::CheckInteractablesCollision(Vector3 position)
{
	float offset = 0.f;
	for (vector<Interactable*>::iterator it = m_interactables.begin(); it != m_interactables.end(); ++it)
	{
		Interactable* current_interactable = *it;
		if (current_interactable->GetEnabled())
		{
			Vector3 interactable_position = current_interactable->GetPosition();
			Vector3 interactable_size = current_interactable->GetCollisionSize();
			if (position.x >= (interactable_position.x - (interactable_size.x / 2) - offset) && position.x <= (interactable_position.x + (interactable_size.x / 2) + offset) &&
				position.y >= (interactable_position.y - (interactable_size.y / 2)) && position.y <= (interactable_size.y + (interactable_size.y / 2)) &&
				position.z >= (interactable_position.z - (interactable_size.z / 2) - offset) && position.z <= (interactable_position.z + (interactable_size.z / 2) + offset))
			{
				// touch
				if (current_interactable->GetType() == InteractableType::GOAL)
				{
					if (m_itemsCollected < m_numberOfItems)
					{
						m_timeToHideTip = m_gameTime + 2.f;
					}
					else
					{
						m_win = true;
						m_started = false;
						m_input.SetMouseMode(Mouse::Mode::MODE_ABSOLUTE);
					}
				}
				else if (current_interactable->GetType() == InteractableType::ITEM)
				{
					current_interactable->SetEnabled(false);
					m_itemsCollected++;
				}
			}

			if (current_interactable->GetType() == InteractableType::GOAL)
			{
				current_interactable->RotateBackAndForth(m_timer.GetElapsedSeconds());
			}
		}
	}
}

void Game::UpdateOverview(float deltaTime)
{
	float width = (m_mazeWidth * 0.1f);
	float height = (m_mazeHeight * 0.1f);
	if (m_gameInputCommands.forward && m_overviewPosition.x < height)
	{

		m_overviewPosition.x += (m_player.getMoveSpeed() * deltaTime);
	}
	if (m_gameInputCommands.back && m_overviewPosition.x > -height)
	{
		m_overviewPosition.x -= (m_player.getMoveSpeed() * deltaTime);
	}
	if (m_gameInputCommands.left && m_overviewPosition.z > -width)
	{
		m_overviewPosition.z -= (m_player.getMoveSpeed() * deltaTime);
	}
	if (m_gameInputCommands.right && m_overviewPosition.z < width)
	{
		m_overviewPosition.z += (m_player.getMoveSpeed() * deltaTime);

	}
	if (m_gameInputCommands.rotUp && m_overviewPosition.y > 1.f)
	{
		m_overviewPosition.y -= (m_player.getMoveSpeed() * deltaTime);
	}
	if (m_gameInputCommands.rotDown && m_overviewPosition.y < 3.f)
	{
		m_overviewPosition.y += (m_player.getMoveSpeed() * deltaTime);
	}
}

void Game::UpdateCameraMovement(float deltaTime)
{
	Vector3 position = m_player.getPosition(); //get the position
	if (m_gameInputCommands.forward)
	{
		
		position += (m_player.getMoveVector() * m_player.getMoveSpeed() * deltaTime); //add the forward vector
	}
	if (m_gameInputCommands.back)
	{
		position -= (m_player.getMoveVector() * m_player.getMoveSpeed() * deltaTime); //add the forward vector
	}
	if (m_gameInputCommands.left)
	{
		Vector3 rightVector;
		m_player.getMoveVector().Cross(DirectX::SimpleMath::Vector3::UnitY, rightVector);
		position -= (rightVector * m_player.getMoveSpeed() * deltaTime); //add the forward vector
	}
	if (m_gameInputCommands.right)
	{
		Vector3 rightVector;
		m_player.getMoveVector().Cross(DirectX::SimpleMath::Vector3::UnitY, rightVector);
		position += (rightVector * m_player.getMoveSpeed() * deltaTime); //add the forward vector

	}

	//// mouse
	Vector3 mouseDelta = m_gameInputCommands.mouseDelta;
	Vector3 rotation = m_player.getRotation();
	rotation.x = rotation.x + (m_player.getRotationSpeed() * deltaTime * (mouseDelta.y < 50.f ? mouseDelta.y : 0.f));

	rotation.y = rotation.y + (m_player.getRotationSpeed() * deltaTime * (mouseDelta.x < 50.f ? mouseDelta.x : 0.f));
	if (rotation.x < 0)
	{
		rotation.x = 0.001;
	}
	else if (rotation.x > 180)
	{
		rotation.x = 180;
	}

	if (rotation.y < -180)
	{
		rotation.y += 360;
	}
	else if (rotation.y > 180)
	{
		rotation.y -= 360;
	}

	if (CheckWallsCollision(position))
	{

		m_player.setPosition(position);
	}
	
	m_player.setRotation(rotation);

	m_player.Update();	//camera update.
	m_view = m_player.getCameraMatrix();
	m_world = Matrix::Identity;
}

void Game::StartGame()
{
	m_started = true;
	m_input.SetMouseMode(Mouse::MODE_RELATIVE);
	m_gameTime = 0.f;
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();

	std::wstring ws1 = std::to_wstring(m_timer.GetFramesPerSecond()) + L" FPS";
	//ws1 += L"\nCam:" + std::to_wstring(m_player.getPosition().x) + L"," + std::to_wstring(m_player.getPosition().y) + L"," + std::to_wstring(m_player.getPosition().z);
	//ws1 += L"\nCamRot:" + std::to_wstring(m_player.getRotation().x) + L"," + std::to_wstring(m_player.getRotation().y) + L"," + std::to_wstring(m_player.getRotation().z);
	//ws1 += L"\nWalls:" + std::to_wstring(m_walls.size());
	//ws1 += L"\nLight:" + std::to_wstring(lightDirection.x) + L"," + std::to_wstring(lightDirection.y) + L"," + std::to_wstring(lightDirection.z);
	//ws1 += L"\nMouse:" + std::to_wstring(m_gameInputCommands.mouseDelta.x) + L"," + std::to_wstring(m_gameInputCommands.mouseDelta.y);
	const wchar_t* fps = ws1.c_str();

    // rendering code here.
	m_batchEffect->SetView(m_view);
	m_batchEffect->SetProjection(m_projection);
	m_batchEffect->SetLightDirection(0, lightDirection);
	m_batchEffect->SetLightDiffuseColor(0, lightDiffuseColor);
	m_batchEffect->SetLightDirection(1, Vector3(0.5f, -1.f, 0.25f));
	m_batchEffect->SetLightDiffuseColor(1, Vector4(0.35f, 0.35f, 0.35f, 1.f));

	//if (!m_isOverview && m_overviewSwitchTime > m_overviewSwitchingTime)
	{
		// render skybox
		m_world = Matrix::Identity;
		/*Matrix newPosition = SimpleMath::Matrix::CreateTranslation(m_player.getPosition());
		m_world = m_world * newPosition;*/
		Matrix newScale = Matrix::CreateScale(10.f);
		m_world = m_world * newScale;
		m_skyboxModel->UpdateEffects([&](IEffect* effect)
			{
				auto bEffect = dynamic_cast<BasicEffect*>(effect);
				if (bEffect)
				{
					bEffect->SetTexture(m_skyboxTex.Get());
				}
			});
		m_skyboxModel->Draw(context, *m_states, m_world, m_view, m_projection);
	}

	// render wall
	m_batchEffect->SetTexture(m_wallTex.Get());
	for (vector<Wall*>::iterator it = m_walls.begin(); it != m_walls.end(); ++it)
	{
		Wall* current_wall = *it;
		current_wall->Render(context, m_batchEffect.get(), m_batchInputLayout.Get(), m_states.get(), m_view, m_projection);
	}

	// render interactables
	for (vector<Interactable*>::iterator it = m_interactables.begin(); it != m_interactables.end(); ++it)
	{
		Interactable* current_interactable = *it;
		if (current_interactable->GetType() == InteractableType::ITEM)
		{
			current_interactable->UpdateEffect(m_deviceResources->GetD3DDevice(), m_batchEffect);
			m_batchEffect->SetTexture(m_itemTex.Get());
		}
		current_interactable->Render(context, m_batchEffect.get(), m_batchInputLayout.Get(), m_states.get(), m_view, m_projection);
	}

	//m_world = SimpleMath::Matrix::Identity; //set world back to identity
	//Matrix newPosition2 = SimpleMath::Matrix::CreateTranslation(0.0f, 0.2f, -1.f);
	//Matrix newScale = SimpleMath::Matrix::CreateScale(0.25f);
	//m_world = m_world * newScale * newPosition2;
	//for (auto it = m_guideModel->meshes.cbegin(); it != m_guideModel->meshes.cend(); ++it)
	//{
	//	auto mesh = it->get();
	//	assert(mesh != nullptr);

	//	for (auto it = mesh->meshParts.cbegin(); it != mesh->meshParts.cend(); ++it)
	//	{
	//		if ((*it)->effect)
	//		{
	//			shared_ptr<IEffect> effect = std::dynamic_pointer_cast<IEffect>(m_batchEffect);
	//			(*it)->ModifyEffect(m_deviceResources->GetD3DDevice(), effect, false);
	//		}
	//	}
	//	//current_wall.Render(context, m_batchEffect.get(), m_batchInputLayout.Get(), m_states.get(), m_view, m_projection);
	//}
	//m_guideModel->Modified();
	//m_guideModel->Draw(context, *m_states, m_world, m_view, m_projection, false);

	m_batchEffect->SetTexture(m_groundTex.Get());
	m_ground.Render(context, m_batchEffect.get(), m_batchInputLayout.Get(), m_states.get(), m_view, m_projection);

    m_deviceResources->PIXEndEvent();

	// Draw Text to the screen
	m_deviceResources->PIXBeginEvent(L"Draw sprite");
	m_sprites->Begin();
	m_font->DrawString(m_sprites.get(), fps, XMFLOAT2(10, 10), Colors::Gray);
	m_sprites->End();
	m_deviceResources->PIXEndEvent();

	auto renderTarget = m_deviceResources->GetRenderTargetView();
	context->OMSetRenderTargets(1, &renderTarget, nullptr);
	m_postProcess->Process(context);

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // Show the new frame.
    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    //context->ClearRenderTargetView(renderTarget, color);
    //context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    //context->OMSetRenderTargets(1, &renderTarget, depthStencil);
	context->ClearRenderTargetView(m_sceneRT.Get(), Colors::Black);
	context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	context->OMSetRenderTargets(1, m_sceneRT.GetAddressOf(), depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowMoved()
{
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

	m_sceneTex.Reset();
	m_sceneRT.Reset();
	m_sceneSRV.Reset();

	m_rtDesc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_B8G8R8A8_UNORM, width, height,
		1, 1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);

	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateTexture2D(&m_rtDesc, nullptr, m_sceneTex.GetAddressOf())
	);

	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateShaderResourceView(m_sceneTex.Get(), nullptr,
			m_sceneSRV.ReleaseAndGetAddressOf())
	);

	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateRenderTargetView(m_sceneTex.Get(), nullptr,
			m_sceneRT.ReleaseAndGetAddressOf()
		));
    CreateWindowSizeDependentResources();
    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
    width = 1280;
    height = 720;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
    auto device = m_deviceResources->GetD3DDevice();

	m_sprites = std::make_unique<SpriteBatch>(context);
	m_font = std::make_unique<SpriteFont>(device, L"Assets/Fonts/SegoeUI_18.spritefont");

	m_states = std::make_unique<CommonStates>(device);

	m_fxFactory = std::make_unique<EffectFactory>(device);

	m_sprites = std::make_unique<SpriteBatch>(context);

	m_postProcess = std::make_unique<BasicPostProcess>(device);

	m_batchEffect = std::make_unique<BasicEffect>(device);
	m_batchEffect->SetTextureEnabled(true);
	m_batchEffect->SetPerPixelLighting(true);
	m_batchEffect->SetLightingEnabled(true);
	m_batchEffect->SetLightEnabled(0, true);
	m_batchEffect->SetLightEnabled(1, true);
	m_batchEffect->SetLightEnabled(2, false);

	{
		void const* shaderByteCode;
		size_t byteCodeLength;

		m_batchEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

		DX::ThrowIfFailed(
			device->CreateInputLayout(VertexPositionNormalColorTexture::InputElements,
				VertexPositionNormalColorTexture::InputElementCount,
				shaderByteCode, byteCodeLength,
				m_batchInputLayout.ReleaseAndGetAddressOf())
		);
	}

	m_fxFactory->SetDirectory(L"Assets/Textures");
	m_fxFactory->EnableNormalMapEffect(true);
	m_guideModel = Model::CreateFromCMO(device, L"Assets/Models/untitled.cmo", *m_fxFactory);

	CreateDDSTextureFromFile(device, L"Assets/Textures/grey.dds", nullptr, m_groundTex.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"Assets/Textures/white.dds", nullptr, m_wallTex.ReleaseAndGetAddressOf());
	CreateWICTextureFromFile(device, L"Assets/Textures/skybox.png", nullptr, m_skyboxTex.ReleaseAndGetAddressOf());
	CreateWICTextureFromFile(device, L"Assets/Textures/item.png", nullptr, m_itemTex.ReleaseAndGetAddressOf());

	auto r = m_deviceResources->GetOutputSize();
	m_rtDesc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_B8G8R8A8_UNORM, r.right, r.bottom,
		1, 1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);

	DX::ThrowIfFailed(
		device->CreateTexture2D(&m_rtDesc, nullptr, m_sceneTex.GetAddressOf())
	);

	DX::ThrowIfFailed(
		device->CreateShaderResourceView(m_sceneTex.Get(), nullptr,
			m_sceneSRV.ReleaseAndGetAddressOf())
	);

	DX::ThrowIfFailed(
		device->CreateRenderTargetView(m_sceneTex.Get(), nullptr,
			m_sceneRT.ReleaseAndGetAddressOf()
		));

	m_postProcess->SetEffect(BasicPostProcess::BloomBlur);
	m_postProcess->SetSourceTexture(m_sceneSRV.Get());

	m_skyboxModel = Model::CreateFromCMO(device, L"Assets/Models/skybox.cmo", *m_fxFactory);
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
	auto size = m_deviceResources->GetOutputSize();
	float aspectRatio = float(size.right) / float(size.bottom);
	float fovAngleY = 70.f * XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	m_projection = Matrix::CreatePerspectiveFieldOfView(
		fovAngleY,
		aspectRatio,
		0.01f,
		100.0f
	);

	m_batchEffect->SetView(m_view);
	m_batchEffect->SetProjection(m_projection);
}

void Game::OnDeviceLost()
{
	m_sprites.reset();
	m_font.reset();

	m_states.reset();
	m_fxFactory.reset();
	m_sprites.reset();

	m_batchEffect.reset();
	m_batchInputLayout.Reset();
	m_groundTex.Reset();

	m_postProcess.reset();
	m_sceneTex.Reset();
	m_sceneSRV.Reset();
	m_sceneRT.Reset();

	m_ground.Free();
	for (vector<Wall*>::iterator it = m_walls.begin(); it != m_walls.end(); ++it)
	{
		Wall* current_wall = *it;
		current_wall->Free();
	}
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
