#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"
#include "MainGame.h"
#include "Player.h"
#include "Boss.h"
#include "Enemies.h"
#include "Debug.h"
#include <random>

GameState gameState;

//-------------------------------------------------------------------------------------------------------------------------------

// The entry point for a PlayBuffer program
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	Play::CreateManager( displayWidth, displayHeight, displayScale );
	Play::CentreAllSpriteOrigins();
	Play::LoadBackground("Data\\Backgrounds\\stage1bgn.png");
	CreateDefaultGameObjects();
	Play::StartAudioLoop("tutorial");
	CreateUI();
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate( float elapsedTime )
{
	static float fTotalGameTime = 0.f;
	fTotalGameTime += elapsedTime;
	gameState.timePassed = fTotalGameTime;
	Play::DrawBackground();
	UpdateDestroyed();
	UpdateGoombas();
	UpdateBobBombs();
	UpdateMagiKoopa();
	UpdateDryBones();
	UpdateEnemyParticles();
	UpdateBossProjectiles();
	UpdateConsumables();
	UpdateBossIcons();
	UpdateRoundState();
	UpdateHammer();
	UpdateSpikes();
	UpdateUI();

	if (gameState.debugging) 
	{ 
		ShowDebugUI(); 
	}
	
	Play::PresentDrawingBuffer();
	return Play::KeyDown( VK_ESCAPE );
}

// Gets called once when the player quits the game 
int MainGameExit( void )
{
	Play::DestroyManager();
	return PLAY_OK;
}


//			STAGE RELATED:

//Used to update round state
void UpdateRoundState() 
{
	GameObject& playerObj = Play::GetGameObjectByType(typePlayer);
	GameObject& bowserObj = Play::GetGameObjectByType(typeBowser);

	switch (gameState.roundState) 
	{
	case RoundState::tutorialRound:
	{
		UpdateTutScreen();

		if (Play::KeyDown(VK_RETURN)) 
		{
			Play::StopAudioLoop("tutorial");
			Play::StartAudioLoop("normal1");
			gameState.playerState = PlayerState::playerAppear;
			gameState.roundState = RoundState::normalRound;
		}
	}
	break;

	case RoundState::normalRound:
	{
		UpdatePlayerState();

		//enemy list is empty wait 3 seconds and then spawn enemies
		if (Play::CollectGameObjectIDsByType(typeGoomba).size() + Play::CollectGameObjectIDsByType(typeBobBombNotAlight).size() + Play::CollectGameObjectIDsByType(typeBobBombAlight).size() + Play::CollectGameObjectIDsByType(typeDryBones).size() + Play::CollectGameObjectIDsByType(typeMagiKoopa).size() == 0) 
		{
			if (gameState.timePassed % 4 == 0) 
			{
				SpawnEnemies();
			}
		}

		//every 9 seconds if needed spawn a consumable if the requirements are met to spawn that consumable
		if (gameState.timePassed % 10 == 0 && gameState.timePassed > 0)
		{
			SpawnPlayerConsumables();
		}

		//if score is more than 9
		if (gameState.score > 9)
		{
			gameState.bossState = BossState::bossAppear;
			gameState.roundState = RoundState::bossRound;
			Play::StopAudioLoop("normal1");
			Play::StartAudioLoop("boss1");
			//stop normal round audio and start boss round audio
		}
	}
	break;
	case RoundState::bossRound:
	{
		UpdatePlayerState();
		UpdateBossState();
	}
	break;
	case RoundState::roundLose:
	{
		Play::StopAudioLoop("boss1");
		ClearEnemiesAndItems();
		ClearHealthIcons();

		playerObj.velocity = { 0,0 };
		Play::SetSprite(playerObj, "mario_shock_6", 0.25f);

		Play::UpdateGameObject(playerObj);
		Play::DrawObject(playerObj);

		Play::SetSprite(bowserObj, "bowser_win_16", 0.25f);

		Play::UpdateGameObject(bowserObj);
		Play::DrawObject(bowserObj);

		//Reset game elements
		if (Play::KeyPressed(VK_RETURN) == true)
		{
			gameState.bossHp = 4;
			gameState.playerHP = 3;
			gameState.score = 0;
			gameState.playerOrientation = "";
			gameState.playerisDebuffed = false;
			gameState.debugging = false;
			gameState.isBossAtkLngRng = false;
			CreateUI();

			//stop lose audio loop
			Play::StartAudioLoop("tutorial");

			//Delete all enemies and move boss to off screen
			playerObj.pos = { -50 ,500 };
			bowserObj.pos = { displayWidth + 75 , 500 };

			gameState.playerState = PlayerState::playerDefault;
			gameState.bossState = BossState::bossDefault;
			gameState.roundState = RoundState::tutorialRound;
		}
	}
	break;
	case RoundState::roundWin:
	{
		Play::StopAudioLoop("boss1");
		ClearEnemiesAndItems();
		ClearHealthIcons();
		UpdatePlayerState();
	
		Play::SetSprite(bowserObj, "bowser_dead_15", 0.25f);
		Play::UpdateGameObject(bowserObj);
		Play::DrawObject(bowserObj);

		//Reset game elements
		if (Play::KeyPressed(VK_RETURN) == true)
		{
			gameState.bossHp = 4;
			gameState.playerHP = 3;
			gameState.score = 0;
			gameState.playerOrientation = "";
			gameState.playerisDebuffed = false;
			gameState.debugging = false;
			gameState.isBossAtkLngRng = false;
			CreateUI();

			//stop win audio loop
			Play::StartAudioLoop("tutorial");

			//Delete all enemies and move boss to off screen
			playerObj.pos = { -50 ,500 };
			bowserObj.pos = { displayWidth + 75 , 500 };

			gameState.playerState = PlayerState::playerDefault;
			gameState.bossState = BossState::bossDefault;
			gameState.roundState = RoundState::tutorialRound;
		}
	}
	break;
	}
}


//Used to draws spikes
void UpdateSpikes() 
{
	//Only used for drawing bottom spikes on the bottom of the screen
	//so player can walk 'behind' it
	GameObject& spikeObj = Play::GetGameObjectByType(typeSpikes);
	Play::UpdateGameObject(spikeObj);
	Play::DrawObject(spikeObj);
}

//Used to draw tutorial screens
void UpdateTutScreen() 
{
	GameObject& tutScreenObj = Play::GetGameObjectByType(typeTutScreen);
	Play::UpdateGameObject(tutScreenObj);
	Play::DrawObject(tutScreenObj);
}

//          MISCELLANEOUS:

//Used to update destroyed objects ()
void UpdateDestroyed() 
{
	std::vector<int> vDestroyed = Play::CollectGameObjectIDsByType(typeDestroyed);

	for (int desID : vDestroyed)
	{
		//Idealy used for objects where animation only has 1 frame --> so it gives flashing effect before deleting
		//but set anim speed to 0.2f
		GameObject& desObj = Play::GetGameObject(desID);
		desObj.animSpeed = 0.2f;
		Play::UpdateGameObject(desObj);

		if (desObj.frame % 2 == 0)
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
void UpdateUI() 
{
	GameObject& bowserObj = Play::GetGameObjectByType(typeBowser);
	GameObject& playerObj = Play::GetGameObjectByType(typePlayer);
	GameObject& powerUpIconObj = Play::GetGameObjectByType(typePowerUpIcon);
	std::vector<int> vPlayerHealthIcons = Play::CollectGameObjectIDsByType(typePlayerHealthIcon);
	std::vector<int> vBossHealthIcons = Play::CollectGameObjectIDsByType(typeBowserHealthIcon);

	//Player Icons

	//Player Health
	if (gameState.roundState != RoundState::tutorialRound) 
	{
		for (int playerHealthIconID : vPlayerHealthIcons)
		{
			GameObject& playerHealthIconObj = Play::GetGameObject(playerHealthIconID);
			Play::UpdateGameObject(playerHealthIconObj);
			Play::DrawObjectRotated(playerHealthIconObj);
		}
	}

	//Powerup
	powerUpIconObj.pos = { playerObj.pos.x , playerObj.pos.y - 40 };
	if (gameState.playerState == PlayerState::playerPowerUp) 
	{
		Play::DrawObject(powerUpIconObj);
	}
	Play::UpdateGameObject(powerUpIconObj);

	//Text

	//Score and Health
	if (gameState.roundState == RoundState::normalRound ) 
	{
		Play::DrawFontText("SuperMario25636px_10x10", "SCORE " + std::to_string(gameState.score), { displayWidth / 2 , 25 }, Play::CENTRE);
		Play::DrawFontText("SuperMario25636px_10x10", "HEALTH", { displayWidth / 2 , displayHeight - 25 }, Play::CENTRE);
	}
	else if (gameState.roundState == RoundState::bossRound) 
	{
		Play::DrawFontText("SuperMario25636px_10x10", "SCORE " + std::to_string(gameState.score), { displayWidth / 2 , 25 }, Play::CENTRE);
		Play::DrawFontText("SuperMario25636px_10x10", "HEALTH", { displayWidth / 2 , displayHeight - 25 }, Play::CENTRE);
		Play::DrawFontText("SuperMario25636px_10x10", "BOSS HEALTH", { displayWidth - 150 ,  displayHeight - 25 }, Play::CENTRE);
	}
	else if (gameState.roundState == RoundState::roundLose || gameState.roundState == RoundState::roundWin)
	{
		Play::DrawFontText("SuperMario25636px_10x10", "SCORE " + std::to_string(gameState.score), { displayWidth / 2 , 25 }, Play::CENTRE);
	}
	//Play Again
	if (gameState.roundState == RoundState::roundLose) 
	{
		Play::DrawFontText("SuperMario25636px_10x10", "YOU LOST", { displayWidth / 2 , (displayHeight / 2) - 60 }, Play::CENTRE);
		Play::DrawFontText("SuperMario25636px_10x10", "PRESS ENTER TO PLAY AGAIN", { displayWidth / 2 , (displayHeight / 2) - 30 }, Play::CENTRE);
	}
	else if (gameState.roundState == RoundState::roundWin) 
	{
		Play::DrawFontText("SuperMario25636px_10x10", "YOU WON", { displayWidth / 2 , (displayHeight / 2) - 60 }, Play::CENTRE);
		Play::DrawFontText("SuperMario25636px_10x10", "PRESS ENTER TO PLAY AGAIN", { displayWidth / 2 , (displayHeight / 2) - 30 }, Play::CENTRE);
	}

	//Boss Health Icons
	if (bowserObj.pos.x < displayWidth - 150) 
	{
		for (int bossHealthIconID : vBossHealthIcons)
		{
			GameObject& bossHealthIconObj = Play::GetGameObject(bossHealthIconID);
			Play::UpdateGameObject(bossHealthIconObj);
			Play::DrawObjectRotated(bossHealthIconObj);
		}
	}
}



//-------------------------------------------------------------------------------------------------------------------------------
//												SPAWN FUNCTIONS	

//Used to spawn enemies in initial stage
void SpawnEnemies() 
{

	if (gameState.roundState == RoundState::normalRound) 
	{
		//Create enemyspawnparticle
		// spawn goomba on enemyspawnparticle pos
		for (int i = 0; i < 2; i++)
		{
			int spawnParticleID = Play::CreateGameObject(typeEnemySpawnParticle, GetRandomPositionInPS(), 0, "spawn_particle_13");
			GameObject& spawnParticleObj = Play::GetGameObject(spawnParticleID);
			spawnParticleObj.animSpeed = 0.55f;
			int goombaID = Play::CreateGameObject(typeGoomba, spawnParticleObj.pos, 10, "goomba_walk_e_8");
			GameObject& goombaObj = Play::GetGameObject(goombaID);
			SetVelocity(goombaObj, "x");
		}

		//Create enemyspawnparticle
		// spawn bobbomb on enemyspawnparticle pos
		for (int i = 0; i < 2; i++)
		{
			int spawnParticleID = Play::CreateGameObject(typeEnemySpawnParticle, GetRandomPositionInPS(), 0, "spawn_particle_13");
			GameObject& spawnParticleObj = Play::GetGameObject(spawnParticleID);
			spawnParticleObj.animSpeed = 0.55f;
			int bobbombID = Play::CreateGameObject(typeBobBombNotAlight, spawnParticleObj.pos, 10, "bobbomb_walk_e_8");
			GameObject& bobBombObj = Play::GetGameObject(bobbombID);
			SetVelocity(bobBombObj, "x");
		}

		//Create enemyspawnparticle
		//Spawn/Create 1 drybones or magikoopa enemy on enemy spawn particle based on the chance variable
		int chance = PickBetween(1, -1);

		if (chance == 1)
		{
			int spawnParticleMKID = Play::CreateGameObject(typeEnemySpawnParticle, GetRandomPositionInPS(), 0, "spawn_particle_13");
			GameObject& spawnParticleMKObj = Play::GetGameObject(spawnParticleMKID);
			spawnParticleMKObj.animSpeed = 0.55f;
			int magiKoopaID = Play::CreateGameObject(typeMagiKoopa, spawnParticleMKObj.pos, 15, "magikoopa_s_8");
			GameObject& magiKoopaObj = Play::GetGameObject(magiKoopaID);
			SetVelocity(magiKoopaObj, "y");
		}
		else
		{
			int spawnParticleDBID = Play::CreateGameObject(typeEnemySpawnParticle, GetRandomPositionInPS(), 0, "spawn_particle_13");
			GameObject& spawnParticleDBObj = Play::GetGameObject(spawnParticleDBID);
			spawnParticleDBObj.animSpeed = 0.55f;
			int dryBonesID = Play::CreateGameObject(typeDryBones, spawnParticleDBObj.pos, 15, "drybones_s_16");
			GameObject& dryBonesObj = Play::GetGameObject(dryBonesID);
			SetVelocity(dryBonesObj, "both");
		}
	}

	else if (gameState.roundState == RoundState::bossRound) 
	{

		int bossSummonChance = PickBetween(1, -1);

		if (bossSummonChance == 1) 
		{
			int spawnParticleMK2ID = Play::CreateGameObject(typeEnemySpawnParticle, { 40 , Play::RandomRollRange(395, 605) }, 0, "spawn_particle_13");
			GameObject& spawnParticleMK2Obj = Play::GetGameObject(spawnParticleMK2ID);
			spawnParticleMK2Obj.animSpeed = 0.55f;
			int magiKoopaID = Play::CreateGameObject(typeMagiKoopa, spawnParticleMK2Obj.pos, 15, "magikoopa_s_8");
			GameObject& magiKoopaObj = Play::GetGameObject(magiKoopaID);
			SetVelocity(magiKoopaObj, "y");
		}
		else if (bossSummonChance == -1) 
		{
			int spawnParticleID = Play::CreateGameObject(typeEnemySpawnParticle, GetRandomPositionInPS(), 0, "spawn_particle_13");
			GameObject& spawnParticleObj = Play::GetGameObject(spawnParticleID);
			spawnParticleObj.animSpeed = 0.55f;
			int goombaID = Play::CreateGameObject(typeGoomba, spawnParticleObj.pos, 10, "goomba_walk_e_8");
			GameObject& goombaObj = Play::GetGameObject(goombaID);
			SetVelocity(goombaObj, "x");
		}
	}
}

// Used to set fireball sheild at an offset
void SpawnFireballs(GameObject& gameObj, char dir)
{

	if (dir == 'n' || dir == 'N')
	{
		for (float rad{ 0.25f }; rad < 2.0f; rad += 0.25f)
		{
			int shortRangeProjID = Play::CreateGameObject(typeBowseSRP, { gameObj.pos.x + 80 * cos(rad + 1.16f * PLAY_PI), gameObj.pos.y + 80 * sin(rad + 1.16f * PLAY_PI) }, 0, "bowser_srproj_4");
			GameObject& shortRangeProj = Play::GetGameObject(shortRangeProjID);
			shortRangeProj.animSpeed = 0.25f;
		}
	}
	else if (dir == 'e' || dir == 'E')
	{
		for (float rad{ 0.25f }; rad < 2.0f; rad += 0.25f)
		{
			int shortRangeProjID = Play::CreateGameObject(typeBowseSRP, { gameObj.pos.x + 80 * cos(rad + 1.7f * PLAY_PI), gameObj.pos.y + 80 * sin(rad + 1.7f * PLAY_PI) }, 0, "bowser_srproj_4");
			GameObject& shortRangeProj = Play::GetGameObject(shortRangeProjID);
			shortRangeProj.animSpeed = 0.25f;
		}
	}
	else if (dir == 's' || dir == 'S')
	{
		for (float rad{ 0.25f }; rad < 2.0f; rad += 0.25f)
		{
			int shortRangeProjID = Play::CreateGameObject(typeBowseSRP, { gameObj.pos.x + 80 * cos(rad + 2.2f * PLAY_PI), gameObj.pos.y + 80 * sin(rad + 2.2f * PLAY_PI) }, 0, "bowser_srproj_4");
			GameObject& shortRangeProj = Play::GetGameObject(shortRangeProjID);
			shortRangeProj.animSpeed = 0.25f;
		}
	}
	else if (dir == 'w' || dir == 'W')
	{
		for (float rad{ 0.25f }; rad < 2.0f; rad += 0.25f)
		{
			int shortRangeProjID = Play::CreateGameObject(typeBowseSRP, { gameObj.pos.x + 80 * cos(rad + 0.75f * PLAY_PI), gameObj.pos.y + 80 * sin(rad + 0.75f * PLAY_PI) }, 0, "bowser_srproj_4");
			GameObject& shortRangeProj = Play::GetGameObject(shortRangeProjID);
			shortRangeProj.animSpeed = 0.25f;
		}
	}
}


//used to spawn player consumables
void SpawnPlayerConsumables() 
{
	if (gameState.roundState == RoundState::normalRound) 
	{
		//if there are more than 5 enemies on screen and there isnt a golden mushroom on screen then spawn a golden mushroom
		//maybe instead of when plkayer is alive we spawn golden mushroom when theres a lot of enemies on screen
		if (Play::CollectGameObjectIDsByType(typeGoomba).size() + Play::CollectGameObjectIDsByType(typeBobBombNotAlight).size() + Play::CollectGameObjectIDsByType(typeBobBombAlight).size() + Play::CollectGameObjectIDsByType(typeDryBones).size() + Play::CollectGameObjectIDsByType(typeMagiKoopa).size() > 5 && Play::CollectGameObjectIDsByType(typeGoldenMushroom).size() < 1)
		{
			int goldenMushroomID = Play::CreateGameObject(typeGoldenMushroom, GetRandomPositionInPS(), 20, "invincible_powerup");
			GameObject& goldenMushroomObj = Play::GetGameObject(goldenMushroomID);
			SetVelocity(goldenMushroomObj, "both");
		}

		//if player health is less than 3 and there isnt a health pickups on screen then spawn a healthup
		else if (gameState.playerHP < 3 && Play::CollectGameObjectIDsByType(typeHealth1UP).size() < 1)
		{
			int healthUpID = Play::CreateGameObject(typeHealth1UP, GetRandomPositionInPS(), 20, "health_up");
			GameObject& healthUpObj = Play::GetGameObject(healthUpID);
			SetVelocity(healthUpObj, "both");
		}

		//if player is debuffed and there isnt a refreshing herb already on screen then spawn refreshing herb
		else if (gameState.playerState == PlayerState::playerDebuffed && Play::CollectGameObjectIDsByType(typeRefreshingHerb).size() < 1)
		{
			int refreshingHerbID = Play::CreateGameObject(typeRefreshingHerb, GetRandomPositionInPS(), 20, "refreshing_herb");
			GameObject& refreshingHerbObj = Play::GetGameObject(refreshingHerbID);
			SetVelocity(refreshingHerbObj, "both");
		}
	}
	else if (gameState.roundState == RoundState::bossRound)
	{
		 if (gameState.playerState == PlayerState::playerDebuffed && Play::CollectGameObjectIDsByType(typeRefreshingHerb).size() < 1)
		{
			int refreshingHerbID = Play::CreateGameObject(typeRefreshingHerb, GetRandomPositionInPS(), 20, "refreshing_herb");
			GameObject& refreshingHerbObj = Play::GetGameObject(refreshingHerbID);
			SetVelocity(refreshingHerbObj, "both");
		}
		 else if (gameState.playerHP < 3 && Play::CollectGameObjectIDsByType(typeHealth1UP).size() < 1)
		 {
			 int healthUpID = Play::CreateGameObject(typeHealth1UP, GetRandomPositionInPS(), 20, "health_up");
			 GameObject& healthUpObj = Play::GetGameObject(healthUpID);
			 SetVelocity(healthUpObj, "both");
		 }
	}
}

//-------------------------------------------------------------------------------------------------------------------------------
//												UTILITY FUNCTIONS	

//          OBJECT RELATED:

//Used to bounce certain objects so they stay in the play space
void ScreenBouncing(GameObject& gameObj, bool choice) 
{
	if (choice == true) 
	{
		//If player is leaving play space horizontally then set old poss current pos
		if (gameObj.type == typePlayer) 
		{
			if (gameObj.pos.x < 0)
			{
				gameObj.pos = gameObj.oldPos;
			}
			else if (gameObj.pos.x > displayWidth) 
			{
				gameObj.pos = gameObj.oldPos;
			}

			//If player is leaving play space vertically then set old poss current pos
			if (gameObj.pos.y > 605)
			{
				gameObj.pos = gameObj.oldPos;
			}
			else if (gameObj.pos.y < 395)
			{
				gameObj.pos = gameObj.oldPos;
			}
		}
		else 
		{
			//else for everyother gameobject type 
			//If object is leaving play space horizontally then set velocity.x to -1 of what it is
			if (gameObj.pos.x < 0)
			{
				gameObj.velocity.x *= -1;
			}
			else if (gameObj.pos.x > displayWidth)
			{
				gameObj.velocity.x *= -1;
			}

			//If object is leaving play space vertically then set velocity.y to -1 of what it is
			if (gameObj.pos.y > 605)
			{
				gameObj.velocity.y *= -1;
			}
			else if (gameObj.pos.y < 395) 
			{
				gameObj.velocity.y *= -1;
			}
		}
	}
}

//Used to set an objects velocity to soley x, y or a combination of the two
void SetVelocity(GameObject& gameObj, std::string choice) 
{
	//Will set gameobject velocity.x to either -1 or 1  --> only moving horizontally
	if (choice == "x")
	{
		gameObj.velocity = Point2f(PickBetween(1,-1)* 2 , 0);
	}

	//Will set gameobject velocity.y to either -1 or 1 --> only moving vertically
	else if (choice == "y")
	{
		gameObj.velocity = Point2f(0, PickBetween(1, -1) * 2);
	}

	//Will set gameobject velocity.x and y to either -1 or 1 --> moving diagonally
	else if (choice == "both")
	{
		gameObj.velocity = Point2f(PickBetween(1, -1) * 2, PickBetween(1, -1) * 2);
	}
}

//Used to get a point2f value that is the play space of the relevant stage
Point2f GetRandomPositionInPS() 
{
	//returns a random position in the play space
	return Point2f(Play::RandomRollRange(0, displayWidth) , Play::RandomRollRange(395, 605));
}

//Used to decelerate game objects
void DecelerateObject(GameObject& gameObj, float rate) 
{
	//Will decelerate object based on the rate
	gameObj.velocity *= rate;
	gameObj.acceleration = {0,0};
}

//Used to clear/destroy all the enemies and effects on screen
void ClearEnemiesAndItems() 
{
	//Loop through consumables vectors and delete gameobjects
	for (int healthUpID : Play::CollectGameObjectIDsByType(typeHealth1UP))
	{
		Play::GetGameObject(healthUpID).type = typeDestroyed;
	}

	for (int refreshingHerbID : Play::CollectGameObjectIDsByType(typeRefreshingHerb))
	{
		Play::GetGameObject(refreshingHerbID).type = typeDestroyed;
	}

	for (int goldenMushID : Play::CollectGameObjectIDsByType(typeGoldenMushroom))
	{
		Play::GetGameObject(goldenMushID).type = typeDestroyed;
	}

	//Loop through enemy and enemy projectiles/effects vectors and delete gameobjects
	for (int goombaID : Play::CollectGameObjectIDsByType(typeGoomba))
	{
		Play::GetGameObject(goombaID).type = typeDestroyed;
	}

	for (int bombaAID : Play::CollectGameObjectIDsByType(typeBobBombAlight))
	{
		Play::GetGameObject(bombaAID).type = typeDestroyed;
	}

	for (int bombaNAID : Play::CollectGameObjectIDsByType(typeBobBombNotAlight))
	{
		Play::GetGameObject(bombaNAID).type = typeDestroyed;
	}

	for (int bombaEx : Play::CollectGameObjectIDsByType(typeBobBombExplosion))
	{
		Play::GetGameObject(bombaEx).type = typeDestroyed;
	}

	for (int dryBonesID : Play::CollectGameObjectIDsByType(typeDryBones))
	{
		Play::GetGameObject(dryBonesID).type = typeDestroyed;
	}

	for (int dryBonesHeadID : Play::CollectGameObjectIDsByType(typeDryBoneHead))
	{
		Play::GetGameObject(dryBonesHeadID).type = typeDestroyed;
	}

	for (int dryBonesBodyID : Play::CollectGameObjectIDsByType(typeDryBoneBody))
	{
		Play::GetGameObject(dryBonesBodyID).type = typeDestroyed;
	}

	for (int magiKoopaID : Play::CollectGameObjectIDsByType(typeMagiKoopa))
	{
		Play::GetGameObject(magiKoopaID).type = typeDestroyed;
	}

	for (int magiKoopaProjID : Play::CollectGameObjectIDsByType(typeMagiKoopaProj))
	{
		Play::GetGameObject(magiKoopaProjID).type = typeDestroyed;
	}
}

//Used to clear health Icons after game has been restarted
void ClearHealthIcons() 
{
	//Loop through health icon vectors and delete gameobjects
	for (int playerHealthIconID : Play::CollectGameObjectIDsByType(typePlayerHealthIcon))
	{
		Play::GetGameObject(playerHealthIconID).type = typeDestroyed;
	}

	for (int bossHealthIconID : Play::CollectGameObjectIDsByType(typeBowserHealthIcon))
	{
		Play::GetGameObject(bossHealthIconID).type = typeDestroyed;
	}
}

//			UI RELATED:

//Used to draw UI
void CreateUI() 
{
	//Create 3 player healths icon gameObjects 
	int healthIcon1ID = Play::CreateGameObject(typePlayerHealthIcon, { displayWidth / 2 - 45, displayHeight - 65 }, 0, "health_icon");
	int healthIcon2ID = Play::CreateGameObject(typePlayerHealthIcon, { displayWidth / 2, displayHeight - 65 }, 0, "health_icon");
	int healthIcon3ID = Play::CreateGameObject(typePlayerHealthIcon, { displayWidth / 2 + 45, displayHeight - 65 }, 0, "health_icon");

	GameObject& healthIcon1Obj = Play::GetGameObject(healthIcon1ID);
	GameObject& healthIcon2Obj = Play::GetGameObject(healthIcon2ID);
	GameObject& healthIcon3Obj = Play::GetGameObject(healthIcon3ID);

	healthIcon1Obj.scale = healthIcon2Obj.scale = healthIcon3Obj.scale = 1.5;

	//Create 4 boss healths icon gameObjects 
	int bossHealthIcon1ID = Play::CreateGameObject(typeBowserHealthIcon, { 1058,  displayHeight - 65 }, 0, "bowser_hp_icon");
	int bossHealthIcon2ID = Play::CreateGameObject(typeBowserHealthIcon, { 1106,  displayHeight - 65 }, 0, "bowser_hp_icon");
	int bossHealthIcon3ID = Play::CreateGameObject(typeBowserHealthIcon, { 1154,  displayHeight - 65 }, 0, "bowser_hp_icon");
	int bossHealthIcon4ID = Play::CreateGameObject(typeBowserHealthIcon, { 1202,  displayHeight - 65 }, 0, "bowser_hp_icon");

	GameObject& bossHealthIcon1Obj = Play::GetGameObject(bossHealthIcon1ID);
	GameObject& bossHealthIcon2Obj = Play::GetGameObject(bossHealthIcon2ID);
	GameObject& bossHealthIcon3Obj = Play::GetGameObject(bossHealthIcon3ID);
	GameObject& bossHealthIcon4Obj = Play::GetGameObject(bossHealthIcon4ID);

	bossHealthIcon1Obj.scale = bossHealthIcon2Obj.scale = bossHealthIcon3Obj.scale = bossHealthIcon4Obj.scale = 0.8;
	
	//Setting death particle to green
	Play::ColourSprite("death_particle_13", Play::cGreen);
}

//			GAME ENTRY RELATED:

// Used to create default gameobjects that will need to be present on game entry
void CreateDefaultGameObjects() 
{
	Play::CreateGameObject(typeSpikes, { displayWidth / 2 , displayHeight / 2 }, 0, "stage1bgn_spikes");
	Play::CreateGameObject(typeTutScreen, { displayWidth / 2 , displayHeight / 2 }, 0, "tut_controls1");
	Play::CreateGameObject(typePlayer, { -50 , 500 }, 15, "mario_idle_s_35");
	int powerUpID = Play::CreateGameObject(typePowerUpIcon, { Play::GetGameObjectByType(typePlayer).pos.x , Play::GetGameObjectByType(typePlayer).pos.y - 40 }, 0, "mario_shield");
	GameObject& powerupIconObj = Play::GetGameObject(powerUpID);
	powerupIconObj.scale = 0.5;
	Play::CreateGameObject(typeHammer, Play::GetGameObjectByType(typePlayer).pos, 33, "ham_mario_s_3");
	Play::CreateGameObject(typeBowser, { displayWidth + 75 , 500 }, 33, "bowser_walk_w_12");
	Play::CreateGameObject(typeBowserAttackIcon, { Play::GetGameObjectByType(typeBowser).pos.x , Play::GetGameObjectByType(typeBowser).pos.y - 30 }, 33, "peachvoice_summon_8");
}

//          MISCELLANEOUS:

//Used to pick between two numbers
int PickBetween(int num1, int num2) 
{
	//Will pick between two numbers based on chance
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

//Used to check if number is between two other numbers
bool IsInBetween(float upperBound, float lowerBound, float num)
{
	return (lowerBound <= num && num <= upperBound);
}
