#include "enemyWarn.hpp"

#include "../../../../../SDK/ICvar.hpp"
#include "../../../../../SDK/ConVar.hpp"
#include "../../../../../SDK/interfaces/interfaces.hpp"
#include "../../../../../SDK/structs/Entity.hpp"

#include "../../../../game.hpp"
#include "../../../../globals.hpp"
#include "../../../../../config/vars.hpp"
#include "../../../../../utilities/math/math.hpp"
#include "../../../../../utilities/renderer/renderer.hpp"


void EnemyWarning::init()
{
	m_scale = interfaces::cvar->findVar(XOR("weapon_recoil_scale"));
}

std::pair<bool, bool> EnemyWarning::check(Player_t* ent)
{
	if (!config.get<bool>(vars.bAimingWarn))
		return { false, false };

	if (!game::isAvailable())
		return { false, false };

	Vector posDelta = ent->getEyePos() - game::localPlayer->getEyePos();
	Vector idealAimAngle = math::vectorToAngle(posDelta);

	idealAimAngle -= ent->m_aimPunchAngle() * m_scale->getFloat();

	Vector curEnemyAngle = ent->m_angEyeAngles();
	curEnemyAngle.normalize();

	// check trace
	bool check = ent->isPossibleToSee(game::localPlayer->getEyePos());

	// dynamic fov
	float fov = math::calcFovReal(ent->getEyePos(), game::localPlayer->getBonePos(3), curEnemyAngle); // 3 is middle body

	// this can be made with tracing
	bool checkFov = fov <= 5.0f;

	return { check, checkFov };
}

void EnemyWarning::draw(const std::pair<bool, bool>& checks)
{
	if(checks.first) // trace
		imRender.text(globals::screenX / 2, 60, ImFonts::tahoma14, XOR("Enemy can see you"), true, Colors::Green);
	if(checks.second) // dynamic fov
		imRender.text(globals::screenX / 2, 80, ImFonts::tahoma14, XOR("Enemy is aiming you"), true, Colors::Red);
}