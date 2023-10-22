#pragma once

using namespace DirectX;

struct ViewBuffer
{
    XMMATRIX m_world;
    XMMATRIX m_view;
    XMMATRIX m_projection;
    XMMATRIX m_invWorld;
    XMFLOAT4 m_cameraPos;
};