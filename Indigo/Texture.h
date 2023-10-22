#pragma once

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;

class Texture
{
public:
    Texture(wstring path);
    Texture(u8* data, sz size);

    void use() const;
    bool isSuccess() const;

private:
    bool m_success;

    ComPtr<ID3D11Resource> m_resource;
    ComPtr<ID3D11Texture2D> m_texture;
    ComPtr<ID3D11ShaderResourceView> m_view;
};
