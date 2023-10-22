#pragma once

using namespace DirectX;

struct Vertex
{
    XMFLOAT4 pos;
    XMFLOAT3 normal;
    XMFLOAT2 tex;
    //XMUINT4  boneIndices; will this be added later??
    //XMFLOAT4 weights;
};