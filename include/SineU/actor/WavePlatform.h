#pragma once

#include <actor/ActorCollision.h>
#include <collision/ActorPolylineBgCollision.h>
#include <map_obj/LiftNormalModelDraw.h>
#include <actor/Profile.h>

namespace SineU {

class WavePlatform : public ActorCollision {
    SEAD_RTTI_OVERRIDE(WavePlatform, ActorCollision);

public:
    static Profile* cProfile;
    static const ActorCreateInfo cCreateInfo;

public:
    WavePlatform(const ActorCreateParam& param);
    ~WavePlatform() override = default;
    
    Result create() override;
    bool execute() override;
    bool draw() override;

    bool updateWaveTargets();

private:
    f32                         mTargetPos;
    u32                         mTargetRot;
    bool                        mOffscreen;
    u8                          mWidth;
    LiftNormalModelDraw         mPlatform;
    ActorPolylineBgCollision<1> mCollider;
};

}