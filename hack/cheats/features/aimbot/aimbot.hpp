#pragma once

#include <cheats/classes/createMove.hpp>
#include <cheats/classes/renderableToPresent.hpp>
#include <SDK/math/Vector.hpp>
#include <config/cfgWeapon.hpp>

#include <vector>

class CUserCmd;
class Player_t;
class Weapon_t;
class IConVar;

struct AimbotTarget_t
{
	Player_t* m_player;
	float m_fov;
	size_t m_index;
	size_t m_bestHitboxID;
	Vec3 m_pos;
};

class AimDraw;

class Aimbot : protected CreateMoveInPredictionType
{
public:
	constexpr Aimbot() :
		CreateMoveInPredictionType{}
	{}

	virtual void init() override;
	virtual void run(CUserCmd* cmd) override;

	Player_t* getTargetted() const;
	Vec3 getCachedView() const;
	Vec3 getBestHibox() const;
	CfgWeapon getCachedConfig() const;
private:
	void resetFields();
	[[nodiscard]] Vec3 smoothAim(CUserCmd* cmd, const Vec3& angle, Player_t* target, float cfgSmooth);
	[[nodiscard]] bool isClicked(CUserCmd* cmd);
	[[nodiscard]] bool getBestTarget(CUserCmd* cmd, Weapon_t* wpn, const Vec3& eye, const Vec3& punch);
	[[nodiscard]] float getRandomizedSmooth(float currentSmooth);

	[[nodiscard]] std::vector<size_t> getHitboxes();

	Player_t* m_bestEnt;
	Vec3 m_bestHitpos;
	bool m_delay;
	float m_delayTime;
	Vec3 m_view;
	int m_bestId;
	CfgWeapon m_config;
	int m_prevMouseDeltaX;
	int m_prevMouseDeltaY;
	// saving here so later can represent plot of smooth
	float smoothFactor;
	IConVar* m_scale = nullptr;

	std::vector<AimbotTarget_t> m_targets;
};

GLOBAL_FEATURE(Aimbot);
