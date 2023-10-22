#include "stdafx.h"
#include "Shader.h"

#include "WindowManager.h"
#include "ModelBuffer.h"

Shader::Shader(
    ID3D11Device* device,
    wstring vertexPath,
    wstring pixelPath,
    sz bufferSize
) : m_modelBuffer()
{
    //Vertex Shader bytecode
    u8* byteCode;
    sz byteCodeLength;
    m_success = initShaderCode(device, vertexPath, pixelPath, &byteCode, &byteCodeLength);
    if (!m_success)
    {
        return;
    }

    m_success = initLayout(device, byteCode, byteCodeLength);
    if (!m_success)
    {
        return;
    }

    m_success = initSampler(device);
    if (!m_success)
    {
        return;
    }

    m_success = initBlendState(device);
    if (!m_success)
    {
        return;
    }

    m_success = initCBuffer(device, bufferSize);
    if (!m_success)
    {
        return;
    }
}

void Shader::use()
{
    auto context = WND->getContext();
    context->IASetInputLayout(m_inputLayout.Get());
    context->VSSetShader(m_vertex.Get(), NULL, 0);
    context->PSSetShader(m_pixel.Get(), NULL, 0);
    context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

    float factor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    context->OMSetBlendState(m_blendState.Get(), factor, 0xffffffff);
}

void Shader::bindModelBuffer(ModelBuffer const& modelBuffer)
{
    ModelBuffer copy = { };
    copy.model = XMMatrixTranspose(modelBuffer.model);

    D3D11_MAPPED_SUBRESOURCE mBufferResource;
    HRESULT result = WND->getContext()->Map(m_modelBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mBufferResource);
    if (FAILED(result))
    {
        cerr << "Failed to lock model buffer!";
        LocalAllocString str;
        if (TryGetLastErrorMessageA(str))
        {
            cerr << " Got: " << str;
        }
        cerr << endl;
        return;
    }

    void* localBuffer = (void*)mBufferResource.pData;
    CopyMemory(localBuffer, &copy, sizeof(ModelBuffer));
    WND->getContext()->Unmap(m_modelBuffer.Get(), 0);
    ////////// END CRITICAL SECTION //////////

    // Put CBuffer in correct register slot
    WND->getContext()->VSSetConstantBuffers(1, 1, m_modelBuffer.GetAddressOf());
    WND->getContext()->PSSetConstantBuffers(1, 1, m_modelBuffer.GetAddressOf());
}

bool Shader::isSuccess() const
{
    return m_success;
}

bool Shader::initShaderCode(
    ID3D11Device* device,
    wstring vertexPath,
    wstring pixelPath,
    u8** vertexByteCode,
    sz* vertexByteCodeLength
)
{
    if (!TryReadFile(vertexPath.c_str(), vertexByteCode, vertexByteCodeLength))
    {
        wcerr << L"Could not open Vertex shader " << vertexPath << L"!";
        LocalAllocWString str;
        if (TryGetLastErrorMessageW(str))
        {
            wcerr << " Got: " << str;
        }
        wcerr << endl;
        return false;
    }

    HRESULT result;
    result = device->CreateVertexShader(
        *vertexByteCode,
        *vertexByteCodeLength,
        nullptr,
        m_vertex.GetAddressOf()
    );

    if (result != S_OK)
    {
        wcerr << L"Failed to load Vertex shader " << vertexPath << L"!";
        LocalAllocWString str;
        if (TryGetErrorMessageW(str, result))
        {
            wcerr << L" Got: " << str;
        }
        wcerr << endl;

        delete* vertexByteCode;
        *vertexByteCode = nullptr;
        return false;
    }

    u8* buffer;
    sz size = 0;

    if (!TryReadFile(pixelPath.c_str(), &buffer, &size))
    {
        wcerr << L"Could not open Pixel Shader " << vertexPath << L"!";
        LocalAllocWString str;
        if (TryGetLastErrorMessageW(str))
        {
            wcerr << L" Got: " << str;
        }
        wcerr << endl;

        delete* vertexByteCode;
        *vertexByteCode = nullptr;
        return false;
    }

    result = device->CreatePixelShader(
        buffer,
        size,
        nullptr,
        m_pixel.GetAddressOf()
    );

    delete[] buffer;
    buffer = NULL;

    if (result != S_OK)
    {
        wcerr << L"Failed to load Pixel shader " << pixelPath << L"!";
        LocalAllocWString str;
        if (TryGetErrorMessageW(str, result))
        {
            wcerr << L" Got: " << str;
        }
        wcerr << endl;
        delete* vertexByteCode;
        return false;
    }
    return true;
}

bool Shader::initLayout(ID3D11Device* device, u8* byteCode, sz byteCodeLength)
{
    const i32 INPUT_SIZE = 5;
    D3D11_INPUT_ELEMENT_DESC polygonLayout[INPUT_SIZE];
    ZeroMemory(polygonLayout, sizeof(D3D11_INPUT_ELEMENT_DESC) * INPUT_SIZE);
    polygonLayout[0].SemanticName = "POSITION";
    polygonLayout[0].SemanticIndex = 0;
    polygonLayout[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    polygonLayout[0].InputSlot = 0;
    polygonLayout[0].AlignedByteOffset = 0;
    polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[0].InstanceDataStepRate = 0;

    polygonLayout[1].SemanticName = "NORMAL";
    polygonLayout[1].SemanticIndex = 0;
    polygonLayout[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[1].InputSlot = 0;
    polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[1].InstanceDataStepRate = 0;

    polygonLayout[2].SemanticName = "TEXCOORD";
    polygonLayout[2].SemanticIndex = 0;
    polygonLayout[2].Format = DXGI_FORMAT_R32G32_FLOAT;
    polygonLayout[2].InputSlot = 0;
    polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[2].InstanceDataStepRate = 0;

    polygonLayout[3].SemanticName = "BONEINDICES";
    polygonLayout[3].SemanticIndex = 0;
    polygonLayout[3].Format = DXGI_FORMAT_R32G32B32A32_SINT;
    polygonLayout[3].InputSlot = 0;
    polygonLayout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[3].InstanceDataStepRate = 0;

    polygonLayout[4].SemanticName = "WEIGHTS";
    polygonLayout[4].SemanticIndex = 0;
    polygonLayout[4].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    polygonLayout[4].InputSlot = 0;
    polygonLayout[4].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[4].InstanceDataStepRate = 0;

    int numElements = _countof(polygonLayout);

    HRESULT result = device->CreateInputLayout(
        polygonLayout,
        numElements,
        byteCode,
        byteCodeLength,
        m_inputLayout.GetAddressOf()
    );

    delete byteCode;
    byteCode = nullptr;

    if (FAILED(result))
    {
        cerr << "Shader input layout error!" << endl;
        PrintDxInfoQueue(WND->getInfoQueue());
        return false;
    }

    return true;
}

bool Shader::initSampler(ID3D11Device* device)
{
    D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    HRESULT result = device->CreateSamplerState(&samplerDesc, m_samplerState.GetAddressOf());
    if (FAILED(result))
    {
        cerr << "Shader sampler creation error!" << endl;
        PrintDxInfoQueue(WND->getInfoQueue());
        return false;
    }

    return true;
}

bool Shader::initBlendState(ID3D11Device* device)
{
    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;
    blendDesc.RenderTarget[0].BlendEnable = true;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    HRESULT result = device->CreateBlendState(&blendDesc, m_blendState.GetAddressOf());
    if (FAILED(result))
    {
        cerr << "Shader blend state creation error!" << endl;
        PrintDxInfoQueue(WND->getInfoQueue());
        return false;
    }

    return true;
}

bool Shader::initCBuffer(ID3D11Device* device, sz bufferSize)
{
    D3D11_BUFFER_DESC matrixBufferDesc;
    ZeroMemory(&matrixBufferDesc, sizeof(D3D11_BUFFER_DESC));
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = (u32)(ceil((float)sizeof(ModelBuffer) / 16.0f) * 16.0f);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    HRESULT result = device->CreateBuffer(&matrixBufferDesc, NULL, m_modelBuffer.GetAddressOf());
    if (FAILED(result))
    {
        cerr << "Failed to create model buffer!" << endl;
        PrintDxInfoQueue(WND->getInfoQueue());
        return false;
    }

    if (!bufferSize)
    {
        return true;
    }
    ZeroMemory(&matrixBufferDesc, sizeof(D3D11_BUFFER_DESC));
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = (u32)(ceil((float)bufferSize / 16.0f) * 16.0f);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    result = device->CreateBuffer(&matrixBufferDesc, NULL, m_cBuffer.GetAddressOf());
    if (FAILED(result))
    {
        cerr << "Failed to create cbuffer!" << endl;
        PrintDxInfoQueue(WND->getInfoQueue());
        return false;
    }

    return true;
}