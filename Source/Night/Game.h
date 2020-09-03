//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "Input.h"
#include "Player.h"
#include "Interactable.h"
#include "Wall.h"
#include "MazeGenerator.h"
#include "pch.h"

using namespace std;

// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game final : public DX::IDeviceNotify
{
public:

    Game() noexcept(false);

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // IDeviceNotify
    virtual void OnDeviceLost() override;
    virtual void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize( int& width, int& height ) const;

private:
    struct MatrixBufferType
    {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
    };

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();
    void UpdateCameraMovement(float deltaTime);
    void UpdateOverview(float deltaTime);
    bool CheckWallsCollision(Vector3 position);
    void CheckInteractablesCollision(Vector3 position);
    void StartGame();

    void GenerateMaze();

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;

    //input manager. 
    Input									m_input;
    InputCommands							m_gameInputCommands;

    DirectX::SimpleMath::Matrix             m_world;
    DirectX::SimpleMath::Matrix             m_view;
    DirectX::SimpleMath::Matrix             m_projection;

    std::unique_ptr<DirectX::SpriteBatch>   m_sprites;
    std::unique_ptr<DirectX::SpriteFont>    m_font;

    std::unique_ptr<DirectX::CommonStates>  m_states;
    std::shared_ptr<DirectX::BasicEffect>   m_batchEffect;
    std::unique_ptr<DirectX::EffectFactory> m_fxFactory;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_batchInputLayout;

    Player                                  m_player;
    std::unique_ptr<DirectX::Model>         m_guideModel;

    vector<Wall*>                            m_walls;
    vector<Interactable*>                    m_interactables;
    Wall                                    m_ground;
    Vector3                                 lightDirection;
    Vector4                                 lightDiffuseColor;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_groundTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_wallTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_itemTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_skyboxTex;

    bool                                    m_isOverview;
    bool                                    m_started;
    bool                                    m_win;

    int                                     m_mazeWidth;
    int                                     m_mazeHeight;
    MazeGenerator                           m_mazeGenerator;
    float                                   m_wallHeight;
    Vector3                                 m_overviewPosition;
    Vector3                                 m_overviewRotation;
    float                                   m_overviewSwitchTime;
    float                                   m_overviewSwitchingTime;
    float                                   m_gameTime;
    float                                   m_timeToHideTip;
    int                                     m_numberOfItems;
    int                                     m_itemsCollected;
    
    std::unique_ptr<DirectX::BasicPostProcess> m_postProcess;
    CD3D11_TEXTURE2D_DESC                   m_rtDesc;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_sceneTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_sceneSRV;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_sceneRT;
    std::unique_ptr<Model> m_skyboxModel;
};
