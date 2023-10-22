#include "stdafx.h"
#include "WindowManager.h"
#include "CubeMap.h"

CubeMap::CubeMap(string const& path) :
    m_success(true),
    m_resource()
{
    wstring windowsPath;
    if (!mbStrToWideChar(path, windowsPath))
    {
        std::cerr << "Failed to convert [" + path + "] to wide string!" << endl;
        m_success = false;
        return;
    }

    HRESULT res = CreateDDSTextureFromFile(
        WND->getDevice(),
        windowsPath.c_str(),
        nullptr, m_resource.GetAddressOf()
    );

    if (FAILED(res))
    {
        std::cerr << "Failed to load cubemap at [" << path << "].";
        LocalAllocString str;
        if (TryGetErrorMessageA(str, res))
        {
            std::cerr << " Got: " << str;
        }
        std::cerr << endl;
        m_success = false;

        return;
    }
}

void CubeMap::use(u32 slot)
{
    WND->getContext()->PSSetShaderResources(slot, 1, m_resource.GetAddressOf());
}

bool CubeMap::isSuccess() const
{
    return m_success;
}