#pragma once

#include "../CUtlVector.hpp"
#include "../vars.hpp"
#include "../Enums.hpp"
#include "../math/Vector.hpp"
#include "../math/matrix.hpp"
#include "../math/AABB.hpp"
#include "../varMapping.hpp"
#include "../EHandle.hpp"
#include "indexes.hpp"

#include "../helpers/netvars.hpp"
#include "../helpers/vfunc.hpp"

class Weapon_t;
class Player_t;
class ClientClass;
class ICollideable;
class AnimationLayer;
struct Model_t;
class WeaponInfo;
struct DataMap_t;
class CUserCmd;
class CMoveData;
struct ClientHitVerify_t;

/*
* This file is so far probably the easiest when it comes to netvar usage
* You find the netvar - throw table and name, pretty easy
* in my source I accept only "raw" table names for netvars, it's more reliable for me
* Sometimes values need extra adress or are pointers
* Try to keep longer methods in cpp file
*/

////////////////////////////////////////////////////////////////////////////////////////////

#define RENDERABLE 0x4
#define NETWORKABLE 0x8

class Entity_t
{
public:
	NETVAR(int, m_iTeamNum, "DT_CSPlayer", "m_iTeamNum");
	NETVAR(Vec3, m_vecOrigin, "DT_BasePlayer", "m_vecOrigin");
	NETVAR(EHandle_t, m_hOwnerEntity, "DT_BaseEntity", "m_hOwnerEntity");
	NETVAR(EHandle_t, m_hVehicle, "DT_BaseEntity", "m_hVehicle");
	NETVAR(int, m_CollisionGroup, "DT_BasePlayer", "m_CollisionGroup");
	NETVAR(Vec3, m_ViewOffset, "DT_BasePlayer", "m_vecViewOffset[0]");
	NETVAR(bool, m_bSpotted, "DT_BaseEntity", "m_bSpotted");
	NETVAR(float, m_flSimulationTime, "DT_BasePlayer", "m_flSimulationTime");
	NETVAR(float, m_flMaxspeed, "DT_BasePlayer", "m_flMaxspeed");
	NETVAR_ADDR(int, m_MoveType, "DT_BasePlayer", "m_nRenderMode", 0x1);
	NETVAR(Vec3, m_vecMins, "DT_BaseEntity", "m_vecMins");
	NETVAR(Vec3, m_vecMaxs, "DT_BaseEntity", "m_vecMaxs");
	NETVAR(int, moveparent, "DT_BaseEntity", "moveparent");
	NETVAR(int, m_fEffects, "DT_BaseEntity", "m_fEffects");
	NETVAR(int, m_nModelIndex, "DT_BaseEntity", "m_nModelIndex");
	NETVAR(PrecipitationType_t, m_nPrecipType, "DT_Precipitation", "m_nPrecipType");
	PTRNETVAR(int, m_nNextThinkTick, "DT_BasePlayer", "m_nNextThinkTick");
	NETVAR_ADDR(int, m_nCreationTick, "DT_BaseEntity", "m_hEffectEntity", 0xC);

	VFUNC(Vec3&, absOrigin, ABS_ORIGIN, (), (this));
	VFUNC(Vec3&, absAngles, ABS_ANGLE, (), (this));
	VFUNC(ClientClass*, clientClass, CLIENT_CLASS, (), (this + NETWORKABLE));
	VFUNC(ICollideable*, collideable, COLLIDEABLE, (), (this));
	VFUNC(int, getIndex, GET_INDEX, (), (this + NETWORKABLE));
	VFUNC(void, getRenderBounds, RENDER_BOUNDS, (Vec3& mins, Vec3& maxs), (this + RENDERABLE, std::ref(mins), std::ref(maxs)));
	VFUNC(bool, isPlayer, ISPLAYER, (), (this));
	VFUNC(bool, isWeapon, ISWEAPON, (), (this));
	VFUNC(bool, setupBones, SETUP_BONES, (Matrix3x4* out, int maxBones, int mask, float time), (this + RENDERABLE, out, maxBones, mask, time));
	bool setupBonesShort(Matrix3x4* out, int maxBones, int mask, float time);
	VFUNC(Model_t*, getModel, GET_MODEL, (), (this + RENDERABLE));
	VFUNC(int, drawModel, DRAW_MODEL, (int flags, uint8_t alpha), (this + RENDERABLE, flags, alpha));
	VFUNC(bool, isDormant, IS_DORMANT, (), (this + NETWORKABLE));
	VFUNC(Matrix3x4&, renderableToWorldTransform, RENDERABLE_TO_WORLD, (), (this + RENDERABLE));
	VFUNC(void, release, RELEASE, (), (this + NETWORKABLE));
	VFUNC(void, onPreDataChanged, PRE_DATA_CHANGED, (DataUpdateType_t type), (this + NETWORKABLE, type));
	VFUNC(void, onDataChanged, DATA_CHANGED, (DataUpdateType_t type), (this + NETWORKABLE, type));
	VFUNC(void, preDataUpdate, PRE_DATA_UPDATE, (DataUpdateType_t type), (this + NETWORKABLE, type));
	VFUNC(void, postDataUpdate, POST_DATA_UPDATE, (DataUpdateType_t type), (this + NETWORKABLE, type));


	[[nodiscard]] CUtlVector<Matrix3x4> m_CachedBoneData();
	[[nodiscard]] Vec3 getAimPunch();
	[[nodiscard]] Vec3 getEyePos() { return m_vecOrigin() + m_ViewOffset(); }
	[[nodiscard]] AnimationLayer* getAnimOverlays();
	[[nodiscard]] size_t getSequenceActivity(size_t sequence);

	[[nodiscard]] bool isBreakable();
};

class Weapon_t : public Entity_t
{
public:
	NETVAR(float, m_flNextPrimaryAttack, "DT_BaseCombatWeapon", "m_flNextPrimaryAttack");
	NETVAR(float, m_flNextSecondaryAttack, "DT_BaseCombatWeapon", "m_flNextSecondaryAttack");
	NETVAR(short, m_iItemDefinitionIndex, "DT_BaseAttributableItem", "m_iItemDefinitionIndex");
	NETVAR(int, m_iClip1, "DT_BaseCombatWeapon", "m_iClip1");
	NETVAR(int, m_iClip2, "DT_BaseCombatWeapon", "m_iClip2");
	NETVAR(int, m_iPrimaryReserveAmmoCount, "DT_BaseCombatWeapon", "m_iPrimaryReserveAmmoCount");
	NETVAR(int, m_iEntityQuality, "DT_BaseCombatWeapon", "m_iEntityQuality");
	NETVAR(bool, m_bBurstMode, "DT_BaseCombatWeapon", "m_bBurstMode");
	NETVAR(int, m_iBurstShotsRemaining, "DT_BaseCombatWeapon", "m_iBurstShotsRemaining");
	NETVAR(int, m_zoomLevel, "DT_WeaponCSBaseGun", "m_zoomLevel");
	NETVAR(bool, m_bPinPulled, "DT_BaseCSGrenade", "m_bPinPulled");
	NETVAR(float, m_fThrowTime, "DT_BaseCSGrenade", "m_fThrowTime");
	NETVAR(float, m_flThrowStrength, "DT_BaseCSGrenade", "m_flThrowStrength");
	NETVAR(int, m_nExplodeEffectTickBegin, "DT_BaseCSGrenadeProjectile", "m_nExplodeEffectTickBegin");
	NETVAR(float, m_fAccuracyPenalty, "DT_WeaponCSBase", "m_fAccuracyPenalty");
	NETVAR(float, m_flRecoilIndex, "DT_WeaponCSBaseGun", "m_flRecoilIndex");

	VFUNC(float, getInaccuracy, INACCURACY, (), (this));
	VFUNC(float, getSpread, SPREAD, (), (this));
	VFUNC(WeaponInfo*, getWpnInfo, WEAPONINFO, (), (this));
	VFUNC(void, updateAccuracyPenalty, UPDATE_WEAPON_PENALTY, (), (this));

	[[nodiscard]] std::string getWpnName();
	[[nodiscard]] std::u8string getIcon(int correctIndex = -1);

	[[nodiscard]] bool isEmpty() { return m_iClip1() <= 0; }
	[[nodiscard]] bool isRifle();
	[[nodiscard]] bool isSmg();
	[[nodiscard]] bool isMachineGun();
	[[nodiscard]] bool isShotgun();
	[[nodiscard]] bool isPistol();
	[[nodiscard]] bool isSniper();
	[[nodiscard]] bool isGrenade();
	[[nodiscard]] bool isKnife();
	[[nodiscard]] bool isNonAimable();

	[[nodiscard]] size_t getNadeRadius();
};

////////////////////////////////////////////////////////////////////////////////////////////

class Inferno_t : public Entity_t
{
public:
	[[nodiscard]] static float expireTime() { return 7.0f; }
	OFFSET(float, spawnTime, 0x20);
	NETVAR(int, m_fireCount, "DT_Inferno", "m_fireCount");
	PTRNETVAR(int, m_fireXDelta, "DT_Inferno", "m_fireXDelta");
	PTRNETVAR(int, m_fireYDelta, "DT_Inferno", "m_fireYDelta");
	PTRNETVAR(int, m_fireZDelta, "DT_Inferno", "m_fireZDelta");
	[[nodiscard]] Vec3 getInfernoPos(size_t indexFire);
};

////////////////////////////////////////////////////////////////////////////////////////////

class Nade_t : public Entity_t
{
public:
	NETVAR_ADDR(float, m_flSpawnTime, "DT_BaseCSGrenadeProjectile", "m_vecExplodeEffectOrigin", 0xC);
	NETVAR(EHandle_t, m_hThrower, "DT_BaseCSGrenadeProjectile", "m_hThrower");
	NETVAR(int, m_nExplodeEffectTickBegin, "DT_BaseCSGrenadeProjectile", "m_nExplodeEffectTickBegin");
	NETVAR(Vec3, m_vecVelocity, "DT_BaseCSGrenadeProjectile", "m_vecVelocity");
	NETVAR(Vec3, m_vecOrigin, "DT_BaseCSGrenadeProjectile", "m_vecOrigin");
};

////////////////////////////////////////////////////////////////////////////////////////////

class Smoke_t : public Entity_t
{
public:
	[[nodiscard]] static float expireTime() { return 19.0f; }
	NETVAR(int, m_nSmokeEffectTickBegin, "DT_SmokeGrenadeProjectile", "m_nSmokeEffectTickBegin");
};

////////////////////////////////////////////////////////////////////////////////////////////

class Bomb_t : public Entity_t
{
public:
	NETVAR(float, m_flDefuseCountDown, "DT_PlantedC4", "m_flDefuseCountDown");
	NETVAR(EHandle_t, m_hBombDefuser, "DT_PlantedC4", "m_hBombDefuser");
	NETVAR(float, m_flC4Blow, "DT_PlantedC4", "m_flC4Blow");
	NETVAR(bool, m_bBombDefused, "DT_PlantedC4", "m_bBombDefused");
	NETVAR(bool, m_nBombSite, "DT_PlantedC4", "m_nBombSite");
	[[nodiscard]] char getBombSiteName() { return m_nBombSite() == 0 ? 'A' : 'B'; }
};

////////////////////////////////////////////////////////////////////////////////////////////

class Player_t : public Entity_t
{
public:
	NETVAR(Vec3, m_angEyeAngles, "DT_CSPlayer", "m_angEyeAngles");
	NETVAR(Vec3, m_viewPunchAngle, "DT_BasePlayer", "m_viewPunchAngle");
	NETVAR(Vec3, m_aimPunchAngle, "DT_BasePlayer", "m_aimPunchAngle");
	NETVAR(Vec3, m_vecVelocity, "DT_BasePlayer", "m_vecVelocity[0]");
	NETVAR(int, m_iHealth, "DT_CSPlayer", "m_iHealth");
	NETVAR(int, m_ArmorValue, "DT_CSPlayer", "m_ArmorValue");
	NETVAR(bool, m_bIsScoped, "DT_CSPlayer", "m_bIsScoped");
	NETVAR(bool, m_bHasHelmet, "DT_CSPlayer", "m_bHasHelmet");
	NETVAR(int, m_iShotsFired, "DT_CSPlayer", "m_iShotsFired");
	NETVAR(bool, m_bGunGameImmunity, "DT_CSPlayer", "m_bGunGameImmunity");
	NETVAR(float, m_flNextAttack, "DT_CSPlayer", "m_flNextAttack");
	NETVAR(bool, m_bHasDefuser, "DT_CSPlayer", "m_bHasDefuser");
	NETVAR(EHandle_t, m_hViewModel, "DT_BasePlayer", "m_hViewModel[0]");
	NETVAR(float, m_flLowerBodyYawTarget, "DT_CSPlayer", "m_flLowerBodyYawTarget");
	NETVAR(float, m_flFlashDuration, "DT_CSPlayer", "m_flFlashDuration");
	NETVAR_ADDR(float, m_flFlashBangTime, "DT_CSPlayer", "m_flFlashDuration", -0x10);
	NETVAR_ADDR(float, m_flNightVisionAlpha, "DT_CSPlayer", "m_flFlashDuration", -0x1C);
	NETVAR(int, m_lifeState, "DT_CSPlayer", "m_lifeState");
	NETVAR(int, m_fFlags, "DT_CSPlayer", "m_fFlags");
	NETVAR(int, m_nHitboxSet, "DT_CSPlayer", "m_nHitboxSet");
	NETVAR(int, m_nTickBase, "DT_CSPlayer", "m_nTickBase");
	NETVAR(float, m_flDuckSpeed, "DT_CSPlayer", "m_flDuckSpeed");
	NETVAR(float, m_flDuckAmount, "DT_CSPlayer", "m_flDuckAmount");
	NETVAR(bool, m_bDucked, "DT_CSPlayer", "m_bDucked");
	NETVAR(bool, m_bDucking, "DT_CSPlayer", "m_bDucking");
	NETVAR(float, m_flHealthShotBoostExpirationTime, "DT_CSPlayer", "m_flHealthShotBoostExpirationTime");
	NETVAR(EHandle_t, m_hObserverTarget, "DT_BasePlayer", "m_hObserverTarget");
	NETVAR(EHandle_t, m_hActiveWeapon, "DT_CSPlayer", "m_hActiveWeapon");
	NETVAR(int, m_iAccount, "DT_CSPlayer", "m_iAccount");
	PTRNETVAR(const char, m_szLastPlaceName, "DT_BasePlayer", "m_szLastPlaceName");
	NETVAR(float, m_flFallVelocity, "DT_BasePlayer", "m_flFallVelocity");
	NETVAR(int, m_nSequence, "DT_BaseAnimating", "m_nSequence");
	NETVAR(float, m_flVelocityModifier, "DT_CSPlayer", "m_flVelocityModifier");	
	NETVAR(int, m_iNumRoundKillsHeadshots, "DT_CSPlayer", "m_iNumRoundKillsHeadshots");
	NETVAR(int, m_totalHitsOnServer, "DT_CSPlayer", "m_totalHitsOnServer");
	PTROFFSET(VarMapping_t, getVarMap, 0x24);
	DATAMAP_FIELD(int, m_afButtonLast, getPredictionDataMap(), "m_afButtonLast");
	DATAMAP_FIELD(int, m_afButtonPressed, getPredictionDataMap(), "m_afButtonPressed");
	DATAMAP_FIELD(int, m_afButtonReleased, getPredictionDataMap(), "m_afButtonReleased");
	NETVAR_ADDR(int, m_afButtonForced, "DT_BasePlayer", "m_iDefaultFOV", 0x8);
	NETVAR_ADDR(int, m_afButtonDisabled, "DT_BasePlayer", "m_iDefaultFOV", 0x4);
	PTRNETVAR_ADDR(CUserCmd*, m_pCurrentCommand, "DT_BasePlayer", "m_iDefaultFOV", 0xC);
	NETVAR_ADDR(float, m_flSpawnTime, "DT_CSPlayer", "m_iAddonBits", -0x4);
	[[nodiscard]] CUserCmd& m_LastCmd();
	PTRDATAMAP_FIELD(int, m_nButtons, getPredictionDataMap(), "m_nButtons");
	PTRDATAMAP_FIELD(int, m_nImpulse, getPredictionDataMap(), "m_nImpulse");
	DATAMAP_FIELD(Vec3, m_vecAbsVelocity, getPredictionDataMap(), "m_vecAbsVelocity");
	DATAMAP_FIELD(Vec3, m_vecNetworkOrigin, getPredictionDataMap(), "m_vecNetworkOrigin");
	DATAMAP_FIELD(int, m_iEFlags, getPredictionDataMap(), "m_iEFlags");
	VFUNC(DataMap_t*, getPredictionDataMap, DATAMAP_PREDICTION, (), (this));
	VFUNC(void, preThink, PRE_THINK, (), (this));
	VFUNC(void, think, THINK, (), (this));
	void runThink();
	void postThink();
	void checkHasThinkFunction(bool isThinkingHint = false);
	VFUNC(void, selectItem, SELECTITEM, (const char* string, int subType = 0), (this, string, subType));
	bool usingStandardWeaponsInVehicle();
	VFUNC(void, updateCollisionBounds, UPDATE_COLLISION_BOUNDS, (), (this));
	VFUNC(void, setSequence, SET_SEQUENCE, (int seq), (this, seq));
	VFUNC(void, studioFrameAdvance, STUDIO_FRAME_ADVANCE, (), (this));
	// VEHICLES ONLY
	VFUNC(void, processMovement, PROCESS_MOVEMENT, (Entity_t* veh, Player_t* player, CMoveData* data), (this, veh, player, data));
	[[nodiscard]] bool physicsRunThink(thinkmethods_t think);
	[[nodiscard]] CUtlVector<ClientHitVerify_t> m_vecBulletVerifyListClient();

	void setAbsOrigin(const Vec3& origin);
	[[nodiscard]] Weapon_t* getActiveWeapon();
	[[nodiscard]] bool isAlive() { return m_iHealth() > 0; }
	[[nodiscard]] bool isInAir() { return !(m_fFlags() & FL_ONGROUND); }
	[[nodiscard]] bool isMoving() { return m_vecVelocity().toVecPrev().length() > 0.1f; }

	[[nodiscard]] Vec3 getHitboxPos(const int id);
	[[nodiscard]] Vec3 getBonePos(const int id);
	[[nodiscard]] Vec3 getHitgroupPos(const int hitgroup);
	[[nodiscard]] bool isC4Owner();
	[[nodiscard]] std::string getName();
	[[nodiscard]] std::string_view getRawName();
	[[nodiscard]] int getKills();
	[[nodiscard]] int getDeaths();
	[[nodiscard]] int getPing();
	[[nodiscard]] std::string getRank(bool useShortName = false);
	[[nodiscard]] int getWins();
	[[nodiscard]] bool isPossibleToSee(Player_t* player, const Vec3& pos);
	[[nodiscard]] bool isViewInSmoke(const Vec3& pos);
	[[nodiscard]] bool isOtherTeam(Player_t* player);
	// address as number
	[[nodiscard]] uintptr_t getLiteralAddress();

	// https://github.com/perilouswithadollarsign/cstrike15_src/blob/f82112a2388b841d72cb62ca48ab1846dfcc11c8/game/shared/collisionproperty.cpp#L845
	[[nodiscard]] AABB_t getOcclusionBounds();
	// https://github.com/perilouswithadollarsign/cstrike15_src/blob/f82112a2388b841d72cb62ca48ab1846dfcc11c8/game/server/gameinterface.cpp#L2772
	[[nodiscard]] AABB_t getCameraBounds();
};

////////////////////////////////////////////////////////////////////////////////////////////

class FogController_t : public Entity_t
{
public:
	NETVAR(bool, m_fogenable, "DT_FogController", "m_fog.enable");
	NETVAR(float, m_fogstart, "DT_FogController", "m_fog.start");
	NETVAR(float, m_fogend, "DT_FogController", "m_fog.end");
	NETVAR(float, m_fogmaxdensity, "DT_FogController", "m_fog.maxdensity");
	NETVAR(int, m_fogcolorPrimary, "DT_FogController", "m_fog.colorPrimary");
	NETVAR(int, m_fogcolorSecondary, "DT_FogController", "m_fog.colorSecondary");
};

////////////////////////////////////////////////////////////////////////////////////////////

class EnvTonemapController_t : public Entity_t
{
public:
	NETVAR(bool, m_bUseCustomAutoExposureMin, "DT_EnvTonemapController", "m_bUseCustomAutoExposureMin");
	NETVAR(bool, m_bUseCustomAutoExposureMax, "DT_EnvTonemapController", "m_bUseCustomAutoExposureMax");
	NETVAR(float, m_flCustomAutoExposureMin, "DT_EnvTonemapController", "m_flCustomAutoExposureMin");
	NETVAR(float, m_flCustomAutoExposureMax, "DT_EnvTonemapController", "m_flCustomAutoExposureMax");
	NETVAR(bool, m_bUseCustomBloomScale, "DT_EnvTonemapController", "m_bUseCustomBloomScale");
	NETVAR(float, m_flCustomBloomScale, "DT_EnvTonemapController", "m_flCustomBloomScale");
};

class EnvAmbientLight_t : public Entity_t
{
public:
	NETVAR(Vec3, m_vecColor, "DT_EnvAmbientLight", "m_vecColor");
};

#undef RENDERABLE
#undef NETWORKABLE

////////////////////////////////////////////////////////////////////////////////////////////