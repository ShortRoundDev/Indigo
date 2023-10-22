#include "stdafx.h"

#include "ViewModel.h"
#include "Texture.h"
#include "Shader.h"
#include "Mesh.h"

ViewModel::ViewModel(
    Texture const* texture,
    Mesh const* mesh/*,
    AnimationMap const& animations*/
) :
    m_texture(texture),
    m_mesh(mesh)/*,
    m_animations(animations)*/
{
}

void ViewModel::draw(Shader* shader, Texture const* const texture) const
{
    shader->bindModelBuffer(m_mesh->getModelBuffer());

    if (texture)
    {
        texture->use();
    }
    else if (m_texture)
    {
        m_texture->use();
    }

    m_mesh->draw();
}