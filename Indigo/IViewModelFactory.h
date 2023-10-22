#pragma once

//#include "AnimationSkeleton.h"
#include "Mesh.h"
#include "Texture.h"

//class IRigidBodyFactory;
class IViewModelFactory
{
public:
    //IViewModelFactory(IRigidBodyFactory* rigidBodyFactory);
    virtual bool getStatus() const = 0;

    virtual bool tryLoad(
        Mesh** mesh,
        Texture** texture//,
        //AnimationMap* animations,
        //PxRigidStatic** actor
    ) = 0;

protected:
    //IRigidBodyFactory* m_rigidBodyFactory;
};

