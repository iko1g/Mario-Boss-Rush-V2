#pragma once

#define displayWidth 1280
#define displayHeight 720
#define displayScale 1

enum RoundState
{
	tutorialRound = 0,
	normalRound,
	bossRound,
	roundLose,
	roundWin,
};

enum PlayerState
{
	playerDefault = 0,
	playerAppear,
	playerNotDebuffed,
	playerDebuffed,
	playerTransform,
	playerDamaged,
	playerHeal,
	playerAttack,
	playerDead,
	playerPowerUp,
};

enum BossState
{
	bossDefault = 0,
	bossAppear,
	bossIdle,
	bossAgro,
	bossDamaged,
	bossSummonMob,
	bossDead,
	bossWin,
};

enum GameObjectType
{
	typeNull = -1,
	//Player Related Types
	typePlayer,
	typeHammer,
	typePlayerHealthIcon,
	typeGoldenMushroom,
	typeRefreshingHerb,
	typeHealth1UP,
	typePowerUpIcon,
	//Enemy Related Types
	typeGoomba,
	typeBobBombNotAlight,
	typeBobBombAlight,
	typeBobBombExplosion,
	typeDryBones,
	typeDryBoneBody,
	typeDryBoneHead,
	typeMagiKoopa,
	typeMagiKoopaProj,
	typeBowser,
	typeBowserAttackIcon,
	typeBowserHealthIcon,
	typeBowseLRP,
	typeBowseSRP,
	typeEnemySpawnParticle,
	typeEnemyDeathParticle,
	//Miscellaneous Types
	typeDestroyed,
	typeSpikes,
	typeTutScreen,
};

struct GameState
{
	int bossHp = 4;
	int playerHP = 3;
	int score = 0;
	int timePassed = 0;
	std::string playerOrientation{};
	bool playerisDebuffed = false;
	bool debugging = false;
	bool isBossAtkLngRng = false;
	PlayerState playerState = PlayerState::playerDefault;
	BossState bossState = BossState::bossDefault;
	RoundState roundState = RoundState::tutorialRound;
	const std::vector<std::string> vOrientations = { "N", "E", "S", "W" };
};

extern GameState gameState;

//-------------------------------------------------------------------------------------------------------------------------------
//												UPDATE FUNCTIONS

//			STAGE RELATED:

void UpdateRoundState();
void UpdateSpikes();
void UpdateTutScreen();

//          MISCELLANEOUS:

void UpdateDestroyed();
void UpdateUI();

//-------------------------------------------------------------------------------------------------------------------------------
//												UTILITY FUNCTIONS	

//          OBJECT RELATED:

void ScreenBouncing(GameObject&, bool);
void SetVelocity(GameObject&, std::string);
Point2f GetRandomPositionInPS();
void DecelerateObject(GameObject&, float);
void ClearEnemiesAndItems();
void ClearHealthIcons();

//			UI RELATED:

void CreateUI();

//			GAME ENTRY RELATED:
void CreateDefaultGameObjects();

//          MISCELLANEOUS:

int PickBetween(int, int);
float FindDistance(GameObject&, GameObject&);
float GenRandomNumRange(float, float);
bool IsInBetween(float, float, float);

//-------------------------------------------------------------------------------------------------------------------------------
//												SPAWN FUNCTIONS	

void SpawnEnemies();
void SpawnFireballs(GameObject&, char);
void SpawnPlayerConsumables();

