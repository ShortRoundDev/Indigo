#include "stdafx.h"

#include "Model.h"

#include "Shader.h"
#include "Texture.h"

Model::Model(vector<ViewModel>&& viewModels) :
    m_viewModels(viewModels)
{
}

void Model::draw(Shader* shader, Texture* texture)
{
    for (ViewModel const& vm : m_viewModels)
    {
        vm.draw(shader, texture);
    }
}