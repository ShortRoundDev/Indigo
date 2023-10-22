#include "stdafx.h"

#include "AssetManager.h"

#include "WindowManager.h"

//#include "ConvexMeshRigidBodyFactory.h"
#include "GLTFViewModelFactory.h"

#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include "Vertex.h"
#include "ViewModel.h"
#include "Model.h"
#include "CubeMap.h"

using namespace std;

void AssetManager::init()
{
    loadTexturePrecacheFile("Textures.yaml");
    loadModelPrecacheFile("Models.yaml");

    initShaders();

    vector<Vertex> vertices = {
        {// Front face
        { XMFLOAT4(-0.5f, -0.5f, 0.5f, 0.1f), XMFLOAT3(1, 2, 3), XMFLOAT2(5.0f, 6.0f)},
        { { 0.5f, -0.5f, 0.5f, 0.0f }, {0, 0, 0 }, { 0.0f, 0.0f } },
        { { 0.5f, 0.5f, 0.5f, 0.0f }, {0, 0, 0 }, { 0.0f, 0.0f } },
        { { -0.5f, 0.5f, 0.5f, 0.0f }, {0, 0, 0 }, { 0.0f, 0.0f } },

        // Back face
        { { -0.5f, -0.5f, -0.5f, 0.0f }, {0, 0, 0 }, { 0.0f, 0.0f } },
        { { 0.5f, -0.5f, -0.5f, 0.0f }, {0, 0, 0 }, { 0.0f, 0.0f } },
        { { 0.5f, 0.5f, -0.5f, 0.0f }, {0, 0, 0 }, { 0.0f, 0.0f } },
        { { -0.5f, 0.5f, -0.5f, 0.0f }, {0, 0, 0 }, { 0.0f, 0.0f } },

        // Top face
        { { -0.5f, 0.5f, -0.5f, 0.0f }, {0, 0, 0 }, { 0.0f, 0.0f } },
        { { 0.5f, 0.5f, -0.5f, 0.0f }, {0, 0, 0 }, { 0.0f, 0.0f } },
        { { 0.5f, 0.5f, 0.5f, 0.0f }, {0, 0, 0 }, { 0.0f, 0.0f } },
        { { -0.5f, 0.5f, 0.5f, 0.0f }, {0, 0, 0 }, { 0.0f, 0.0f } },

        // Bottom face
        { { -0.5f, -0.5f, -0.5f, 0.0f }, {0, 0, 0 }, { 0.0f, 0.0f } },
        { { 0.5f, -0.5f, -0.5f, 0.0f }, {0, 0, 0 }, { 0.0f, 0.0f } },
        { { 0.5f, -0.5f, 0.5f, 0.0f }, {0, 0, 0 }, { 0.0f, 0.0f } },
        { { -0.5f, -0.5f, 0.5f, 0.0f }, {0, 0, 0 }, { 0.0f, 0.0f } },

        // Left face
        { { -0.5f, -0.5f, -0.5f, 0.0f }, {0, 0, 0 }, { 0.0f, 0.0f } },
        { { -0.5f, 0.5f, -0.5f, 0.0f }, {0, 0, 0 }, { 0.0f, 0.0f } },
        { { -0.5f, 0.5f, 0.5f, 0.0f }, {0, 0, 0 }, { 0.0f, 0.0f } },
        { { -0.5f, -0.5f, 0.5f, 0.0f }, {0, 0, 0 }, { 0.0f, 0.0f } },

        // Right face
        { { 0.5f, -0.5f, -0.5f, 0.0f }, {0, 0, 0 }, { 0.0f, 0.0f } },
        { { 0.5f, 0.5f, -0.5f, 0.0f }, {0, 0, 0 }, { 0.0f, 0.0f } },
        { { 0.5f, 0.5f, 0.5f, 0.0f }, {0, 0, 0 }, { 0.0f, 0.0f } },
        { { 0.5f, -0.5f, 0.5f, 0.0f}, {0, 0, 0 }, { 0.0f, 0.0f } }
    } };

    vector<u32> indices = {
        // Front face
        0, 1, 2,
        2, 3, 0,

        // Back face
        4, 5, 6,
        6, 7, 4,

        // Top face
        8, 9, 10,
        10, 11, 8,

        // Bottom face
        12, 13, 14,
        14, 15, 12,

        // Left face
        16, 17, 18,
        18, 19, 16,

        // Right face
        20, 21, 22,
        22, 23, 20
    };

    m_cube = new Mesh(vertices, indices);
}

Texture* AssetManager::getTexture(string const& name)
{
    auto iter = m_textures.find(name);
    if (iter != m_textures.end())
    {
        return iter->second;
    }
    return nullptr;
}

Texture* AssetManager::lazyLoadTexture(string const& name, string const& path)
{
    auto texture = getTexture(name);

    if (!texture)
    {
        wstring windowsPath;
        if (mbStrToWideChar(path, windowsPath))
        {
            texture = new Texture(windowsPath);
            if (texture->isSuccess())
            {
                m_textures[name] = texture;
            }
            else
            {
                delete texture;
                texture = nullptr;
                cerr << "Couldn't load texture " << name << endl;
                return nullptr;
            }
        }
        else
        {
            cerr << "Couldn't convert texture filepath name to wstring: " << name << endl;
            return nullptr;
        }
    }
    return texture;
}

Texture* AssetManager::lazyLoadTexture(string const& name, u8* data, sz size)
{
    auto texture = getTexture(name);
    if (!texture)
    {
        texture = new Texture(data, size);
        if (texture->isSuccess())
        {
            m_textures[name] = texture;
        }
        else
        {
            delete texture;
            texture = nullptr;
            cerr << "Couldn't load texture " << name << endl;
            return nullptr;
        }
    }
    return texture;
}

Model* AssetManager::getModel(string const& name)
{
    auto model = m_models.find(name);
    if (model != m_models.end())
    {
        return model->second;
    }
    return nullptr;
}

Model* AssetManager::lazyLoadModel(string const& name, string const& path)
{
    auto model = getModel(name);
    if (model)
    {
        return model;
    }
    string extension = ".gltf";
    if (equal(extension.rbegin(), extension.rend(), path.rbegin()))
    {
        //ConvexMeshRigidBodyFactory convex;
        GLTFViewModelFactory factory(path);
        Mesh* mesh;
        Texture* texture;
        vector<ViewModel> viewModels;
        //AnimationMap animations;

        while (factory.tryLoad(&mesh, &texture))
        {
            viewModels.push_back(ViewModel(texture, mesh));
        }

        model = new Model(move(viewModels));
        m_models[name] = model;
    }
    return model;
}

Mesh* AssetManager::getCube() const
{
    return m_cube;
}

CubeMap* AssetManager::getCubeMap(string const& name) const
{
    auto found = m_cubeMaps.find(name);
    if (found == m_cubeMaps.end())
    {
        return nullptr;
    }
    return found->second;
}

CubeMap* AssetManager::lazyLoadCubeMap(string const& name, string const& path)
{
    auto cubeMap = getCubeMap(name);
    if (!cubeMap)
    {
        cubeMap = new CubeMap(path);
        if (!cubeMap->isSuccess())
        {
            delete cubeMap;
            return nullptr;
        }
        m_cubeMaps[name] = cubeMap;
    }
    return cubeMap;
}

Shader* AssetManager::getShader(string const& name) const
{
    auto shaderIter = m_shaders.find(name);
    if (shaderIter == m_shaders.end())
    {
        return nullptr;
    }
    return shaderIter->second;
}

void AssetManager::loadModelPrecacheFile(string const& path)
{
    YAML::Node node;
    try
    {
        node = YAML::LoadFile(path);
    }
    catch (exception& e)
    {
        cerr << "Failed to load model file at [" << path << "]. Got " << e.what() << endl;
        return;
    }
    for (const auto& n : node)
    {
        auto name = n.first.as<string>();
        auto value = n.second.as<string>();

        lazyLoadModel(name, value);
    }
}

void AssetManager::loadTexturePrecacheFile(string const& path)
{
    YAML::Node node;
    try
    {
        node = YAML::LoadFile(path);
    }
    catch (exception& e)
    {
        cerr << "Failed to load texture file at [" << path << "]. Got " << e.what() << endl;
        return;
    }

    auto texturesNode = node["Textures"];
    if (!texturesNode.IsDefined())
    {
        cerr << "Missing 'Textures' node in Texture config file!" << endl;
    }
    else
    {
        for (const auto& n : texturesNode)
        {
            auto name = n.first.as<string>();
            auto value = n.second.as<string>();

            if (!lazyLoadTexture(name, value))
            {
                cerr << "Failed to load texture [" << name << "]!" << endl;
            }
        }
    }

    auto cubeMapsNode = node["CubeMaps"];
    if (!cubeMapsNode.IsDefined())
    {
        cerr << "Missing 'CubeMaps' node in Texture config file!" << endl;
    }
    else
    {
        for (const auto& n : cubeMapsNode)
        {
            auto name = n.first.as<string>();
            auto value = n.second.as<string>();

            if (!lazyLoadCubeMap(name, value))
            {
                cerr << "Failed to load cubemap [" << name << "]!" << endl;
            }
        }
    }
}

void AssetManager::initShaders()
{
    initShader("World", L"WorldVertex.cso", L"WorldPixel.cso");
    initShader("SkyBox", L"SkyBoxVertex.cso", L"SkyBoxPixel.cso");
    initShader("SSAO", L"WorldVertex.cso", L"SSAOPixel.cso");
    initShader("ViewModel", L"ViewModelVertex.cso", L"WorldPixel.cso");
}

void AssetManager::initShader(string const& name, wstring const& vertex, wstring const& pixel)
{
    Shader* shader = new Shader(WND->getDevice(), vertex, pixel, 0);
    if (!shader->isSuccess())
    {
        cerr << "Failed to load shader [" << name << "] with ";
        wcerr << "Vertex: [" << vertex << "] and Pixel: [" << pixel << "]" << endl;
        delete shader;
    }
    else
    {
        m_shaders[name] = shader;
    }
}