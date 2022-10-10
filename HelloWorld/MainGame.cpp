#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"
#include <random>

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
void UpdateEnemyProjectiles();

//			STAGE RELATED:

void UpdateStageState();

//          MISCELLANEOUS:

void UpdateDestroyed();
void UpdateUI();

//-------------------------------------------------------------------------------------------------------------------------------
//												CONTROL FUNCTIONS	

void HandleNotDebuffedControls();
void HandleDebuffedControls();

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
void SetSpriteDirection(GameObject&);

//			UI RELATED:

void DrawUI();
void UpdateStageBackground(int);

//          MISCELLANEOUS:

int PickBetween(int, int);
float FindDistance(GameObject&, GameObject&);
float GenRandomNumRange(float,float);


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
	UpdateDestroyed();
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
		//Play Hammer Animation 
	}
	break;
	case PlayerState::playerDead:
	{
		//Play Death Animation
	}
	break;
	}
	ScreenBouncing(playerObj, gameState.stage);
	Play::UpdateGameObject(playerObj);
	Play::DrawObject(playerObj);
}


void UpdateConsumables() {}
void UpdateHammer(){}

//			ENEMY RELATED:

//Used to update boss state
void UpdateBossState() {}

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
		Play::DrawObject(goombaObj);
	}
}

//Used to update dry bones enemies
void UpdateDryBones() 
{
	GameObject& playerObj = Play::GetGameObjectByType(typePlayer);
	GameObject& hammerObj = Play::GetGameObjectByType(typeHammer);
	std::vector<int> vDryBones = Play::CollectGameObjectIDsByType(typeDryBones);
}

//Used to update bobbomb enemies
void UpdateBobBombs() 
{
	GameObject& playerObj = Play::GetGameObjectByType(typePlayer);
	GameObject& hammerObj = Play::GetGameObjectByType(typeHammer);
	std::vector<int> vBobBombs = Play::CollectGameObjectIDsByType(typeBobBomb);

	for (int bobBombID: vBobBombs)
	{
		GameObject& bobBombObj = Play::GetGameObject(bobBombID);
		bool isAlight = false;

		if (gameState.playerState == PlayerState::playerNotDebuffed || gameState.playerState == PlayerState::playerDebuffed)
		{
			if (FindDistance(playerObj, bobBombObj) < 25 || isAlight == false) 
			{
				isAlight = true;
				bobBombObj.velocity *= 1.5;
				int wait = 0;

				for (int i = 0; i < 120; i++) 
				{
					wait++;
				}

				if (wait == 119) 
				{
					bobBombObj.velocity = { 0,0 };
					Play::SetSprite(bobBombObj, "bobbomb_walk_s_a_8", 0.25f);
				}

				wait = 0;

				for (int i = 0; i < 60; i++)
				{
					wait++;
				}

				if (wait == 59)
				{
					Play::DrawObjectTransparent(bobBombObj, 0.0f);

					int explosion1ID = Play::CreateGameObject(typeBobBombExplosion, bobBombObj.pos, 15 , "explosion_12");
					int explosion2ID = Play::CreateGameObject(typeBobBombExplosion, bobBombObj.pos, 15, "explosion_12");
					int explosion3ID = Play::CreateGameObject(typeBobBombExplosion, bobBombObj.pos, 15, "explosion_12");

					GameObject& explosion1Obj = Play::GetGameObject(explosion1ID);
					GameObject& explosion2Obj = Play::GetGameObject(explosion2ID);
					GameObject& explosion3Obj = Play::GetGameObject(explosion3ID);

					explosion1Obj.scale = GenRandomNumRange(0.7, 1.9);
					explosion2Obj.scale = GenRandomNumRange(0.7, 1.9);
					explosion3Obj.scale = GenRandomNumRange(0.7, 1.9);

					explosion1Obj.rotation = GenRandomNumRange(0.0, PLAY_PI);
					explosion2Obj.rotation = GenRandomNumRange(0.0, PLAY_PI);
					explosion3Obj.rotation = GenRandomNumRange(0.0, PLAY_PI);
				}
			}

			//Player collision
			//when alight
			//when not alight
			//when in a attackState

		}

		ScreenBouncing(bobBombObj, gameState.stage);

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
		Play::UpdateGameObject(bobBombObj);
		Play::DrawObject(bobBombObj);
	}
}

//Used to update magikoopa enemy
void UpdateMagiKoopa() 
{
	//if player is x is less than mk x then launch projectile left else right
	GameObject& playerObj = Play::GetGameObjectByType(typePlayer);
	GameObject& hammerObj = Play::GetGameObjectByType(typeHammer);
	std::vector<int> vMagiKoopas = Play::CollectGameObjectIDsByType(typeMagiKoopa);
}

//Used to update enemy projectiles and effects
void UpdateEnemyProjectiles() {}

//			STAGE RELATED:

void UpdateStageState() {}

//          MISCELLANEOUS:

//Used to update destroyed objects
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
		playerObj.velocity = { 0,-4 };
		Play::SetSprite(playerObj, "mario_walk_n_12", 0.35f);
		playerOrientation = vOrientations.at(0);
	}
	else if (Play::KeyDown(VK_RIGHT)) 
	{
		playerObj.velocity = { 4,0 };
		Play::SetSprite(playerObj, "mario_walk_e_12", 0.35f);
		playerOrientation = vOrientations.at(1);
	}
	else if (Play::KeyDown(VK_DOWN)) 
	{
		playerObj.velocity = { 0,4 };
		Play::SetSprite(playerObj, "mario_walk_s_12", 0.35f);
		playerOrientation = vOrientations.at(2);
	}
	else if (Play::KeyDown(VK_LEFT)) 
	{
		playerObj.velocity = { -4,0 };
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
			int goombaID = Play::CreateGameObject(typeGoomba, GetRandomPositionInPS(stage), 15, "goomba_win_s_24");
			GameObject& goombaObj = Play::GetGameObject(goombaID);
			SetVelocity(goombaObj, "x");
		}

		int bobbombID = Play::CreateGameObject(typeBobBomb, GetRandomPositionInPS(stage), 15, "bobbomb_walk_s_8");
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

//Used to set sprite direction
void SetSpriteDirection(GameObject& gameObj) 
{
	if (gameObj.type == typeGoomba) 
	{
		if (gameObj.velocity.x < 0)
		{
			Play::SetSprite(gameObj, "goomba_walk_l_8", 0.25f);
		}
		else if (gameObj.velocity.x > 0)
		{
			Play::SetSprite(gameObj, "goomba_walk_r_8", 0.25f);
		}
	}
	else if (gameObj.type == typeBobBomb)
	{

	}
	else if (gameObj.type == typeMagiKoopa)
	{

	}
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

	float distance = sqrt(pow((x2 -x1),2) + pow((y2 - y1), 2));
	return distance;
}

//Generate Random number between two float numbers
float GenRandomNumRange(float lowerNum, float upperNum) 
{
	std::random_device rd;
	std::uniform_real_distribution<double> dist(lowerNum, upperNum);
	return dist(rd);
}


//-------------------------------------------------------------------------------------------------------------------------------
//												DEBUG FUNCTIONS	

void ShowDebugUI() {}
