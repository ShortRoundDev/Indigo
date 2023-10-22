#pragma once
class CubeMap
{
public:
    CubeMap(string const& path);

    void use(u32 slot);

    bool isSuccess() const;
private:
    ComPtr<ID3D11ShaderResourceView> m_resource;
    bool m_success;
};

