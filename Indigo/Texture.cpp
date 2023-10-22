#include "stdafx.h"

#include "Texture.h"
#include "WindowManager.h"

using namespace DirectX;

Texture::Texture(wstring path) :
    m_resource(),
    m_texture(),
    m_view(),
    m_success(true)
{
    HRESULT res = CreateWICTextureFromFile(
        WND->getDevice(),
        WND->getContext(),
        path.c_str(),
        m_resource.GetAddressOf(),
        m_view.GetAddressOf()
    );
    if (FAILED(res))
    {
        wcerr << L"Failed to load texture [" << path << "]!";
        LocalAllocWString str;
        if (TryGetLastErrorMessageW(str))
        {
            wcerr << L" Got: " << str;
        }
        wcerr << endl;
        m_success = false;
        return;
    }
    WND->getContext()->GenerateMips(m_view.Get());

    res = m_resource->QueryInterface(IID_ID3D11Texture2D, (void**)m_texture.GetAddressOf());
    if (FAILED(res))
    {
        wcerr << L"Failed to create texture for path [" << path << "]!";
        LocalAllocWString str;
        if (TryGetLastErrorMessageW(str))
        {
            wcerr << " Got: " << str;
        }
        wcerr << endl;
        m_success = false;
        return;
    }

}

Texture::Texture(u8* data, sz size) :
    m_resource(),
    m_texture(),
    m_view()
{
    HRESULT res = CreateWICTextureFromMemory(
        WND->getDevice(),
        data,
        size,
        m_resource.GetAddressOf(),
        m_view.GetAddressOf()
    );
    if (FAILED(res))
    {
        wcerr << L"Failed to load texture from memory!";
        LocalAllocWString str;
        if (TryGetLastErrorMessageW(str))
        {
            wcerr << L" Got: " << str;
        }
        wcerr << endl;
        m_success = false;
        return;
    }
    WND->getContext()->GenerateMips(m_view.Get());

    res = m_resource->QueryInterface(IID_ID3D11Texture2D, (void**)m_texture.GetAddressOf());
    if (FAILED(res))
    {
        wcerr << L"Failed to create texture from memory!";
        LocalAllocWString str;
        if (TryGetLastErrorMessageW(str))
        {
            wcerr << " Got: " << str;
        }
        wcerr << endl;
        m_success = false;
        return;
    }
    m_success = true;
}

void Texture::use() const
{
    if (m_view == NULL)
    {
        return;
    }

    WND->getContext()->PSSetShaderResources(0, 1, m_view.GetAddressOf());
}

bool Texture::isSuccess() const
{
    return m_success;
}