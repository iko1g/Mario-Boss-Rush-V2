#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"
#include "MainGame.h"
#include "Player.h"
#include "Boss.h"
#include "Enemies.h"
#include "Debug.h"
#include <random>

//-------------------------------------------------------------------------------------------------------------------------------
//												DEBUG FUNCTIONS	

void ShowDebugUI()
{
	GameObject& playerObj = Play::GetGameObjectByType(typePlayer);
	GameObject& hammerObj = Play::GetGameObjectByType(typeHammer);
	GameObject& bowserObj = Play::GetGameObjectByType(typeBowser);
	GameObject& magiKoopaObj = Play::GetGameObjectByType(typeMagiKoopa);

	std::vector<int> vGoombas = Play::CollectGameObjectIDsByType(typeGoomba);
	std::vector<int> vBobBombAlights = Play::CollectGameObjectIDsByType(typeBobBombAlight);
	std::vector<int> vBobBombs = Play::CollectGameObjectIDsByType(typeBobBombNotAlight);
	std::vector<int> vExplosions = Play::CollectGameObjectIDsByType(typeBobBombExplosion);
	std::vector<int> vDryBones = Play::CollectGameObjectIDsByType(typeDryBones);
	std::vector<int> vMagiKoopas = Play::CollectGameObjectIDsByType(typeMagiKoopa);

	//		Position Checks

	Play::DrawDebugText(playerObj.pos, "Player Here", Play::cGreen);

	Play::DrawDebugText(hammerObj.pos, "Hammer Here", Play::cGreen);

	Play::DrawDebugText(bowserObj.pos, "Bowser Here", Play::cGreen);
	Play::DrawCircle(bowserObj.pos, 200, Play::cGreen);

	for (int goombaID : vGoombas)
	{
		GameObject& goombaObj = Play::GetGameObject(goombaID);
		Play::DrawDebugText(goombaObj.pos, "Goomba Here", Play::cGreen);
	}

	for (int bobBombID : vBobBombs)
	{
		GameObject& bobBombObj = Play::GetGameObject(bobBombID);
		float distance = FindDistance(bobBombObj, playerObj);
		std::string distanceAsString = std::to_string(distance);
		Play::DrawDebugText(bobBombObj.pos, "BB Here", Play::cGreen);
		Play::DrawDebugText({ bobBombObj.pos.x , bobBombObj.pos.y + 10 }, distanceAsString.c_str(), Play::cGreen);

		//BobBmb agro range
		Play::DrawCircle(bobBombObj.pos, 100, Play::cGreen);
	}

	for (int bobBombID : vBobBombAlights)
	{
		GameObject& bobBombObj = Play::GetGameObject(bobBombID);

		float distance = FindDistance(bobBombObj, playerObj);
		std::string distanceAsString = std::to_string(distance);
		Play::DrawDebugText(bobBombObj.pos, "BB_A Here", Play::cGreen);
		Play::DrawDebugText({ bobBombObj.pos.x , bobBombObj.pos.y + 10 }, distanceAsString.c_str(), Play::cGreen);

		//BobBmb agro range
		Play::DrawCircle(bobBombObj.pos, 100, Play::cGreen);
	}

	for (int dryBonesID : vDryBones)
	{
		GameObject& dryBonesObj = Play::GetGameObject(dryBonesID);
		float playerToDryBonesAng = atan2f(playerObj.pos.y - dryBonesObj.pos.y, playerObj.pos.x - dryBonesObj.pos.x) + PLAY_PI / 2;
		std::string playerToDryBonesAngString = std::to_string(playerToDryBonesAng);
		float dryBonesDistance = FindDistance(dryBonesObj, playerObj);
		std::string dryBonesDistanceString = std::to_string(dryBonesDistance);
		Play::DrawDebugText({ dryBonesObj.pos.x, dryBonesObj.pos.y + 20 }, dryBonesDistanceString.c_str(), Play::cGreen);
		Play::DrawDebugText({ dryBonesObj.pos.x, dryBonesObj.pos.y + 10 }, playerToDryBonesAngString.c_str(), Play::cGreen);
		Play::DrawDebugText(dryBonesObj.pos, "DD Here", Play::cGreen);
	}

	for (int magiKooapID : vMagiKoopas)
	{
		GameObject& magiKoopaObj = Play::GetGameObject(magiKooapID);
		float playerToMagiAng = atan2f(magiKoopaObj.pos.y - playerObj.pos.y, magiKoopaObj.pos.x - playerObj.pos.x);
		std::string playerToMagiKoopaAngString = std::to_string(playerToMagiAng);
		Play::DrawDebugText({ magiKoopaObj.pos.x, magiKoopaObj.pos.y + 10 }, playerToMagiKoopaAngString.c_str(), Play::cGreen);
		Play::DrawDebugText(magiKoopaObj.pos, "MK Here", Play::cGreen);
	}

	//		Collision Checks

	for (int goombaID : vGoombas)
	{
		GameObject& goombaObj = Play::GetGameObject(goombaID);

		if (Play::IsColliding(goombaObj, playerObj))
		{
			Play::DrawDebugText(goombaObj.pos, "Goomba Colliding", Play::cGreen);
		}
	}

	for (int bobBombID : vBobBombs)
	{
		GameObject& bobBombObj = Play::GetGameObject(bobBombID);

		if (Play::IsColliding(bobBombObj, playerObj))
		{
			Play::DrawDebugText(bobBombObj.pos, "Bomba Colliding", Play::cGreen);
		}
	}

	for (int bobBombID : vBobBombAlights)
	{
		GameObject& bobBombObj = Play::GetGameObject(bobBombID);

		if (Play::IsColliding(bobBombObj, playerObj))
		{
			Play::DrawDebugText(bobBombObj.pos, "Bomba_A Colliding", Play::cGreen);
		}
	}

	for (int explosionID : vExplosions)
	{
		GameObject& explosionObj = Play::GetGameObject(explosionID);

		if (Play::IsColliding(explosionObj, playerObj))
		{
			Play::DrawDebugText(explosionObj.pos, "Explosion Colliding", Play::cGreen);
		}
	}

	for (int dryBonesID : vDryBones)
	{
		GameObject& dryBonesObj = Play::GetGameObject(dryBonesID);
		if (Play::IsColliding(dryBonesObj, playerObj))
		{
			Play::DrawDebugText(dryBonesObj.pos, "DD Colliding", Play::cGreen);
		}
	}

	for (int magiKooapID : vMagiKoopas)
	{
		GameObject& magiKoopaObj = Play::GetGameObject(magiKooapID);
		if (Play::IsColliding(magiKoopaObj, playerObj))
		{
			Play::DrawDebugText(magiKoopaObj.pos, "MK Colliding", Play::cGreen);
		}
	}

	//Timer Stuff

	//seconds
	std::string timePassedString = std::to_string(gameState.timePassed);
	Play::DrawDebugText({ 80 , 65 }, timePassedString.c_str(), Play::cGreen);
}
