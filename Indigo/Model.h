#pragma once

#include "ViewModel.h"
class Shader;
class Texture;

class Model
{
public:
    Model(vector<ViewModel>&& viewModels);

    void draw(Shader* shader, Texture* texture = nullptr);

private:
    vector<ViewModel> m_viewModels;
};
