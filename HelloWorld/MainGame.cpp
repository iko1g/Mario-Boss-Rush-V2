#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

int DISPLAY_WIDTH = 1280;
int DISPLAY_HEIGHT = 720;
int DISPLAY_SCALE = 1;

const std::vector<std::string> vOrientations = { "N", "E", "S", "W" };
std::string PlayerOrientation{};
bool playerisDebuffed;

enum PlayerState 
{
	playerAppear = 0,
	playerNotDebuffed,
	playerDebuffed,
	playerDamaged,
	playerHeal,
	playerHammer,
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
	normalStage = 0,
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
	typeDryBone,
	typeDryBoneBody,
	typeDryBoneHead,
	typeBowser,
	//Miscellaneous Types
	typeDestroyed,
};

struct GameState 
{
	int bossHp = 4;
	int playerHP = 3;
	int stage = 1;
	PlayerState playerState = PlayerState::playerAppear;
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
void UpdateBobBomb();
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
Point2f GetPositionInPS(int);
void DecelerateObject(GameObject&, float);
void SetSpriteDirection(GameObject&);

//			UI RELATED:
void DrawUI();
void UpdateStageBackground(int);

//          MISCELLANEOUS:
float PickBetween(float, float);

//-------------------------------------------------------------------------------------------------------------------------------
//												DEBUG FUNCTIONS	

// The entry point for a PlayBuffer program
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	Play::CreateManager( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE );
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate( float elapsedTime )
{
	Play::ClearDrawingBuffer( Play::cOrange );
	Play::DrawDebugText( { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 }, "Hello World!" );
	Play::PresentDrawingBuffer();
	return Play::KeyDown( VK_ESCAPE );
}

// Gets called once when the player quits the game 
int MainGameExit( void )
{
	Play::DestroyManager();
	return PLAY_OK;
}

