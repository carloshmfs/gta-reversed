/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/

#include "StdInc.h"

#include "CarCtrl.h"
#include "TrafficLights.h"
#include "TheScripts.h"
#include "GangWars.h"
#include "Game.h"
#include "General.h"
#include "GameLogic.h"
#include "CutsceneMgr.h"
#include "TheCarGenerators.h"
#include "eAreaCodes.h"

#include <reversiblebugfixes/Bugs.hpp>

uint32& CCarCtrl::NumLawEnforcerCars = *(uint32*)0x969098;
uint32& CCarCtrl::NumParkedCars = *(uint32*)0x9690A0;
uint32& CCarCtrl::NumAmbulancesOnDuty = *(uint32*)0x9690A8;
uint32& CCarCtrl::NumFireTrucksOnDuty = *(uint32*)0x9690AC;
uint32& CCarCtrl::MaxNumberOfCarsInUse = *(uint32*)0x8A5B24;
float& CCarCtrl::CarDensityMultiplier = *(float*)0x8A5B20;
int32& CCarCtrl::NumRandomCars = *(int32*)0x969094;
int32& CCarCtrl::NumMissionCars = *(int32*)0x96909C;
int32& CCarCtrl::NumPermanentVehicles = *(int32*)0x9690A4;
int32& CCarCtrl::LastTimeAmbulanceCreated = *(int32*)0x9690B0;
int32& CCarCtrl::LastTimeFireTruckCreated = *(int32*)0x9690B4;
bool& CCarCtrl::bAllowEmergencyServicesToBeCreated = *(bool*)0x8A5B28;
bool& CCarCtrl::bCarsGeneratedAroundCamera = *(bool*)0x9690C1;
int8& CCarCtrl::CountDownToCarsAtStart = *(int8*)0x9690C0;
float& CCarCtrl::TimeNextMadDriverChaseCreated = *(float*)0x9690BC;
int32& CCarCtrl::SequenceElements = *(int32*)0x969078;
int32& CCarCtrl::SequenceRandomOffset = *(int32*)0x969074;
bool& CCarCtrl::bSequenceOtherWay = *(bool*)0x969070;
int32& CCarCtrl::LastTimeLawEnforcerCreated = *(int32*)0x9690B8;

CVehicle* (&apCarsToKeep)[2] = *(CVehicle*(*)[2])0x969084;
uint32 (&aCarsToKeepTime)[2] = *(uint32(*)[2])0x96907C;

void CCarCtrl::InjectHooks()
{
    RH_ScopedClass(CCarCtrl);
    RH_ScopedCategoryGlobal();

    using namespace ReversibleHooks;
    RH_ScopedInstall(Init, 0x4212E0);
    RH_ScopedInstall(ReInit, 0x4213B0);
    RH_ScopedInstall(InitSequence, 0x421740);
    RH_ScopedInstall(ChooseGangCarModel, 0x421A40, { .jmpCodeSize = 7 });
    RH_ScopedInstall(ChoosePoliceCarModel, 0x421980, { .jmpCodeSize = 7 });
    RH_ScopedInstall(CreateCarForScript, 0x431F80);
    RH_ScopedInstall(ChooseBoatModel, 0x421970);
    RH_ScopedInstall(ChooseCarModelToLoad, 0x421900);
    RH_ScopedInstall(GetNewVehicleDependingOnCarModel, 0x421440, { .reversed = false });
    RH_ScopedInstall(IsAnyoneParking, 0x42C250);
    RH_ScopedInstall(IsThisVehicleInteresting, 0x423EA0);
    RH_ScopedInstall(JoinCarWithRoadAccordingToMission, 0x432CB0);
    RH_ScopedInstall(PossiblyFireHSMissile, 0x429600);
    RH_ScopedInstall(PruneVehiclesOfInterest, 0x423F10);
    RH_ScopedInstall(RemoveCarsIfThePoolGetsFull, 0x4322B0);
    RH_ScopedInstall(RemoveDistantCars, 0x42CD10);
    RH_ScopedInstall(RemoveFromInterestingVehicleList, 0x423ED0);
    RH_ScopedInstall(ScriptGenerateOneEmergencyServicesCar, 0x42FBC0);
    RH_ScopedInstall(SlowCarDownForObject, 0x426220);
    RH_ScopedInstall(SlowCarOnRailsDownForTrafficAndLights, 0x434790);
    RH_ScopedInstall(FindMaxSteerAngle, 0x427FE0);
    RH_ScopedInstall(GenerateRandomCars, 0x4341C0);
}

// 0x4212E0
void CCarCtrl::Init() {
    ZoneScoped;

    CarDensityMultiplier = 1.0f;
    NumRandomCars = 0;
    NumLawEnforcerCars = 0;
    NumMissionCars = 0;
    NumParkedCars = 0;
    NumPermanentVehicles = 0;
    NumAmbulancesOnDuty = 0;
    NumFireTrucksOnDuty = 0;

    LastTimeAmbulanceCreated = 0;
    LastTimeFireTruckCreated = 0;
    bAllowEmergencyServicesToBeCreated = true;
    bCarsGeneratedAroundCamera = false;
    CountDownToCarsAtStart = 2;

    TimeNextMadDriverChaseCreated = CGeneral::GetRandomNumberInRange(600.0f, 1200.0f);

    std::ranges::fill(apCarsToKeep, nullptr);
    for (auto& group : CPopulation::m_LoadedGangCars) {
        group.Clear();
    }
    CPopulation::m_AppropriateLoadedCars.Clear();
    CPopulation::m_InAppropriateLoadedCars.Clear();
    CPopulation::m_LoadedBoats.Clear();
}

// 0x4213B0
void CCarCtrl::ReInit() {
    CarDensityMultiplier = 1.0f;
    NumRandomCars = 0;
    NumLawEnforcerCars = 0;
    NumMissionCars = 0;
    NumParkedCars = 0;
    NumPermanentVehicles = 0;
    NumAmbulancesOnDuty = 0;
    NumFireTrucksOnDuty = 0;

    LastTimeLawEnforcerCreated = 0;

    bAllowEmergencyServicesToBeCreated = true;
    CountDownToCarsAtStart = 2;

    std::ranges::fill(apCarsToKeep, nullptr);
    for (auto& group : CPopulation::m_LoadedGangCars) {
        group.Clear();
    }
    CPopulation::m_AppropriateLoadedCars.Clear();
    CPopulation::m_InAppropriateLoadedCars.Clear();
    CPopulation::m_LoadedBoats.Clear();
}

// 0x421970
int32 CCarCtrl::ChooseBoatModel() {
    return CPopulation::m_LoadedBoats.PickLeastUsedModel(1);
}

// 0x421900
int32 CCarCtrl::ChooseCarModelToLoad(int32 groupID) {
    const auto numCarsInGroup = CPopulation::m_nNumCarsInGroup[groupID];
    if (numCarsInGroup > 0) {
        for (auto i = 0; i < 16; i++) { // 16 tries
            const auto model = CPopulation::m_CarGroups[groupID][CGeneral::GetRandomNumberInRange(numCarsInGroup)];
            if (!CStreaming::IsModelLoaded(model)) {
                return model;
            }
        }
    }
    return -1;
}

eModelID CCarCtrl::ChooseGangCarModel(eGangID loadedCarGroupId) {
    return CPopulation::PickGangCar(loadedCarGroupId);
}

// 0x424CE0
int32 CCarCtrl::ChooseModel(int32* arg1) {
    return plugin::CallAndReturn<int32, 0x424CE0, int32*>(arg1);
}

int32 CCarCtrl::ChoosePoliceCarModel(uint32 ignoreLvpd1Model) {
    CWanted* playerWanted = FindPlayerWanted();
    if (playerWanted->AreSwatRequired()
        && CStreaming::IsModelLoaded(MODEL_ENFORCER)
        && CStreaming::IsModelLoaded(MODEL_SWAT)
    ) {
        if (CGeneral::GetRandomNumberInRange(0, 3) == 2)
            return MODEL_ENFORCER;
    }
    else
    {
        if (playerWanted->AreFbiRequired()
            && CStreaming::IsModelLoaded(MODEL_FBIRANCH)
            && CStreaming::IsModelLoaded(MODEL_FBI))
            return MODEL_FBIRANCH;

        if (playerWanted->AreArmyRequired()
            && CStreaming::IsModelLoaded(MODEL_RHINO)
            && CStreaming::IsModelLoaded(MODEL_BARRACKS)
            && CStreaming::IsModelLoaded(MODEL_ARMY))
            return (CGeneral::GetRandomNumber() < 0x3FFF) + MODEL_RHINO;
    }
    return CStreaming::GetDefaultCopCarModel(ignoreLvpd1Model);
}

// 0x423F00
void CCarCtrl::ClearInterestingVehicleList() {
    plugin::Call<0x423F00>();
}

// 0x422760
void CCarCtrl::ClitargetOrientationToLink(CVehicle* vehicle, CCarPathLinkAddress arg2, int8 arg3, float* arg4, float arg5, float arg6) {
    plugin::Call<0x422760, CVehicle*, CCarPathLinkAddress, int8, float*, float, float>(vehicle, arg2, arg3, arg4, arg5, arg6);
}

// 0x431F80
CVehicle* CCarCtrl::CreateCarForScript(int32 modelid, CVector posn, bool doMissionCleanup) {
    if (CModelInfo::IsBoatModel(modelid))
    {
        auto* boat = new CBoat(modelid, eVehicleCreatedBy::MISSION_VEHICLE);
        if (posn.z <= MAP_Z_LOW_LIMIT)
            posn.z = CWorld::FindGroundZForCoord(posn.x, posn.y);

        posn.z += boat->GetDistanceFromCentreOfMassToBaseOfModel();
        boat->SetPosn(posn);

        CTheScripts::ClearSpaceForMissionEntity(posn, boat);
        boat->vehicleFlags.bEngineOn = false;
        boat->vehicleFlags.bIsLocked = true;
        boat->m_nStatus = eEntityStatus::STATUS_ABANDONED;
        JoinCarWithRoadSystem(boat);

        boat->m_autoPilot.SetCarMission(eCarMission::MISSION_NONE);
        boat->m_autoPilot.m_nTempAction = TEMPACT_NONE;
        boat->m_autoPilot.m_speed = 20.0F;
        boat->m_autoPilot.SetCruiseSpeed(20);

        if (doMissionCleanup)
            boat->m_bIsStaticWaitingForCollision = true;

        boat->m_autoPilot.movementFlags.bIsStopped = true;
        CWorld::Add(boat);

        if (doMissionCleanup)
            CTheScripts::MissionCleanUp.AddEntityToList(GetVehiclePool()->GetRef(boat), MISSION_CLEANUP_ENTITY_TYPE_VEHICLE);

        return boat;
    }

    auto* vehicle = GetNewVehicleDependingOnCarModel(modelid, eVehicleCreatedBy::MISSION_VEHICLE);
    if (posn.z <= MAP_Z_LOW_LIMIT)
        posn.z = CWorld::FindGroundZForCoord(posn.x, posn.y);

    posn.z += vehicle->GetDistanceFromCentreOfMassToBaseOfModel();
    vehicle->SetPosn(posn);

    if (!doMissionCleanup)
    {
        if (vehicle->IsAutomobile())
            vehicle->AsAutomobile()->PlaceOnRoadProperly();
        else if (vehicle->IsBike())
            vehicle->AsBike()->PlaceOnRoadProperly();
    }

    if (vehicle->IsTrain())
        vehicle->AsTrain()->trainFlags.bNotOnARailRoad = true;

    CTheScripts::ClearSpaceForMissionEntity(posn, vehicle);
    vehicle->vehicleFlags.bIsLocked = true;
    vehicle->m_nStatus = eEntityStatus::STATUS_ABANDONED;
    JoinCarWithRoadSystem(vehicle);
    vehicle->vehicleFlags.bEngineOn = false;
    vehicle->vehicleFlags.bHasBeenOwnedByPlayer = true;

    vehicle->m_autoPilot.SetCarMission(eCarMission::MISSION_NONE);
    vehicle->m_autoPilot.m_nTempAction = TEMPACT_NONE;
    vehicle->m_autoPilot.m_nCarDrivingStyle = DRIVING_STYLE_STOP_FOR_CARS;
    vehicle->m_autoPilot.m_speed = 13.0F;
    vehicle->m_autoPilot.SetCruiseSpeed(13);
    vehicle->m_autoPilot.m_nCurrentLane = 0;
    vehicle->m_autoPilot.m_nNextLane = 0;

    if (doMissionCleanup)
        vehicle->m_bIsStaticWaitingForCollision = true;

    CWorld::Add(vehicle);
    if (doMissionCleanup)
        CTheScripts::MissionCleanUp.AddEntityToList(GetVehiclePool()->GetRef(vehicle), MISSION_CLEANUP_ENTITY_TYPE_VEHICLE);

    if (vehicle->IsSubRoadVehicle())
        vehicle->m_autoPilot.movementFlags.bIsStopped = true;

    return vehicle;
}

// 0x42C740
bool CCarCtrl::CreateConvoy(CVehicle* vehicle, int32 arg2) {
    return plugin::CallAndReturn<bool, 0x42C740, CVehicle*, int32>(vehicle, arg2);
}

// 0x42C2B0
bool CCarCtrl::CreatePoliceChase(CVehicle* vehicle, int32 arg2, CNodeAddress NodeAddress) {
    return plugin::CallAndReturn<bool, 0x42C2B0, CVehicle*, int32, CNodeAddress>(vehicle, arg2, NodeAddress);
}

// 0x428040
bool CCarCtrl::DealWithBend_Racing(CVehicle* vehicle, CCarPathLinkAddress LinkAddress1, CCarPathLinkAddress LinkAddress2, CCarPathLinkAddress LinkAddress3, CCarPathLinkAddress LinkAddress4, char arg6, char arg7, char arg8, char arg9, float arg10, float* arg11, float* arg12, float* arg13, float* arg14, CVector* pos) {
    return plugin::CallAndReturn<bool, 0x428040, CVehicle*, CCarPathLinkAddress, CCarPathLinkAddress, CCarPathLinkAddress, CCarPathLinkAddress, int8, int8, int8, int8, float, float*, float*, float*, float*, CVector*>(vehicle, LinkAddress1, LinkAddress2, LinkAddress3, LinkAddress4, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, pos);
}

// 0x42EC90
void CCarCtrl::DragCarToPoint(CVehicle* vehicle, CVector* pos) {
    plugin::Call<0x42EC90, CVehicle*, CVector*>(vehicle, pos);
}

// 0x4325C0
float CCarCtrl::FindAngleToWeaveThroughTraffic(CVehicle* vehicle, CPhysical* physical, float arg3, float arg4, float arg5) {
    return plugin::CallAndReturn<float, 0x4325C0, CVehicle*, CPhysical*, float, float, float>(vehicle, physical, arg3, arg4, arg5);
}

// 0x4226F0
void CCarCtrl::FindIntersection2Lines(float arg1, float arg2, float arg3, float arg4, float arg5, float arg6, float arg7, float arg8, float* arg9, float* arg10) {
    plugin::Call<0x4226F0, float, float, float, float, float, float, float, float, float*, float*>(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
}

// 0x42B470
void CCarCtrl::FindLinksToGoWithTheseNodes(CVehicle* vehicle) {
    plugin::Call<0x42B470, CVehicle*>(vehicle);
}

// 0x434400
float CCarCtrl::FindMaximumSpeedForThisCarInTraffic(CVehicle* vehicle) {
    return plugin::CallAndReturn<float, 0x434400, CVehicle*>(vehicle);
}

// 0x42BD20
void CCarCtrl::FindNodesThisCarIsNearestTo(CVehicle* vehicle, CNodeAddress& nodeAddress1, CNodeAddress& nodeAddress2) {
    plugin::Call<0x42BD20, CVehicle*, CNodeAddress&, CNodeAddress&>(vehicle, nodeAddress1, nodeAddress2);
}

// 0x422090
int8 CCarCtrl::FindPathDirection(CNodeAddress nodeAddress1, CNodeAddress nodeAddress2, CNodeAddress nodeAddress3, bool* arg4) {
    return plugin::CallAndReturn<int8, 0x422090, CNodeAddress, CNodeAddress, CNodeAddress, bool*>(nodeAddress1, nodeAddress2, nodeAddress3, arg4);
}

// 0x422620
float CCarCtrl::FindPercDependingOnDistToLink(CVehicle* vehicle, CCarPathLinkAddress linkAddress) {
    return plugin::CallAndReturn<float, 0x422620, CVehicle*, CCarPathLinkAddress>(vehicle, linkAddress);
}

// 0x421770
int32 CCarCtrl::FindSequenceElement(int32 arg1) {
    return plugin::CallAndReturn<int32, 0x421770, int32>(arg1);
}

// 0x4224E0
float CCarCtrl::FindSpeedMultiplier(float arg1, float arg2, float arg3, float arg4) {
    return plugin::CallAndReturn<float, 0x4224E0, float, float, float, float>(arg1, arg2, arg3, arg4);
}

// 0x424130
float CCarCtrl::FindSpeedMultiplierWithSpeedFromNodes(int8 arg1) {
    return plugin::CallAndReturn<float, 0x424130, int8>(arg1);
}

float CCarCtrl::FindGhostRoadHeight(CVehicle* vehicle) {
    return plugin::CallAndReturn<float, 0x422370, CVehicle*>(vehicle);
}

// 0x42B270
void CCarCtrl::FireHeliRocketsAtTarget(CAutomobile* entityLauncher, CEntity* entity) {
    plugin::Call<0x42B270, CAutomobile*, CEntity*>(entityLauncher, entity);
}

// 0x429A70
void CCarCtrl::FlyAIHeliInCertainDirection(CHeli* heli, float arg2, float arg3, bool arg4) {
    plugin::Call<0x429A70, CHeli*, float, float, bool>(heli, arg2, arg3, arg4);
}

// 0x423940
void CCarCtrl::FlyAIHeliToTarget_FixedOrientation(CHeli* heli, float Orientation, CVector posn) {
    plugin::Call<0x423940, CHeli*, float, CVector>(heli, Orientation, posn);
}

// 0x423000
void CCarCtrl::FlyAIPlaneInCertainDirection(CPlane* pPlane) {
    plugin::Call<0x423000, CPlane*>(pPlane);
}

// 0x424210
bool CCarCtrl::GenerateCarCreationCoors2(CVector posn, float radius, float arg3, float arg4, bool arg5, float arg6, float arg7, CVector* pOrigin, CNodeAddress* pNodeAddress1, CNodeAddress* pNodeAddress12, float* arg11, bool arg12, bool arg13) {
    return plugin::CallAndReturn<bool, 0x424210, CVector, float, float, float, bool, float, float, CVector*, CNodeAddress*, CNodeAddress*, float*, bool, bool>(posn, radius, arg3, arg4, arg5, arg6, arg7, pOrigin, pNodeAddress1, pNodeAddress12, arg11, arg12, arg13);
}

// 0x42F9C0
void CCarCtrl::GenerateEmergencyServicesCar() {
    plugin::Call<0x42F9C0>();
}

// 0x42B7D0
CAutomobile* CCarCtrl::GenerateOneEmergencyServicesCar(uint32 modelId, CVector posn) {
    return plugin::CallAndReturn<CAutomobile*, 0x42B7D0, uint32, CVector>(modelId, posn);
}

// 0x430050
void CCarCtrl::GenerateOneRandomCar() {
    const CVector*         playerPos;         // eax
    float            v2;         // esi
    float            v3;         // ebx
    float            v4;         // ebp
    CVector*         playerSpeed;         // eax
    float            playerSpeedX;         // ecx
    float            playerSpeedY;         // edx
    float            playerSpeedY;         // eax
    CWanted*         playerWanted;         // edi
    int              v10;        // edi
    CAutomobile*     v11;        // eax
    double           vehicleSpeedY;        // st7
    long double      v14;        // st6
    long double      v15;        // st6
    bool             generateCarArg12;        // al
    float            generateCarArg6;         // ST1C_4
            // ST00_12
    float            v19;        // eax
    int              v20;        // esi
    CPathNode*       v21;        // ecx
    int              v22;        // ebx
    CPathNode*       v23;        // edx
    unsigned __int8  v24;        // al
    eModelID         v25;        // eax
    CPathNode*       v26;        // esi
    __int16          v27;        // di
    int              v28;        // ebp
    int              v29;        // eax
    int              v30;        // edx
    CNodeAddress**   v31;        // eax
    CCarPathLink*    v32;        // esi
    __int16          v33;        // ax
    int32            v34;        // edi
    bool             v35;        // zf
    int              v36;        // eax
    CVehicle*        v37;        // eax
    int              v38;        // ebp
    int              v39;        // ecx
    CVehicle* generatedVehicle;        // esi
    float            v41;        // eax
    double           v42;        // st7
    eModelID         v43;        // cx
    CColModel*       v44;        // eax
    double           v45;        // st7
    unsigned __int8  v46;        // al
    int              v47;        // edx
    int              v48;        // eax
    int              v49;        // eax
    CColModel*       v50;        // eax
    CPathNode*       v51;        // edi
    RwV3d*           v52;        // eax
    CPathNode*       v53;        // edx
    CPathNode*       v54;        // ebp
    float            v55;        // ST4C_4
    RwV3d*           v56;        // edi
    float            v57;        // ST4C_4
    RwV3d*           v58;        // eax
    long double      v59;        // st7
    long double      v60;        // st7
    long double      v61;        // st7
    CPathNode*       v62;        // edi
    int              v63;        // eax
    unsigned int     v64;        // ebp
    int              v65;        // edx
    CPathNode*       v66;        // eax
    unsigned __int16 v67;        // ax
    int              v68;        // eax
    CNodeAddress**   v69;        // edx
    CPathNode*       v70;        // edi
    CVector*         a6;         // ST38_4
    CVector*         v72;        // eax
    CVector*         v73;        // eax
    float            v74;        // edx
    long double      v75;        // st7
    long double      v76;        // st7
    double           v77;        // st7
    unsigned int     v78;        // ebx
    eAERadioType     v79;        // bp
    CVector*         v80;        // ST38_4
    CVector*         v81;        // eax
    CVector*         v82;        // eax
    CPathNode*       v83;        // ecx
    CVector*         v84;        // ST38_4
    CVector*         v85;        // eax
    CVector*         v86;        // eax
    unsigned __int16 v87;        // di
    CVector*         v88;        // ST38_4
    CVector*         v89;        // eax
    CVector*         v90;        // eax
    float            v91;        // ST50_4
    CVector*         v92;        // eax
    CVector*         v93;        // eax
    long double      v94;        // st7
    double           v95;        // st6
    unsigned int     v96;        // ebx
    CVector*         v97;        // ST38_4
    CVector*         v98;        // eax
    CVector*         v99;        // eax
    CVector*         v100;       // ST38_4
    CVector*         v101;       // eax
    CVector*         v102;       // eax
    float            v103;       // ST50_4
    CVector*         v104;       // eax
    CVector*         v105;       // eax
    double           v106;       // st7
    double           v107;       // st6
    float            v108;       // edx
    double           v109;       // st7
    signed int       v110;       // ecx
    signed int       v111;       // ST4C_4
    signed int       v112;       // edx
    double           v113;       // st7
    signed int       v114;       // ST4C_4
    CCarPathLink*    v115;       // ecx
    double           v116;       // st7
    CCarPathLink*    v117;       // ecx
    CCarCtrl*        v118;       // eax
    double           v119;       // st7
    unsigned __int16 v120;       // cx
    unsigned __int16 v121;       // di
    double           v122;       // st7
    signed int       v123;       // ST4C_4
    float            v124;       // ST50_4
    CCarPathLink*    v125;       // edx
    float            v126;       // ST4C_4
    double           v127;       // st7
    signed int       v128;       // ST54_4
    signed int       v129;       // ebx
    double           v130;       // st6
    signed int       v131;       // ST54_4
    double           v132;       // st7
    double           v133;       // st6
    float            v134;       // ST38_4
    float            a;          // ST34_4
    float            a4;         // ST30_4
    float            a3;         // ST2C_4
    int              v138;       // edi
    double           v139;       // st7
    unsigned __int64 v140;       // rax
    CCarPathLink*    v141;       // ebx
    double           v142;       // st6
    signed int       v143;       // ecx
    CCarPathLink*    v144;       // ebx
    double           v145;       // st6
    double           v146;       // st5
    float            v147;       // ST2C_4
    CPathNode*       v148;       // edi
    CPathNode*       v149;       // ebp
    CVector*         v150;       // ST38_4
    CVector*         v151;       // eax
    CVector*         v152;       // eax
    float            v153;       // ecx
    float            v154;       // edx
    float            v155;       // eax
    float            v156;       // ST38_4
    CVector*         v157;       // eax
    CVector*         v158;       // eax
    float            v159;       // ebx
    RwV3d*           v160;       // eax
    RwV3d*           v161;       // eax
    double           v162;       // st7
    double           v163;       // st7
    double           v164;       // st6
    double           v165;       // st7
    DWORD*           v166;       // edx
    CVector*         v167;       // eax
    CVector*         v168;       // eax
    float            v169;       // edx
    int              v170;       // ebx
    float            v171;       // ecx
    float            v172;       // edx
    unsigned __int8  v173;       // al
    char             v174;       // al
    CVector*         v175;       // edi
    CVector*         v176;       // eax
    CVector*         v177;       // eax
    double           v178;       // st7
    double           v179;       // st6
    float            v180;       // ST38_4
    CVector*         v181;       // ecx
    CVector*         v182;       // eax
    CVector*         v183;       // eax
    CVector*         v184;       // eax
    double           v185;       // st7
    double           v186;       // st6
    float            v187;       // ST54_4
    float            v188;       // ST34_4
    CVector*         v189;       // eax
    __int16          v190;       // ax
    CPed*            v191;       // eax
    __int16          v192;       // ax
    char             v193;       // al
    unsigned __int8  v194;       // cl
    double           v195;       // st7
    unsigned __int64 v196;       // rax
    float            v197;       // ST34_4
    CVector*         v198;       // eax
    CVector*         v199;       // eax
    CVector*         v200;       // eax
    int              v201;       // eax
    CPopulation*     v202;       // [esp+20h] [ebp-13Ch]
    CPopulation*     v203;       // [esp+20h] [ebp-13Ch]
    CCarAI*          v204;       // [esp+20h] [ebp-13Ch]
    bool             v205;       // [esp+20h] [ebp-13Ch]
    int              totalVehiclesGenerated;       // [esp+30h] [ebp-12Ch]
    unsigned __int16 v207;       // [esp+30h] [ebp-12Ch]
    float            v208;       // [esp+30h] [ebp-12Ch]
    float            carDensity;         // [esp+34h] [ebp-128h]
    float            vehicleGenerationRadius;        // [esp+34h] [ebp-128h]
    float            vehicleSpeedX;        // [esp+34h] [ebp-128h]
    __int16          a5c;        // [esp+34h] [ebp-128h]
    float            a5d;        // [esp+34h] [ebp-128h]
    float            a5e;        // [esp+34h] [ebp-128h]
    char             v215;       // [esp+3Fh] [ebp-11Dh]
    signed int       generateCarArg4;         // [esp+40h] [ebp-11Ch]
    float            a7a;        // [esp+40h] [ebp-11Ch]
    float            generateCarArg3;       // [esp+44h] [ebp-118h]
    float            generateCarArg5;         // [esp+4Ch] [ebp-110h]
    float            pLevel;     // [esp+50h] [ebp-10Ch]
    char             v222;       // [esp+56h] [ebp-106h]
    char             v223;       // [esp+57h] [ebp-105h]
    CVector          v;          // [esp+58h] [ebp-104h]
    char             v225;       // [esp+67h] [ebp-F5h]
    CNodeAddress     carGenerationNodeAddr2; // [esp+68h] [ebp-F4h]
    CNodeAddress     carGenerationNodeAddr1;        // [esp+6Ch] [ebp-F0h]
    float            v228;       // [esp+70h] [ebp-ECh]
    float            v229;       // [esp+74h] [ebp-E8h]
    int              modelArg1;       // [esp+78h] [ebp-E4h]
    float            v231;       // [esp+7Ch] [ebp-E0h]
    float            v232;       // [esp+80h] [ebp-DCh]
    CVector          outVec;     // [esp+84h] [ebp-D8h]
    float            v234;       // [esp+90h] [ebp-CCh]
    unsigned int     v235;       // [esp+94h] [ebp-C8h]
    float            y[3];       // [esp+98h] [ebp-C4h]
    CVector          a1;         // [esp+A4h] [ebp-B8h]
    CVector          carGenerationOrigin; // [esp+B0h] [ebp-ACh]
    float            v239;       // [esp+B4h] [ebp-A8h]
    CVector          a2;         // [esp+BCh] [ebp-A0h]
    float            v241;       // [esp+C8h] [ebp-94h]
    CVector          what;       // [esp+CCh] [ebp-90h]
    RwV3d            v243;       // [esp+D8h] [ebp-84h]
    float            v244;       // [esp+DCh] [ebp-80h]
    float            v245;       // [esp+E0h] [ebp-7Ch]
    CVector          from;       // [esp+E4h] [ebp-78h]
    CVector          out;        // [esp+F0h] [ebp-6Ch]
    CVector          pPosition;  // [esp+FCh] [ebp-60h]
    CVector          v249;       // [esp+108h] [ebp-54h]
    CEntity*         v250;       // [esp+114h] [ebp-48h]
    CVector          v251;       // [esp+118h] [ebp-44h]
    RwV3d            in;         // [esp+124h] [ebp-38h]
    RwV3d            v253;       // [esp+130h] [ebp-2Ch]
    float            v254;       // [esp+138h] [ebp-24h]

    LOWORD(carGenerationNodeAddr1) = -1;
    *carGenerationNodeAddr2        = -1;
    v223        = 0;
    v225        = 0;
    v222        = 0;
    playerPos          = FindPlayerCentreOfWorld(CWorld::PlayerInFocus);
    v2          = playerPos->_pad0;
    v3          = playerPos[1]._pad0;
    v4          = playerPos[2]._pad0;
    from.x      = playerPos->_pad0;
    from.y      = v3;
    from.z      = v4;
    playerSpeed = &FindPlayerSpeed(-1);
    playerSpeedX = playerSpeed->x;
    playerSpeedY = playerSpeed->y;
    playerSpeedY = playerSpeed->z;
    what.y      = playerSpeedY;
    what.x      = playerSpeedX;
    what.z      = playerSpeedY;

    auto canGenerateVehicle = []() -> bool {
        auto totalVehiclesGenerated = CCarCtrl::NumRandomCars + CCarCtrl::NumLawEnforcerCars + CCarCtrl::NumMissionCars + CCarCtrl::NumAmbulancesOnDuty + CCarCtrl::NumFireTrucksOnDuty;
        auto carDensityMultiplier   = CCarCtrl::CarDensityMultiplier;

        if (CCullZones::FewerCars()) {
            carDensityMultiplier *= 0.60000002;
        }

        if (CPopulation::FindCarMultiplierMotorway() * CCarCtrl::MaxNumberOfCarsInUse * CCarCtrl::CarDensityMultiplier <= totalVehiclesGenerated
            || CPopulation::FindCarMultiplierMotorway()
                    * (CPopCycle::m_NumOther_Cars
                       + CPopCycle::m_NumCops_Cars
                       + CPopCycle::m_NumGangs_Cars
                       + CPopCycle::m_NumDealers_Cars)
                    * CCarCtrl::CarDensityMultiplier
                <= totalVehiclesGenerated) {
            return false;
        }

        return true;
    };

    if (!canGenerateVehicle()) {
        return;
    }

    eGeneratedVehicleType vehicleModelType;
    eModelID              vehicleModel;

    if (FindPlayerWanted(-1) && FindPlayerWanted(-1)->m_nWantedLevel <= 1
        || CCarCtrl::NumLawEnforcerCars >= FindPlayerWanted(-1)->m_nMaxCopCarsInPursuit
        || (FindPlayerWanted(-1)->m_nCopsInPursuit >= FindPlayerWanted(-1)->m_nMaxCopsInPursuit)
        || CGame::currArea
        || CGangWars::GangWarFightingGoingOn()
        || FindPlayerWanted(-1)->m_nWantedLevel <= 3
            && (FindPlayerWanted(-1)->m_nWantedLevel <= 2
                || CTimer::m_snTimeInMilliseconds <= (CCarCtrl::LastTimeLawEnforcerCreated + 5'000))
            && CTimer::m_snTimeInMilliseconds <= (CCarCtrl::LastTimeLawEnforcerCreated + 8'000)) {
        vehicleModel = static_cast<eModelID>(CCarCtrl::ChooseModel(reinterpret_cast<int32*>(&vehicleModelType)));
        if (vehicleModel == eModelID::MODEL_INVALID) {
            return;
        }

        if ((vehicleModelType == eGeneratedVehicleType::COP_CARS || vehicleModelType == 24) && FindPlayerWanted(-1)->m_nWantedLevel >= 1) {
            return;
        }
    } else {
        vehicleModel = static_cast<eModelID>(CCarCtrl::ChoosePoliceCarModel(0));
        vehicleModelType = eGeneratedVehicleType::COP_CARS;
    }

    if (CGameLogic::LaRiotsActiveHere() && !gbLARiots_NoPoliceCars && (rand() & 0x7F) < 55) {
        vehicleModel = static_cast<eModelID>(CCarCtrl::ChoosePoliceCarModel(0));
        vehicleModelType = eGeneratedVehicleType::COP_CARS;
    }

    if (TheCamera.m_mCameraMatrix.GetForward().z >= -0.89999998) {
        auto playerVehicle = FindPlayerVehicle(-1, 0);
        if (playerVehicle) {
            // a²=b²+c²
            auto vehicleSpeed = sqrt(playerVehicle->m_vecMoveSpeed.y * playerVehicle->m_vecMoveSpeed.y + playerVehicle->m_vecMoveSpeed.x * playerVehicle->m_vecMoveSpeed.x);
            if (vehicleSpeed > 0.40000001) {
                vehicleGenerationRadius = playerVehicle->m_vecMoveSpeed.x * (1.0 / vehicleSpeed);
                generateCarArg3 = (1.0 / vehicleSpeed) * vehicleSpeedY;
                switch (CTimer::m_FrameCounter & 3) {
                case 0:
                case 1:
                    generateCarArg4 = 1'062'836'634;
                case 2:
                    generateCarArg4 = 1'060'437'492;
                case 3:
                    generateCarArg4 = 1'060'437'492;
                    generateCarArg5 = 0;
                    break;
                }
            }

            if (vehicleSpeed > 0.1) {
                vehicleGenerationRadius = playerVehicle->m_vecMoveSpeed.x * (1.0 / vehicleSpeed);
                generateCarArg3 = (1.0 / vehicleSpeed) * vehicleSpeedY;
                switch (CTimer::m_FrameCounter & 3) {
                case 0:
                    generateCarArg4 = 1'062'836'634;
                    generateCarArg5 = 1;
                    break;
                case 1:
                    generateCarArg4 = 1'060'437'492;
                    break;
                case 2:
                case 3:
                    generateCarArg4 = 1'060'437'492;
                    generateCarArg5 = 0;
                    break;
                }
            
        } else {
            vehicleGenerationRadius  = TheCamera.m_fCamFrontXNorm;
            generateCarArg3 = TheCamera.m_fCamFrontYNorm;
        }

        if ((CTimer::m_FrameCounter & 1) == 1) {
            generateCarArg4 = 1'060'437'492;
            generateCarArg5 = 0;
        }
    } else {
        vehicleGenerationRadius  = 0.70700002;
        generateCarArg3 = 0.70700002;
        generateCarArg4   = -1'082'130'432;
    }

    generateCarArg12        = vehicleModelType != eGeneratedVehicleType::COP_CARS || FindPlayerWanted(-1)->m_nWantedLevel < 1;
    generateCarArg6         = TheCamera.m_fGenerationDistMultiplier * 160.0;
    auto vehicleGeneratePos = FindPlayerCentreOfWorld(CWorld::PlayerInFocus);
    CNodeAddress outAddr1;
    CNodeAddress outAddr2;
    if (CCarCtrl::GenerateCarCreationCoors2(vehicleGeneratePos, vehicleGenerationRadius, generateCarArg3, generateCarArg4, generateCarArg5, generateCarArg6, 38.0, &carGenerationOrigin, &outAddr1, &outAddr2, &generateCarArg3, generateCarArg12, 0)) {
        auto pathNode1               = ThePaths.m_pPathNodes[outAddr1.m_wAreaId][outAddr1.m_wNodeId];
        auto pathNode2               = ThePaths.m_pPathNodes[outAddr2.m_wAreaId][outAddr2.m_wNodeId];
        LOBYTE(v19)     = v21->m_dwFlags[LODWORD(v19) + 2];
        LOBYTE(v21)     = v23[v20].m_dwFlags[2] & 0xF;
        v24             = LOBYTE(v19) & 0xF;
        bool isBoatNode = false;
        LODWORD(pLevel) = 4 * LOWORD(carGenerationNodeAddr1);
        v235            = 28 * HIWORD(carGenerationNodeAddr1);
        LOBYTE(generateCarArg5)      = v21;
        if (v21 >= v24) {
            LOBYTE(generateCarArg5) = v24;
        }
        if ((rand() & 0xFu) <= LOBYTE(generateCarArg5)) {
            if (ThePaths.m_pPathNodes[v22][v20].m_dwFlags[0] >= 0) {
                CWorld::FindObjectsKindaColliding(&carGenerationOrigin, 8.0, 1, &generateCarArg5, 2, 0, 0, 1, 1, 0, 0);
            } else {
                isBoatNode = true;
                if (vehicleModelType == eGeneratedVehicleType::COP_CARS) {
                    vehicleModel = eModelID::MODEL_PREDATOR;
                    modelArg1       = 24;
                    if (unk_8E6E68 != 1) {
                        CStreaming::RequestModel(eModelID::MODEL_PREDATOR, 8);
                        return;
                    }
                } else {
                    vehicleModel = CLoadedCarGroup::PickLeastUsedModel(1);
                    if (vehicleModel == eModelID::MODEL_INVALID || CStreaming::ms_aInfoForModel[vehicleModel].m_LoadState != eStreamingLoadState::LOADSTATE_LOADED) {
                        return;
                    }
                }
                CWorld::FindObjectsKindaColliding(&carGenerationOrigin, 40.0, 1, &generateCarArg5, 2, 0, 0, 1, 1, 0, 0);
            }
            if (LOWORD(generateCarArg5)) {
                return;
            }
            v26 = &ThePaths.m_pPathNodes[v22][v20];
            v27 = 0;
            v28 = v26->m_dwFlags[0] & 0xF;
            if (v28 > 0) {
                v29 = 0;
                do {
                    v30 = v29 + v26->m_wConnectedNodesStartId;
                    v31 = ThePaths.m_pNodeLinks[v22];
                    if (!sub_420950(carGenerationNodeAddr2)) {
                        break;
                    }
                    v29 = ++v27;
                } while (v27 < v28);
            }
            v207 = *(ThePaths.pNaviLinks[v22] + v27 + v26->m_wConnectedNodesStartId);
            v32  = &ThePaths.pNaviNodes[v207 >> 10][v207 & 0x3FF];
            v33  = CNodeAddress::operator__(&v32->info, carGenerationNodeAddr2) ? v32->m_nFlags[0] & 7 : (v32->m_nFlags[0] >> 3) & 7;
            v34  = vehicleModel;
            a5c  = v33;
            if (v33 <= 1) {
                if (vehicleModel == eModelID::MODEL_COACH) {
                    return;
                }
                v35 = vehicleModel == eModelID::MODEL_BUS;
            } else {
                v35 = CModelInfo::ms_modelInfoPtrs[vehicleModel]->m_nVehicleType == 10;
            }
            if (!v35 && v33) {
                if (CPopCycle::m_pCurrZone) {
                    v36 = CTheZones::GetZoneInfo(&carGenerationOrigin, 0)->m_nFlags & 0x1F;
                    if (v36 >= 17 && v36 <= 19) {
                        if (v36 != CPopCycle::m_nCurrentZoneType) {
                            return;
                        }
                        v222 = 1;
                    }
                }

                /*
                * 17 = m_nModelIndex
                * 45 =
                * 228 = m_vecLastCollisionImpactVelocity.y
                * 467 = m_vehicleAudio.m_AuSettings.RadioType
                * 949 = m_autoPilot._smthCurr
                * 950 = m_autoPilot._smthNext
                * 951 = m_autoPilot.m_nCurrentLane
                * 952 = m_autoPilot.m_nNextLane
                * 953 = m_autoPilot.m_nCarDrivingStyle
                * 954 = m_autoPilot.m_nCarMission
                * 955 = m_autoPilot.m_nTempAction
                * 976 = m_autoPilot.m_nCruiseSpeed
                */

                v37          = CCarCtrl::GetNewVehicleDependingOnCarModel(v34, 1);
                v39          = *carGenerationNodeAddr2;
                generatedVehicle = v37;
                v41          = carGenerationNodeAddr1;
                generatedVehicle->m_vehicleAudio.m_AuSettings.HornPitch = -1;
                generatedVehicle->m_vecLastCollisionImpactVelocity.y = v41;
                //*(generatedVehicle + 229) = v39;
                if (modelArg1 == eGeneratedVehicleType::COP_CARS) {
                    generatedVehicle->m_autoPilot.m_nTempAction = eAutoPilotTempAction::TEMPACT_NONE;
                    if (FindPlayerWanted(-1)->m_nWantedLevel) {
                        generatedVehicle->m_autoPilot.m_nCruiseSpeed = CCarAI::FindPoliceCarSpeedForWantedLevel(generatedVehicle);
                        eCarMission vehicleMission;
                        if (generatedVehicle->GetVehicleAppearance() == eVehicleAppearance::VEHICLE_APPEARANCE_BIKE) {
                            vehicleMission = CCarAI::FindPoliceBikeMissionForWantedLevel();
                        } else {
                            vehicleMission = CCarAI::FindPoliceCarMissionForWantedLevel();
                        }
                        generatedVehicle->m_autoPilot.m_nCarMission = vehicleMission;
                        generatedVehicle->m_autoPilot.m_nCarDrivingStyle = eCarDrivingStyle::DRIVING_STYLE_AVOID_CARS;
                    } else {
                        generatedVehicle->m_autoPilot.m_nCruiseSpeed = CGeneral::GetRandomNumberInRange(18.0, 24.0);
                        generatedVehicle->m_autoPilot.m_nCarDrivingStyle = eCarDrivingStyle::DRIVING_STYLE_STOP_FOR_CARS;
                        generatedVehicle->m_autoPilot.m_nCarMission = eCarMission::MISSION_CRUISE;
                    }
                    if (v34 == eModelID::MODEL_FBIRANCH) {
                        generatedVehicle->m_nPrimaryColor = 0;
                        generatedVehicle->m_nSecondaryColor = 0;
                    }
                } else {
                    if (modelArg1 != 24) {
                        generatedVehicle->m_autoPilot.m_nCruiseSpeed = CGeneral::GetRandomNumberInRange(13.0, 21.0);
                        if (modelArg1 == 3) {
                            v42 = CGeneral::GetRandomNumberInRange(18.0, 27.0);
                        } else {
                            if (modelArg1 != 1) {
                                goto LABEL_81;
                            }
                            v42 = CGeneral::GetRandomNumberInRange(10.0, 15.0);
                        }
                        generatedVehicle->m_autoPilot.m_nCruiseSpeed = v42;
LABEL_81:
                        auto vehicleModelInfo = CModelInfo::ms_modelInfoPtrs[generatedVehicle->m_nModelIndex]->m_pColModel;
                        if (vehicleModelInfo->GetBoundingBox().m_vecMax.y - vehicleModelInfo->GetBoundingBox().m_vecMin.y > 10.0 || vehicleModelInfo == 5) {
                            generatedVehicle->m_autoPilot.m_nCruiseSpeed = 3 * generatedVehicle->m_autoPilot.m_nCruiseSpeed / 4;
                        }
                        if (isBoatNode) {
                            float cruiseSpeed;
                            if (generatedVehicle->m_nModelIndex == eModelID::MODEL_SQUALO || generatedVehicle->m_nModelIndex == eModelID::MODEL_SPEEDER || generatedVehicle->m_nModelIndex == eModelID::MODEL_JETMAX) {
                                cruiseSpeed = CGeneral::GetRandomNumberInRange(25.0, 35.0);
                            } else {
                                cruiseSpeed = CGeneral::GetRandomNumberInRange(15.0, 24.0);
                            }
                            generatedVehicle->m_autoPilot.m_nCruiseSpeed = cruiseSpeed;
                        }

                        generatedVehicle->m_autoPilot.m_nCarMission      = eCarMission::MISSION_CRUISE;
                        generatedVehicle->m_autoPilot.m_nTempAction      = eAutoPilotTempAction::TEMPACT_NONE;
                        generatedVehicle->m_autoPilot.m_nCarDrivingStyle = eCarDrivingStyle::DRIVING_STYLE_STOP_FOR_CARS;
LABEL_102:
                        if (generatedVehicle->m_nModelIndex == eModelID::MODEL_MRWHOOP) {
                            generatedVehicle->vehicleFlags |= 0x80u; // TODO: offset: 1069
                        }
                        generatedVehicle->m_vehicleAudio.m_AuSettings.RadioType = v207;
                        v47          = rand() % a5c;
                        generatedVehicle->m_autoPilot.m_nCurrentLane = v47;
                        generatedVehicle->m_autoPilot.m_nNextLane    = v47;
                        if (CGameLogic::LaRiotsActiveHere()) {
                            v48 = 80;
                        } else {
                            auto vehicleApperance = generatedVehicle->GetVehicleAppearance();
                            if (vehicleApperance) {
                                if (vehicleApperance == eVehicleAppearance::VEHICLE_APPEARANCE_BIKE) {
                                    v48 = 10;
                                } else {
                                    v48 = 200;
                                }
                            } else {
                                v48 = 50;
                            }
                        }
                        if (!v215
                            && v38 != eGeneratedVehicleType::COP_CARS
                            && !v222
                            && (!CGeneral::GetRandomNumberInRange(0, v48) || CCheat::m_aCheatsActive[CHEAT_AGGRESSIVE_DRIVERS])) {
                            v225 = 1;
                            generateCarArg3 = 1.0;
                        }
                        v50  = CModelInfo::ms_modelInfoPtrs[generatedVehicle->m_nModelIndex]->clump.base.m_pColModel;
                        v51  = &ThePaths.m_pPathNodes[v22][v235 / 0x1C];
                        a5d  = (v50->m_Box.max.y - v50->m_Box.min.y) * 0.5 + 1.0;
                        v52  = CPathNode::GetNodeCoors(v51, &pPosition);
                        v53  = ThePaths.m_pPathNodes[*carGenerationNodeAddr2];
                        v228 = v52->y;
                        v54  = (v53 + LODWORD(v229));
                        v55  = CPathNode::GetNodeCoors(v51, &v251)->y;
                        v234 = CPathNode::GetNodeCoors(v51, &a1)->x;
                        v231 = COERCE_FLOAT(CPathNode::GetNodeCoors(v54, &v));
                        v232 = CPathNode::GetNodeCoors(v51, &v249)->x;
                        v56  = CPathNode::GetNodeCoors(v54, &out);
                        v57  = v55 - CPathNode::GetNodeCoors(v54, &a2)->y;
                        v58  = CPathNode::GetNodeCoors(v54, &outVec);
                        v59  = sqrt((v228 - v58->y) * v57 + (v232 - v56->x) * (v234 - *LODWORD(v231)));
                        if (0.5 * v59 >= a5d) {
                            v60 = a5d / v59;
                            if (generateCarArg3 <= v60) {
                                generateCarArg3 = v60;
                            }
                            v61 = 1.0 - v60;
                            if (generateCarArg3 >= v61) {
                                generateCarArg3 = v61;
                            }
                        } else {
                            generateCarArg3 = 0.5;
                        }
                        if (sub_420980(carGenerationNodeAddr2)) {
                            generatedVehicle->m_autoPilot._smthNext = -1;
                        } else {
                            generatedVehicle->m_autoPilot._smthNext = 1;
                        }
                        v62 = &ThePaths.m_pPathNodes[v22][v235 / 0x1C];
                        if ((v62->m_dwFlags[0] & 0xF) == 1) {
                            goto LABEL_189;
                        }
                        do {
                            v63 = rand();
                            v64 = v235;
                            v65 = v63 % (v62->m_dwFlags[0] & 0xF);
                            v66 = ThePaths.m_pPathNodes[v22];
                            v62 = &v66[v235 / 0x1C];
                            v67 = *(ThePaths.pNaviLinks[v22] + v65 + v66[v235 / 0x1C].m_wConnectedNodesStartId);
                        } while (v67 == *(generatedVehicle + 467));
                        *(generatedVehicle + 466) = v67;
                        if (!ThePaths.m_pPathNodes[v67 >> 10]) {
                            goto LABEL_189;
                        }
                        v68 = *(&ThePaths.m_pPathNodes[v22]->m_wConnectedNodesStartId + v64);
                        v69 = ThePaths.m_pNodeLinks[v22];
                        if (sub_420980(&carGenerationNodeAddr1)) {
                            generatedVehicle->m_autoPilot._smthCurr = -1;
                        } else {
                            generatedVehicle->m_autoPilot._smthCurr = 1;
                        }
                        v70        = (ThePaths.m_pPathNodes[*carGenerationNodeAddr2] + LODWORD(v229));
                        a6         = CPathNode::GetNodeCoors((ThePaths.m_pPathNodes[v22] + v64), &outVec);
                        v72        = CPathNode::GetNodeCoors(v70, &a2);
                        v73        = vectorSub(&out, v72, a6);
                        v243._pad0 = v73->x;
                        v244       = v73->y;
                        v245       = v73->z;
                        v74        = v73->x;
                        v.y        = v73->y;
                        v.x        = v74;
                        v.z        = v73->z;
                        v75        = sqrt(v.y * v.y + v74 * v74);
                        if (v75 == 0.0) {
                            v.x = 1.0;
                        } else {
                            v76 = 1.0 / v75;
                            v.x = v.x * v76;
                            v.y = v76 * v.y;
                        }
                        CVector::Normalise(&v243);
                        v77                = -v.x;
                        *(*(generatedVehicle + 5) + 16) = v243;
                        *(*(generatedVehicle + 5) + 20) = v244;
                        *(*(generatedVehicle + 5) + 24) = v245;
                        **(generatedVehicle + 5)        = LODWORD(v.y);
                        *(*(generatedVehicle + 5) + 4)  = v77;
                        *(*(generatedVehicle + 5) + 8)  = 0;
                        *(*(generatedVehicle + 5) + 32) = 0;
                        *(*(generatedVehicle + 5) + 36) = 0;
                        *(*(generatedVehicle + 5) + 40) = 1'065'353'216;
                        v78                = generatedVehicle->m_vecLastCollisionImpactVelocity.y;
                        v79                = generatedVehicle->m_vehicleAudio.m_AuSettings.RadioType;
                        v80                = CPathNode::GetNodeCoors(&ThePaths.m_pPathNodes[v78][*(generatedVehicle + 228) >> 16], &outVec);
                        v81                = CCompressedVector::to3dVector(&ThePaths.pNaviNodes[v79 >> 10][v79 & 0x3FF], &a2);
                        v82                = vectorSub(&out, v81, v80);
                        v83                = &ThePaths.m_pPathNodes[*(generatedVehicle + 229)][*(generatedVehicle + 229) >> 16];
                        *&vehicleModel       = sqrt(v82->x * v82->x + v82->y * v82->y);
                        v84                = CPathNode::GetNodeCoors(v83, &outVec);
                        v85                = CCompressedVector::to3dVector(&ThePaths.pNaviNodes[v79 >> 10][v79 & 0x3FF], &v249);
                        v86                = vectorSub(&pPosition, v85, v84);
                        if (vehicleModel / (sqrt(v86->x * v86->x + v86->y * v86->y) + vehicleModel) <= generateCarArg3) {
                            CCarCtrl::PickNextNodeRandomly(generatedVehicle);
                            v96        = generatedVehicle->m_vecLastCollisionImpactVelocity.y;
                            v79        = generatedVehicle->m_vehicleAudio.m_AuSettings.RadioType;
                            v97        = CPathNode::GetNodeCoors(&ThePaths.m_pPathNodes[v96][*(generatedVehicle + 228) >> 16], &outVec);
                            v98        = CCompressedVector::to3dVector(&ThePaths.pNaviNodes[v79 >> 10][v79 & 0x3FF], &a2);
                            *&v99      = COERCE_FLOAT(vectorSub(&out, v98, v97));
                            v87        = *(generatedVehicle + 466);
                            v228       = *&v99;
                            v100       = CPathNode::GetNodeCoors(&ThePaths.m_pPathNodes[v96][v96 >> 16], &v249);
                            v101       = CCompressedVector::to3dVector(&ThePaths.pNaviNodes[v87 >> 10][v87 & 0x3FF], &pPosition);
                            v102       = vectorSub(&v251, v101, v100);
                            v103       = sqrt(v102->x * v102->x + v102->y * v102->y);
                            v104       = CPathNode::GetNodeCoors(&ThePaths.m_pPathNodes[v96][v96 >> 16], &v249);
                            v105       = vectorSub(&a1, &carGenerationOrigin, v104);
                            v106       = v105->y;
                            v107       = v105->x;
                            v108       = *(LODWORD(v228) + 4);
                            vehicleModel = *LODWORD(v228);
                            v94        = v103 - sqrt(v107 * v107 + v106 * v106);
                            v95        = sqrt(*&vehicleModel * *&vehicleModel + v108 * v108) + v103;
                        } else {
                            v87 = *(generatedVehicle + 466);
                            v88 = CPathNode::GetNodeCoors(&ThePaths.m_pPathNodes[v78][v78 >> 16], &outVec);
                            v89 = CCompressedVector::to3dVector(&ThePaths.pNaviNodes[v87 >> 10][v87 & 0x3FF], &a2);
                            v90 = vectorSub(&out, v89, v88);
                            v91 = sqrt(v90->x * v90->x + v90->y * v90->y);
                            v92 = CPathNode::GetNodeCoors(&ThePaths.m_pPathNodes[v78][v78 >> 16], &outVec);
                            v93 = vectorSub(&v249, &carGenerationOrigin, v92);
                            v94 = sqrt(v93->x * v93->x + v93->y * v93->y) + v91;
                            v95 = v91 + *&vehicleModel;
                        }
                        a7a = v94 / v95;
                        if (a7a < 0.0) {
                            a7a = 0.0;
                        } else if (a7a > 1.0) {
                            a7a = 1.0;
                        }
                        v109         = generatedVehicle[949];
                        v110         = generatedVehicle[950];
                        v111         = ThePaths.pNaviNodes[v87 >> 10][v87 & 0x3FF].dirY;
                        v234         = ThePaths.pNaviNodes[v87 >> 10][v87 & 0x3FF].dirX * 0.0099999998 * v109;
                        v112         = ThePaths.pNaviNodes[v79 >> 10][v79 & 0x3FF].dirX;
                        v232         = v111 * 0.0099999998 * v109;
                        v113         = v110;
                        v114         = ThePaths.pNaviNodes[v79 >> 10][v79 & 0x3FF].dirY;
                        v241         = v112 * 0.0099999998 * v113;
                        v115         = &ThePaths.pNaviNodes[v87 >> 10][v87 & 0x3FF];
                        v231         = v114 * 0.0099999998 * v113;
                        v116         = CCarPathLink::OneWayLaneOffset(v115);
                        v117         = &ThePaths.pNaviNodes[*(generatedVehicle + 467) >> 10][*(generatedVehicle + 467) & 0x3FF];
                        *&vehicleModel = (v116 + generatedVehicle[951]) * 5.4000001;
                        a5e          = (CCarPathLink::OneWayLaneOffset(v117) + generatedVehicle[952]) * 5.4000001;
                        if (*(generatedVehicle + 357) == 10) {
                            *&vehicleModel = *&vehicleModel + 1.4580001;
                            a5e          = a5e + 1.4580001;
                        }
                        v118         = ((ThePaths.m_pPathNodes[*(generatedVehicle + 229)][*(generatedVehicle + 229) >> 16].m_dwFlags[1] >> 4) & 3);
                        generatedVehicle[977]     = v118;
                        v119         = CCarCtrl::FindSpeedMultiplierWithSpeedFromNodes(v118, v204);
                        *(generatedVehicle + 245) = v119;
                        v120         = *(generatedVehicle + 467);
                        *(generatedVehicle + 242) = generatedVehicle[976] * v119;
                        v228         = a5e * v241;
                        v121         = *(generatedVehicle + 466);
                        v122         = ThePaths.pNaviNodes[v120 >> 10][v120 & 0x3FF].y * 0.125;
                        v123         = ThePaths.pNaviNodes[v120 >> 10][v120 & 0x3FF].x;
                        v124         = a5e * v231;
                        v125         = ThePaths.pNaviNodes[v121 >> 10];
                        v.z          = 0.0;
                        v.x          = v123 * 0.125 + v124;
                        v.y          = v122 - v228;
                        v126         = *&vehicleModel * v234;
                        v127         = v125[v121 & 0x3FF].y * 0.125;
                        v128         = ThePaths.pNaviNodes[v121 >> 10][v121 & 0x3FF].x;
                        *&vehicleModel = *&vehicleModel * v232;
                        v129         = generatedVehicle[949];
                        v130         = v128 * 0.125;
                        v131         = generatedVehicle[950];
                        a1.z         = 0.0;
                        a1.x         = v130 + *&vehicleModel;
                        a1.y         = v127 - v126;
                        v132         = v131;
                        v133         = v129;
                        v134         = ThePaths.pNaviNodes[v120 >> 10][v120 & 0x3FF].dirY * 0.0099999998 * v132;
                        a            = ThePaths.pNaviNodes[v120 >> 10][v120 & 0x3FF].dirX * 0.0099999998 * v132;
                        a4           = ThePaths.pNaviNodes[v121 >> 10][v121 & 0x3FF].dirY * 0.0099999998 * v133;
                        a3           = ThePaths.pNaviNodes[v121 >> 10][v121 & 0x3FF].dirX * 0.0099999998 * v133;
                        v138         = (CCurves::CalcSpeedScaleFactor(&a1, &v, a3, a4, a, v134) * (1000.0 / *(generatedVehicle + 242)));
                        v139         = v138;
                        *(generatedVehicle + 232) = v138;
                        v140         = (CTimer::m_snTimeInMilliseconds - a7a * v139);
                        a1.x         = v241;
                        a1.y         = v231;
                        v.x          = v234;
                        v.y          = v232;
                        WORD2(v140)  = *(generatedVehicle + 467);
                        *(generatedVehicle + 231) = v140;
                        v141         = ThePaths.pNaviNodes[WORD2(v140) >> 10];
                        a1.z         = 0.0;
                        v.z          = 0.0;
                        v142         = v141[WORD2(v140) & 0x3FF].y * 0.125;
                        v143         = ThePaths.pNaviNodes[WORD2(v140) >> 10][WORD2(v140) & 0x3FF].x;
                        WORD2(v140)  = *(generatedVehicle + 466);
                        v144         = ThePaths.pNaviNodes[WORD2(v140) >> 10];
                        v251.z       = 0.0;
                        v251.x       = v143 * 0.125 + v124;
                        v251.y       = v142 - v228;
                        v145         = v144[WORD2(v140) & 0x3FF].y * 0.125;
                        v146         = ThePaths.pNaviNodes[WORD2(v140) >> 10][WORD2(v140) & 0x3FF].x * 0.125 + *&vehicleModel;
                        pPosition.z  = 0.0;
                        pPosition.x  = v146;
                        pPosition.y  = v145 - v126;
                        v147         = (CTimer::m_snTimeInMilliseconds - v140) / v139;
                        CCurves::CalcCurvePoint(&pPosition, &v251, COERCE_FLOAT(&v), COERCE_FLOAT(&a1), v147, v138, &carGenerationOrigin, &in);
                        v148   = &(*(ThePaths.m_pPathNodes + LODWORD(pLevel)))[v235 / 0x1C];
                        v149   = (ThePaths.m_pPathNodes[*carGenerationNodeAddr2] + LODWORD(v229));
                        v150   = CPathNode::GetNodeCoors(v149, &outVec);
                        v151   = CPathNode::GetNodeCoors(v148, &a2);
                        v152   = vectorSub(&out, v151, v150);
                        v153   = v152->x;
                        v154   = v152->y;
                        v155   = v152->z;
                        v.x    = v153;
                        v.y    = v154;
                        v.z    = v155;
                        v156   = 2.0 / CVector::Magnitude(&v);
                        v157   = VectorScale(&outVec, &v, v156);
                        v158   = vectorAdd(&a2, &carGenerationOrigin, v157);
                        v159   = v158->x;
                        y[0]   = v158->x;
                        y[1]   = v158->y;
                        y[2]   = v158->z;
                        v160   = CPathNode::GetNodeCoors(v148, &outVec);
                        pLevel = (1.0 - generateCarArg3) * v160->z;
                        v161   = CPathNode::GetNodeCoors(v149, &v249);
                        v162   = generateCarArg3 * v161->z;
                        generateCarArg3   = 1000000000.0;
                        y[2]   = v162 + pLevel;
                        if (v215) {
                            if (!CWaterLevel::GetWaterLevel(v159, y[1], y[2], &pLevel, 1, 0)) {
                                goto LABEL_195;
                            }
                            generateCarArg3 = pLevel;
                        } else {
                            if (CWorld::ProcessVerticalLine(y, 1000.0, &v253, &v250, 1, 0, 0, 0, 1, 0, 0)) {
                                generateCarArg3 = v254;
                            }
                            if (CWorld::ProcessVerticalLine(y, -1000.0, &v253, &v250, 1, 0, 0, 0, 1, 0, 0)) {
                                v163 = v254 - y[2];
                                if (v163 < 0.0) {
                                    v163 = -v163;
                                }
                                v164 = generateCarArg3 - y[2];
                                if (v164 < 0.0) {
                                    v164 = -v164;
                                }
                                if (v163 < v164) {
                                    generateCarArg3 = v254;
                                }
                            }
                        }
                        if (generateCarArg3 == 1000000000.0) {
                            goto LABEL_189;
                        }
                        v165 = generateCarArg3 - y[2];
                        if (v165 < 0.0) {
                            v165 = -v165;
                        }
                        if (v165 > 7.0) {
                            goto LABEL_189;
                        }
                        if (CModelInfo::IsBoatModel(*(generatedVehicle + 17))) {
                            y[2]         = generateCarArg3;
                            *(generatedVehicle + 595) = 255;
                        } else {
                            (*(*generatedVehicle + 212))(generatedVehicle);
                            y[2] = v165 + generateCarArg3;
                        }
                        CPlaceable::setPosition(generatedVehicle, y);
                        v.x     = 0.0;
                        v166    = generatedVehicle + 68;
                        *v166   = 0;
                        v.y     = 0.0;
                        v166[1] = 0;
                        v.z     = 0.0;
                        v166[2] = 0;
                        v167    = VectorDivide(&outVec, &in, 60.0);
                        v168    = vectorSub(&a2, v167, &what);
                        v169    = v168->x;
                        v170    = modelArg1;
                        v171    = v168->y;
                        what.x  = carGenerationOrigin - from.x;
                        v.x     = v169;
                        v172    = v168->z;
                        v.y     = v171;
                        what.y  = v239 - from.y;
                        v.z     = v172;
                        if (modelArg1 == 13) {
                            if (generatedVehicle[954] == 1) {
                                generatedVehicle[54] = generatedVehicle[54] & 7 | 0x10;
                            } else {
                                generatedVehicle[54] = generatedVehicle[54] & 7 | 0x18;
                            }
                        } else if (modelArg1 == 24) {
                            generatedVehicle[54] = generatedVehicle[54] & 7 | 0x18;
                        } else {
                            v173 = generatedVehicle[54];
                            if (v215) {
                                generatedVehicle[54] = v173 & 7 | 0x18;
                            } else if ((v173 & 0xF8) != 24) {
                                generatedVehicle[54] = v173 & 7 | 0x10;
                            }
                        }
                        CVisibilityPlugins::SetClumpAlpha(*(generatedVehicle + 6), 0);
                        if (CCheat::m_aCheatsActive.FunhouseTheme && !*(generatedVehicle + 356)) {
                            CVehicle::AddVehicleUpgrade(generatedVehicle, MI_HYDRAULICS);
                        }
                        v174 = CEntity::GetIsOnScreen(generatedVehicle);
                        v175 = *(generatedVehicle + 5);
                        v35  = v174 == 0;
                        v176 = v175 + 4;
                        if (!v35) {
                            if (!v175) {
                                v176 = (generatedVehicle + 4);
                            }
                            v177            = vectorSub(&outVec, &from, v176);
                            v178            = v177->y;
                            v179            = v177->x;
                            LODWORD(pLevel) = *(generatedVehicle + 595);
                            v208            = sqrt(v179 * v179 + v178 * v178);
                            v180            = SLODWORD(pLevel);
                            if (max_0(170.0, v180) * TheCamera.m_fGenerationDistMultiplier < v208
                                || TheCamera.m_fGenerationDistMultiplier * 150.0 > v208) {
                                goto LABEL_189;
                            }
                            v181 = v175 + 4;
                            if (!v175) {
                                v181 = (generatedVehicle + 4);
                            }
                            if (TheCamera.placeable.m_pMatrix) {
                                v182 = &TheCamera.placeable.m_pMatrix->mat.pos;
                            } else {
                                v182 = &TheCamera.placeable.m_SimpleCoors.m_vPosn;
                            }
                            v183 = vectorSub(&outVec, v182, v181);
                            if (TheCamera.m_fGenerationDistMultiplier * 120.0 > sqrt(v183->x * v183->x + v183->y * v183->y) || v223) {
                                goto LABEL_253;
                            }
                            if (*(generatedVehicle + 17) == 484) {
LABEL_189:
                                (**generatedVehicle)(generatedVehicle, 1);
                                return;
                            }
LABEL_196:
                            pLevel = CModelInfo::ms_modelInfoPtrs[*(generatedVehicle + 17)]->clump.base.m_pColModel->m_Sphere.sphere.m_fRadius;
                            v189   = v175 + 4;
                            if (!v175) {
                                v189 = (generatedVehicle + 4);
                            }
                            CWorld::FindObjectsKindaColliding(v189, pLevel, 1, &generateCarArg5, 2, 0, 0, 1, 1, 0, 0);
                            if (!LOWORD(generateCarArg5) && v.x * what.x + v.y * what.y < 0.0) {
                                CVehicleModelInfo::ChooseVehicleColour(
                                    CModelInfo::ms_modelInfoPtrs[*(generatedVehicle + 17)],
                                    generatedVehicle + 1'076,
                                    generatedVehicle + 1'077,
                                    generatedVehicle + 1'078,
                                    generatedVehicle + 1'079,
                                    1
                                );
                                CWorld::Add(generatedVehicle);
                                v190 = *(generatedVehicle + 17);
                                if (v190 == 531 || v190 == 532 || *(generatedVehicle + 357) == 10) {
                                    generatedVehicle[976] /= 3;
                                }
                                if (CGameLogic::LaRiotsActiveHere()) {
                                    LODWORD(pLevel) = rand() % 1'000;
                                    *(generatedVehicle + 304)    = SLODWORD(pLevel);
                                }
                                if (v170 == 13) {
                                    CCarCtrl::LastTimeLawEnforcerCreated = CTimer::m_snTimeInMilliseconds;
                                }
                                if (*(generatedVehicle + 17) == 457) {
                                    generatedVehicle[54]  = generatedVehicle[54] & 7 | 0x18;
                                    generatedVehicle[953] = 2;
                                }
                                if (!*(generatedVehicle + 356)) {
                                    switch (v170) {
                                    case 0:
                                    case 4:
                                    case 5:
                                    case 6:
                                    case 13:
                                        if (!CGeneral::GetRandomNumberInRange(0, 20)) {
                                            CAutomobile::SetRandomDamage(0, v205);
                                        }
                                        break;
                                    case 1:
                                    case 14:
                                    case 15:
                                    case 16:
                                    case 17:
                                    case 18:
                                    case 19:
                                    case 20:
                                    case 21:
                                    case 22:
                                    case 23:
                                        if (!CGeneral::GetRandomNumberInRange(0, 8)) {
                                            CAutomobile::SetRandomDamage(1, v205);
                                        }
                                        break;
                                    default:
                                        break;
                                    }
                                }
                                if (*(generatedVehicle + 357) == 9 && !generatedVehicle[953]) {
                                    generatedVehicle[54]  = generatedVehicle[54] & 7 | 0x18;
                                    generatedVehicle[953] = 6;
                                }
                                if (v215
                                    || v170 == 13
                                    || (v191 = FindPlayerPed(-1), CPlayerPed::GetWantedLevel(v191))
                                    || !CCheat::m_aCheatsActive.AggressiveDrivers && CCarCtrl::TimeNextMadDriverChaseCreated > 0.0
                                    || v222
                                    || !CCarCtrl::CreatePoliceChase(generatedVehicle, v170, LODWORD(carGenerationNodeAddr1))) {
                                    if (v225) {
                                        v192 = generatedVehicle->m_nModelIndex;
                                        if (v192 != eModelID::MODEL_FREEWAY && v192 != eModelID::MODEL_PCJ600 && v192 != eModelID::MODEL_FCR900 && v192 != eModelID::MODEL_NRG500 && v192 != eModelID::MODEL_BF400 && v192 != eModelID::MODEL_WAYFARER
                                            || gbLARiots
                                            || CGeneral::GetRandomNumberInRange(0, 7)
                                            || (CCarCtrl::CreateConvoy(generatedVehicle, v170), !v193)) {
                                            CCarCtrl::SetUpDriverAndPassengersForVehicle(generatedVehicle, v170, 1, 1, 0, 99);
                                            v194            = generatedVehicle[54];
                                            LODWORD(pLevel) = generatedVehicle[976];
                                            v195            = SLODWORD(pLevel);
                                            generatedVehicle[54]         = v194 & 7 | 0x18;
                                            generatedVehicle[953]        = 2;
                                            v196            = (v195 + 10.0);
                                            generatedVehicle[976]        = v196;
                                            LODWORD(pLevel) = v196;
                                            v197            = v196;
                                            v198            = CPlaceable__getTopVector(generatedVehicle, &outVec);
                                            v199            = VectorScale(&a2, v198, v197);
                                            v200            = VectorScale(&out, v199, 0.02);
                                            *(generatedVehicle + 17)     = LODWORD(v200->x);
                                            *(generatedVehicle + 18)     = LODWORD(v200->y);
                                            *(generatedVehicle + 19)     = LODWORD(v200->z);
                                            if (CGameLogic::LaRiotsActiveHere() || CCheat::m_aCheatsActive.AggressiveDrivers) {
                                                v201 = *(generatedVehicle + 280);
                                                if (v201) {
                                                    *(v201 + 1'144) |= 0x800u;
                                                }
                                            }
                                            generatedVehicle[1'070] |= 8u;
                                        } else {
                                            CCarCtrl::SetUpDriverAndPassengersForVehicle(generatedVehicle, v170, 1, 1, 0, 99);
                                        }
                                    } else if (v170 == 13 || v170 == 24) {
                                        CCarAI::AddPoliceCarOccupants(generatedVehicle, 0);
                                    } else {
                                        byte_9690CC = 1;
                                        CCarCtrl::SetUpDriverAndPassengersForVehicle(generatedVehicle, v170, 0, 0, 0, 99);
                                        byte_9690CC = 0;
                                    }
                                } else if (CGameLogic::LaRiotsActiveHere()) {
                                    CCarCtrl::TimeNextMadDriverChaseCreated = CGeneral::GetRandomNumberInRange(240.0, 480.0);
                                } else {
                                    CCarCtrl::TimeNextMadDriverChaseCreated = CGeneral::GetRandomNumberInRange(600.0, 1200.0);
                                }
                                if (v170 == 13 || v170 == 24) {
                                    CVehicle::ChangeLawEnforcerState(generatedVehicle, 1);
                                }
                                CStreaming::PossiblyStreamCarOutAfterCreation(*(generatedVehicle + 17));
                                sub_421120(CModelInfo::ms_modelInfoPtrs[*(generatedVehicle + 17)]);
                                return;
                            }
LABEL_253:
                            (**generatedVehicle)(generatedVehicle, 1);
                            return;
                        }
                        if (!v175) {
                            v176 = (generatedVehicle + 4);
                        }
                        v184            = vectorSub(&outVec, &from, v176);
                        v185            = v184->y;
                        v186            = v184->x;
                        LODWORD(pLevel) = *(generatedVehicle + 595);
                        v187            = sqrt(v186 * v186 + v185 * v185);
                        v188            = SLODWORD(pLevel);
                        if (max_0(v188, 170.0) * 0.0058823531 * 45.0 >= v187 || v223) {
                            goto LABEL_196;
                        }
LABEL_195:
                        (**generatedVehicle)(generatedVehicle, 1);
                        return;
                    }
                    generatedVehicle[955] = 0;
                    generatedVehicle[976] = CGeneral::GetRandomNumberInRange(14.0, 18.0);
                    generatedVehicle[953] = 2;
                    generatedVehicle[954] = CCarAI::FindPoliceBoatMissionForWantedLevel(v204);
                }
                generatedVehicle[1'068] |= 1u;
                goto LABEL_102;
            }
        }
    }
}

// 0x4341C0
void CCarCtrl::GenerateRandomCars() {
    if (CCutsceneMgr::ms_running) {
        CountDownToCarsAtStart = 2;
        return;
    }
    if (CGangWars::DontCreateCivilians() || !CGame::CanSeeOutSideFromCurrArea()) {
        return;
    }

    if (CGameLogic::LaRiotsActiveHere() && TimeNextMadDriverChaseCreated > 480.0f) {
        TimeNextMadDriverChaseCreated = CGeneral::GetRandomNumberInRange(240.0f, 480.0f);
    }
    TimeNextMadDriverChaseCreated -= (CTimer::GetTimeStep() * 0.02f);

    if (NumRandomCars < 45) {
        if (CountDownToCarsAtStart) {
            CountDownToCarsAtStart--;
            for (auto i = 100; i --> 0;) {
                GenerateOneRandomCar();
            }
            CTheCarGenerators::GenerateEvenIfPlayerIsCloseCounter = 20;
        } else {
            GenerateOneRandomCar();
            GenerateOneRandomCar();
        }
    }

}

// 0x42F3C0
void CCarCtrl::GetAIHeliToAttackPlayer(CAutomobile* automobile) {
    plugin::Call<0x42F3C0, CAutomobile*>(automobile);
}

// 0x42A730
void CCarCtrl::GetAIHeliToFlyInDirection(CAutomobile* automobile) {
    plugin::Call<0x42A730, CAutomobile*>(automobile);
}

// 0x429780
void CCarCtrl::GetAIPlaneToAttackPlayer(CAutomobile* automobile) {
    plugin::Call<0x429780, CAutomobile*>(automobile);
}

// 0x429890
void CCarCtrl::GetAIPlaneToDoDogFight(CAutomobile* automobile) {
    plugin::Call<0x429890, CAutomobile*>(automobile);
}

// 0x42F370
void CCarCtrl::GetAIPlaneToDoDogFightAgainstPlayer(CAutomobile* automobile) {
    plugin::Call<0x42F370, CAutomobile*>(automobile);
}

// 0x421440
CVehicle* CCarCtrl::GetNewVehicleDependingOnCarModel(int32 modelId, uint8 createdBy) {
    return plugin::CallAndReturn<CVehicle*, 0x421440, int32, uint8>(modelId, createdBy);
    /*
    switch (CModelInfo::GetModelInfo(modelId)->AsVehicleModelInfoPtr()->m_nVehicleType) {
    case VEHICLE_TYPE_MTRUCK:
        return new CMonsterTruck(modelId, createdBy);
    case VEHICLE_TYPE_QUAD:
        return new CQuadBike(modelId, createdBy);
    case VEHICLE_TYPE_HELI:
        return new CHeli(modelId, createdBy);
    case VEHICLE_TYPE_PLANE:
        return new CPlane(modelId, createdBy);
    case VEHICLE_TYPE_BOAT:
        return new CBoat(modelId, createdBy);
    case VEHICLE_TYPE_TRAIN:
        return new CTrain(modelId, createdBy);
    case VEHICLE_TYPE_BIKE:
        return new CBike(modelId, createdBy);
    case VEHICLE_TYPE_BMX:
        return new CBmx(modelId, createdBy);
    case VEHICLE_TYPE_TRAILER:
        return new CTrailer(modelId, createdBy);
    case VEHICLE_TYPE_AUTOMOBILE:
        return new CAutomobile(modelId, createdBy, 1);
    }
    return nullptr;
    */
}

// 0x42C250
bool CCarCtrl::IsAnyoneParking() {
    for (auto& veh : GetVehiclePool()->GetAllValid()) {
        switch (veh.m_autoPilot.m_nCarMission) {
        case eCarMission::MISSION_PARK_PARALLEL:
        case eCarMission::MISSION_PARK_PARALLEL_2:
        case eCarMission::MISSION_PARK_PERPENDICULAR:
        case eCarMission::MISSION_PARK_PERPENDICULAR_2:
            return true;
        }
    }
    return false;
}

// 0x42DAB0
bool CCarCtrl::IsThisAnAppropriateNode(CVehicle* vehicle, CNodeAddress nodeAddress1, CNodeAddress nodeAddress2, CNodeAddress nodeAddress3, bool arg5) {
    return plugin::CallAndReturn<bool, 0x42DAB0, CVehicle*, CNodeAddress, CNodeAddress, CNodeAddress, bool>(vehicle, nodeAddress1, nodeAddress2, nodeAddress3, arg5);
}

// 0x423EA0
bool CCarCtrl::IsThisVehicleInteresting(CVehicle* vehicle) {
    for (auto& car : apCarsToKeep) {
        if (car == vehicle) {
            return true;
        }
    }
    return false;
}

// 0x432CB0
void CCarCtrl::JoinCarWithRoadAccordingToMission(CVehicle* vehicle) {
    switch (vehicle->m_autoPilot.m_nCarMission) {
    case MISSION_NONE:
    case MISSION_CRUISE:
    case MISSION_WAITFORDELETION:
    case MISSION_EMERGENCYVEHICLE_STOP:
    case MISSION_STOP_FOREVER:
    case MISSION_FOLLOW_RECORDED_PATH:
    case MISSION_PARK_PERPENDICULAR:
    case MISSION_PARK_PARALLEL:
    case MISSION_PARK_PERPENDICULAR_2:
    case MISSION_PARK_PARALLEL_2:
        return JoinCarWithRoadSystem(vehicle);
    case MISSION_RAMPLAYER_FARAWAY:
    case MISSION_RAMPLAYER_CLOSE:
    case MISSION_BLOCKPLAYER_FARAWAY:
    case MISSION_BLOCKPLAYER_CLOSE:
    case MISSION_BLOCKPLAYER_HANDBRAKESTOP:
    case MISSION_BOAT_ATTACKPLAYER:
    case MISSION_SLOWLY_DRIVE_TOWARDS_PLAYER_1:
    case MISSION_SLOWLY_DRIVE_TOWARDS_PLAYER_2:
    case MISSION_BLOCKPLAYER_FORWARDANDBACK:
    case MISSION_APPROACHPLAYER_FARAWAY:
    case MISSION_APPROACHPLAYER_CLOSE:
    case MISSION_BOAT_CIRCLEPLAYER: {
        JoinCarWithRoadSystemGotoCoors(vehicle, FindPlayerCoors(-1), true, vehicle->IsSubBoat());
        break;
    }
    case MISSION_GOTOCOORDINATES:
    case MISSION_GOTOCOORDINATES_STRAIGHTLINE:
    case MISSION_GOTOCOORDINATES_ACCURATE:
    case MISSION_GOTOCOORDINATES_STRAIGHTLINE_ACCURATE:
    case MISSION_GOTOCOORDINATES_ASTHECROWSWIMS:
    case MISSION_GOTOCOORDINATES_RACING: {
        JoinCarWithRoadSystemGotoCoors(vehicle, vehicle->m_autoPilot.m_vecDestinationCoors, true, vehicle->IsSubBoat());
        break;
    }
    case MISSION_RAMCAR_FARAWAY:
    case MISSION_RAMCAR_CLOSE:
    case MISSION_BLOCKCAR_FARAWAY:
    case MISSION_BLOCKCAR_CLOSE:
    case MISSION_BLOCKCAR_HANDBRAKESTOP:
    case MISSION_PROTECTION_REAR:
    case MISSION_PROTECTION_FRONT:
    case MISSION_ESCORT_LEFT:
    case MISSION_ESCORT_RIGHT:
    case MISSION_ESCORT_REAR:
    case MISSION_ESCORT_FRONT:
    case MISSION_FOLLOWCAR_FARAWAY:
    case MISSION_FOLLOWCAR_CLOSE:
    case MISSION_KILLPED_FARAWAY:
    case MISSION_KILLPED_CLOSE:
    case MISSION_DO_DRIVEBY_CLOSE:
    case MISSION_DO_DRIVEBY_FARAWAY:
    case MISSION_ESCORT_LEFT_FARAWAY:
    case MISSION_ESCORT_RIGHT_FARAWAY:
    case MISSION_ESCORT_REAR_FARAWAY:
    case MISSION_ESCORT_FRONT_FARAWAY: {
        JoinCarWithRoadSystemGotoCoors(vehicle, vehicle->m_autoPilot.m_TargetEntity->GetPosition(), true, vehicle->IsSubBoat());
        break;
    }
    }
}

// 0x42F5A0
void CCarCtrl::JoinCarWithRoadSystem(CVehicle* vehicle) {
    plugin::Call<0x42F5A0, CVehicle*>(vehicle);
}

// 0x42F870
bool CCarCtrl::JoinCarWithRoadSystemGotoCoors(CVehicle* vehicle, const CVector& posn, bool unused, bool bIsBoat) {
    return plugin::CallAndReturn<bool, 0x42F870, CVehicle*, const CVector&, bool, bool>(vehicle, posn, unused, bIsBoat);
}

// 0x432B10
bool CCarCtrl::PickNextNodeAccordingStrategy(CVehicle* vehicle) {
    return plugin::CallAndReturn<bool, 0x432B10, CVehicle*>(vehicle);
}

// 0x421740
void CCarCtrl::InitSequence(int32 numSequenceElements) {
    SequenceElements = numSequenceElements;
    SequenceRandomOffset = CGeneral::GetRandomNumber() % numSequenceElements;
    bSequenceOtherWay = (CGeneral::GetRandomNumber() / 4) % 2;
}

// 0x42DE80
void CCarCtrl::PickNextNodeRandomly(CVehicle* vehicle) {
    plugin::Call<0x42DE80, CVehicle*>(vehicle);
}

// 0x426EF0
bool CCarCtrl::PickNextNodeToChaseCar(CVehicle* vehicle, float destX, float destY, float destZ) {
    return plugin::CallAndReturn<bool, 0x426EF0, CVehicle*, float, float, float>(vehicle, destX, destY, destZ);
}

// 0x427740
bool CCarCtrl::PickNextNodeToFollowPath(CVehicle* vehicle) {
    return plugin::CallAndReturn<bool, 0x427740, CVehicle*>(vehicle);
}

// 0x429600
void CCarCtrl::PossiblyFireHSMissile(CVehicle* entityLauncher, CEntity* targetEntity) {
    if (!targetEntity)
        return;

    if (CTimer::GetTimeInMS() / 2000u == CTimer::GetPreviousTimeInMS() / 2000u)
        return;

    const CVector launcherPos = entityLauncher->GetPosition();
    const CVector targetPos = targetEntity->GetPosition();
    CVector dir = targetPos - launcherPos;
    const float dist = dir.Magnitude();
    if (dist < 160.0f && dist > 30.0f) {
        CMatrix launcherMat = entityLauncher->GetMatrix();
        CVector dirNormalized = dir;
        dir.Normalise();
        if (DotProduct(launcherMat.GetForward(), dirNormalized) > 0.8f) {
            CProjectileInfo::AddProjectile(
                entityLauncher,
                eWeaponType::WEAPON_ROCKET_HS,
                launcherPos + launcherMat.GetForward() * 4.0f - launcherMat.GetUp() * 3.0f,
                1.0f,
                &entityLauncher->GetMatrix().GetForward(),
                targetEntity
            );
        }
    }
}

// 0x424F80
void CCarCtrl::PossiblyRemoveVehicle(CVehicle* vehicle) {
    plugin::Call<0x424F80, CVehicle*>(vehicle);
}

// 0x423F10
void CCarCtrl::PruneVehiclesOfInterest() {
    ZoneScoped;

    if ((CTimer::GetFrameCounter() % 64) == 19 && FindPlayerCoors(-1).z < 950.0f) {
        for (size_t i = 0; i < std::size(apCarsToKeep); i++) {
            if (apCarsToKeep[i]) {
                if (CTimer::GetTimeInMS() > aCarsToKeepTime[i] + 180000) {
                    apCarsToKeep[i] = nullptr;
                }
            }
        }
    }
}

// 0x42FC40
void CCarCtrl::ReconsiderRoute(CVehicle* vehicle) {
    plugin::Call<0x42FC40, CVehicle*>(vehicle);
}

// 0x423DE0
void CCarCtrl::RegisterVehicleOfInterest(CVehicle* vehicle) {
    plugin::Call<0x423DE0, CVehicle*>(vehicle);
}

// 0x4322B0
void CCarCtrl::RemoveCarsIfThePoolGetsFull() {
    ZoneScoped;

    if (CTimer::GetFrameCounter() % 8 != 3)
        return;

    if (GetVehiclePool()->GetNoOfFreeSpaces() >= 8)
        return;

    // Find closest deletable vehicle
    const CVector camPos = TheCamera.GetPosition();
    float fClosestDist = std::numeric_limits<float>::max();
    CVehicle* closestVeh = nullptr;
    for (auto& veh : GetVehiclePool()->GetAllValid()) {
        if (IsThisVehicleInteresting(&veh)) {
            continue;
        }
        if (veh.vehicleFlags.bIsLocked) {
            continue;
        }
        if (!veh.CanBeDeleted()) {
            continue;
        }
        if (CCranes::IsThisCarBeingTargettedByAnyCrane(&veh)) {
            continue;
        }

        const float fCamVehDist = (camPos - veh.GetPosition()).Magnitude();
        if (fClosestDist > fCamVehDist) {
            fClosestDist = fCamVehDist;
            closestVeh   = &veh;
        }
    }
    if (closestVeh) {
        CWorld::Remove(closestVeh);
        delete closestVeh;
    }
}

// 0x42CD10
void CCarCtrl::RemoveDistantCars() {
    ZoneScoped;

    // FIXBUGS: First remove vehicles that can be removed
    if (notsa::bugfixes::CCarCtrl_RemoveDistantCars_UseAfterFree) {
        for (auto& veh : GetVehiclePool()->GetAllValid()) {
            PossiblyRemoveVehicle(&veh);
        }
    }

    //... only then process them, this way we don't do use-after-free
    // only other solution would be `PossiblyRemoveVehicle` returning a `bool`
    // to indicate whenever the vehicle was deleted or not.
    for (auto& veh : GetVehiclePool()->GetAllValid()) {
        if (!notsa::bugfixes::CCarCtrl_RemoveDistantCars_UseAfterFree) {
            PossiblyRemoveVehicle(&veh); // This may or may not invalidate `veh`
        }
        if (!veh.vehicleFlags.bCreateRoadBlockPeds) {
            continue;
        }
        if (DistanceBetweenPoints(FindPlayerCentreOfWorld(), veh.GetPosition()) >= 54.5f) {
            continue;
        }
        CRoadBlocks::GenerateRoadBlockCopsForCar(
            &veh,
            veh.m_nPedsPositionForRoadBlock,
            veh.IsLawEnforcementVehicle() ? PED_TYPE_COP : PED_TYPE_GANG1
        );
        veh.vehicleFlags.bCreateRoadBlockPeds = false;
    }
}

// 0x423ED0
void CCarCtrl::RemoveFromInterestingVehicleList(CVehicle* vehicle) {
    for (auto& car : apCarsToKeep) {
        if (car == vehicle) {
            car = nullptr;
            break;
        }
    }
}

// 0x42CE40
void CCarCtrl::ScanForPedDanger(CVehicle* vehicle) {
    plugin::Call<0x42CE40, CVehicle*>(vehicle);
}

// 0x42FBC0
bool CCarCtrl::ScriptGenerateOneEmergencyServicesCar(uint32 modelId, CVector posn) {
    if (CStreaming::IsModelLoaded(modelId)) {
        if (auto pAuto = GenerateOneEmergencyServicesCar(modelId, posn)) {
            pAuto->m_autoPilot.m_vecDestinationCoors = posn;
            pAuto->m_autoPilot.SetCarMission(JoinCarWithRoadSystemGotoCoors(pAuto, posn, false, false) ? MISSION_GOTOCOORDINATES_STRAIGHTLINE : MISSION_GOTOCOORDINATES);
            return true;
        }
    }
    return false;
}

// 0x4342A0
void CCarCtrl::SetCoordsOfScriptCar(CVehicle* vehicle, float x, float y, float z, uint8 arg5, uint8 arg6) {
    plugin::Call<0x4342A0, CVehicle*, float, float, float, uint8, uint8>(vehicle, x, y, z, arg5, arg6);
}

// 0x4217C0
void CCarCtrl::SetUpDriverAndPassengersForVehicle(CVehicle* vehicle, int32 arg2, int32 arg3, bool arg4, bool arg5, int32 passengersNum) {
    plugin::Call<0x4217C0, CVehicle*, int32, int32, bool, bool, int32>(vehicle, arg2, arg3, arg4, arg5, passengersNum);
}

// 0x432420
template<typename PtrListType>
void CCarCtrl::SlowCarDownForCarsSectorList(PtrListType& ptrList, CVehicle* vehicle, float arg3, float arg4, float arg5, float arg6, float* arg7, float arg8) {
    plugin::Call<0x432420, PtrListType&, CVehicle*, float, float, float, float, float*, float>(ptrList, vehicle, arg3, arg4, arg5, arg6, arg7, arg8);
}

// 0x426220
void CCarCtrl::SlowCarDownForObject(CEntity* entity, CVehicle* vehicle, float* arg3, float arg4) {
    const CVector entityDir = entity->GetPosition() - vehicle->GetPosition();
    const float entityHeading = DotProduct(entityDir, vehicle->GetMatrix().GetForward());
    if (entityHeading > 0.0f && entityHeading < 20.0f) {
        if (entity->GetColModel()->GetBoundRadius() + vehicle->GetColModel()->GetBoundingBox().m_vecMax.x > fabs(DotProduct(entityDir, vehicle->GetMatrix().GetRight()))) {
            if (entityHeading >= 7.0f) {
                *arg3 = std::min(*arg3, (1.0f - (entityHeading - 7.0f) / 13.0f)) * arg4; // Original code multiplies by 0.07692308, which is the recp. of 13
            } else {
                *arg3 = 0.0f;
            }
        }
    }
}

// 0x42D4F0
template<typename PtrListType>
void CCarCtrl::SlowCarDownForObjectsSectorList(PtrListType& ptrList, CVehicle* vehicle, float arg3, float arg4, float arg5, float arg6, float* arg7, float arg8) {
    plugin::Call<0x42D4F0, PtrListType&, CVehicle*, float, float, float, float, float*, float>(ptrList, vehicle, arg3, arg4, arg5, arg6, arg7, arg8);
}

// 0x42D0E0
void CCarCtrl::SlowCarDownForOtherCar(CEntity* car1, CVehicle* car2, float* arg3, float arg4) {
    plugin::Call<0x42D0E0, CEntity*, CVehicle*, float*, float>(car1, car2, arg3, arg4);
}

// 0x425440
template<typename PtrListType>
void CCarCtrl::SlowCarDownForPedsSectorList(PtrListType& PtrList, CVehicle* vehicle, float arg3, float arg4, float arg5, float arg6, float* arg7, float arg8) {
    plugin::Call<0x425440, PtrListType&, CVehicle*, float, float, float, float, float*, float>(PtrList, vehicle, arg3, arg4, arg5, arg6, arg7, arg8);
}

// 0x434790
void CCarCtrl::SlowCarOnRailsDownForTrafficAndLights(CVehicle* vehicle) {
    auto& autoPilot = vehicle->m_autoPilot;

    if ((((int8)CTimer::GetFrameCounter() + (int8)(vehicle->m_nRandomSeed)) & 3) == 0) {
        if (CTrafficLights::ShouldCarStopForLight(vehicle, false) || CTrafficLights::ShouldCarStopForBridge(vehicle)) {
            CCarAI::CarHasReasonToStop(vehicle);
            autoPilot.m_fMaxTrafficSpeed = 0.0f;
        } else {
            autoPilot.m_fMaxTrafficSpeed = FindMaximumSpeedForThisCarInTraffic(vehicle);
        }
    }

    if (autoPilot.m_fMaxTrafficSpeed >= autoPilot.m_speed) {
        autoPilot.ModifySpeed(std::min(autoPilot.m_fMaxTrafficSpeed, CTimer::GetTimeStep() * 0.05f + autoPilot.m_speed));
    } else if (autoPilot.m_speed >= 0.1f) {
        autoPilot.ModifySpeed(std::max(autoPilot.m_fMaxTrafficSpeed, autoPilot.m_speed - CTimer::GetTimeStep() * 0.7f));
    } else if (autoPilot.m_speed != 0.0f) {
        autoPilot.ModifySpeed(0.0f);
    }
}

// 0x428DE0
void CCarCtrl::SteerAIBoatWithPhysicsAttackingPlayer(CVehicle* vehicle, float* arg2, float* arg3, float* arg4, bool* arg5) {
    plugin::Call<0x428DE0, CVehicle*, float*, float*, float*, bool*>(vehicle, arg2, arg3, arg4, arg5);
}

// 0x429090
void CCarCtrl::SteerAIBoatWithPhysicsCirclingPlayer(CVehicle* vehicle, float* arg2, float* arg3, float* arg4, bool* arg5) {
    plugin::Call<0x429090, CVehicle*, float*, float*, float*, bool*>(vehicle, arg2, arg3, arg4, arg5);
}

// 0x428BE0
void CCarCtrl::SteerAIBoatWithPhysicsHeadingForTarget(CVehicle* vehicle, float arg2, float arg3, float* arg4, float* arg5, float* arg6) {
    plugin::Call<0x428BE0, CVehicle*, float, float, float*, float*, float*>(vehicle, arg2, arg3, arg4, arg5, arg6);
}

// 0x422B20
void CCarCtrl::SteerAICarBlockingPlayerForwardAndBack(CVehicle* vehicle, float* arg2, float* arg3, float* arg4, bool* arg5) {
    plugin::Call<0x422B20, CVehicle*, float*, float*, float*, bool*>(vehicle, arg2, arg3, arg4, arg5);
}

// 0x433BA0
void CCarCtrl::SteerAICarParkParallel(CVehicle* vehicle, float* arg2, float* arg3, float* arg4, bool* arg5) {
    plugin::Call<0x433BA0, CVehicle*, float*, float*, float*, bool*>(vehicle, arg2, arg3, arg4, arg5);
}

// 0x433EA0
void CCarCtrl::SteerAICarParkPerpendicular(CVehicle* vehicle, float* arg2, float* arg3, float* arg4, bool* arg5) {
    plugin::Call<0x433EA0, CVehicle*, float*, float*, float*, bool*>(vehicle, arg2, arg3, arg4, arg5);
}

// 0x4336D0
void CCarCtrl::SteerAICarTowardsPointInEscort(CVehicle* vehicle1, CVehicle* vehicle2, float arg3, float arg4, float* arg5, float* arg6, float* arg7, bool* arg8) {
    plugin::Call<0x4336D0, CVehicle*, CVehicle*, float, float, float*, float*, float*, bool*>(vehicle1, vehicle2, arg3, arg4, arg5, arg6, arg7, arg8);
}

// 0x437C20
void CCarCtrl::SteerAICarWithPhysics(CVehicle* vehicle) {
    plugin::Call<0x437C20, CVehicle*>(vehicle);
}

// 0x434900
void CCarCtrl::SteerAICarWithPhysicsFollowPath(CVehicle* vehicle, float* arg2, float* arg3, float* arg4, bool* arg5) {
    plugin::Call<0x434900, CVehicle*, float*, float*, float*, bool*>(vehicle, arg2, arg3, arg4, arg5);
}

// 0x435830
void CCarCtrl::SteerAICarWithPhysicsFollowPath_Racing(CVehicle* vehicle, float* arg2, float* arg3, float* arg4, bool* arg5) {
    plugin::Call<0x435830, CVehicle*, float*, float*, float*, bool*>(vehicle, arg2, arg3, arg4, arg5);
}

// 0x432DD0
void CCarCtrl::SteerAICarWithPhysicsFollowPreRecordedPath(CVehicle* vehicle, float* arg2, float* arg3, float* arg4, bool* arg5) {
    plugin::Call<0x432DD0, CVehicle*, float*, float*, float*, bool*>(vehicle, arg2, arg3, arg4, arg5);
}

// 0x433280
void CCarCtrl::SteerAICarWithPhysicsHeadingForTarget(CVehicle* vehicle, CPhysical* target, float arg3, float arg4, float* arg5, float* arg6, float* arg7, bool* arg8) {
    plugin::Call<0x433280, CVehicle*, CPhysical*, float, float, float*, float*, float*, bool*>(vehicle, target, arg3, arg4, arg5, arg6, arg7, arg8);
}

// 0x4335E0
void CCarCtrl::SteerAICarWithPhysicsTryingToBlockTarget(CVehicle* vehicle, CEntity* Unusued, float arg3, float arg4, float arg5, float arg6, float* arg7, float* arg8, float* arg9, bool* arg10) {
    plugin::Call<0x4335E0, CVehicle*, CEntity*, float, float, float, float, float*, float*, float*, bool*>(vehicle, Unusued, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
}

// 0x428990
void CCarCtrl::SteerAICarWithPhysicsTryingToBlockTarget_Stop(CVehicle* vehicle, float x, float y, float arg4, float arg5, float* arg6, float* arg7, float* arg8, bool* arg9) {
    plugin::Call<0x428990, CVehicle*, float, float, float, float, float*, float*, float*, bool*>(vehicle, x, y, arg4, arg5, arg6, arg7, arg8, arg9);
}

// 0x436A90
void CCarCtrl::SteerAICarWithPhysics_OnlyMission(CVehicle* vehicle, float* arg2, float* arg3, float* arg4, bool* arg5) {
    plugin::Call<0x436A90, CVehicle*, float*, float*, float*, bool*>(vehicle, arg2, arg3, arg4, arg5);
}

// 0x42AAD0
void CCarCtrl::SteerAIHeliAsPoliceHeli(CAutomobile* automobile) {
    plugin::Call<0x42AAD0, CAutomobile*>(automobile);
}

// 0x42ACB0
void CCarCtrl::SteerAIHeliFlyingAwayFromPlayer(CAutomobile* automobile) {
    plugin::Call<0x42ACB0, CAutomobile*>(automobile);
}

// 0x4238E0
void CCarCtrl::SteerAIHeliToCrashAndBurn(CAutomobile* automobile) {
    plugin::Call<0x4238E0, CAutomobile*>(automobile);
}

// 0x42A750
void CCarCtrl::SteerAIHeliToFollowEntity(CAutomobile* automobile) {
    plugin::Call<0x42A750, CAutomobile*>(automobile);
}

// 0x42AEB0
void CCarCtrl::SteerAIHeliToKeepEntityInView(CAutomobile* automobile) {
    plugin::Call<0x42AEB0, CAutomobile*>(automobile);
}

// 0x42AD30
void CCarCtrl::SteerAIHeliToLand(CAutomobile* automobile) {
    plugin::Call<0x42AD30, CAutomobile*>(automobile);
}

// 0x42A630
void CCarCtrl::SteerAIHeliTowardsTargetCoors(CAutomobile* automobile) {
    plugin::Call<0x42A630, CAutomobile*>(automobile);
}

// 0x423880
void CCarCtrl::SteerAIPlaneToCrashAndBurn(CAutomobile* automobile) {
    plugin::Call<0x423880, CAutomobile*>(automobile);
}

// 0x4237F0
void CCarCtrl::SteerAIPlaneToFollowEntity(CAutomobile* automobile) {
    plugin::Call<0x4237F0, CAutomobile*>(automobile);
}

// 0x423790
void CCarCtrl::SteerAIPlaneTowardsTargetCoors(CAutomobile* automobile) {
    plugin::Call<0x423790, CAutomobile*>(automobile);
}

// 0x422590
bool CCarCtrl::StopCarIfNodesAreInvalid(CVehicle* vehicle) {
    return plugin::CallAndReturn<bool, 0x422590, CVehicle*>(vehicle);
}

// 0x4222A0
void CCarCtrl::SwitchBetweenPhysicsAndGhost(CVehicle* vehicle) {
    plugin::Call<0x4222A0, CVehicle*>(vehicle);
}

// 0x423FC0
void CCarCtrl::SwitchVehicleToRealPhysics(CVehicle* vehicle) {
    plugin::Call<0x423FC0, CVehicle*>(vehicle);
}

// 0x425B30
float CCarCtrl::TestCollisionBetween2MovingRects(CVehicle* vehicle1, CVehicle* vehicle2, float arg3, float arg4, CVector* pos1, CVector* pos2) {
    return plugin::CallAndReturn<float, 0x425B30, CVehicle*, CVehicle*, float, float, CVector*, CVector*>(vehicle1, vehicle2, arg3, arg4, pos1, pos2);
}

// 0x425F70
float CCarCtrl::TestCollisionBetween2MovingRects_OnlyFrontBumper(CVehicle* vehicle1, CVehicle* vehicle2, float arg3, float arg4, CVector* pos1, CVector* pos2) {
    return plugin::CallAndReturn<float, 0x425F70, CVehicle*, CVehicle*, float, float, CVector*, CVector*>(vehicle1, vehicle2, arg3, arg4, pos1, pos2);
}

// 0x429520
void CCarCtrl::TestWhetherToFirePlaneGuns(CVehicle* vehicle, CEntity* target) {
    plugin::Call<0x429520, CVehicle*, CEntity*>(vehicle, target);
}

// 0x421FE0
bool CCarCtrl::ThisVehicleShouldTryNotToTurn(CVehicle* vehicle) {
    return plugin::CallAndReturn<bool, 0x421FE0, CVehicle*>(vehicle);
}

// 0x429300
void CCarCtrl::TriggerDogFightMoves(CVehicle* vehicle1, CVehicle* vehicle2) {
    plugin::Call<0x429300, CVehicle*, CVehicle*>(vehicle1, vehicle2);
}

// 0x424000
void CCarCtrl::UpdateCarCount(CVehicle* vehicle, uint8 bDecrease) {
    plugin::Call<0x424000, CVehicle*, uint8>(vehicle, bDecrease);
}

// 0x436540
void CCarCtrl::UpdateCarOnRails(CVehicle* vehicle) {
    plugin::Call<0x436540, CVehicle*>(vehicle);
}

// 0x426BC0
void CCarCtrl::WeaveForObject(CEntity* entity, CVehicle* vehicle, float* arg3, float* arg4) {
    plugin::Call<0x426BC0, CEntity*, CVehicle*, float*, float*>(entity, vehicle, arg3, arg4);
}

// 0x426350
void CCarCtrl::WeaveForOtherCar(CEntity* entity, CVehicle* vehicle, float* arg3, float* arg4) {
    plugin::Call<0x426350, CEntity*, CVehicle*, float*, float*>(entity, vehicle, arg3, arg4);
}

// 0x42D680
template<typename PtrListType>
void CCarCtrl::WeaveThroughCarsSectorList(PtrListType& ptrList, CVehicle* vehicle, CPhysical* physical, float arg4, float arg5, float arg6, float arg7, float* arg8, float* arg9) {
    plugin::Call<0x42D680, PtrListType&, CVehicle*, CPhysical*, float, float, float, float, float*, float*>(ptrList, vehicle, physical, arg4, arg5, arg6, arg7, arg8, arg9);
}

// 0x42D950
template<typename PtrListType>
void CCarCtrl::WeaveThroughObjectsSectorList(PtrListType& ptrList, CVehicle* vehicle, float arg3, float arg4, float arg5, float arg6, float* arg7, float* arg8) {
    plugin::Call<0x42D950, PtrListType&, CVehicle*, float, float, float, float, float*, float*>(ptrList, vehicle, arg3, arg4, arg5, arg6, arg7, arg8);
}

// 0x42D7E0
template<typename PtrListType>
void CCarCtrl::WeaveThroughPedsSectorList(PtrListType& ptrList, CVehicle* vehicle, CPhysical* physical, float arg4, float arg5, float arg6, float arg7, float* arg8, float* arg9) {
    plugin::Call<0x42D7E0, PtrListType&, CVehicle*, CPhysical*, float, float, float, float, float*, float*>(ptrList, vehicle, physical, arg4, arg5, arg6, arg7, arg8, arg9);
}

// 0x427FE0
float CCarCtrl::FindMaxSteerAngle(CVehicle* veh) {
    return std::clamp(0.9f - veh->GetMoveSpeed().Magnitude(), 0.2f, 0.7f);
}
