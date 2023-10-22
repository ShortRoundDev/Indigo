#pragma once

#include "ViewBuffer.h"

using namespace DirectX;
using namespace Microsoft::WRL;

struct KeyState
{
    bool down;
    bool edge;
    u64 downTime;
};

class WindowManager
{
public:
    bool init();

    void clear();
    void clearDepth();
    void swap();
    void bindViewBuffer();

    void update();
    void draw();

    LRESULT CALLBACK messageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

    void setWorld(const XMMATRIX& world);
    void setView(const XMMATRIX& view);
    void setProjection(const XMMATRIX& projection);
    void setCameraPos(const XMFLOAT3& cameraPos);
    void setRotation(const XMFLOAT3& rotation);
    void setMouseLook(bool mouseLook);

    void useSSAO(bool use);

    i32 getMouseX() const;
    i32 getMouseY() const;

    bool keyDown(u32 key) const;
    bool edgeDown(u32 key) const;
    bool keyUp(u32 key) const;
    bool edgeUp(u32 key) const;

    ID3D11Device* const getDevice() const;
    ID3D11DeviceContext* const getContext() const;
    ID3D11InfoQueue* const getInfoQueue() const;

    bool isRunning() const;

private:
    bool initWindow();
    bool initDirectX();

    bool initInfrastructure();
    bool initSwapchain();
    bool initBackBuffer();
    bool initDepthStencilBuffer();
    bool initRasterizer();
    bool initGlobalBuffer();
    bool initInput();
    bool initSSAO();

    void updateMouse();

    void onKeyDown(u32 key);
    void onKeyUp(u32 key);

    inline void inverseTranspose(const XMMATRIX& world, XMMATRIX& invWorld);

    HWND        m_window;
    HINSTANCE   m_instance;

    bool m_isRunning;
    ///// INPUT STUFF /////
    Mouse* m_mouse;
    KeyState m_keys[256];
    bool m_mouseLook;

    i32 m_mouseX;
    i32 m_mouseY;

    ///// DIRECTX STUFF /////
    ComPtr<IDXGISwapChain> m_swapchain;
    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_deviceContext;
    ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    ComPtr<ID3D11RenderTargetView> m_ssaoRenderTargetView;
    ComPtr<ID3D11Texture2D> m_depthStencilBuffer;
    ComPtr<ID3D11DepthStencilState> m_depthStencilState;
    ComPtr<ID3D11DepthStencilState> m_noTestDepthStencilState;
    ComPtr<ID3D11DepthStencilView> m_depthStencilView;
    ComPtr<ID3D11RasterizerState> m_sceneRasterizer;
    ComPtr<ID3D11RasterizerState> m_wireRasterizer;
    ComPtr<ID3D11RasterizerState> m_shadowMapRasterizer;
    ComPtr<ID3D11InfoQueue> m_infoQueue;
    ComPtr<ID3D11Texture2D> m_ssaoMap;

    D3D11_VIEWPORT m_viewport;
    ViewBuffer m_viewBuffer;
    ComPtr<ID3D11Buffer> m_viewBufferBuffer;

    i32 m_numerator;
    i32 m_denominator;

    f32 m_clientWidth;
    f32 m_clientHeight;

    XMFLOAT4 m_clearColor = { 4.0f / 255.0f, 3.0f / 255.0f, 30.0f / 255.0f, 1.0f };

    XMMATRIX m_projection;
    XMMATRIX m_ortho;
    XMMATRIX m_world;
    XMMATRIX m_view;
    XMFLOAT3 m_camera;
    XMFLOAT3 m_rotation;
};

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

extern WindowManager* g_window;
#define WND (g_window)