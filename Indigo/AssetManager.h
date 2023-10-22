#pragma once

class Texture;
class Model;
class Mesh;
class CubeMap;
class Shader;

class AssetManager
{
public:
    void init();

    Texture* getTexture(string const& name);
    Texture* lazyLoadTexture(string const& name, string const& path);
    Texture* lazyLoadTexture(string const& name, u8* data, sz size);

    Model* getModel(string const& name);
    Model* lazyLoadModel(string const& name, string const& path);

    Mesh* getCube() const;

    CubeMap* getCubeMap(string const& name) const;
    CubeMap* lazyLoadCubeMap(string const& name, string const& path);

    Shader* getShader(string const& name) const;

private:
    map<string, Texture*> m_textures;
    map<string, Model*> m_models;
    map<string, CubeMap*> m_cubeMaps;
    map<string, Shader*> m_shaders;
    Mesh* m_cube;

    void loadTexturePrecacheFile(string const& path);
    void loadModelPrecacheFile(string const& path);

    void initShaders();
    void initShader(string const& name, wstring const& vertex, wstring const& pixel);
};

extern AssetManager* g_assetManager;
#define ASSET (g_assetManager)