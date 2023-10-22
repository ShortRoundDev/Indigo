#pragma once

#include "Vertex.h"
#include "ModelBuffer.h"

class Mesh
{
public:
    Mesh(
        vector<Vertex>& vertices,
        vector<u32>& indices,
        ModelBuffer const& modelBuffer = ModelBuffer()
    );
    ~Mesh() = default;

    void draw() const;
    ModelBuffer const& getModelBuffer() const;

    bool isSuccess() const;
private:

    void init();

    ComPtr<ID3D11Buffer> m_vertexBuffer;
    u32 m_vertCount;
    vector<Vertex> m_vertices;

    ComPtr<ID3D11Buffer> m_indexBuffer;
    u32 m_indexCount;
    vector<u32> m_indices;

    ModelBuffer m_modelBuffer;

    bool m_success;
};
