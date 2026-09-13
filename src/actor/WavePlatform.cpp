#include <actor/ActorBase.h>
#include <map_obj/LiftNormalModelDraw.h>
#include <SineU/SineU.h>
#include <SineU/actor/WavePlatform.h>
#include <telkin/Print.h>
#include <collision/ActorBgCollisionMgr.h>
#include <red/util/SpriteUtil.h>
#include <map/Bg.h>
#include <scroll/BgScrollMgr.h>

namespace SineU {

SEAD_RTTI_OVERRIDE_IMPL(WavePlatform, ActorCollision);

const ActorCreateInfo WavePlatform::cCreateInfo = {
    .offset_x = 0, .offset_y = -0,
    .spawn_range = {
        .offset_x = 0, .offset_y = 0,
        .half_size_x = 512, .half_size_y = 512
    },
    .cull_range = {
        .up = 0, .down = 0, .left = 0, .right = 0
    },
    .flag = ActorCreateInfo::cFlag_MapObj
};

Profile* WavePlatform::cProfile = SineU::getRegistrar()->newProfile<WavePlatform>("platform_wave")
    .resources<"lift_han_wood", "lift_han_stone", "lift_kinoko_yoko", "lift_han_spin", "lift_kinoko_shiso", "lift_han_sky">(ProfileInfo::cResType_Course)
    .createInfo(&cCreateInfo)
    .build();

WavePlatform::WavePlatform(const ActorCreateParam& param)
    : ActorCollision(param)
    , mPlatform()
    , mCollider()
    , mWidth()
    , mOffscreen()
    , mTargetRot()
    , mTargetPos()
{ }

ActorBase::Result WavePlatform::create() {
    mWidth = mParam0 & 0xF; // nybble 12
    if (mWidth < 2) {
        mWidth = 2;
    }

    mPlatform.getPos() = this->mPos;
    mPlatform.setModelType(LiftNormalModelDraw::ModelType::cModelType_Wood); // nybble 6
    // mPlatform.setModelType(static_cast<LiftNormalModelDraw::ModelType>(mParam0 >> 0x18 & 0xF)); // nybble 6
    mPlatform.setLength(mWidth);
    mPlatform.getAngle() = Angle3(0, 0, 0);
    // mPlatform.getModelOffset() = sead::Vector3f(-(mWidth * 16.0f * 0.5f), 0.0f, 0.0f);
    mPlatform.getModelOffset() = sead::Vector3f::zero;//(-(mWidth * 16.0f * 0.5f), 0.0f, 0.0f);
    mPlatform.setModelScale(1.0f);
    mPlatform.init();

    const sead::Vector2f points[2] = {
        { 4.0f - mPlatform.getLength() * 8.0f, 8.0f},
        {-4.0f + mPlatform.getLength() * 8.0f, 8.0f}
    };
    
    mCollider.set(this, {
        .pos_offset       = { 0.0f, 0.0f },
        .rot_pivot_offset = { 0.0f, 0.0f },
        .points           = points,
        .angle            = 0
    });

    ActorBgCollisionMgr::instance()->entry(mCollider);

    sead::Vector3f platpos = sead::Vector3f(mPos.x - mWidth * 8, mPos.y, mPos.z);
    mPlatform.getPos() = platpos;
    mPlatform.move(mPlatform.getPos(), mPlatform.getLength() * 16.0);

    mOffscreen = true;
    execute();

    return cResult_Success;
}

bool WavePlatform::execute() {
    if (!updateWaveTargets()) {
        mOffscreen = true;
        return true;
    }

    if (mOffscreen) {
        mPos.y = mTargetPos;
        mAngle.z() = mTargetRot;
        mOffscreen = false;
    } else {
        sead::Mathf::chase(&mPos.y, mTargetPos, 2.0f);
        mAngle.z().chaseRest(mTargetRot, 0x01000000);
    }

    this->mCollider.setAngle(mAngle.z());
    this->mCollider.execute();
    sead::Vector3f platpos = sead::Vector3f(this->mPos.x - mWidth * 8, this->mPos.y, this->mPos.z);
    mPlatform.getPos() = platpos;
    mPlatform.getAngle().z() = mAngle.z();
    mPlatform.move(mPlatform.getPos(), mPlatform.getLength() * 16.0f);
    return true;
}

bool WavePlatform::draw() {
    mPlatform.draw();
    return true;
}

bool WavePlatform::updateWaveTargets() {
    f32 leftSurfacePos;
    getBgCheck()->checkWater(&leftSurfacePos,   mPos + sead::Vector3f(-10.0f, -512.0f, 0.0f), mLayer);
    f32 middleSurfacePos;
    getBgCheck()->checkWater(&middleSurfacePos, mPos + sead::Vector3f(0.0f, -512.0f, 0.0f),   mLayer);
    f32 rightSurfacePos;
    getBgCheck()->checkWater(&rightSurfacePos,  mPos + sead::Vector3f(10.0f, -512.0f, 0.0f),  mLayer);

    if (leftSurfacePos == -8192 || middleSurfacePos == -8192 || rightSurfacePos == -8192 || leftSurfacePos == 0 || middleSurfacePos == 0 || rightSurfacePos == 0) {
        return 0;
    }

    mTargetPos = ((leftSurfacePos + middleSurfacePos + rightSurfacePos) / 3) + 6.0f;

    f32 ydiff1 = middleSurfacePos - leftSurfacePos;
	f32 ydiff2 = rightSurfacePos - middleSurfacePos;
	f32 ydiffavg = (ydiff1 + ydiff2) / 2;
	mTargetRot = (u32)(sead::Mathf::atan2(ydiffavg, 10.0) / (2 * sead::Mathf::pi()) * 0x100000000);

	return true;
}

// bool WavePlatform::updateWaveTargets() {
//     float leftedge = BgScrollMgr::instance()->getScreenRect().getMin().x;

//     s32 index = (s32)((mPos.x - leftedge) * 0.5 + 0.5);
//     if (index < 10 - 144 || index >= 1190 + 1140) return false;
	
// 	if (index < 10) index += 144;
// 	if (index >= 1190) index -= 144;

//     if (Bg::instance()->getWaveSurfaceY(index + 10) == -8192.0f) {
//         index -= 144;
//     }

//     tk::println("bru %u", getBgCheck()->checkWaterDepth(mPos.x + 10.0f, mPos.y + 10.0f, mLayer));
//     // Bg::instance()->getwa
//     // getBgCheck()->checkWaterDepth(mPos.x, mPos.y, mLayer);
//     f32 ypos_1, ypos_m, ypos_r;
//     ypos_m = Bg::instance()->getWaveSurfaceY(index);
//     ypos_1 = Bg::instance()->getWaveSurfaceY(index - 10);
//     ypos_r = Bg::instance()->getWaveSurfaceY(index + 10);

//     mTargetPos = (ypos_1 + ypos_m + ypos_r) / 3 + 4.0;

//     if (ypos_m == -8192 || ypos_1 == -8192 || ypos_r == -8192 || ypos_m == 0 || ypos_1 == 0 || ypos_r == 0) return 0;

//     f32 ydiff1 = ypos_m - ypos_1;
// 	f32 ydiff2 = ypos_r - ypos_m;
// 	f32 ydiffavg = (ydiff1 + ydiff2) / 2;
// 	mTargetRot = (u32)(sead::Mathf::atan2(ydiffavg, 20.0) / (2 * sead::Mathf::pi()) * 0x100000000);

// 	return true;
// }

}
