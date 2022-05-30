#include "player.hpp"

#include "../../../SDK/CGlobalVars.hpp"
#include "../../../SDK/IClientEntityList.hpp"
#include "../../../SDK/IVModelInfo.hpp"
#include "../../../SDK/IVEngineClient.hpp"
#include "../../../SDK/IVEffects.hpp"
#include "../../../SDK/IGameEvent.hpp"
#include "../../../SDK/IViewRenderBeams.hpp"
#include "../../../SDK/animations.hpp"
#include "../../../SDK/ICvar.hpp"
#include "../../../SDK/ConVar.hpp"
#include "../../../SDK/IWeapon.hpp"
#include "../../../SDK/structs/Entity.hpp"
#include "../../../SDK/Color.hpp"
#include "../../../SDK/interfaces/interfaces.hpp"

#include "../../../config/vars.hpp"
#include "../../game.hpp"
#include "../../../utilities/renderer/renderer.hpp"
#include "../../../utilities/math/math.hpp"
#include "../../globals.hpp"

#include "../../features/aimbot/aimbot.hpp"
#include "../../features/backtrack/backtrack.hpp"

void Visuals::run()
{
	if (!config.get<bool>(vars.bEsp))
		return;

	if (!game::isAvailable())
		return;

	bool drawDead = config.get<bool>(vars.bDrawDeadOnly);

	for (int i = 1; i <= interfaces::globalVars->m_maxClients; i++)
	{
		auto entity = reinterpret_cast<Player_t*>(interfaces::entList->getClientEntity(i));

		if (!entity)
			continue;

		if (game::localPlayer == entity)
			continue;

		if (entity->isDormant())
			continue;

		if (!entity->isAlive())
			continue;

		if (entity->m_iTeamNum() == game::localPlayer->m_iTeamNum())
			continue;

		auto runFeatures = [=]()
		{
			drawPlayer(entity);
			drawSkeleton(entity);
			runDLight(entity);
			drawLaser(entity);
		};

		if (drawDead)
		{
			if (!game::localPlayer->isAlive())
				runFeatures();
		}
		else
			runFeatures();

		enemyIsAimingAtYou(entity);
	}
}

void Visuals::drawHealth(Player_t* ent, const Box& box)
{
	if (!config.get<bool>(vars.bDrawHealth))
		return;

	auto& health = m_health.at(ent->getIndex());

	if (auto realHealth = ent->m_iHealth(); health > realHealth)
		health -= 2.0f * interfaces::globalVars->m_frametime;
	else
		health = realHealth;

	auto offset = health * box.h / 100.0f;
	auto pad = box.h - offset;

	Box newBox =
	{
		box.x - 5.0f,
		box.y,
		2.0f,
		box.h,
	};

	// fill first
	imRender.drawRoundedRectFilled(newBox.x - 1.0f, newBox.y - 1.0f, 4.0f, newBox.h + 2.0f, 120.0f, Colors::Black);
	imRender.drawRoundedRectFilled(newBox.x, newBox.y + pad, 2.0f, offset, 120.0f, healthBased(ent));

	// if the player has health below max, then draw HP info
	/*if(health < 100)
		imRender.text(newBox.x - 2.0f, newBox.y + pad - 4.0f,
			ImFonts::franklinGothic, std::format(XOR("{}"), health), false, Colors::White);*/
}

void Visuals::drawArmor(Player_t* ent, const Box& box)
{
	if (!config.get<bool>(vars.bDrawArmor))
		return;

	auto& armor = m_armor.at(ent->getIndex());

	if (auto realArmor = ent->m_ArmorValue(); armor > realArmor)
		armor -= 2.0f * interfaces::globalVars->m_frametime;
	else
		armor = realArmor;

	auto offset = armor * box.h / 100.0f;
	auto pad = box.h - offset;

	Box newBox =
	{
		box.x + box.w + 3.0f,
		box.y,
		2.0f,
		box.h,
	};

	Color armorCol = Color(0, static_cast<int>(armor * 1.4f), 255); // light to blue, something simple

	if (armor != 0)
	{
		imRender.drawRoundedRectFilled(newBox.x - 1.0f, newBox.y - 1.0f, 4.0f, newBox.h + 2.0f, 120.0f, Colors::Black);
		imRender.drawRoundedRectFilled(newBox.x, newBox.y + pad, 2.0f, offset, 120.0f, armorCol);
	}

	/*if (armor < 100 && armor != 0)
		imRender.text(newBox.x - 2.0f, newBox.y + pad - 4.0f,
			ImFonts::franklinGothic, std::format(XOR("{}"), armor), false, Colors::White);*/
}

void Visuals::drawWeapon(Player_t* ent, const Box& box)
{
	if (!config.get<bool>(vars.bDrawWeapon))
		return;

	auto weapon = ent->getActiveWeapon();
	if (!weapon)
		return;

	Color tex = config.get<Color>(vars.cWeaponText);

	int maxAmmo = weapon->getWpnInfo()->m_maxClip1;
	int currentAmmo = weapon->m_iClip1();

	imRender.text(box.x + box.w / 2, box.y + box.h + 5, ImFonts::franklinGothic12, FORMAT(XOR("{} {}/{}"),
		ent->getActiveWeapon()->getWpnName(), currentAmmo, maxAmmo), true, tex);

	// skip useless trash for calculations
	if (weapon->isNonAimable())
		return;

	Box newBox =
	{
		box.x,
		box.y + box.h + 3.0f,
		box.w + 2.0f,
		2.0f
	};

	int barWidth = currentAmmo * box.w / maxAmmo;
	bool isReloading = false;
	auto animlayer = ent->getAnimOverlays()[1];

	if (animlayer.m_sequence)
	{
		auto sequenceActivity = ent->getSequenceActivity(animlayer.m_sequence);
		isReloading = sequenceActivity == 967 && animlayer.m_weight; // ACT_CSGO_RELOAD

		if (isReloading && animlayer.m_weight != 0.0f && animlayer.m_cycle < 0.99f)
			barWidth = (animlayer.m_cycle * box.w) / 1.0f;
	}

	imRender.drawRectFilled(newBox.x - 1.0f, newBox.y - 1.0f, newBox.w, 4.0f, Colors::Black);
	imRender.drawRectFilled(newBox.x, newBox.y, barWidth, 2.0f, config.get<Color>(vars.cReloadbar));
}

void Visuals::drawInfo(Player_t* ent, const Box& box)
{
	std::vector<std::pair<std::string, Color>> flags = {};
	using cont = std::vector<bool>; // container

	PlayerInfo_t info = {};
	if (!interfaces::engine->getPlayerInfo(ent->getIndex(), &info))
		return;

	if (config.get<cont>(vars.vFlags).at(E2T(EspFlags::BOT)))
		if(info.m_fakePlayer)
			flags.emplace_back(std::make_pair(XOR("BOT"), Colors::Yellow));

	if (config.get<cont>(vars.vFlags).at(E2T(EspFlags::MONEY)))
		flags.emplace_back(std::make_pair(FORMAT(XOR("{}$"), ent->m_iAccount()), Colors::Green));

	if (config.get<cont>(vars.vFlags).at(E2T(EspFlags::WINS)))
		flags.emplace_back(std::make_pair(FORMAT(XOR("Wins {}"), ent->getWins()), Colors::Green));

	if (config.get<cont>(vars.vFlags).at(E2T(EspFlags::RANK)))
		flags.emplace_back(std::make_pair(FORMAT(XOR("Rank {}"), ent->getRank()), Colors::White));

	if (config.get<cont>(vars.vFlags).at(E2T(EspFlags::ARMOR)))
	{
		std::string text = "";
		if (ent->m_bHasHelmet() && ent->m_ArmorValue())
			text = XOR("H KEV");
		else if (!ent->m_bHasHelmet() && ent->m_ArmorValue())
			text = XOR("KEV");

		flags.emplace_back(std::make_pair(text, Colors::White));
	}

	if (config.get<cont>(vars.vFlags).at(E2T(EspFlags::ZOOM)))
		if(ent->m_bIsScoped())
			flags.emplace_back(std::make_pair(XOR("ZOOM"), Colors::White));

	if (config.get<cont>(vars.vFlags).at(E2T(EspFlags::C4)))
		if (ent->isC4Owner())
			flags.emplace_back(std::make_pair(XOR("C4"), Colors::Orange));

	float fontSize = getScaledFontSize(ent, 60.0f, 11.0f, 16.0f);

	float padding = 0.0f;
	float addon = config.get<bool>(vars.bDrawArmor) ? 6.0f : 0.0f;

	for (size_t i = 0; const auto & [name, color] : flags)
	{
		imRender.text(box.x + box.w + addon + 2.0f, box.y + padding, fontSize, ImFonts::verdana, name, false, color, false);
		auto textSize = ImFonts::verdana->CalcTextSizeA(fontSize, std::numeric_limits<float>::max(), 0.0f, name.c_str());
		padding += textSize.y;
		i++;

		if (i != flags.size() && padding + fontSize > box.h) // when too many flags for long distances
		{
			imRender.text(box.x + box.w + addon + 2.0f, box.y + padding, fontSize, ImFonts::verdana,
				FORMAT(XOR("{} more..."), flags.size() - i), false, Colors::White, false);
			break;
		}
	}
}

void Visuals::drawnName(Player_t* ent, const Box& box)
{
	if (!config.get<bool>(vars.bDrawName))
		return;

	float fontSize = getScaledFontSize(ent);

	imRender.text(box.x + box.w / 2.0f, box.y - fontSize - 2.0f, fontSize, ImFonts::verdana, ent->getName(), true, healthBased(ent), false);
}

// yoinked: https://www.unknowncheats.me/wiki/Counter_Strike_Global_Offensive:Bone_ESP
void Visuals::drawSkeleton(Player_t* ent)
{
	if (!config.get<bool>(vars.bDrawSkeleton))
		return;

	auto model = ent->getModel();
	if (!model)
		return;

	auto studio = interfaces::modelInfo->getStudioModel(model);
	if (!studio)
		return;

	// have to check if selected record is filled, if no then just skip
	auto record = !backtrack.getAllRecords().at(ent->getIndex()).empty()  ? &backtrack.getAllRecords().at(ent->getIndex()) : nullptr;
	auto skeletPos = [=](const size_t idx)
	{
		auto child = record != nullptr
			? record->back().m_matrix[idx].origin()
			: ent->getBonePos(idx);
		return child;
	};

	// bone IDs
	constexpr auto chest = 6;
	constexpr auto lowerChest = 5;

	for (int i = 0; i < studio->m_bonesCount; i++)
	{
		auto bone = studio->getBone(i);
		if (!bone)
			continue;

		if (bone->m_parent == -1)
			continue;

		if (!(bone->m_flags & BONE_USED_BY_HITBOX))
			continue;

		if (record && !backtrack.isValid(record->front().m_simtime)) // if backtrack
			continue;

		auto child = skeletPos(i);
		auto parent = skeletPos(bone->m_parent);
		auto chestbone = skeletPos(chest);
		auto upper = skeletPos(chest + 1) - chestbone;
		auto breast = chestbone + upper / 2.0f;

		auto deltachild = child - breast;
		auto deltaparent = parent - breast;

		if (deltaparent.length() < 9.0f && deltachild.length() < 9.0f)
			parent = breast;

		if (i == lowerChest)
			child = breast;

		if (std::abs(deltachild.z) < 5.0f && deltaparent.length() < 5.0f && deltachild.length() < 5.0f || i == chest)
			continue;

		if (Vector2D start, end; imRender.worldToScreen(parent, start) && imRender.worldToScreen(child, end))
			imRender.drawLine(start, end, config.get<Color>(vars.cSkeleton));
	}
}

void Visuals::drawSnapLine(Player_t* ent, const Box& box)
{
	if (ent == aimbot.getTargetted())
	{
		// lines on the bottom and center bottom box
		imRender.drawLine(globals::screenX / 2, globals::screenY, box.x + box.w / 2, box.y + box.h, Colors::Purple);
	}
}

void Visuals::drawLaser(Player_t* ent)
{
	if (!config.get<bool>(vars.bEspLasers))
		return;

	// get from where to start, "laser ESP" is always starting from head I think
	auto start = ent->getBonePos(8);
	// get angle to draw with correct view
	auto forward = math::angleVec(ent->m_angEyeAngles());
	// end is where lines just ends, this 70 is hardcoded, but whatever here tbh
	auto end = start + forward * 70.f;

	if (Vector2D startP, endLine; imRender.worldToScreen(start, startP) && imRender.worldToScreen(end, endLine))
	{
		imRender.drawCircleFilled(startP.x, startP.y, 3, 32, Colors::Red);
		imRender.drawLine(startP, endLine, Colors::Purple);
	}
}

void Visuals::runDLight(Player_t* ent)
{
	// https://github.com/ValveSoftware/source-sdk-2013/blob/0d8dceea4310fde5706b3ce1c70609d72a38efdf/sp/src/game/client/c_spotlight_end.cpp#L112

	if (!config.get<bool>(vars.bDLight))
		return;

	auto dLight = interfaces::efx->clAllocDLight(ent->getIndex());
	dLight->m_color = config.get<Color>(vars.cDlight);
	dLight->m_origin = ent->m_vecOrigin();
	dLight->m_radius = config.get<float>(vars.fDlightRadius);
	dLight->m_die = interfaces::globalVars->m_curtime + 0.1f;
	dLight->m_exponent = static_cast<char>(config.get<float>(vars.fDlightExponent));
	dLight->m_decay = config.get<float>(vars.fDlightDecay);
	dLight->m_key = ent->getIndex();
}

void Visuals::drawPlayer(Player_t* ent)
{
	if (!config.get<bool>(vars.bEsp))
		return;

	Box box; Box3D box3d;
	if (!utilities::getBox(ent, box, box3d))
		return;

	switch (config.get<int>(vars.iEsp))
	{
	case E2T(BoxTypes::BOX2D):
		drawBox2D(box);
		break;
	case E2T(BoxTypes::FILLED2D):
		drawBox2DFilled(box);
		break;
	case E2T(BoxTypes::BOX3D):
		drawBox3D(box3d);
		break;
	case E2T(BoxTypes::FILLED3D):
		drawBox3DFilled(box3d);
		break;
	default:
		break;
	}

	drawHealth(ent, box);
	drawArmor(ent, box);
	drawWeapon(ent, box);
	drawInfo(ent, box);
	drawSnapLine(ent, box);
	drawnName(ent, box);
}

// add this to events manager 
void Visuals::drawSound(IGameEvent* event)
{
	if (!config.get<bool>(vars.bSoundEsp))
		return;

	auto userid = interfaces::engine->getPlayerID(event->getInt(XOR("userid")));

	auto entity = reinterpret_cast<Player_t*>(interfaces::entList->getClientEntity(userid));

	if (!entity)
		return;

	if (!entity->isPlayer())
		return;

	if (!entity->isAlive())
		return;

	return; // FIXME: add esp records in vector. Use simple logic of storing records in some struct
}

Color Visuals::healthBased(Player_t* ent)
{
	int health = ent->m_iHealth();
	int g = health * 2.55f;
	int r = 255 - g;
	return Color(r, g, 0, 255);
}

void Visuals::enemyIsAimingAtYou(Player_t* ent)
{
	if (!config.get<bool>(vars.bAimingWarn))
		return;

	if (!game::localPlayer)
		return;

	if (!interfaces::engine->isInGame())
		return;

	Vector posDelta = ent->getEyePos() - game::localPlayer->getEyePos();
	Vector idealAimAngle = math::vectorToAngle(posDelta);

	// account for their spray control
	static const auto scale = interfaces::cvar->findVar(XOR("weapon_recoil_scale"))->getFloat();
	idealAimAngle -= ent->m_aimPunchAngle() * scale;

	Vector curEnemyAngle = ent->m_angEyeAngles();
	curEnemyAngle.normalize();

	// check trace
	bool check = ent->isPossibleToSee(game::localPlayer->getEyePos());

	// dynamic fov
	float fov = math::calcFovReal(ent->getEyePos(), game::localPlayer->getBonePos(3), curEnemyAngle); // 3 is middle body

	if (check) // no, check it differently
	{
		imRender.text(globals::screenX / 2, 60, ImFonts::tahoma, XOR("Enemy can see you"), true, Colors::Green);
	}
	// this can be made with tracing
	if (fov <= 5.0f)
	{
		imRender.text(globals::screenX / 2, 80, ImFonts::tahoma, XOR("Enemy is aiming you"), true, Colors::Red);
	}
}

float Visuals::getScaledFontSize(Entity_t* ent, const float division, const float min, const float max)
{
	float dist = ent->absOrigin().distTo(game::localPlayer->absOrigin());
	// clamping is hardcoded, for csgo there are no huge distances. For other games that have long distances rendered, this will still run ok.
	float fontSize = std::clamp(division / (dist / division), min, max);
	return fontSize;
}

#pragma region drawing_boxes

void Visuals::drawBox2D(const Box& box)
{
	Color cfgCol = config.get<Color>(vars.cBox);

	imRender.drawRect(box.x - 1.0f, box.y - 1.0f, box.w + 2.0f, box.h + 2.0f, Colors::Black);
	imRender.drawRect(box.x + 1.0f, box.y + 1.0f, box.w - 2.0f, box.h - 2.0f, Colors::Black);
	imRender.drawRect(box.x, box.y, box.w, box.h, cfgCol);
}

void Visuals::drawBox2DFilled(const Box& box)
{
	Color fill = config.get<Color>(vars.cBoxFill);

	imRender.drawRectFilled(box.x + 1.0f, box.y + 1.0f, box.w - 2.0f, box.h - 2.0f, fill);
	drawBox2D(box);
}

void Visuals::drawBox3DFilled(const Box3D& box, const float thickness)
{
	Color color = config.get<Color>(vars.cBox);
	Color fill = config.get<Color>(vars.cBoxFill);

	imRender.drawQuadFilled(box.points.at(0), box.points.at(1), box.points.at(2), box.points.at(3), fill);
	// top
	imRender.drawQuadFilled(box.points.at(4), box.points.at(5), box.points.at(6), box.points.at(7), fill);
	// front
	imRender.drawQuadFilled(box.points.at(3), box.points.at(2), box.points.at(6), box.points.at(7), fill);
	// back
	imRender.drawQuadFilled(box.points.at(0), box.points.at(1), box.points.at(5), box.points.at(4), fill);
	// right
	imRender.drawQuadFilled(box.points.at(2), box.points.at(1), box.points.at(5), box.points.at(6), fill);
	// left
	imRender.drawQuadFilled(box.points.at(3), box.points.at(0), box.points.at(4), box.points.at(7), fill);

	for (size_t i = 0; i < 3; i++)
	{
		imRender.drawLine(box.points.at(i), box.points.at(i + 1), color, thickness);
	}
	// missing part at the bottom
	imRender.drawLine(box.points.at(0), box.points.at(3), color, thickness);
	// top parts
	for (size_t i = 4; i < 7; i++)
	{
		imRender.drawLine(box.points.at(i), box.points.at(i + 1), color, thickness);
	}
	// missing part at the top
	imRender.drawLine(box.points.at(4), box.points.at(7), color, thickness);
	// now all 4 box.points missing parts for 3d box
	for (size_t i = 0; i < 4; i++)
	{
		imRender.drawLine(box.points.at(i), box.points.at(i + 4), color, thickness);
	}
}

void Visuals::drawBox3D(const Box3D& box, const float thickness)
{
	Color color = config.get<Color>(vars.cBox);

	for (size_t i = 0; i < 3; i++)
	{
		imRender.drawLine(box.points.at(i), box.points.at(i + 1), color, thickness);
	}
	// missing part at the bottom
	imRender.drawLine(box.points.at(0), box.points.at(3), color, thickness);
	// top parts
	for (size_t i = 4; i < 7; i++)
	{
		imRender.drawLine(box.points.at(i), box.points.at(i + 1), color, thickness);
	}
	// missing part at the top
	imRender.drawLine(box.points.at(4), box.points.at(7), color, thickness);
	// now all 4 box.points missing parts for 3d box
	for (size_t i = 0; i < 4; i++)
	{
		imRender.drawLine(box.points.at(i), box.points.at(i + 4), color, thickness);
	}
}

#pragma endregion