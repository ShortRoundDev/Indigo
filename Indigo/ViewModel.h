#pragma once
#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"
//#include "AnimationSkeleton.h"

class ViewModel
{
public:
    ViewModel(
        Texture const* texture,
        Mesh const* mesh/*,
        AnimationMap const& animations*/
    );

    void draw(Shader* shader, Texture const* const texture = nullptr) const;

private:
    Texture const* const m_texture;
    Mesh const* const m_mesh;
    //AnimationMap const m_animations;

};
