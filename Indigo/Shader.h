#pragma once

#include "ModelBuffer.h"

class Shader
{
public:
    Shader(
        ID3D11Device* device,
        wstring vertexPath,
        wstring pixelPath,
        sz bufferSize
    );

    void use();
    void bindModelBuffer(ModelBuffer const& modelBuffer);

    bool isSuccess() const;

private:
    ComPtr<ID3D11VertexShader> m_vertex;
    ComPtr<ID3D11PixelShader> m_pixel;
    ComPtr<ID3D11InputLayout> m_inputLayout;
    ComPtr<ID3D11Buffer> m_matrixBuffer;
    ComPtr<ID3D11SamplerState> m_samplerState;
    ComPtr<ID3D11BlendState> m_blendState;
    ComPtr<ID3D11Buffer> m_cBuffer;
    ComPtr<ID3D11Buffer> m_modelBuffer;

    bool m_success;

    bool initShaderCode(
        ID3D11Device* device,
        wstring vertexPath,
        wstring pixelPath,
        u8** vertexByteCode,
        sz* vertexByteCodeLength
    );
    bool initLayout(ID3D11Device* device, u8* byteCode, sz byteCodeLength);
    bool initSampler(ID3D11Device* device);
    bool initBlendState(ID3D11Device* device);
    bool initCBuffer(ID3D11Device* device, sz bufferSize);

    void logShaderError(ID3D10Blob* shaderError, std::wstring path);
};

