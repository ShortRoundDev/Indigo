#include "stdafx.h"

#include "GLTFViewModelFactory.h"

//#include "AnimationSkeleton.h"
#include "AssetManager.h"
//#include "IRigidBodyFactory.h"
#include "Mesh.h"
#include "ModelBuffer.h"
#include "Texture.h"
#include "Vertex.h"

using namespace Microsoft;
using namespace std;

/* Annoying custom GLTF stream reader BS */
class StreamReader : public glTF::IStreamReader
{
public:
    shared_ptr<istream> GetInputStream(const string& filename) const override
    {
        auto stream = make_shared<ifstream>(filename);

        // Check if the stream has no errors and is ready for I/O operations
        if (!stream || !(*stream))
        {
            throw runtime_error("Unable to create a valid input stream for uri: " + filename);
        }

        return stream;
    }
};

GLTFViewModelFactory::GLTFViewModelFactory(
    //IRigidBodyFactory* rigidBodyFactory,
    string const& path
) :
    //IViewModelFactory(rigidBodyFactory),
    m_path(path),
    m_status(false),
    m_reader(nullptr),
    m_iterator(0)
{
    /* Reading a glTF Document is a PITA because Microsoft makes you
       Write your own file and string stream shit. I understand why, but
       it'd be cool if they just provided a default impl, but I think
       they're not targeting indie devs. */
    auto streamReader = make_unique<StreamReader>();
    auto stream = streamReader->GetInputStream(path);
    auto resourceReader = make_unique<glTF::GLTFResourceReader>(move(streamReader));

    stringstream manifestStream;
    manifestStream << stream->rdbuf();

    string manifest = manifestStream.str();

    try
    {
        m_document = glTF::Deserialize(manifest);
    }
    catch (const glTF::GLTFException& e)
    {
        cerr << "glTF Deserialize failed! Got " << e.what() << endl;
        return;
    }

    m_reader = move(resourceReader);
    m_status = true;
}

bool GLTFViewModelFactory::tryLoad(
    Mesh** mesh,
    Texture** texture/*,
    //AnimationMap* animations,
    PxRigidStatic** actor*/
)
{
    if ((!mesh || !texture) || (!m_status) || (m_iterator >= m_document.nodes.Size()))
    {
        return false;
    }

    glTF::Node element;
    bool found = false;
    for (; m_iterator < m_document.nodes.Size(); m_iterator++)
    {
        element = m_document.nodes.Elements()[m_iterator];
        if (!element.meshId.empty())
        {
            found = true;
            break;
        }
    }
    if (!found)
    {
        return false;
    }
    m_iterator++;

    XMFLOAT3 translate = XMFLOAT3(
        element.translation.x,
        element.translation.y,
        element.translation.z
    );
    XMFLOAT3 scale = XMFLOAT3(
        element.scale.x,
        element.scale.y,
        element.scale.z
    );
    XMFLOAT4 rotation = XMFLOAT4(
        element.rotation.x,
        element.rotation.y,
        element.rotation.z,
        element.rotation.w
    );

    XMMATRIX affine = XMMatrixAffineTransformation(
        XMLoadFloat3(&scale),
        XMVectorZero(),
        XMLoadFloat4(&rotation),
        XMLoadFloat3(&translate)
    );

    glTF::Mesh gMesh = m_document.meshes[element.meshId];
    if (gMesh.primitives.size() == 0)
    {
        // empty mesh; iterate and return false
        m_iterator++;
        return false;
    }
    glTF::MeshPrimitive primitive = gMesh.primitives[0];

    meshFromPrimitive(mesh, primitive, affine);
    textureFromPrimitive(texture, primitive);
    /*if (m_document.skins.Size() > 0)
    {
        animationFromSkin(animations, m_document.skins[element.skinId]);
    }*/

    return true;
}

bool GLTFViewModelFactory::getStatus() const
{
    return m_status;
}

/* ========== Private methods ========== */

template<typename T, typename U, auto V>
void GLTFViewModelFactory::getAttributeData(
    glTF::MeshPrimitive const& primitive,
    string const& accessor,
    vector<U>& list
)
{
    getAttributeData<T, T, U, V>(primitive, accessor, list);
}

template<typename T, typename W, typename U, auto V>
void GLTFViewModelFactory::getAttributeData(
    glTF::MeshPrimitive const& primitive,
    string const& accessor,
    vector<U>& list
)
{
    static thread_local W buffer[V];
    string accessorId;
    if (primitive.TryGetAttributeAccessorId(accessor, accessorId))
    {
        glTF::Accessor accessor = m_document.accessors.Get(accessorId);
        auto data = m_reader->ReadBinaryData<T>(m_document, accessor);

        for (u32 i = 0; i < data.size(); i += V)
        {
            for (u32 j = 0; j < V; j++)
            {
                buffer[j] = (W)data[i + j];
            }
            list.push_back(U(buffer));
        }
    }
}


void GLTFViewModelFactory::meshFromPrimitive(
    Mesh** mesh,
    glTF::MeshPrimitive const& primitive,
    XMMATRIX const& affine
)
{
    vector<Vertex> vertices;
    vector<u32> indices;

    string accessorId;

    vector<XMFLOAT3> pos;
    vector<XMFLOAT3> normals;
    vector<XMFLOAT2> texCoords;
    //vector<XMINT4> bones;
    //vector<XMFLOAT4> weights;

    getAttributeData<f32, XMFLOAT3, 3>(primitive, glTF::ACCESSOR_POSITION, pos);
    getAttributeData<f32, XMFLOAT3, 3>(primitive, glTF::ACCESSOR_NORMAL, normals);
    getAttributeData<f32, XMFLOAT2, 2>(primitive, glTF::ACCESSOR_TEXCOORD_0, texCoords);
    //getAttributeData<f32, XMFLOAT4, 4>(primitive, glTF::ACCESSOR_WEIGHTS_0, weights);
    //getAttributeData<u8, i32, XMINT4, 4>(primitive, glTF::ACCESSOR_JOINTS_0, bones);

    glTF::Accessor accessor = m_document.accessors.Get(primitive.indicesAccessorId);
    vector<u16> indexData = m_reader->ReadBinaryData<u16>(m_document, accessor);

    for (u32 i = 0; i < indexData.size(); i++)
    {
        indices.push_back((u32)indexData[i]);
    }

    for (u32 i = 0; i < pos.size(); i++)
    {
        auto pos4 = XMFLOAT4(pos[i].x, pos[i].y, pos[i].z, 1.0f);
        auto tex = texCoords[i];

        Vertex vertex(
            XMFLOAT4(pos[i].x, pos[i].y, pos[i].z, 1.0f),
            XMFLOAT3(normals[i].x, normals[i].y, normals[i].z),
            XMFLOAT2(texCoords[i].x, texCoords[i].y)
        );

        vertices.push_back(vertex);
    }
    *mesh = new Mesh(vertices, indices, ModelBuffer(affine));
}

void GLTFViewModelFactory::textureFromPrimitive(
    Texture** texture,
    glTF::MeshPrimitive const& primitive
)
{
    *texture = nullptr;
    if (primitive.materialId.empty())
    {
        return;
    }
    glTF::Material material = m_document.materials[primitive.materialId];
    auto textures = material.GetTextures();
    auto textureIterator = find_if(textures.begin(), textures.end(), [](pair<string, glTF::TextureType> p) -> bool
        {
            return p.second == glTF::TextureType::BaseColor;
        });
    if (textureIterator == textures.end())
    {
        return;
    }
    auto gTexture = m_document.textures[textureIterator->first];

    glTF::Image image = m_document.images[gTexture.imageId];

    vector<u8> data = m_reader->ReadBinaryData(m_document, image);
    string name = image.name;
    if (name.length() == 0)
    {
        stringstream ss;
        ss << name << "::" << m_iterator;
        name = ss.str();
    }

    *texture = ASSET->lazyLoadTexture(name, data.data(), data.size());
}

/*void GLTFViewModelFactory::animationFromSkin(
    AnimationMap* animations,
    glTF::Skin const& skin
)
{
    sz jointSize = skin.jointIds.size();

    for (u32 i = 0; i < m_document.animations.Size(); i++)
    {
        vector<tuple<
            vector<AnimationKeyFrame>,
            vector<AnimationKeyFrame>,
            vector<AnimationKeyFrame>
            >> timelineBuffer(jointSize);

        glTF::Animation animation = m_document.animations[i];
        for (u32 j = 0; j < animation.channels.Size(); j++)
        {
            glTF::AnimationChannel channel = animation.channels[j];
            glTF::AnimationSampler sampler = animation.samplers[channel.samplerId];
            i32 jointIndex = (i32)distance(
                skin.jointIds.begin(),
                find(
                    skin.jointIds.begin(), skin.jointIds.end(),
                    channel.target.nodeId
                )
            );
            glTF::TargetPath path = channel.target.path;

            auto& jointChannelBuffer = timelineBuffer[jointIndex]; // tuple

            vector<AnimationKeyFrame>* channelBuffer = nullptr;
            switch (path)
            {
            case glTF::TARGET_TRANSLATION:
            {
                channelBuffer = &get<0>(jointChannelBuffer);
                break;
            }
            case glTF::TARGET_SCALE:
            {
                channelBuffer = &get<1>(jointChannelBuffer);
                break;
            }
            case glTF::TARGET_ROTATION:
            {
                channelBuffer = &get<2>(jointChannelBuffer);
                break;
            }
            }

            //glTF::AnimationSampler sampler = animation.samplers[j];
            auto inputId = sampler.inputAccessorId;
            auto outputId = sampler.outputAccessorId;

            auto input = m_document.accessors[inputId];
            auto output = m_document.accessors[outputId];

            auto time = m_reader->ReadBinaryData<f32>(m_document, input);
            auto values = m_reader->ReadBinaryData<f32>(m_document, output);

            u32 vectorDisposition = (u32)(values.size() / time.size()); // 3 or 4
            for (u32 k = 0; k < time.size(); k++)
            {
                f32 vecBuffer[4] = { 0, 0, 0, 1.0f };
                for (u32 l = 0; l < vectorDisposition; l++)
                {
                    vecBuffer[l] = values[k * vectorDisposition + l];
                }

                channelBuffer->push_back(AnimationKeyFrame(
                    time[k],
                    XMVectorSet(vecBuffer[0], vecBuffer[1], vecBuffer[2], vecBuffer[3])
                ));
            }
        }

        vector<AnimationChannel> timeline;
        for (u32 j = 0; j < timelineBuffer.size(); j++)
        {
            timeline.push_back(AnimationChannel(
                get<0>(timelineBuffer[j]),
                get<1>(timelineBuffer[j]),
                get<2>(timelineBuffer[j])
            ));
        }

        u8* tree = new u8[jointSize];

        tree[0] = 0;
        for (u32 j = 0; j < jointSize; j++)
        {
            auto jointNode = m_document.nodes[skin.jointIds[j]];
            for (u32 k = 0; k < jointNode.children.size(); k++)
            {
                auto child = jointNode.children[k];
                auto childSkinJointItr = find(
                    skin.jointIds.begin(),
                    skin.jointIds.end(),
                    child
                );
                i64 childSkinJointIndex = (i64)distance(
                    skin.jointIds.begin(),
                    childSkinJointItr
                );

                tree[childSkinJointIndex] = j;
            }
        }

        vector<XMMATRIX> inverseBindMatrices;
        auto invBindAccessor = m_document.accessors[skin.inverseBindMatricesAccessorId];
        auto invBindData = m_reader->ReadBinaryData<f32>(m_document, invBindAccessor);
        for (u32 j = 0; j < invBindData.size(); j += 16)
        {
            inverseBindMatrices.push_back(XMMatrixSet(
                invBindData[j + 0], invBindData[j + 1], invBindData[j + 2], invBindData[j + 3],
                invBindData[j + 4], invBindData[j + 5], invBindData[j + 6], invBindData[j + 7],
                invBindData[j + 8], invBindData[j + 9], invBindData[j + 10], invBindData[j + 11],
                invBindData[j + 12], invBindData[j + 13], invBindData[j + 14], invBindData[j + 15]
            ));
        }

        vector<AnimationJoint> finalJoints;
        for (u32 j = 0; j < timeline.size(); j++)
        {
            auto node = m_document.nodes[skin.jointIds[j]];

            auto glTranslate = node.translation;
            auto glRotate = node.rotation;
            auto glScale = node.scale;

            XMVECTOR translate(XMVectorSet(
                glTranslate.x,
                glTranslate.y,
                glTranslate.z,
                1.0f
            ));
            XMVECTOR rotate(XMVectorSet(
                glRotate.x,
                glRotate.y,
                glRotate.z,
                glRotate.w
            ));
            XMVECTOR scale(XMVectorSet(
                glScale.x,
                glScale.y,
                glScale.z,
                1.0f
            ));

            XMVECTOR position = XMVectorSet(0, 0, 0, 1.0f);
            XMMATRIX localMatrix = XMMatrixAffineTransformation(scale, g_XMZero, rotate, translate);
            position = XMVector4Transform(position, localMatrix);

            int parent = tree[j];
            while (j > 0 && true)
            {
                auto matrix = finalJoints[parent].getLocalMatrix();
                position = XMVector4Transform(position, matrix);

                if (parent == tree[parent])
                {
                    break;
                }

                parent = tree[parent];
            }

            XMFLOAT3 _position;
            XMStoreFloat3(&_position, position);
            finalJoints.push_back(AnimationJoint(
                j, inverseBindMatrices[j], timeline[j], localMatrix,
                _position
            ));
        }

        animations->emplace(animation.name, new AnimationSkeleton(tree, finalJoints));
    }
}*/