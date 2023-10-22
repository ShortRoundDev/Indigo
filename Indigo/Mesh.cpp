#include "stdafx.h"

#include "Mesh.h"
#include "WindowManager.h"

Mesh::Mesh(
    vector<Vertex>& vertices,
    vector<u32>& indices,
    ModelBuffer const& modelBuffer
) :
    m_vertexBuffer(),
    m_vertCount((u32)vertices.size()),
    m_vertices(vertices),

    m_indexBuffer(),
    m_indexCount((u32)indices.size()),
    m_indices(indices),

    m_modelBuffer(modelBuffer)
{
    init();
}

void Mesh::draw() const
{
    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    auto context = WND->getContext();

    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->DrawIndexed(m_indexCount, 0, 0);
}

ModelBuffer const& Mesh::getModelBuffer() const
{
    return m_modelBuffer;
}

bool Mesh::isSuccess() const
{
    return m_success;
}

/* ========== Private ========== */
void Mesh::init()
{
    Vertex const* pVertices = m_vertices.data();
    u32 const* pIndices = m_indices.data();

    D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = (UINT)(m_vertCount * sizeof(Vertex));
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
    vertexBufferData.pSysMem = pVertices;
    vertexBufferData.SysMemPitch = 0;
    vertexBufferData.SysMemSlicePitch = 0;

    HRESULT res = WND->getDevice()->CreateBuffer(
        &vertexBufferDesc,
        &vertexBufferData,
        m_vertexBuffer.GetAddressOf()
    );
    if (FAILED(res))
    {
        cerr << "Failed to create vertex buffer!";
        LocalAllocString str;
        if (TryGetLastErrorMessageA(str))
        {
            cerr << " Got: " << str;
        }
        cerr << endl;
        PrintDxInfoQueue(WND->getInfoQueue());
        m_success = false;
        return;
    }

    D3D11_BUFFER_DESC indexBufferDesc = { 0 };
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = (u32)(sizeof(u32) * m_indexCount);
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA indexBufferData{ 0 };
    indexBufferData.pSysMem = pIndices;
    indexBufferData.SysMemPitch = 0;
    indexBufferData.SysMemSlicePitch = 0;

    res = WND->getDevice()->CreateBuffer(
        &indexBufferDesc,
        &indexBufferData,
        m_indexBuffer.GetAddressOf()
    );
    if (FAILED(res))
    {
        cerr << "Failed to create index buffer!";
        LocalAllocString str;
        if (TryGetLastErrorMessageA(str))
        {
            cerr << " Got: " << str;
        }
        cerr << endl;
        PrintDxInfoQueue(WND->getInfoQueue());
        m_success = false;
        return;
    }

    m_success = true;
}