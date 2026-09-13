#include "SineU/map_obj/Wave.h"
#include <telkin/Print.h>
#include <SineU/SineU.h>
#include <SineU/actor/BgActorBase.h>
#include <map/Bg.h>

red::Registrar* SineU::getRegistrar() {
    static red::Registrar sRegistrar("SineU");
    return &sRegistrar;
}

void main() { } // de-still don't care :)

class Water : public BgActorBase {
    SEAD_RTTI_OVERRIDE(Water, BgActorBase);
public:
    // Address: 0x026F7C90
    Water(const ActorCreateParam& param);
};

class WavyWater : public Water {

public:
    WavyWater(const ActorCreateParam& param);
    static Profile* sProfile;
};

const ActorCreateInfo cCreateInfoWavyWater = {
    .offset_x = 8, .offset_y = 0,
    .spawn_range = {
        .offset_x = 0, .offset_y = 0,
        .half_size_x = 128, .half_size_y = 128
    },
    .cull_range = { 
        .up = 0, .down = 0, .left = 0, .right = 0
    },
    .flag = static_cast<ActorCreateInfo::Flag>(4) // the old headers are mega cooked so i have to do some shenanigans
};

Profile* WavyWater::sProfile = SineU::getRegistrar()->newProfile<WavyWater>("water_waves")
    .resources<"obj_waterfull", "obj_waterhalf">(ProfileInfo::cResType_Course)
    .createInfo(&cCreateInfoWavyWater)
    .build();

WavyWater::WavyWater(const ActorCreateParam& param)
    : Water(param)
    { }

class Poison : public BgActorBase {
    SEAD_RTTI_OVERRIDE(Poison, BgActorBase);
public:
    // Address: 0x026F5F70
    Poison(const ActorCreateParam& param);
};

const ActorCreateInfo cCreateInfoWavyPoison = {
    .offset_x = 8, .offset_y = 0,
    .spawn_range = {
        .offset_x = 0, .offset_y = 0,
        .half_size_x = 24, .half_size_y = 24
    },
    .cull_range = { 
        .up = 0, .down = 0, .left = 0, .right = 0
    },
    .flag = static_cast<ActorCreateInfo::Flag>(4) // the old headers are mega cooked so i have to do some shenanigans
};

Profile* sProfileWavyPoison = SineU::getRegistrar()->newProfile<Poison>("poison_waves")
    .resources<"obj_poisonwater", "obj_magmadeco">(ProfileInfo::cResType_Course)
    .createInfo(&cCreateInfoWavyPoison)
    .build();


class Quicksand : public BgActorBase {
    SEAD_RTTI_OVERRIDE(Quicksand, BgActorBase);
public:
    // Address: 0x026F6E78
    Quicksand(const ActorCreateParam& param);
};

const ActorCreateInfo cCreateInfoWavyQuicksand = {
    .offset_x = 8, .offset_y = 0,
    .spawn_range = {
        .offset_x = 0, .offset_y = 0,
        .half_size_x = 128, .half_size_y = 128
    },
    .cull_range = { 
        .up = 0, .down = 0, .left = 0, .right = 0
    },
    .flag = static_cast<ActorCreateInfo::Flag>(4) // the old headers are mega cooked so i have to do some shenanigans
};

Profile* sProfileWavyQuicksand = SineU::getRegistrar()->newProfile<Quicksand>("quicksand_waves")
    .resources<"obj_quicksand">(ProfileInfo::cResType_Course)
    .createInfo(&cCreateInfoWavyQuicksand)
    .build();

// Address: 0x028B74F4
extern "C" void setWaveParam(Wave* _this,
    int major_amp, int minor_amp,
    int major_speed, int minor_speed,
    int major_freq, int minor_freq
);

static void activateWave(BgActorBase* _this, WaterType type) {
    // Parameters
    setWaveParam(&_this->getWave(),
        _this->getParam1() >> 0x4 & 0xF, _this->getParam1() >> 0x10 & 0xF,
        _this->getParam1() >> 0x8 & 0xF, _this->getParam1() >> 0x14 & 0xF,
        _this->getParam1() >> 0xC & 0xF, _this->getParam1() >> 0x18 & 0xF
    );
    
    // disables the secondary bob
    _this->getWave().setMajorSineWaveMult(1.0f);
    _this->getWave().setMinorSineWaveMult(1.0f);

    // Collision
    Bg::instance()->setHasTerrain(true);
    Bg::instance()->setHasLavaWaves(type);

    switch (type) {
        case cWaterType_Water: { 
            _this->getWave().setLiquidCollisionTypeOverride(Wave::cTerrainType_Water);
            break;
        }
        case cWaterType_Poison: { 
            _this->getWave().setLiquidCollisionTypeOverride(Wave::cTerrainType_Poison);
            break;
        }
        case cWaterType_Quicksand: { 
            _this->getWave().setLiquidCollisionTypeOverride(Wave::cTerrainType_Quicksand);
            break;
        }
        default: {
            _this->getWave().setLiquidCollisionTypeOverride(Wave::cTerrainType_LavaWaves);
            break;
        }
    }

    _this->getWave().updateWave();
}

void setWaterWaveValues(BgActorBase* _this) {
    if (_this->getParam1() & 0b1 && _this->getProfile() == WavyWater::sProfile) {
        activateWave(_this, cWaterType_Water);
    }
}

// We're not in a class so we must do this manually
extern "C" u32 _ZN11BgActorBase7execute(BgActorBase*);

u32 Poison_onExecute(BgActorBase* _this) { // Replaces poison water onExecute()
    if (_this->getParam1() & 01 && _this->getProfile() == sProfileWavyPoison) {
        activateWave(_this, cWaterType_Poison);
    }

    return _ZN11BgActorBase7execute(_this);
}

u32 Quicksand_onExecute(BgActorBase* _this) { // Replaces quicksand onExecute()
    if (_this->getParam1() & 0x1 && _this->getProfile() == sProfileWavyQuicksand) {
        activateWave(_this, cWaterType_Quicksand);
    }

    return _ZN11BgActorBase7execute(_this);
}

#define TELKIN_REGISTERS
#include <telkin/Telkin.h>

void SetWaterWaveValues() tAssembly(
    tSaveVolatileRegisters;
    
    mr r3, r30;
    bl _Z18setWaterWaveValuesP11BgActorBase;
    
    tRestoreVolatileRegisters;

    // replaced instruction
    cmpwi r0, 0;
    blr;
)

tBranch(0x026F9444, SetWaterWaveValues, tk::BranchType::bl); // Water::onExecute()
tPointerCode(0x100FDDDC, Poison_onExecute); // vtable entry for Poison::onExecute
tPointerCode(0x100FE19C, Quicksand_onExecute); // vtable entry for Quicksand::onExecute

void WaveCollision_r4() tAssembly(
    // # Check for lava waves
    li    r3, 4;//WaveCollision_Lava; # Return value
    cmpwi r4, 1;//WaveType_Lava;
    beqlr;

    // # Check for water waves
    li    r3, 1;//WaveCollision_Water; # Return value
    cmpwi r4, 2;//WaveType_Water;
    beqlr;

    // # Check for poison waves
    li    r3, 5;//WaveCollision_Poison; # Return value
    cmpwi r4, 3;//WaveType_Poison;
    beqlr;

    // # Must be quicksand waves
    li    r3, 2;//WaveCollision_Quicksand;
    blr;
)

tBranch(0x0218F150, WaveCollision_r4, tk::BranchType::bl); // r4 is Bg::instance()->mWaveType

void WaveCollision_r12() tAssembly(
    // # Check for lava waves
    li    r3, 4;//WaveCollision_Lava; # Return value
    cmpwi r12, 1;//WaveType_Lava;
    beqlr;

    // # Check for water waves
    li    r3, 1;//WaveCollision_Water; # Return value
    cmpwi r12, 2;//WaveType_Water;
    beqlr;

    // # Check for poison waves
    li    r3, 5;//WaveCollision_Poison; # Return value
    cmpwi r12, 3;//WaveType_Poison;
    beqlr;

    // # Must be quicksand waves
    li    r3, 2;//WaveCollision_Quicksand;
    blr;
)

tBranch(0x0218F884, WaveCollision_r12, tk::BranchType::bl); // r12 is Bg::instance()->mWaveType
tPatchNop(0x028B7FC4);