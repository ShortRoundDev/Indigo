#pragma once

#include "IViewModelFactory.h"

class Mesh;
class Texture;

using namespace std;
using namespace Microsoft;

class GLTFViewModelFactory : public IViewModelFactory
{
public:
    GLTFViewModelFactory(
        //IRigidBodyFactory* rigidBodyFactory,
        string const& path
    );
    bool tryLoad(
        Mesh** mesh,
        Texture** texture//,
        //AnimationMap* animations,
        //PxRigidStatic** actor
    ) override;
    bool getStatus() const override;

private:
    unique_ptr<glTF::GLTFResourceReader> m_reader;
    u32 m_iterator;

    bool m_status;
    glTF::Document m_document;

    string m_path;

    template<typename T, typename U, auto V>
    void getAttributeData(
        glTF::MeshPrimitive const& primitive,
        string const& accessor,
        vector<U>& list
    );

    template<typename T, typename W, typename U, auto V>
    void getAttributeData(
        glTF::MeshPrimitive const& primitive,
        string const& accessor,
        vector<U>& list
    );

    void meshFromPrimitive(
        Mesh** mesh,
        glTF::MeshPrimitive const& primitive,
        XMMATRIX const& affine
    );

    void textureFromPrimitive(
        Texture** mesh,
        glTF::MeshPrimitive const& primitive
    );

    /*void animationFromSkin(
        //AnimationMap* animations,
        glTF::Skin const& skin
    );*/
};
