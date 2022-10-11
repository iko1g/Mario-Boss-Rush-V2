#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"
#include <random>
#include <chrono>
#include <thread>
#include <time.h>

int displayWidth = 1280;
int displayHeight = 720;
int displayScale = 1;

const std::vector<std::string> vOrientations = { "N", "E", "S", "W" };
std::string playerOrientation{};
bool playerisDebuffed;

enum PlayerState 
{
	playerAppear = 0,
	playerNotDebuffed,
	playerDebuffed,
	playerDamaged,
	playerHeal,
	playerAttack,
	playerDead,
	playerPowerUp,
};

enum BossState 
{
	bossAppear = 0,
	bossIdle,
	bossAgro,
	bossDamaged,
	bossSummonMob,
	bossDead,
};

enum StageState
{
	stageStart = 0,
	normalStage,
	stageWin,
	bossStage,
};

enum GameObjectType
{
	typeNull = -1,
	//Player Related Types
	typePlayer,
	typeHammer,
	typeHealthIcon,
	typeGoldenMushroom,
	typeRefreshingHerb,
	typeHealth1UP,
	//Enemy Related Types
	typeGoomba,
	typeBobBomb,
	typeBobBombExplosion,
	typeDryBones,
	typeDryBoneBody,
	typeDryBoneHead,
	typeMagiKoopa,
	typeMagiKoopaProj,
	typeBowser,
	//Miscellaneous Types
	typeDestroyed,
};

struct GameState 
{
	int bossHp = 4;
	int playerHP = 3;
	int score = 0;
	int stage = 1;
	PlayerState playerState = PlayerState::playerNotDebuffed;
	BossState bossState = BossState::bossAppear;
	StageState stageState = StageState::normalStage;
};

GameState gameState;

//-------------------------------------------------------------------------------------------------------------------------------
//												UPDATE FUNCTIONS

//			PLAYER RELATED:

void UpdatePlayerState();
void UpdateConsumables();
void UpdateHammer();

//			ENEMY RELATED:

void UpdateBossState();
void UpdateEnemies();
void UpdateGoombas();
void UpdateDryBones();
void UpdateBobBombs();
void UpdateMagiKoopa();

//			STAGE RELATED:

void UpdateStageState();

//          MISCELLANEOUS:

void UpdateDestroyed();
void UpdateUI();

//-------------------------------------------------------------------------------------------------------------------------------
//												HANDLE FUNCTIONS	

void HandleNotDebuffedControls();
void HandleDebuffedControls();
void HandleHammerAnimations();

//-------------------------------------------------------------------------------------------------------------------------------
//												SPAWN FUNCTIONS	

void SpawnEnemies(int);
void SpawnPlayerConsumables();

//-------------------------------------------------------------------------------------------------------------------------------
//												UTILITY FUNCTIONS	

//          OBJECT RELATED:

void ScreenBouncing(GameObject&, int);
void SetVelocity(GameObject&, std::string);
Point2f GetRandomPositionInPS(int);
void DecelerateObject(GameObject&, float);

//			UI RELATED:

void DrawUI();
void UpdateStageBackground(int);

//          MISCELLANEOUS:

int PickBetween(int, int);
float FindDistance(GameObject&, GameObject&);
float GenRandomNumRange(float,float);
bool timeHasPassed(int, int);

//-------------------------------------------------------------------------------------------------------------------------------
//												DEBUG FUNCTIONS	

void ShowDebugUI();

//-------------------------------------------------------------------------------------------------------------------------------

// The entry point for a PlayBuffer program
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	Play::CreateManager( displayWidth, displayHeight, displayScale );
	UpdateStageBackground(gameState.stage);
	Play::CreateGameObject(typePlayer, { displayWidth / 2, 505 }, 25, "mario_idle_s_35");
	int hammerID = Play::CreateGameObject(typeHammer, Play::GetGameObjectByType(typePlayer).pos, 25, "ham_mario_s_3");
	SpawnEnemies(gameState.playerState);
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate( float elapsedTime )
{
	Play::DrawBackground();
	Play::DrawFontText("SuperMario25636px_10x10", "STAGE " + std::to_string(gameState.stage), { 80 , 5 }, Play::CENTRE);
	Play::DrawFontText("SuperMario25636px_10x10", "SCORE " + std::to_string(gameState.score), {displayWidth - 80 , 5 }, Play::CENTRE);
	Play::DrawFontText("SuperMario25636px_10x10", "HEALTH", { displayWidth / 2 , 5 }, Play::CENTRE);
	/*Play::DrawFontText("SuperMario25636px_10x10", std::to_string(gameState.playerHP), { displayWidth / 2 , 15 }, Play::CENTRE);*/
	UpdateEnemies();
	UpdatePlayerState();
	UpdateHammer();
	UpdateDestroyed();
	ShowDebugUI();
	Play::PresentDrawingBuffer();
	return Play::KeyDown( VK_ESCAPE );
}

// Gets called once when the player quits the game 
int MainGameExit( void )
{
	Play::DestroyManager();
	return PLAY_OK;
}

//-------------------------------------------------------------------------------------------------------------------------------
//												UPDATE FUNCTIONS

//			PLAYER RELATED:

//used to update player state
void UpdatePlayerState()
{
	GameObject& playerObj = Play::GetGameObjectByType(typePlayer);
	GameObject& hammerObj = Play::GetGameObjectByType(typeHammer);

	switch (gameState.playerState)
	{
	case PlayerState::playerAppear:
	{
		//Play walks from left when x is certain value stateappear
	}
	break;
	case PlayerState::playerNotDebuffed:
	{
		playerisDebuffed = false;
		HandleNotDebuffedControls();
	}
	break;
	case PlayerState::playerDebuffed:
	{
		playerisDebuffed = true;
		HandleDebuffedControls();
	}
	break;
	case PlayerState::playerDamaged:
	{
		//Minus player health and move to relevant move state
		if (gameState.playerHP - 1 == 0)
		{
			gameState.playerHP--;
			gameState.playerState = PlayerState::playerDead;
		}
		else 
		{
			gameState.playerHP--;

			if (playerisDebuffed) 
			{
				gameState.playerState = PlayerState::playerDebuffed;
			}
			else 
			{
				gameState.playerState = PlayerState::playerNotDebuffed;
			}
		}
	}
	break;
	case PlayerState::playerHeal:
	{
		// Plus player health and move to relevant move state
		gameState.playerHP++;

		if (playerisDebuffed)
		{
			gameState.playerState = PlayerState::playerDebuffed;
		}
		else
		{
			gameState.playerState = PlayerState::playerNotDebuffed;
		}
	}
	break;
	case PlayerState::playerAttack:
	{
		HandleHammerAnimations();
	}
	break;
	case PlayerState::playerDead:
	{
		//Play Death Animation
	}
	break;
	case PlayerState::playerPowerUp:
	{
		//Be in this state for 7 seconds 
		//Player is invincible
	}
	break;
	}
	ScreenBouncing(playerObj, gameState.stage);
	Play::UpdateGameObject(playerObj);
	Play::DrawObject(playerObj);
}

//Used to update player consumable items
void UpdateConsumables() 
{
	GameObject& playerObj = Play::GetGameObjectByType(typePlayer);
	std::vector<int> vHealthUps = Play::CollectGameObjectIDsByType(typeHealth1UP);
	std::vector<int> vGoldenMushrooms = Play::CollectGameObjectIDsByType(typeGoldenMushroom);
	std::vector<int> vRefreshingHerbs = Play::CollectGameObjectIDsByType(typeRefreshingHerb);

	for (int healthUpID : vHealthUps)
	{
		GameObject& healthUpObj = Play::GetGameObject(healthUpID);
		bool hasCollidedHealth = false;

		if (gameState.playerState != PlayerState::playerDead)
		{
			if (Play::IsColliding(healthUpObj, playerObj))
			{
				hasCollidedHealth = true;
				gameState.playerState = PlayerState::playerHeal;
			}

			if (hasCollidedHealth)
			{
				healthUpObj.type = typeDestroyed;
			}
		}

		ScreenBouncing(healthUpObj, gameState.stage);
		Play::UpdateGameObject(healthUpObj);
		Play::DrawObject(healthUpObj);
	}

	for (int goldenMushroomsID : vGoldenMushrooms)
	{
		GameObject& goldenMushroomObj = Play::GetGameObject(goldenMushroomsID);
		bool hasCollidedGoldM = false;

		if (gameState.playerState != PlayerState::playerDead)
		{
			if (Play::IsColliding(goldenMushroomObj, playerObj))
			{
				hasCollidedGoldM = true;
				gameState.playerState = PlayerState::playerPowerUp;
			}

			if (hasCollidedGoldM)
			{
				goldenMushroomObj.type = typeDestroyed;
			}
		}
		ScreenBouncing(goldenMushroomObj, gameState.stage);
		Play::UpdateGameObject(goldenMushroomObj);
		Play::DrawObject(goldenMushroomObj);
	}

	for (int refreshingHerbID : vRefreshingHerbs)
	{
		GameObject& refreshingHerbObj = Play::GetGameObject(refreshingHerbID);
		bool hasCollidedRefreshingHerbs = false;

		if (gameState.playerState == PlayerState::playerDebuffed)
		{
			if (Play::IsColliding(refreshingHerbObj, playerObj))
			{
				hasCollidedRefreshingHerbs = true;
				Play::SetSprite(playerObj, "mario_normaltofat_22", 0.25f);

				if (Play::IsAnimationComplete(playerObj)) 
				{
					gameState.playerState = PlayerState::playerNotDebuffed;
				}
			}

			if (hasCollidedRefreshingHerbs)
			{
				refreshingHerbObj.type = typeDestroyed;
			}
		}
		ScreenBouncing(refreshingHerbObj, gameState.stage);
		Play::UpdateGameObject(refreshingHerbObj);
		Play::DrawObject(refreshingHerbObj);
	}

}

//Used to update Hammer game object
void UpdateHammer()
{
	GameObject& playerObj = Play::GetGameObjectByType(typePlayer);
	GameObject& hammerObj = Play::GetGameObjectByType(typeHammer);

	hammerObj.pos = playerObj.pos;

	if (gameState.playerState == PlayerState::playerAttack)
	{
		Play::UpdateGameObject(hammerObj);
		Play::DrawObject(hammerObj);
	}
	else
	{
		Play::UpdateGameObject(hammerObj);
		Play::DrawObjectTransparent(hammerObj, 0);
	}
}

//			ENEMY RELATED:

//Used to update boss state
void UpdateBossState() 
{
	GameObject& bowserObj = Play::GetGameObjectByType(typeBowser);
	bool longRangedAtk = false;

	switch (gameState.bossState) 
	{
	case BossState::bossAppear:
	{
		//boss will walk from left
		//once they reach certain x amount move to idle
	}
	break;
	case BossState::bossIdle:
	{
		//play Idle animation for 4 seconds
		//if player is long ranged  set long range to true then move to boss agro
		//if player is close ranged set long range to false then move to boss agro
		//if enemy lists are empty then move to boss summon mob

	}
	break;
	case BossState::bossAgro:
	{

	}
	break;
	case BossState::bossDamaged:
	{
		//-- from bossHp and play damage anim
		//if bosshp -- is 0 then move to bossDead
		//if damage anim is finished then move to state idle
	}
	break;
	case BossState::bossSummonMob:
	{
		//play summon mob anim
		//use spawnenemies function 
		//if enemy list .size = 6 then move to idle
	}
	break;
	case BossState::bossDead:
	{
		//play death animation
		//delete boss object
	}
	break;
	}
}

//Used to update enemies
void UpdateEnemies() 
{
	UpdateGoombas();
	UpdateBobBombs();
	UpdateMagiKoopa();
	UpdateDryBones();
}

//Used to update Goomba enemies
void UpdateGoombas() 
{
	GameObject& playerObj = Play::GetGameObjectByType(typePlayer);
	GameObject& hammerObj = Play::GetGameObjectByType(typeHammer);
	std::vector<int> vGoombas = Play::CollectGameObjectIDsByType(typeGoomba);
	
	for (int goombaID : vGoombas) 
	{
		bool hammerCollided = false;
		bool userCollided = false;
		GameObject& goombaObj = Play::GetGameObject(goombaID);

		if (gameState.playerState == PlayerState::playerNotDebuffed || gameState.playerState == PlayerState::playerDebuffed) 
		{
			if (Play::IsColliding(goombaObj, playerObj))
			{
				userCollided = true;
				gameState.playerState = PlayerState::playerDamaged;
				//play enemy attack/ damaging anim if 
			}
		}
		else if (gameState.playerState == PlayerState::playerAttack)
		{
			if (Play::IsColliding(goombaObj,hammerObj)) 
			{
				hammerCollided = true;
				gameState.score++;
				//play enemy death anim
			}
		}

		if (hammerCollided || userCollided) 
		{
			goombaObj.type = typeDestroyed;
		}

		ScreenBouncing(goombaObj, gameState.stage);

		if (goombaObj.velocity.x < 0)
		{
			Play::SetSprite(goombaObj, "goomba_walk_w_8", 0.25f);
		}
		else if (goombaObj.velocity.x > 0)
		{
			Play::SetSprite(goombaObj, "goomba_walk_e_8", 0.25f);
		}

		Play::UpdateGameObject(goombaObj);
		Play::DrawObjectRotated(goombaObj);
	}
}

//Used to update dry bones enemies
void UpdateDryBones() 
{
	GameObject& playerObj = Play::GetGameObjectByType(typePlayer);
	GameObject& hammerObj = Play::GetGameObjectByType(typeHammer);
	std::vector<int> vDryBones = Play::CollectGameObjectIDsByType(typeDryBones);

	for (int dryBonesID: vDryBones) 
	{
		GameObject& dryBonesObj = Play::GetGameObject(dryBonesID);
		
		float playerToDryBonesAng = atan2f(playerObj.pos.y - dryBonesObj.pos.y, playerObj.pos.x - dryBonesObj.pos.x);
		//if angle between player and object is equal to 90 degrees or 270 degrees
		//spawn head and body 
		//head will follow player for 3 seconds 
		//if player hammers head or body destroy them

		ScreenBouncing(dryBonesObj, gameState.stage);

		if (dryBonesObj.velocity.y < 0)
		{
			Play::SetSprite(dryBonesObj, "drybones_n_16", 0.25f);
		}
		else if (dryBonesObj.velocity.y > 0)
		{
			Play::SetSprite(dryBonesObj, "drybones_s_16", 0.25f);
		}

		Play::UpdateGameObject(dryBonesObj);
		Play::DrawObjectRotated(dryBonesObj);
	}
}

//Used to update bobbomb enemies
void UpdateBobBombs() 
{
	GameObject& playerObj = Play::GetGameObjectByType(typePlayer);
	GameObject& hammerObj = Play::GetGameObjectByType(typeHammer);
	std::vector<int> vBobBombs = Play::CollectGameObjectIDsByType(typeBobBomb);

	for (int bobBombID : vBobBombs) 
	{
		GameObject& bobBombObj = Play::GetGameObject(bobBombID);
		bool isAlight = false;
		float distanceFromPlayer = FindDistance(bobBombObj, playerObj);

		if (distanceFromPlayer < 100 && isAlight == false &&( gameState.playerState == PlayerState::playerNotDebuffed || gameState.playerState == PlayerState::playerDebuffed))
		{
			isAlight = true;
			bobBombObj.velocity *= 1.5;
		}

		if (isAlight)
		{
			if (bobBombObj.velocity.x < 0)
			{
				Play::SetSprite(bobBombObj, "bobbomb_walk_w_a_8", 0.25f);
			}
			else if (bobBombObj.velocity.x > 0)
			{
				Play::SetSprite(bobBombObj, "bobbomb_walk_e_a_8", 0.25f);
			}

			if (!Play::IsColliding(bobBombObj, playerObj))
			{
				//if desired time = elapsedtime + 4000 ms do below 
				//wait 4 seconds

				bobBombObj.velocity = { 0,0 };
				Play::SetSprite(bobBombObj, "bobbomb_walk_s_a_8", 0.25f);

				//if desired time = elapsedtime + 2000 ms do below 
				//wait 2 seconds

				Play::DrawObjectTransparent(bobBombObj, 0);

				int explosion1ID = Play::CreateGameObject(typeBobBombExplosion, bobBombObj.pos, 15, "explosion_12");
				int explosion2ID = Play::CreateGameObject(typeBobBombExplosion, bobBombObj.pos, 15, "explosion_12");
				int explosion3ID = Play::CreateGameObject(typeBobBombExplosion, bobBombObj.pos, 15, "explosion_12");

				GameObject& explosion1Obj = Play::GetGameObject(explosion1ID);
				GameObject& explosion2Obj = Play::GetGameObject(explosion2ID);
				GameObject& explosion3Obj = Play::GetGameObject(explosion3ID);

				explosion1Obj.animSpeed = explosion2Obj.animSpeed = explosion3Obj.animSpeed = 0.25f;

				//Set scale to random number between 0.7 and 0.9
				explosion1Obj.scale = GenRandomNumRange(0.7, 0.9);
				explosion2Obj.scale = GenRandomNumRange(0.7, 0.9);
				explosion3Obj.scale = GenRandomNumRange(0.7, 0.9);

				//Set Random Rotation
				//explosion1Obj.rotation = GenRandomNumRange(0.0, PLAY_PI);
				//explosion2Obj.rotation = GenRandomNumRange(0.0, PLAY_PI);
				//explosion3Obj.rotation = GenRandomNumRange(0.0, PLAY_PI);
				Play::DestroyGameObject(bobBombID);
			}
			else
			{
				bobBombObj.velocity = { 0,0 };
				Play::DrawObjectTransparent(bobBombObj, 0);

				int explosion1ID = Play::CreateGameObject(typeBobBombExplosion, bobBombObj.pos, 15, "explosion_12");
				int explosion2ID = Play::CreateGameObject(typeBobBombExplosion, bobBombObj.pos, 15, "explosion_12");
				int explosion3ID = Play::CreateGameObject(typeBobBombExplosion, bobBombObj.pos, 15, "explosion_12");

				GameObject& explosion1Obj = Play::GetGameObject(explosion1ID);
				GameObject& explosion2Obj = Play::GetGameObject(explosion2ID);
				GameObject& explosion3Obj = Play::GetGameObject(explosion3ID);

				explosion1Obj.animSpeed = explosion2Obj.animSpeed = explosion3Obj.animSpeed = 0.25f;

				//Set scale to random number between 0.7 and 0.9
				explosion1Obj.scale = GenRandomNumRange(0.7, 0.9);
				explosion2Obj.scale = GenRandomNumRange(0.7, 0.9);
				explosion3Obj.scale = GenRandomNumRange(0.7, 0.9);

				//Set Random Rotation
				explosion1Obj.rotation = GenRandomNumRange(0.0, PLAY_PI);
				explosion2Obj.rotation = GenRandomNumRange(0.0, PLAY_PI);
				explosion3Obj.rotation = GenRandomNumRange(0.0, PLAY_PI);
				Play::DestroyGameObject(bobBombID);
			}
		}
		else
		{
			if (bobBombObj.velocity.x < 0)
			{
				Play::SetSprite(bobBombObj, "bobbomb_walk_w_8", 0.25f);
			}
			else if (bobBombObj.velocity.x > 0)
			{
				Play::SetSprite(bobBombObj, "bobbomb_walk_e_8", 0.25f);
			}
		}
		ScreenBouncing(bobBombObj, gameState.stage);
		Play::UpdateGameObject(bobBombObj);
		Play::DrawObjectRotated(bobBombObj);
	}

	std::vector<int> vExplosions = Play::CollectGameObjectIDsByType(typeBobBombExplosion);

	for (int explosionID : vExplosions) 
	{
		GameObject& explosionObj = Play::GetGameObject(explosionID);

		if (gameState.playerState == PlayerState::playerNotDebuffed || gameState.playerState == PlayerState::playerDebuffed) 
		{
			if (Play::IsColliding(explosionObj, playerObj)) 
			{
				gameState.playerState == PlayerState::playerDamaged;
				explosionObj.type = typeDestroyed;
			}
		}

		if (Play::IsAnimationComplete(explosionObj)) 
		{
			explosionObj.type = typeDestroyed;
		}
		Play::UpdateGameObject(explosionObj);
		Play::DrawObjectRotated(explosionObj);
	}
}


//Used to update magikoopa enemy
void UpdateMagiKoopa() 
{
	//if player is x is less than mk x then launch projectile left else right
	GameObject& playerObj = Play::GetGameObjectByType(typePlayer);
	GameObject& hammerObj = Play::GetGameObjectByType(typeHammer);
	std::vector<int> vMagiKoopas = Play::CollectGameObjectIDsByType(typeMagiKoopa);

	for (int magiKoopaID : vMagiKoopas)
	{
		GameObject& magiKoopaObj = Play::GetGameObject(magiKoopaID);

		float playerToMagiAng = atan2f(playerObj.pos.y - magiKoopaObj.pos.y, playerObj.pos.x - magiKoopaObj.pos.x);
		//if angle between player and object is equal to 90 degrees or 270 degrees
		//set temp value to old Velocity value
		// if player is left then do left animation --> spawn projectile going left
		// else right animation --> spawn projectile going right
		//if projecile is not visible and hasnt collided with playe then destroy
		//if player collides with projectile then set player state to debuffed and minus 1 from health
		//if player hammers koopa delete koopa

		ScreenBouncing(magiKoopaObj, gameState.stage);

		if (magiKoopaObj.velocity.y < 0)
		{
			Play::SetSprite(magiKoopaObj, "magikoopa_n_8", 0.25f);
		}
		else if (magiKoopaObj.velocity.y > 0)
		{
			Play::SetSprite(magiKoopaObj, "magikoopa_s_8", 0.25f);
		}

		Play::UpdateGameObject(magiKoopaObj);
		Play::DrawObjectRotated(magiKoopaObj);
	}


}

//			STAGE RELATED:

void UpdateStageState() {}

//          MISCELLANEOUS:

//Used to update destroyed objects ()
void UpdateDestroyed() 
{
	std::vector<int> vDestroyed = Play::CollectGameObjectIDsByType(typeDestroyed);

	for (int desID : vDestroyed)
	{
		GameObject& desObj = Play::GetGameObject(desID);
		desObj.animSpeed = 0.2f;
		Play::UpdateGameObject(desObj);

		if (desObj.frame % 2)
		{
			Play::DrawObjectRotated(desObj, (10 - desObj.frame) / 10.0f);
		}

		if (!Play::IsVisible(desObj) || desObj.frame >= 8)
		{
			Play::DestroyGameObject(desID);
		}
	}
}

//Used to update UI
void UpdateUI() {}

//-------------------------------------------------------------------------------------------------------------------------------
//												CONTROL FUNCTIONS	

//Used for player controls when they are not debuffed
void HandleNotDebuffedControls() 
{
	GameObject& playerObj = Play::GetGameObjectByType(typePlayer);

	if (Play::KeyDown(VK_UP)) 
	{
		playerObj.velocity = { 0,-2.5f };
		Play::SetSprite(playerObj, "mario_walk_n_12", 0.35f);
		playerOrientation = vOrientations.at(0);
	}
	else if (Play::KeyDown(VK_RIGHT)) 
	{
		playerObj.velocity = { 2.5f,0 };
		Play::SetSprite(playerObj, "mario_walk_e_12", 0.35f);
		playerOrientation = vOrientations.at(1);
	}
	else if (Play::KeyDown(VK_DOWN)) 
	{
		playerObj.velocity = { 0,2.5f };
		Play::SetSprite(playerObj, "mario_walk_s_12", 0.35f);
		playerOrientation = vOrientations.at(2);
	}
	else if (Play::KeyDown(VK_LEFT)) 
	{
		playerObj.velocity = { -2.5f,0 };
		Play::SetSprite(playerObj, "mario_walk_w_12", 0.35f);
		playerOrientation = vOrientations.at(3);
	}
	else 
	{
		DecelerateObject(playerObj, 0.2f);
		if (playerObj.velocity == Point2f{ 0,0 })
		{
			if (playerOrientation == vOrientations.at(0))
			{
				Play::SetSprite(playerObj, "mario_idle_n_35", 0.35f);
			}
			else if (playerOrientation == vOrientations.at(1))
			{
				Play::SetSprite(playerObj, "mario_idle_e_35", 0.35f);
			}
			else if (playerOrientation == vOrientations.at(2))
			{
				Play::SetSprite(playerObj, "mario_idle_s_35", 0.35f);
			}
			else if (playerOrientation == vOrientations.at(3))
			{
				Play::SetSprite(playerObj, "mario_idle_w_35", 0.35f);
			}
		}
	}
}

//Used for player controls when they are debuffed
void HandleDebuffedControls() 
{
	GameObject& playerObj = Play::GetGameObjectByType(typePlayer);

	if (gameState.playerState != PlayerState::playerDead)
	{
		if (Play::KeyDown(VK_UP))
		{
			playerObj.velocity = { 0,-1.5f };
			Play::SetSprite(playerObj, "mario_fat_walk_n_14", 0.25f);
			playerOrientation = vOrientations.at(0);
		}
		else if (Play::KeyDown(VK_RIGHT))
		{
			playerObj.velocity = { 1.5f,0 };
			Play::SetSprite(playerObj, "mario_fat_walk_e_14", 0.25f);
			playerOrientation = vOrientations.at(1);
		}
		else if (Play::KeyDown(VK_DOWN))
		{
			playerObj.velocity = { 0,1.5f };
			Play::SetSprite(playerObj, "mario_fat_walk_s_14", 0.25f);
			playerOrientation = vOrientations.at(2);
		}
		else if (Play::KeyDown(VK_LEFT))
		{
			playerObj.velocity = { -1.5f,0 };
			Play::SetSprite(playerObj, "mario_fat_walk_w_14", 0.25f);
			playerOrientation = vOrientations.at(3);
		}
		else
		{
			DecelerateObject(playerObj, 0.2f);
			if (playerObj.velocity == Point2f{ 0,0 })
			{
				if (playerOrientation == vOrientations.at(0))
				{
					Play::SetSprite(playerObj, "mario_fat_idle_n_45", 0.25f);
				}
				else if (playerOrientation == vOrientations.at(1))
				{
					Play::SetSprite(playerObj, "mario_fat_idle_e_45", 0.25f);
				}
				else if (playerOrientation == vOrientations.at(2))
				{
					Play::SetSprite(playerObj, "mario_fat_idle_s_45", 0.25f);
				}
				else if (playerOrientation == vOrientations.at(3))
				{
					Play::SetSprite(playerObj, "mario_fat_idle_w_45", 0.25f);
				}
			}
		}
	}
}

//Handle Hammer Animations
void HandleHammerAnimations() 
{
	GameObject& playerObj = Play::GetGameObjectByType(typePlayer);
	GameObject& hammerObj = Play::GetGameObjectByType(typeHammer);

	if (playerOrientation == vOrientations.at(0))
	{
		Play::SetSprite(playerObj, "mario_hammer_n_3", 0.25f);
		Play::SetSprite(hammerObj, "ham_mario_n_3", 0.25f);

		if (Play::IsAnimationComplete(hammerObj) && Play::IsAnimationComplete(playerObj))
		{
			gameState.playerState = PlayerState::playerNotDebuffed;
		}
	}
	else if (playerOrientation == vOrientations.at(1))
	{
		Play::SetSprite(playerObj, "mario_hammer_e_3", 0.25f);
		Play::SetSprite(hammerObj, "ham_mario_w_3", 0.25f);

		if (Play::IsAnimationComplete(hammerObj) && Play::IsAnimationComplete(playerObj))
		{
			gameState.playerState = PlayerState::playerNotDebuffed;
		}
	}
	else if (playerOrientation == vOrientations.at(2))
	{
		Play::SetSprite(playerObj, "mario_hammer_s_3", 0.25f);
		Play::SetSprite(hammerObj, "ham_mario_s_3", 0.25f);

		if (Play::IsAnimationComplete(hammerObj) && Play::IsAnimationComplete(playerObj))
		{
			gameState.playerState = PlayerState::playerNotDebuffed;
		}
	}
	else if (playerOrientation == vOrientations.at(3))
	{
		Play::SetSprite(playerObj, "mario_hammer_w_3", 0.25f);
		Play::SetSprite(hammerObj, "ham_mario_e_3", 0.25f);

		if (Play::IsAnimationComplete(hammerObj) && Play::IsAnimationComplete(playerObj))
		{
			gameState.playerState = PlayerState::playerNotDebuffed;
		}
	}
}

//-------------------------------------------------------------------------------------------------------------------------------
//												SPAWN FUNCTIONS	

//Used to spawn enemies in initial stage
void SpawnEnemies(int stage) 
{
	if (stage == 1) 
	{
		//spawn 3 goombas
		for (int i = 0; i < 3; i++) 
		{
			int goombaID = Play::CreateGameObject(typeGoomba, GetRandomPositionInPS(stage), 10, "goomba_win_s_24");
			GameObject& goombaObj = Play::GetGameObject(goombaID);
			SetVelocity(goombaObj, "x");
		}

		int bobbombID = Play::CreateGameObject(typeBobBomb, GetRandomPositionInPS(stage), 10, "bobbomb_walk_s_8");
		GameObject& bobBombObj = Play::GetGameObject(bobbombID);
		SetVelocity(bobBombObj, "x");

		int chance = PickBetween(1, -1);

		if (chance == 1) 
		{
			int magiKoopaID = Play::CreateGameObject(typeMagiKoopa, GetRandomPositionInPS(stage), 15, "magikoopa_s_8");
			GameObject& magiKoopaObj = Play::GetGameObject(magiKoopaID);
			SetVelocity(magiKoopaObj, "y");
		}
		else 
		{
			int dryBonesID = Play::CreateGameObject(typeDryBones, GetRandomPositionInPS(stage), 15, "drybones_s_16");
			GameObject& dryBonesObj = Play::GetGameObject(dryBonesID);
			SetVelocity(dryBonesObj, "both");
		}
	}
	else if (stage == 2) 
	{

	}
}

//used to spawn player consumables
void SpawnPlayerConsumables() 
{
	if (gameState.playerHP <= 3 && Play::CollectGameObjectIDsByType(typeGoldenMushroom).size() < 1)
	{
		int goldenMushroomID = Play::CreateGameObject(typeGoldenMushroom, GetRandomPositionInPS(gameState.stage), 20, "invincible_powerup");
		GameObject& goldenMushroomObj = Play::GetGameObject(goldenMushroomID);
		SetVelocity(goldenMushroomObj, "both");
	}
	else if (gameState.playerHP < 3 && Play::CollectGameObjectIDsByType(typeHealth1UP).size() < 2)
	{
		int healthUpID = Play::CreateGameObject(typeHealth1UP, GetRandomPositionInPS(gameState.stage), 20, "health_up");
		GameObject& healthUpObj = Play::GetGameObject(healthUpID);
		SetVelocity(healthUpObj, "both");
	}
	else if (gameState.playerState == PlayerState::playerDebuffed && Play::CollectGameObjectIDsByType(typeRefreshingHerb).size() < 1)
	{
		int refreshingHerbID = Play::CreateGameObject(typeRefreshingHerb, GetRandomPositionInPS(gameState.stage), 20, "refreshing_herb");
		GameObject& refreshingHerbObj = Play::GetGameObject(refreshingHerbID);
		SetVelocity(refreshingHerbObj, "both");
	}
}

//-------------------------------------------------------------------------------------------------------------------------------
//												UTILITY FUNCTIONS	

//          OBJECT RELATED:

//Used to bounce certain objects so they stay in the play space
void ScreenBouncing(GameObject& gameObj, int stage) 
{
	if (stage == 1) 
	{
		if (gameObj.type == typePlayer) 
		{
			if (Play::IsLeavingDisplayArea(gameObj , Play::HORIZONTAL)) 
			{
				gameObj.pos = gameObj.oldPos;
			}
			if (gameObj.pos.y > 660)
			{
				gameObj.pos = gameObj.oldPos;
			}
			if (gameObj.pos.y < 470)
			{
				gameObj.pos = gameObj.oldPos;
			}
		}
		else 
		{
			if (Play::IsLeavingDisplayArea(gameObj, Play::HORIZONTAL)) 
			{
				gameObj.velocity.x *= -1;
			}

			if (gameObj.pos.y > 660 || gameObj.pos.y < 470)
			{
				gameObj.velocity.y *= -1;
			}
		}
	}
	else if (stage == 2) 
	{
		
		if (gameObj.type == typePlayer)
		{
			if (Play::IsLeavingDisplayArea(gameObj, Play::HORIZONTAL))
			{
				gameObj.pos = gameObj.oldPos;
			}
			if (gameObj.pos.y > 630 || gameObj.pos.y < 380)
			{
				gameObj.pos = gameObj.oldPos;
			}
		}
		else
		{
			if (Play::IsLeavingDisplayArea(gameObj, Play::HORIZONTAL))
			{
				gameObj.velocity.x *= -1;
			}

			if (gameObj.pos.y > 630 || gameObj.pos.y < 380)
			{
				gameObj.velocity.y *= -1;
			}
		}
	}
}

//Used to set an objects velocity to soley x, y or a combination of the two
void SetVelocity(GameObject& gameObj, std::string choice) 
{
	if (choice == "x")
	{
		gameObj.velocity = Point2f(PickBetween(1,-1)* 2 , 0);
	}
	else if (choice == "y")
	{
		gameObj.velocity = Point2f(0, PickBetween(1, -1) * 2);
	}
	else if (choice == "both")
	{
		gameObj.velocity = Point2f(PickBetween(1, -1) * 2, PickBetween(1, -1) * 2);
	}
}

//Used to get a point2f value that is the play space of the relevant stage
Point2f GetRandomPositionInPS(int stage) 
{
	if (stage == 1) 
	{
		return Point2f(Play::RandomRollRange(0, displayWidth) , Play::RandomRollRange(470, 660));
	}
	else if (stage == 2) 
	{
		return Point2f(Play::RandomRollRange(0, displayWidth), Play::RandomRollRange(380, 630));
	}
}

//Used to decelerate game objects
void DecelerateObject(GameObject& gameObj, float rate) 
{
	gameObj.velocity *= rate;
	gameObj.acceleration = {0,0};
}

//			UI RELATED:

//Used to draw UI
void DrawUI() {}

//Used to load different stage backgrounds
void UpdateStageBackground(int stage) 
{
	if (stage == 1)
	{
		Play::LoadBackground("Data\\Backgrounds\\stage1bg.png");
	}
	else if (stage == 2)
	{
		Play::LoadBackground("Data\\Backgrounds\\stage2bg.png");
	}
}

//          MISCELLANEOUS:

//Used to pick between two numbers
int PickBetween(int num1, int num2) 
{
	float chance = Play::RandomRollRange(0, 50);

	if (chance < 25) 
	{
		return num1;
	}
	else 
	{
		return num2;
	}
}

//Used to find the distance between two game objects
float FindDistance(GameObject& gameObj1, GameObject& gameObj2) 
{
	float x1 = gameObj1.pos.x;
	float x2 = gameObj2.pos.x;
	float y1 = gameObj1.pos.y;
	float y2 = gameObj2.pos.y;

	float distance = sqrt(pow((x2 - x1), 2) + pow((y2 - y1), 2));
	return distance;
}

//Generate Random number between two float numbers
float GenRandomNumRange(float lowerNum, float upperNum) 
{
	std::random_device rd;
	std::uniform_real_distribution<double> dist(lowerNum, upperNum);
	return dist(rd);
}


//Used to return if a specific amount of time has passed
bool timeHasPassed(int elapsedTime, int timePassed) 
{
	//add timepasssed to elapsedTime --> desiredtime
	//if elapsedTime  == desiredTime return true else return false
	// else false;
}


//-------------------------------------------------------------------------------------------------------------------------------
//												DEBUG FUNCTIONS	

void ShowDebugUI() 
{
	GameObject& playerObj = Play::GetGameObjectByType(typePlayer);
	GameObject& hammerObj = Play::GetGameObjectByType(typeHammer);
	GameObject& bowserObj = Play::GetGameObjectByType(typeBowser);

	std::vector<int> vGoombas = Play::CollectGameObjectIDsByType(typeGoomba);
	std::vector<int> vBobBombs = Play::CollectGameObjectIDsByType(typeBobBomb);
	std::vector<int> vDryBones = Play::CollectGameObjectIDsByType(typeDryBones);
	std::vector<int> vMagiKoopa = Play::CollectGameObjectIDsByType(typeMagiKoopa);
	std::vector<int> vExplosions = Play::CollectGameObjectIDsByType(typeBobBombExplosion);
	
	//		Position Checks
	Play::DrawDebugText(playerObj.pos, "Player Here", Play::cGreen);
	Play::DrawDebugText(hammerObj.pos, "Hammer Here", Play::cGreen);
	Play::DrawDebugText(bowserObj.pos, "Bowser Here", Play::cGreen);

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
		Play::DrawDebugText({ bobBombObj.pos.x , bobBombObj.pos.y + 5}, distanceAsString.c_str(), Play::cGreen);
	}

	for (int dryBonesID : vDryBones)
	{
		GameObject& dryBonesObj = Play::GetGameObject(dryBonesID);
		Play::DrawDebugText(dryBonesObj.pos, "DD Here", Play::cGreen);
	}

	for (int magiKoopaID : vMagiKoopa)
	{
		GameObject& magiKoopaObj = Play::GetGameObject(magiKoopaID);
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

	for (int magiKoopaID : vMagiKoopa)
	{
		GameObject& magiKoopaObj = Play::GetGameObject(magiKoopaID);

		if (Play::IsColliding(magiKoopaObj, playerObj))
		{
			Play::DrawDebugText(magiKoopaObj.pos, "MK Colliding", Play::cGreen);
		}
	}
}
