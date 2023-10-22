#include "stdafx.h"
#include "WindowManager.h"
#include "SystemManager.h"
#include "ViewBuffer.h"

bool WindowManager::init()
{
    m_isRunning = initWaterfall(this, {
        &WindowManager::initWindow,
        &WindowManager::initDirectX,
        &WindowManager::initInput,
        &WindowManager::initSSAO
        });
    return m_isRunning;
}

void WindowManager::clear()
{
    m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), (f32*)(&m_clearColor));
    clearDepth();
}

void WindowManager::clearDepth()
{
    m_deviceContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void WindowManager::swap()
{
    m_swapchain->Present(1, 0);
}

void WindowManager::bindViewBuffer()
{
    //auto& camera = m_viewBuffer.m_cameraPos;
    m_viewBuffer = {
        .m_world = XMMatrixTranspose(m_world),
        .m_view = XMMatrixTranspose(m_view),
        .m_projection = XMMatrixTranspose(m_projection),
        .m_cameraPos = { m_camera.x, m_camera.y, m_camera.z, 1.0f },
    };
    inverseTranspose(m_viewBuffer.m_world, m_viewBuffer.m_invWorld);

    D3D11_MAPPED_SUBRESOURCE viewBufferResource;

    HRESULT result = m_deviceContext->Map(
        m_viewBufferBuffer.Get(),
        0,
        D3D11_MAP_WRITE_DISCARD,
        0,
        &viewBufferResource
    );
    {
        if (FAILED(result))
        {
            cerr << "Failed to lock global buffer! Got 0x" << hex << result << endl;
            return;
        }

        ViewBuffer* localViewBuffer = (ViewBuffer*)viewBufferResource.pData;
        CopyMemory(localViewBuffer, &m_viewBuffer, sizeof(ViewBuffer));
    }

    m_deviceContext->Unmap(m_viewBufferBuffer.Get(), 0);
    m_deviceContext->VSSetConstantBuffers(0, 1, m_viewBufferBuffer.GetAddressOf());
    m_deviceContext->PSSetConstantBuffers(0, 1, m_viewBufferBuffer.GetAddressOf());
}

void WindowManager::update()
{
    MSG msg = { 0 };
    for (u32 i = 0; i < 256; i++)
    {
        m_keys[i].edge = false;
    }

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (msg.message == WM_QUIT)
        {
            m_isRunning = false;
        }
    }

    updateMouse();
}

void WindowManager::draw()
{

}

LRESULT CALLBACK WindowManager::messageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
    switch (umsg)
    {
    case WM_KEYDOWN:
        onKeyDown((UINT)wparam);
        return 0;
    case WM_KEYUP:
        onKeyUp((UINT)wparam);
        return 0;
    case WM_ACTIVATEAPP:
    case WM_INPUT:
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MOUSEWHEEL:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    case WM_MOUSEHOVER:
        Mouse::ProcessMessage(umsg, wparam, lparam);
    default:
        return DefWindowProc(hwnd, umsg, wparam, lparam);
    }
}

void WindowManager::setWorld(const XMMATRIX& world)
{
    m_world = world;
}

void WindowManager::setView(const XMMATRIX& view)
{
    m_view = view;
}

void WindowManager::setProjection(const XMMATRIX& projection)
{
    m_projection = projection;
}

void WindowManager::setCameraPos(const XMFLOAT3& cameraPos)
{
    m_camera = cameraPos;
}

void WindowManager::setRotation(const XMFLOAT3& rotation)
{
    m_rotation = rotation;
}

void WindowManager::setMouseLook(bool mouseLook)
{
    m_mouseLook = mouseLook;
    m_mouse->SetMode(mouseLook ? Mouse::MODE_RELATIVE : Mouse::MODE_ABSOLUTE);
    ShowCursor(!mouseLook);
}

void WindowManager::useSSAO(bool use)
{
    if (use)
    {
        WND->getContext()->OMSetRenderTargets(1, m_ssaoRenderTargetView.GetAddressOf(), m_depthStencilView.Get());
    }
    else
    {
        m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
    }

}

i32 WindowManager::getMouseX() const
{
    return m_mouseX;
}

i32 WindowManager::getMouseY() const
{
    return m_mouseY;
}

bool WindowManager::keyDown(u32 key) const
{
    if (key >= 256)
    {
        return false;
    }
    return m_keys[key].down;
}

bool WindowManager::edgeDown(u32 key) const
{
    if (key >= 256)
    {
        return false;
    }
    return m_keys[key].down && m_keys[key].edge;
}

bool WindowManager::keyUp(u32 key) const
{
    if (key >= 256)
    {
        return false;
    }
    return !m_keys[key].down;
}

bool WindowManager::edgeUp(u32 key) const
{
    if (key >= 256)
    {
        return false;
    }
    return !m_keys[key].down && m_keys[key].edge;
}

ID3D11Device* const WindowManager::getDevice() const
{
    return m_device.Get();
}

ID3D11DeviceContext* const WindowManager::getContext() const
{
    return m_deviceContext.Get();
}

ID3D11InfoQueue* const WindowManager::getInfoQueue() const
{
    return m_infoQueue.Get();
}

bool WindowManager::isRunning() const
{
    return m_isRunning;
}

/* ---------- Private Methods ---------- */

bool WindowManager::initWindow()
{
    m_instance = GetModuleHandle(NULL);
    WNDCLASSEX wc = { 0 };
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_instance;
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    wc.hIconSm = wc.hIcon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = L"Blue";
    wc.cbSize = sizeof(WNDCLASSEX);

    if (!RegisterClassEx(&wc))
    {
        return false;
    }

    u16 screenWidth, screenHeight;

    if (SYS->getVars().m_fullscreen.value())
    {
        screenWidth = GetSystemMetrics(SM_CXSCREEN);
        screenHeight = GetSystemMetrics(SM_CYSCREEN);
    }
    else
    {
        screenWidth = SYS->getVars().m_width.value();
        screenHeight = SYS->getVars().m_height.value();
    }

    int posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2,
        posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;

    m_window = CreateWindowExW(
        WS_EX_APPWINDOW,
        L"Blue",
        L"Blue",
        WS_TILEDWINDOW,
        posX, posY,
        screenWidth, screenHeight,
        NULL,
        NULL,
        m_instance,
        NULL
    );
    if (!m_window)
    {
        cerr << "Failed to create Window!";
        LocalAllocString str;
        if (TryGetLastErrorMessageA(str))
        {
            cerr << " Got: " << str;
        }
        cerr << endl;
        return false;
    }

    ShowWindow(m_window, SW_SHOW);
    SetForegroundWindow(m_window);
    SetFocus(m_window);

    RECT rect;
    GetClientRect(m_window, &rect);
    m_clientWidth = (f32)rect.right;
    m_clientHeight = (f32)rect.bottom;

    HRESULT result = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(result))
    {
        return false;
    }

    return true;

}

bool WindowManager::initDirectX()
{
    initWaterfall(this, {
        &WindowManager::initInfrastructure,
        &WindowManager::initSwapchain,
        &WindowManager::initBackBuffer,
        &WindowManager::initDepthStencilBuffer,
        &WindowManager::initRasterizer,
        &WindowManager::initGlobalBuffer
        });
    m_camera = XMFLOAT3(0, 0, 0);
    m_rotation = XMFLOAT3(0, 0, 0);
    return true;
}

bool WindowManager::initInfrastructure()
{
    ComPtr<IDXGIFactory> factory = NULL;
    if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory)))
    {
        return false;
    }

    IDXGIAdapter* adapter = NULL;
    if (FAILED(factory->EnumAdapters(0, &adapter)))
    {
        return false;
    }

    ComPtr<IDXGIOutput> output = NULL;
    if (FAILED(adapter->EnumOutputs(0, &output)))
    {
        return false;
    }

    //Count total
    UINT numModes = 0;
    if (FAILED(output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL)))
    {
        return false;
    }

    //Actually enumerate now
    DXGI_MODE_DESC* displayModeList = new DXGI_MODE_DESC[numModes];
    ZeroMemory(displayModeList, numModes * sizeof(DXGI_MODE_DESC));
    if (FAILED(output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList)))
    {
        return false;
    }

    auto& vars = g_system->getVars();

    for (UINT i = 0; i < numModes; i++)
    {
        if (
            (displayModeList[i].Width == (UINT)vars.m_width.value()) &&
            (displayModeList[i].Height == (UINT)vars.m_height.value())
            )
        {
            m_numerator = displayModeList[i].RefreshRate.Numerator;
            m_denominator = displayModeList[i].RefreshRate.Denominator;
            break;
        }
    }

    delete[] displayModeList;

    return true;
}

bool WindowManager::initSwapchain()
{
    auto& vars = g_system->getVars();

    DXGI_SWAP_CHAIN_DESC swapchainDesc = { };
    ZeroMemory(&swapchainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
    swapchainDesc.BufferCount = 1;
    swapchainDesc.BufferDesc.Width = static_cast<u32>(m_clientWidth);
    swapchainDesc.BufferDesc.Height = static_cast<u32>(m_clientHeight);
    swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchainDesc.BufferDesc.RefreshRate.Numerator = m_numerator; // or 0 if !vsync
    swapchainDesc.BufferDesc.RefreshRate.Denominator = m_denominator; // or 1 if !vsync
    swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc.OutputWindow = m_window;
    swapchainDesc.SampleDesc.Count = 4;
    swapchainDesc.SampleDesc.Quality = 0;
    swapchainDesc.Windowed = !vars.m_fullscreen.value();
    swapchainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapchainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapchainDesc.Flags = 0;

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

    if (
        FAILED(
            D3D11CreateDeviceAndSwapChain(
                NULL,
                D3D_DRIVER_TYPE_HARDWARE,
                NULL,
                vars.m_dxDebug.value() ? (BYTE)D3D11_CREATE_DEVICE_DEBUG : 0,
                &featureLevel,
                1,
                D3D11_SDK_VERSION,
                &swapchainDesc,
                m_swapchain.GetAddressOf(),
                m_device.GetAddressOf(),
                NULL,
                m_deviceContext.GetAddressOf()
            )
        )
        )
    {
        return false;
    }

    if (vars.m_dxDebug.value())
    {
        if (FAILED(m_device->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)m_infoQueue.GetAddressOf())))
        {
            cerr << "No debugger info queue available" << endl;
        }
    }

    ComPtr<IDXGIOutput> output;
    m_swapchain->GetContainingOutput(output.GetAddressOf());

    return true;
}

bool WindowManager::initBackBuffer()
{
    ComPtr<ID3D11Texture2D> backBuffer;
    if (FAILED(m_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)(backBuffer.GetAddressOf()))))
    {
        return false;
    }

    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    ZeroMemory(&rtvDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;

    if (FAILED(m_device->CreateRenderTargetView(backBuffer.Get(), &rtvDesc, m_renderTargetView.GetAddressOf())))
    {
        return false;
    }

    return true;
}
bool WindowManager::initDepthStencilBuffer()
{
    D3D11_TEXTURE2D_DESC depthBufferDesc = { };
    ZeroMemory(&depthBufferDesc, sizeof(D3D11_TEXTURE2D_DESC));
    depthBufferDesc.Width = static_cast<u32>(m_clientWidth);
    depthBufferDesc.Height = static_cast<u32>(m_clientHeight);
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.SampleDesc.Count = 4;
    depthBufferDesc.SampleDesc.Quality = 0;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthBufferDesc.CPUAccessFlags = 0;
    depthBufferDesc.MiscFlags = 0;

    if (FAILED(m_device->CreateTexture2D(&depthBufferDesc, NULL, m_depthStencilBuffer.GetAddressOf())))
    {
        return false;
    }

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = { };
    ZeroMemory(&depthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthStencilDesc.StencilEnable = true;
    depthStencilDesc.StencilReadMask = 0xff;
    depthStencilDesc.StencilWriteMask = 0xff;
    depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    if (FAILED(m_device->CreateDepthStencilState(&depthStencilDesc, m_depthStencilState.GetAddressOf())))
    {
        return false;
    }

    depthStencilDesc.DepthEnable = false;

    if (FAILED(m_device->CreateDepthStencilState(&depthStencilDesc, m_noTestDepthStencilState.GetAddressOf())))
    {
        return false;
    }

    m_deviceContext->OMSetDepthStencilState(m_depthStencilState.Get(), 1);

    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = { };
    ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    if (FAILED(m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &depthStencilViewDesc, m_depthStencilView.GetAddressOf())))
    {
        return false;
    }

    m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

    return true;
}

bool WindowManager::initRasterizer()
{
    auto& vars = g_system->getVars();

    D3D11_RASTERIZER_DESC rasterDesc = { };
    ZeroMemory(&rasterDesc, sizeof(D3D11_RASTERIZER_DESC));
    rasterDesc.AntialiasedLineEnable = true;
    rasterDesc.CullMode = D3D11_CULL_FRONT; // CW triangles. Might change later
    rasterDesc.DepthBias = 0;
    rasterDesc.DepthBiasClamp = 0.0f;
    rasterDesc.DepthClipEnable = true;
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.MultisampleEnable = true;
    rasterDesc.ScissorEnable = false;
    rasterDesc.SlopeScaledDepthBias = 0.0f;
    rasterDesc.FrontCounterClockwise = false;

    if (FAILED(m_device->CreateRasterizerState(&rasterDesc, m_sceneRasterizer.GetAddressOf())))
    {
        return false;
    }

    rasterDesc.CullMode = D3D11_CULL_NONE;
    rasterDesc.FillMode = D3D11_FILL_WIREFRAME;

    if (FAILED(m_device->CreateRasterizerState(&rasterDesc, m_wireRasterizer.GetAddressOf())))
    {
        return false;
    }


    rasterDesc.DepthBias = 5;
    rasterDesc.DepthBiasClamp = 0.0f;
    rasterDesc.SlopeScaledDepthBias = 4.0f;
    //rasterDesc.CullMode = D3D11_CULL_FRONT;
    rasterDesc.FillMode = D3D11_FILL_SOLID;

    if (FAILED(m_device->CreateRasterizerState(&rasterDesc, m_shadowMapRasterizer.GetAddressOf())))
    {
        return false;
    }

    m_deviceContext->RSSetState(m_sceneRasterizer.Get());

    ZeroMemory(&m_viewport, sizeof(D3D11_VIEWPORT));
    m_viewport.Width = m_clientWidth;
    m_viewport.Height = m_clientHeight;
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;
    m_viewport.TopLeftX = 0.0f;
    m_viewport.TopLeftY = 0.0f;

    m_deviceContext->RSSetViewports(1, &m_viewport);
    float fov = vars.m_fov.value_or(M_PI_4);
    float aspect = m_clientWidth / m_clientHeight;

    m_projection = XMMatrixPerspectiveFovLH(fov, aspect, 0.1f, 10000.0f);
    m_ortho = XMMatrixOrthographicLH((float)vars.m_width.value(), (float)vars.m_height.value(), 0.1f, 10000.0f);
    m_world = XMMatrixIdentity();

    return true;
}

bool WindowManager::initGlobalBuffer()
{
    D3D11_BUFFER_DESC matrixBufferDesc;
    ZeroMemory(&matrixBufferDesc, sizeof(D3D11_BUFFER_DESC));
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(ViewBuffer);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    HRESULT result = m_device->CreateBuffer(&matrixBufferDesc, NULL, m_viewBufferBuffer.GetAddressOf());
    if (FAILED(result))
    {
        return false;
    }

    ZeroMemory(&m_viewBuffer, sizeof(ViewBuffer));

    return true;
}

bool WindowManager::initInput()
{
    m_mouse = new Mouse;
    m_mouse->SetWindow(m_window);
    m_mouse->SetMode(Mouse::MODE_RELATIVE);

    ZeroMemory(m_keys, sizeof(m_keys));
    m_mouseLook = true;
    setMouseLook(m_mouseLook);

    return true;
}

bool WindowManager::initSSAO()
{
    D3D11_TEXTURE2D_DESC ssaoDesc;
    ssaoDesc.Width = static_cast<u32>(m_clientWidth);
    ssaoDesc.Height = static_cast<u32>(m_clientHeight);
    ssaoDesc.MipLevels = 1;
    ssaoDesc.ArraySize = 1;
    ssaoDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    ssaoDesc.SampleDesc.Count = 1;
    ssaoDesc.SampleDesc.Quality = 0;
    ssaoDesc.Usage = D3D11_USAGE_DEFAULT;
    ssaoDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
    ssaoDesc.CPUAccessFlags = 0;
    ssaoDesc.MiscFlags = 0;

    if (FAILED(m_device->CreateTexture2D(&ssaoDesc, NULL, m_ssaoMap.GetAddressOf())))
    {
        return false;
    }

    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    ZeroMemory(&rtvDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
    rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

    if (FAILED(m_device->CreateRenderTargetView(m_ssaoMap.Get(), &rtvDesc, m_ssaoRenderTargetView.GetAddressOf())))
    {
        return false;
    }

    return true;
}

void WindowManager::updateMouse()
{
    auto state = m_mouse->GetState();
    m_mouseX = state.x;
    m_mouseY = state.y;
    m_mouseLook = true;
}

void WindowManager::onKeyDown(u32 key)
{
    if (key >= 256)
    {
        return;
    }
    if (!m_keys[key].down)
    {
        m_keys[key].edge = true;
    }
    m_keys[key].down = true;
}

void WindowManager::onKeyUp(u32 key)
{
    if (key >= 256)
    {
        return;
    }
    if (m_keys[key].down)
    {
        m_keys[key].edge = true;
    }
    m_keys[key].down = false;
}

inline void WindowManager::inverseTranspose(const XMMATRIX& world, XMMATRIX& invWorld)
{
    CopyMemory(&invWorld, &world, sizeof(XMMATRIX));
    invWorld.r[3] = XMVectorSet(0, 0, 0, 1.0f);
    XMVECTOR determinant = XMMatrixDeterminant(invWorld);
    invWorld = XMMatrixTranspose(XMMatrixInverse(&determinant, invWorld));
}

/* ---------- WNDPROC/Handlers ----------*/

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
    {
        return true;
    }

    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_CLOSE:
        PostQuitMessage(0);
        //System.setRunning(false);
        return 0;
    default:
        return g_window->messageHandler(hWnd, uMsg, wParam, lParam);
    }
}
