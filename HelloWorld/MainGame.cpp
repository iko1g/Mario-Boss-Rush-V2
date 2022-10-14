#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"
#include <random>

const int displayWidth = 1280;
const int displayHeight = 720;
const int displayScale = 1;

const std::vector<std::string> vOrientations = { "N", "E", "S", "W" };

enum PlayerState 
{
	playerAppear = 0,
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
	bossAppear = 0,
	bossIdle,
	bossAgro,
	bossDamaged,
	bossSummonMob,
	bossDead,
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
	typeBobBombNotAlight,
	typeBobBombAlight,
	typeBobBombExplosion,
	typeDryBones,
	typeDryBoneBody,
	typeDryBoneHead,
	typeMagiKoopa,
	typeMagiKoopaProj,
	typeBowser,
	//Miscellaneous Types
	typeDestroyed,
	typeSpikes,
};

struct GameState 
{
	int bossHp = 4;
	int playerHP = 3;
	int score = 0;
	int framesPassed = 0;
	int seconds = 0;
	std::string playerOrientation{};
	bool playerisDebuffed = false;
	bool debugging = false;
	PlayerState playerState = PlayerState::playerNotDebuffed;
	BossState bossState = BossState::bossAppear;
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
void UpdateGoombas();
void UpdateDryBones();
void UpdateBobBombs();
void UpdateMagiKoopa();

//			STAGE RELATED:

void UpdateSpikes();
//          MISCELLANEOUS:

void UpdateDestroyed();
void UpdateUI();
void UpdateTime();

//-------------------------------------------------------------------------------------------------------------------------------
//												HANDLE FUNCTIONS	

void HandleNotDebuffedControls();
void HandleDebuffedControls();
void HandleHammerAnimations();

//-------------------------------------------------------------------------------------------------------------------------------
//												SPAWN FUNCTIONS	

void SpawnEnemies();
void SpawnPlayerConsumables();

//-------------------------------------------------------------------------------------------------------------------------------
//												UTILITY FUNCTIONS	

//          OBJECT RELATED:

void ScreenBouncing(GameObject&, bool);
void SetVelocity(GameObject&, std::string);
Point2f GetRandomPositionInPS();
void DecelerateObject(GameObject&, float);
void ClearEnemiesOnScreen();

//			UI RELATED:

void CreateUI();

//          MISCELLANEOUS:

int PickBetween(int, int);
float FindDistance(GameObject&, GameObject&);
float GenRandomNumRange(float,float);
bool IsInBetween(float, float, float);

//-------------------------------------------------------------------------------------------------------------------------------
//												DEBUG FUNCTIONS	

void ShowDebugUI();

//-------------------------------------------------------------------------------------------------------------------------------

// The entry point for a PlayBuffer program
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	Play::CreateManager( displayWidth, displayHeight, displayScale );
	Play::CentreAllSpriteOrigins();
	Play::LoadBackground("Data\\Backgrounds\\stage1bgn.png");
	int spikeID = Play::CreateGameObject(typeSpikes, { displayWidth / 2 , displayHeight/ 2}, 0, "stage1bgn_spikes");
	Play::CreateGameObject(typePlayer, { displayWidth / 2, 505 }, 15, "mario_idle_s_35");
	int hammerID = Play::CreateGameObject(typeHammer, Play::GetGameObjectByType(typePlayer).pos, 33, "ham_mario_s_3");
	CreateUI();
	SpawnEnemies();
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate( float elapsedTime )
{
	gameState.framesPassed ++;
	Play::DrawBackground();
	Play::DrawFontText("SuperMario25636px_10x10", "SCORE " + std::to_string(gameState.score), {displayWidth /2 , 25 }, Play::CENTRE);
	Play::DrawFontText("SuperMario25636px_10x10", "HEALTH", { displayWidth /2 , displayHeight - 25 }, Play::CENTRE);
	UpdateTime();
	UpdateDestroyed();
	UpdateGoombas();
	UpdateBobBombs();
	UpdateMagiKoopa();
	UpdateDryBones();;
	UpdatePlayerState();
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
		//Play walks from left when x is certain value statenotdebuffed
		//or maybe just in case if enemy is spawned on player 
		//move to playerpowerup
	}
	break;
	case PlayerState::playerNotDebuffed:
	{
		HandleNotDebuffedControls();
	}
	break;
	case PlayerState::playerDebuffed:
	{
		HandleDebuffedControls();
	}
	break;
	case PlayerState::playerTransform:
	{
		//Used to move player from debuffed to notDebuffed state 
		//after they have collided with a magikoopa projectile or refreshing herb
		if (gameState.playerisDebuffed)
		{
			Play::SetSprite(playerObj, "mario_normaltofat_22", 0.35f);
			if (Play::IsAnimationComplete(playerObj)) 
			{
				gameState.playerState = PlayerState::playerDebuffed;
			}
		}
		else if (gameState.playerisDebuffed == false)
		{
			Play::SetSprite(playerObj, "mario_fattonormal_18", 0.25f);
			if (Play::IsAnimationComplete(playerObj))
			{
				gameState.playerState = PlayerState::playerNotDebuffed;
			}
		}
	}
	break;
	case PlayerState::playerDamaged:
	{
		std::vector<int> vHealthIcons = Play::CollectGameObjectIDsByType(typeHealthIcon);

		//Minus 1 player health and move to relevant move state
		if (gameState.playerHP - 1 == 0)
		{

			gameState.playerHP--;

			//Destory first object in list as they will have 1hp
			GameObject& frontHealthIconToDelObj = Play::GetGameObject(vHealthIcons.front());
			frontHealthIconToDelObj.type = typeDestroyed;

			gameState.playerState = PlayerState::playerDead;
		}
		else 
		{
			gameState.playerHP--;

			//Destroy the last healthIcon in the list
			GameObject& backHealthIconToDelObj = Play::GetGameObject(vHealthIcons.back());
			backHealthIconToDelObj.type = typeDestroyed;

			if (gameState.playerisDebuffed)
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
		std::vector<int> vHealthIcons = Play::CollectGameObjectIDsByType(typeHealthIcon);
		// Plus 1 to player health and move to relevant move state
		gameState.playerHP++;

		//Get health icon at the back
		GameObject& backHealthIconObj = Play::GetGameObject(vHealthIcons.back());
		
		//Create new health icon at an offset of the health icon at the back of the vector
		int healthIconToAddID = Play::CreateGameObject(typeHealthIcon, {backHealthIconObj.pos.x + 45,displayHeight - 65 }, 0, "health_icon");
		GameObject& addedHealthIconObj = Play::GetGameObject(healthIconToAddID);
		addedHealthIconObj.scale = 1.5;

		if (gameState.playerisDebuffed)
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
		playerObj.velocity = { 0,0 };
		Play::SetSprite(playerObj, "mario_shock_6", 0.25f);

		Play::DrawFontText("SuperMario25636px_10x10", "PRESS ENTER TO CONTINUE", { displayWidth / 2 , displayHeight / 2 }, Play::CENTRE);

		if (Play::KeyPressed(VK_RETURN) == true) 
		{
			gameState.bossHp = 4;
			gameState.playerHP = 3;
			gameState.score = 0;
			gameState.framesPassed = 0;
			gameState.seconds = 0;
			gameState.playerOrientation = "";
			gameState.playerisDebuffed = false;
			gameState.debugging = false;
			
			//start audio loop

			//Delete all enemies and move boss to off screen
			ClearEnemiesOnScreen();

			gameState.playerState = PlayerState::playerAppear;
			gameState.bossState = BossState::bossAppear;
		}
	}
	break;
	case PlayerState::playerPowerUp:
	{
		//player will be in this state for 5 seconds and will be invincible
		//have counter variable
		//keep adding to counter while in state
		//if counter % 4 = 0 colour player sprite yellow
		//else colour it white (clear colouring)
		//if 5 seconds have passed move to relevant debuff or nondebuff state
	}
	break;
	}

	if (gameState.playerState != PlayerState::playerAppear) 
	{
		ScreenBouncing(playerObj, true);
	}
	else 
	{
		ScreenBouncing(playerObj, false);
	}
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

	//Healthup object
	//If player collides with healthup then move state to player heal
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

			//destroy gameobject if player has collided with it
			if (hasCollidedHealth)
			{
				healthUpObj.type = typeDestroyed;
			}
		}

		ScreenBouncing(healthUpObj, true);
		Play::UpdateGameObject(healthUpObj);
		Play::DrawObject(healthUpObj);
	}

	//Golden mushroom object
	//If player collides with  Golden mushroom then move state to player power up
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

			//destroy gameobject if player has collided with it
			if (hasCollidedGoldM)
			{
				goldenMushroomObj.type = typeDestroyed;
			}
		}
		ScreenBouncing(goldenMushroomObj, true);
		Play::UpdateGameObject(goldenMushroomObj);
		Play::DrawObject(goldenMushroomObj);
	}
	
	//Refreshing herb object
    //If player collides with refreshing herb then move state to playernotdebuffed
	for (int refreshingHerbID : vRefreshingHerbs)
	{
		GameObject& refreshingHerbObj = Play::GetGameObject(refreshingHerbID);
		bool hasCollidedRefreshingHerbs = false;

		if (gameState.playerState == PlayerState::playerDebuffed)
		{
			if (Play::IsColliding(refreshingHerbObj, playerObj))
			{
				hasCollidedRefreshingHerbs = true;
				gameState.playerisDebuffed = false;
				gameState.playerState = PlayerState::playerTransform;
			}

			if (hasCollidedRefreshingHerbs)
			{
				refreshingHerbObj.type = typeDestroyed;
			}
		}
		ScreenBouncing(refreshingHerbObj, true);
		Play::UpdateGameObject(refreshingHerbObj);
		Play::DrawObject(refreshingHerbObj);
	}
}

//Used to update Hammer game object
void UpdateHammer()
{
	GameObject& playerObj = Play::GetGameObjectByType(typePlayer);
	GameObject& hammerObj = Play::GetGameObjectByType(typeHammer);

	//keeps hammer and player positions the same 
	hammerObj.pos = playerObj.pos;

	//Dont draw hammer unless in attack state
	if (gameState.playerState == PlayerState::playerAttack)
	{
		Play::UpdateGameObject(hammerObj);
		Play::DrawObject(hammerObj);
	}
	else
	{
		Play::UpdateGameObject(hammerObj);
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
		//boss will walk from right
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
	//c+v screenbouncing function from player update 
	//update object
	//drawobject --> maybe rotated have to see how things are implemented
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

		//if player is normal or debuff state and colliding with goomba move to damage state
		if (gameState.playerState == PlayerState::playerNotDebuffed || gameState.playerState == PlayerState::playerDebuffed) 
		{
			if (Play::IsColliding(goombaObj, playerObj))
			{
				userCollided = true;
				gameState.playerState = PlayerState::playerDamaged;
			}
		}
		else if (gameState.playerState == PlayerState::playerAttack)
		{
			//Set hammer collision to true so we can destroy object
			if (Play::IsColliding(goombaObj,hammerObj)) 
			{
				hammerCollided = true;
				gameState.score++;
			}
		}

		//If hammer or user has collided with the goomba then destroy game object
		if (hammerCollided || userCollided) 
		{
			goombaObj.type = typeDestroyed;
		}

		if (goombaObj.velocity.x < 0)
		{
			Play::SetSprite(goombaObj, "goomba_walk_w_8", 0.25f);
		}
		else if (goombaObj.velocity.x > 0)
		{
			Play::SetSprite(goombaObj, "goomba_walk_e_8", 0.25f);
		}
		ScreenBouncing(goombaObj, true);
		Play::UpdateGameObject(goombaObj);
		Play::DrawObjectRotated(goombaObj);
	}
}

//Used to update dry bones enemies
void UpdateDryBones() 
{
	GameObject& playerObj = Play::GetGameObjectByType(typePlayer);
	GameObject& hammerObj = Play::GetGameObjectByType(typeHammer);
	
	std::vector<int> vdryBones = Play::CollectGameObjectIDsByType(typeDryBones);

	for (int dryBoneID:vdryBones) 
	{
		GameObject& dryBonesObj = Play::GetGameObject(dryBoneID);
		float distanceFromPlayer = FindDistance(dryBonesObj, playerObj);
		bool isDistanceMet = false;

		if (distanceFromPlayer < 110)
		{
			//if player is left of dryBones
			if (playerObj.pos.x < dryBonesObj.pos.x)
			{
				Play::DrawObjectTransparent(dryBonesObj, 0);
				dryBonesObj.velocity = { 0,0 };

				//Spawn left facing drybones body and head
				int dryBonesBody = Play::CreateGameObject(typeDryBoneBody, dryBonesObj.pos, 10, "drybones_body_w_12");
				int dryBonesHead = Play::CreateGameObject(typeDryBoneHead, dryBonesObj.pos, 10, "drybones_head_8");
				GameObject& dryBonesHeadObj = Play::GetGameObject(dryBonesHead);

				dryBonesHeadObj.frame = 2;
				dryBonesHeadObj.animSpeed = 0.25f;
				isDistanceMet = true;

			}
			//if player is right of dryBones
			else if (playerObj.pos.x > dryBonesObj.pos.x)
			{
				Play::DrawObjectTransparent(dryBonesObj, 0);
				dryBonesObj.velocity = { 0,0 };

				//Spawn right facing drybones body and head
				int dryBonesBody = Play::CreateGameObject(typeDryBoneBody, dryBonesObj.pos, 10, "drybones_body_e_12");
				int dryBonesHead = Play::CreateGameObject(typeDryBoneHead, dryBonesObj.pos, 10, "drybones_head_8");
				GameObject& dryBonesHeadObj = Play::GetGameObject(dryBonesHead);

				dryBonesHeadObj.frame = 6;
				dryBonesHeadObj.animSpeed = 0.25f;
				isDistanceMet = true;
			}
		}

		//Set drybone to correct sprite based on ro
		if (dryBonesObj.velocity.y < 0)
		{
			Play::SetSprite(dryBonesObj, "drybones_n_16", 0.25f);
		}
		else if (dryBonesObj.velocity.y > 0)
		{
			Play::SetSprite(dryBonesObj, "drybones_s_16", 0.25f);
		}

		// If distance is met then delete main drybones gameobject
		if (isDistanceMet || !Play::IsVisible(dryBonesObj)) 
		{
			dryBonesObj.type = typeDestroyed;
		}

		ScreenBouncing(dryBonesObj, true);
		Play::UpdateGameObject(dryBonesObj);
		Play::DrawObject(dryBonesObj);
	}

	std::vector<int> vdryBonesHeads = Play::CollectGameObjectIDsByType(typeDryBoneHead);
	std::vector<int> vdryBonesBodies = Play::CollectGameObjectIDsByType(typeDryBoneBody);

	//Update detached drybones body and head
	for ( int dryBoneHeadID: vdryBonesHeads) 
	{
		GameObject& dryBoneHeadObj = Play::GetGameObject(dryBoneHeadID);
		bool userHasCollided = false;
		bool hammerHasCollided = false;

		//if player collides with head then move state to player Damaged
		if (Play::IsColliding(dryBoneHeadObj, playerObj)) 
		{
			userHasCollided = true;
			gameState.playerState = PlayerState::playerDamaged;
		}
		else 
		{
			//have head follow player
			Play::PointGameObject(dryBoneHeadObj, 1, playerObj.pos.x, playerObj.pos.y);

			//if 7 seconds have paassed and the head hasnt collided with the player then destroy the head and body
			if (gameState.seconds % 7 == 0) 
			{
				dryBoneHeadObj.type = typeDestroyed;

				for (int dryBoneBodyID : vdryBonesBodies) 
				{
					GameObject& dryBoneBodyObj = Play::GetGameObject(dryBoneBodyID);
					dryBoneBodyObj.type = typeDestroyed;
				}
			}
		}

		//if drybones head has collided with hammer set hammerhascollided to true so we can destroy it
		if (gameState.playerState == PlayerState::playerAttack && Play::IsColliding(dryBoneHeadObj, hammerObj))
		{
			hammerHasCollided = true;
			gameState.score + 3;
		}

		//If user has collided then move to damage and destroy head and body
		if (userHasCollided) 
		{
			dryBoneHeadObj.type = typeDestroyed;
			gameState.playerState = PlayerState::playerDamaged;

			for (int dryBoneBodyID : vdryBonesBodies)
			{
				GameObject& dryBoneBodyObj = Play::GetGameObject(dryBoneBodyID);
				dryBoneBodyObj.type = typeDestroyed;
			}
		}

		//If hammer has collided add to score and delete object
		if (hammerHasCollided)
		{
			dryBoneHeadObj.type = typeDestroyed;
			gameState.score += 3;

			for (int dryBoneBodyID : vdryBonesBodies)
			{
				GameObject& dryBoneBodyObj = Play::GetGameObject(dryBoneBodyID);
				dryBoneBodyObj.type = typeDestroyed;
			}
		}

		for (int dryBoneBodyID : vdryBonesBodies)
		{
			GameObject& dryBoneBodyObj = Play::GetGameObject(dryBoneBodyID);
			Play::UpdateGameObject(dryBoneBodyObj);
			Play::DrawObjectRotated(dryBoneBodyObj);
		}

		Play::UpdateGameObject(dryBoneHeadObj);
		Play::DrawObjectRotated(dryBoneHeadObj);
	}
}

//Used to update bobbomb enemies
void UpdateBobBombs() 
{
	GameObject& playerObj = Play::GetGameObjectByType(typePlayer);
	GameObject& hammerObj = Play::GetGameObjectByType(typeHammer);
	std::vector<int> vBobBombNotAlights = Play::CollectGameObjectIDsByType(typeBobBombNotAlight);
	
	for (int bobBombID : vBobBombNotAlights)
	{
		bool isAlight = false;
		GameObject& bobBombObj = Play::GetGameObject(bobBombID);
		float distanceFromPlayer = FindDistance(bobBombObj, playerObj);

		//If bobBomb is not alight and player is within 100 px then change type of bobBomb to alight and increase velocity of bobBomb 
		if (isAlight == false && distanceFromPlayer < 100) 
		{
			bobBombObj.velocity *= 1.8;
			bobBombObj.type = typeBobBombAlight;
		}

		//Set bobobmb to right sprite based on velocity
		if (bobBombObj.velocity.x > 0) 
		{
			Play::SetSprite(bobBombObj, "bobbomb_walk_e_8", 0.25f);
		}
		else if (bobBombObj.velocity.x < 0)
		{
			Play::SetSprite(bobBombObj, "bobbomb_walk_w_8", 0.25f);
		}
		ScreenBouncing(bobBombObj, true);
		Play::UpdateGameObject(bobBombObj);
		Play::DrawObjectRotated(bobBombObj);
	}

	std::vector<int> vBobBombAlights = Play::CollectGameObjectIDsByType(typeBobBombAlight);

	for (int bobBombID : vBobBombAlights)
	{
		GameObject& bobBombObj = Play::GetGameObject(bobBombID);
		bool isHammerColliding = false;

		//If player has collided with bobBomb then spawn explosions and delete bobBomb object 
		if (Play::IsColliding(bobBombObj, playerObj)) 
		{
			Play::DrawObjectTransparent(bobBombObj, 0);
			bobBombObj.velocity = { 0,0 };

			int explosion1ID = Play::CreateGameObject(typeBobBombExplosion, { bobBombObj.pos.x + 5 , bobBombObj.pos.y }, 15, "explosion_12");
			int explosion2ID = Play::CreateGameObject(typeBobBombExplosion, { bobBombObj.pos.x  , bobBombObj.pos.y }, 15, "explosion_12");
			int explosion3ID = Play::CreateGameObject(typeBobBombExplosion, { bobBombObj.pos.x - 5 , bobBombObj.pos.y }, 15, "explosion_12");

			GameObject& explosion1Obj = Play::GetGameObject(explosion1ID);
			GameObject& explosion2Obj = Play::GetGameObject(explosion2ID);
			GameObject& explosion3Obj = Play::GetGameObject(explosion3ID);

			explosion1Obj.animSpeed = explosion2Obj.animSpeed = explosion3Obj.animSpeed = 0.25f;

			//Set scale to random number between 0.7 and 0.9
			explosion1Obj.scale = GenRandomNumRange(0.7, 0.9);
			explosion2Obj.scale = GenRandomNumRange(0.7, 0.9);
			explosion3Obj.scale = GenRandomNumRange(0.7, 0.9);

			//Set Random Rotation
			explosion1Obj.rotation = GenRandomNumRange(0.0, 2 * PLAY_PI);
			explosion2Obj.rotation = GenRandomNumRange(0.0, 2 * PLAY_PI);
			explosion3Obj.rotation = GenRandomNumRange(0.0, 2 * PLAY_PI);
			Play::DestroyGameObject(bobBombID);
		}
		else 
		{
			//if player hasnt collided with bobbomb and 3 seconds have passed
			//set velocity to zero and set bobBomb to "sitting" sprite
			if (gameState.seconds % 3 == 0)
			{
				bobBombObj.velocity = { 0,0 };
				Play::SetSprite(bobBombObj, "bobbomb_walk_s_a_8", 0.25f);

				//if 4 seconds have passed after that then then spawn explosions and delete bobBomb object 
				if (gameState.seconds % 4 == 0)
				{
					Play::DrawObjectTransparent(bobBombObj, 0);

					int explosion1ID = Play::CreateGameObject(typeBobBombExplosion, { bobBombObj.pos.x + 5 , bobBombObj.pos.y }, 15, "explosion_12");
					int explosion2ID = Play::CreateGameObject(typeBobBombExplosion, { bobBombObj.pos.x , bobBombObj.pos.y }, 15, "explosion_12");
					int explosion3ID = Play::CreateGameObject(typeBobBombExplosion, { bobBombObj.pos.x - 5 , bobBombObj.pos.y }, 15, "explosion_12");

					GameObject& explosion1Obj = Play::GetGameObject(explosion1ID);
					GameObject& explosion2Obj = Play::GetGameObject(explosion2ID);
					GameObject& explosion3Obj = Play::GetGameObject(explosion3ID);

					explosion1Obj.animSpeed = explosion2Obj.animSpeed = explosion3Obj.animSpeed = 0.25f;

					//Set scale to random number between 0.7 and 0.9
					explosion1Obj.scale = GenRandomNumRange(0.7, 0.9);
					explosion2Obj.scale = GenRandomNumRange(0.7, 0.9);
					explosion3Obj.scale = GenRandomNumRange(0.7, 0.9);

					//Set Random Rotation
					explosion1Obj.rotation = GenRandomNumRange(0.0, 2 * PLAY_PI);
					explosion2Obj.rotation = GenRandomNumRange(0.0, 2 * PLAY_PI);
					explosion3Obj.rotation = GenRandomNumRange(0.0, 2 * PLAY_PI);
					Play::DestroyGameObject(bobBombID);
				}
			}
		}

		//setting hammer colliding to true so we can delete game oject later
		if (gameState.playerState == PlayerState::playerAttack && Play::IsColliding(bobBombObj, hammerObj))
		{
			isHammerColliding = true;
		}

		if (isHammerColliding)
		{
			gameState.score += 2;
			bobBombObj.type = typeDestroyed;
		}

		//Setting right sprite based on velocity
		if (bobBombObj.velocity.x > 0)
		{
			Play::SetSprite(bobBombObj, "bobbomb_walk_e_a_8", 0.25f);
		}
		else if (bobBombObj.velocity.x < 0)
		{
			Play::SetSprite(bobBombObj, "bobbomb_walk_w_a_8", 0.25f);
		}
		ScreenBouncing(bobBombObj, true);
		Play::UpdateGameObject(bobBombObj);
		Play::DrawObject(bobBombObj);
	}

	std::vector<int> vExplosions = Play::CollectGameObjectIDsByType(typeBobBombExplosion);

	for (int explosionID : vExplosions) 
	{
		GameObject& explosionObj = Play::GetGameObject(explosionID);

		//if player colliedes with explosion then move to state damage
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
	std::vector<int> vMagiKoopa = Play::CollectGameObjectIDsByType(typeMagiKoopa);

	for (int magiKoopaID: vMagiKoopa) 
	{
		GameObject& magiKoopaObj = Play::GetGameObject(magiKoopaID);
		//Angle from player to magikoopa
		float playerToMagiAng = atan2f(magiKoopaObj.pos.y - playerObj.pos.y, magiKoopaObj.pos.x - playerObj.pos.x);
		//going down variable used to set object to previous velocity when velocity = 0,0
		bool wasGoingDown = false;
		bool isHammerColliding = false;

		//set right sprite based on velocity
		if (magiKoopaObj.velocity.y < 0)
		{
			Play::SetSprite(magiKoopaObj, "magikoopa_n_8", 0.25f);
			wasGoingDown = false;

		}
		else if (magiKoopaObj.velocity.y > 0)
		{
			Play::SetSprite(magiKoopaObj, "magikoopa_s_8", 0.25f);
			wasGoingDown = true;
		}

		//if angle between player is between positive 5 degrees and negative 5 degrees and there is only 1 projectile on screen
		if (playerObj.pos.x < magiKoopaObj.pos.x && IsInBetween(0.087f ,-0.087f,playerToMagiAng) && Play::CollectGameObjectIDsByType(typeMagiKoopaProj).size() < 1)
		{
			magiKoopaObj.velocity = {0,0};
			Play::SetSprite(magiKoopaObj, "magikoopabattle_w_12", 0.25f);

			//if animation is in frame 3 spawn projectile
			if (magiKoopaObj.frame == 3) 
			{
				int projectileID = Play::CreateGameObject(typeMagiKoopaProj, {magiKoopaObj.pos.x -2, magiKoopaObj.pos.y}, 10, "peachvoice_proj_8");
				GameObject& projectileObj = Play::GetGameObject(projectileID);

				//setting projectile to right scale or orientation so it moves to the left
				projectileObj.scale = 0.7;
				projectileObj.rotation = (3 * PLAY_PI) / 2;
				projectileObj.animSpeed = 0.25f;
				projectileObj.velocity = { -2, 0 };

				//set magikoopa velocity back to previous velocity
				if (wasGoingDown)
				{
					magiKoopaObj.velocity.y = 2;
				}
				else
				{
					magiKoopaObj.velocity.y = -2;
				}
			} 
		}
		//if angle between player is between positive 175 degrees and negative 1755 degrees and there is only 1 projectile on screen
		else if (playerObj.pos.x > magiKoopaObj.pos.x && IsInBetween(3.054f, -3.054f, playerToMagiAng) && Play::CollectGameObjectIDsByType(typeMagiKoopaProj).size() < 1)
		{
			magiKoopaObj.velocity = { 0,0 };
			Play::SetSprite(magiKoopaObj, "magikoopabattle_e_12", 0.25f);

			//if animation is in frame 3 spawn projectile
			if (magiKoopaObj.frame == 3)
			{
				int projectileID = Play::CreateGameObject(typeMagiKoopaProj, { magiKoopaObj.pos.x + 2, magiKoopaObj.pos.y }, 10, "peachvoice_proj_8");
				GameObject& projectileObj = Play::GetGameObject(projectileID);

				//setting projectile to right scale or orientation so it moves to the right
				projectileObj.scale = 0.7;
				projectileObj.rotation = PLAY_PI / 2;
				projectileObj.animSpeed = 0.25f;
				projectileObj.velocity = { 2, 0 };

				//set magikoopa velocity back to previous velocity
				if (wasGoingDown)
				{
					magiKoopaObj.velocity.y = 2;
				}
				else
				{
					magiKoopaObj.velocity.y = -2;
				}
			}
		}

		//if player is in attack state and hammer and magikoopa is colliding set ishammercolliding to true so we can destroy magikoopa later
		if (gameState.playerState == PlayerState::playerAttack && Play::IsColliding(magiKoopaObj, hammerObj)) 
		{
			isHammerColliding = true;
		}

		if (isHammerColliding) 
		{
			gameState.score += 3;
			magiKoopaObj.type = typeDestroyed;
		}

		ScreenBouncing(magiKoopaObj, true);
		Play::UpdateGameObject(magiKoopaObj);
		Play::DrawObjectRotated(magiKoopaObj);
	}

	//Updating projectile
	std::vector<int> vProjectiles = Play::CollectGameObjectIDsByType(typeMagiKoopaProj);

	for (int mProjectileID : vProjectiles) 
	{
		bool hasCollided = false;
		GameObject& mProjectileObj = Play::GetGameObject(mProjectileID);
		
		//if player collides with projectile and player is notdebuffed then move to transform state
		if (Play::IsColliding(mProjectileObj, playerObj) && gameState.playerisDebuffed == false)
		{
			hasCollided = true;
			playerObj.velocity = { 0,0 };
			gameState.playerisDebuffed = true;
	        gameState.playerState = PlayerState::playerTransform;
		}

		//if player collides with projectile and player is debuffed then move to damage state
		else if (Play::IsColliding(mProjectileObj, playerObj) && gameState.playerisDebuffed)
		{
			hasCollided = true;
			gameState.playerState = PlayerState::playerDamaged;
		}

		//delete object if projectile is off screen or hascollided with player
		if (!Play::IsVisible(mProjectileObj) || hasCollided) 
		{
			mProjectileObj.type = typeDestroyed;
		}
		Play::UpdateGameObject(mProjectileObj);
		Play::DrawObjectRotated(mProjectileObj);
	}
}

//			STAGE RELATED:

//Used to draws spikes
void UpdateSpikes() 
{
	//Only used for drawing bottom spikes on the bottom of the screen
	//so player can walk 'behind' it
	GameObject& spikeObj = Play::GetGameObjectByType(typeSpikes);
	Play::UpdateGameObject(spikeObj);
	Play::DrawObject(spikeObj);
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

		if (!Play::IsVisible(desObj) || desObj.frame >= 10)
		{
			Play::DestroyGameObject(desID);
		}
	}
}

//Used to update UI
void UpdateUI() 
{
	//if player is damaged remove 1 from healthicons
	//if healed add one to health icons
	std::vector<int> vHealthIcons = Play::CollectGameObjectIDsByType(typeHealthIcon);

	for (int healthIconID : vHealthIcons) 
	{
		GameObject& healthIconObj = Play::GetGameObject(healthIconID);
		Play::UpdateGameObject(healthIconObj);
		Play::DrawObjectRotated(healthIconObj);
	}
}

//Used to update time passed
void UpdateTime() 
{
	//if framesPassed is divisible by 60 then a second has passed so add 1 to seconds
	if (gameState.framesPassed % 60 == 0 ) 
	{
		gameState.seconds++;
	}
}

//-------------------------------------------------------------------------------------------------------------------------------
//												CONTROL FUNCTIONS	

//Used for player controls when they are not debuffed
void HandleNotDebuffedControls() 
{
	GameObject& playerObj = Play::GetGameObjectByType(typePlayer);

	//User controls
	//based on key pressed set correct velocity and update playerOrientation so we can use the right sprite
	if (Play::KeyDown(VK_UP)) 
	{
		playerObj.velocity = { 0,-2.5f };
		Play::SetSprite(playerObj, "mario_walk_n_12", 0.25f);
		gameState.playerOrientation = vOrientations.at(0);
	}
	else if (Play::KeyDown(VK_RIGHT)) 
	{
		playerObj.velocity = { 2.5f,0 };
		Play::SetSprite(playerObj, "mario_walk_e_12", 0.25f);
		gameState.playerOrientation = vOrientations.at(1);
	}
	else if (Play::KeyDown(VK_DOWN)) 
	{
		playerObj.velocity = { 0,2.5f };
		Play::SetSprite(playerObj, "mario_walk_s_12", 0.25f);
		gameState.playerOrientation = vOrientations.at(2);
	}
	else if (Play::KeyDown(VK_LEFT)) 
	{
		playerObj.velocity = { -2.5f,0 };
		Play::SetSprite(playerObj, "mario_walk_w_12", 0.25f);
		gameState.playerOrientation = vOrientations.at(3);
	}

	// set velocity to 0,0 and mvoe to state attack
	else if (Play::KeyDown(VK_SPACE))
	{
		playerObj.velocity = { 0,0 };
		gameState.playerState = PlayerState::playerAttack;
	}
	else 
	{
		//Decelerate object and if velocity is certain value play relevant idle animations 
		DecelerateObject(playerObj, 0.2f);
		if (playerObj.velocity == Point2f{ 0,0 })
		{
			if (gameState.playerOrientation == vOrientations.at(0))
			{
				Play::SetSprite(playerObj, "mario_idle_n_35", 0.25f);
			}
			else if (gameState.playerOrientation == vOrientations.at(1))
			{
				Play::SetSprite(playerObj, "mario_idle_e_35", 0.25f);
			}
			else if (gameState.playerOrientation == vOrientations.at(2))
			{
				Play::SetSprite(playerObj, "mario_idle_s_35", 0.25f);
			}
			else if (gameState.playerOrientation == vOrientations.at(3))
			{
				Play::SetSprite(playerObj, "mario_idle_w_35", 0.25f);
			}
		}
	}
}

//Used for player controls when they are debuffed
void HandleDebuffedControls() 
{
	GameObject& playerObj = Play::GetGameObjectByType(typePlayer);

	//User controls
	//based on key pressed set correct velocity and update playerOrientation so we can use the right sprite
	if (gameState.playerState != PlayerState::playerDead)
	{
		if (Play::KeyDown(VK_UP))
		{
			playerObj.velocity = { 0,-1.5f };
			Play::SetSprite(playerObj, "mario_fat_walk_n_14", 0.25f);
			gameState.playerOrientation = vOrientations.at(0);
		}
		else if (Play::KeyDown(VK_RIGHT))
		{
			playerObj.velocity = { 1.5f,0 };
			Play::SetSprite(playerObj, "mario_fat_walk_e_14", 0.25f);
			gameState.playerOrientation = vOrientations.at(1);
		}
		else if (Play::KeyDown(VK_DOWN))
		{
			playerObj.velocity = { 0,1.5f };
			Play::SetSprite(playerObj, "mario_fat_walk_s_14", 0.25f);
			gameState.playerOrientation = vOrientations.at(2);
		}
		else if (Play::KeyDown(VK_LEFT))
		{
			playerObj.velocity = { -1.5f,0 };
			Play::SetSprite(playerObj, "mario_fat_walk_w_14", 0.25f);
			gameState.playerOrientation = vOrientations.at(3);
		}
		else
		{
			//Decelerate object and if velocity is certain value play relevant idle animations 
			DecelerateObject(playerObj, 0.2f);
			if (playerObj.velocity == Point2f{ 0,0 })
			{
				if (gameState.playerOrientation == vOrientations.at(0))
				{
					Play::SetSprite(playerObj, "mario_fat_idle_n_45", 0.25f);
				}
				else if (gameState.playerOrientation == vOrientations.at(1))
				{
					Play::SetSprite(playerObj, "mario_fat_idle_e_45", 0.25f);
				}
				else if (gameState.playerOrientation == vOrientations.at(2))
				{
					Play::SetSprite(playerObj, "mario_fat_idle_s_45", 0.25f);
				}
				else if (gameState.playerOrientation == vOrientations.at(3))
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

	//Based on value of playerOrientation play right hammer and player attack animation
	//once animation is finished them move to notdebuffed state
	if (gameState.playerOrientation == vOrientations.at(0))
	{
		Play::SetSprite(playerObj, "mario_hammer_n_3", 0.15f);
		Play::SetSprite(hammerObj, "ham_mario_n_3", 0.10f);

		if (Play::IsAnimationComplete(hammerObj) && Play::IsAnimationComplete(playerObj))
		{
			gameState.playerState = PlayerState::playerNotDebuffed;
		}
	}
	else if (gameState.playerOrientation == vOrientations.at(1))
	{
		Play::SetSprite(playerObj, "mario_hammer_e_3", 0.15f);
		Play::SetSprite(hammerObj, "ham_mario_w_3", 0.10f);

		if (Play::IsAnimationComplete(hammerObj) && Play::IsAnimationComplete(playerObj))
		{
			gameState.playerState = PlayerState::playerNotDebuffed;
		}
	}
	else if (gameState.playerOrientation == vOrientations.at(2))
	{
		Play::SetSprite(playerObj, "mario_hammer_s_3", 0.15f);
		Play::SetSprite(hammerObj, "ham_mario_s_3", 0.10f);

		if (Play::IsAnimationComplete(hammerObj) && Play::IsAnimationComplete(playerObj))
		{
			gameState.playerState = PlayerState::playerNotDebuffed;
		}
	}
	else if (gameState.playerOrientation == vOrientations.at(3))
	{
		Play::SetSprite(playerObj, "mario_hammer_w_3", 0.15f);
		Play::SetSprite(hammerObj, "ham_mario_e_3", 0.10f);

		if (Play::IsAnimationComplete(hammerObj) && Play::IsAnimationComplete(playerObj))
		{
			gameState.playerState = PlayerState::playerNotDebuffed;
		}
	}
}

//-------------------------------------------------------------------------------------------------------------------------------
//												SPAWN FUNCTIONS	

//Used to spawn enemies in initial stage
void SpawnEnemies() 
{
	//Spawn/Create 2 goomba gameobjects
	for (int i = 0; i < 2; i++) 
	{
		int goombaID = Play::CreateGameObject(typeGoomba, GetRandomPositionInPS(), 10, "goomba_walk_e_8");
		GameObject& goombaObj = Play::GetGameObject(goombaID);
		SetVelocity(goombaObj, "x");
	}

	//Spawn/Create 2 bobBomb gameobjects
	for (int i = 0; i < 2; i++) 
	{
		int bobbombID = Play::CreateGameObject(typeBobBombNotAlight, GetRandomPositionInPS(), 10, "bobbomb_walk_e_8");
		GameObject& bobBombObj = Play::GetGameObject(bobbombID);
		SetVelocity(bobBombObj, "x");
	}

	//Spawn/Create 1 drybones or magikoopa enemy based on the chance variable
	int chance = PickBetween(1, -1);

	if (chance == 1) 
	{
		int magiKoopaID = Play::CreateGameObject(typeMagiKoopa, GetRandomPositionInPS(), 15, "magikoopa_s_8");
		GameObject& magiKoopaObj = Play::GetGameObject(magiKoopaID);
		SetVelocity(magiKoopaObj, "y");
	}
	else 
	{
		int dryBonesID = Play::CreateGameObject(typeDryBones, GetRandomPositionInPS(), 15, "drybones_s_16");
		GameObject& dryBonesObj = Play::GetGameObject(dryBonesID);
		SetVelocity(dryBonesObj, "both");
	}
	
}

//used to spawn player consumables
void SpawnPlayerConsumables() 
{
	//if player health is not 0 and there isnt a golden mushroom on screen then spawn a golden mushroom
	if (gameState.playerHP != 0 && Play::CollectGameObjectIDsByType(typeGoldenMushroom).size() < 1)
	{
		int goldenMushroomID = Play::CreateGameObject(typeGoldenMushroom, GetRandomPositionInPS(), 20, "invincible_powerup");
		GameObject& goldenMushroomObj = Play::GetGameObject(goldenMushroomID);
		SetVelocity(goldenMushroomObj, "both");
	}

	//if player health is less than 3 (so less than max health) and there isnt 2 health pickups on screen then spawn a healthup
	else if (gameState.playerHP < 3 && Play::CollectGameObjectIDsByType(typeHealth1UP).size() < 2)
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
void ClearEnemiesOnScreen() 
{
	//Loop through enemy vectors and delete gameobjects
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

//			UI RELATED:

//Used to draw UI
void CreateUI() 
{
	//Create 3 health icon gameObjects 
	int healthIcon1ID = Play::CreateGameObject(typeHealthIcon, { displayWidth / 2 - 45, displayHeight - 65 }, 0, "health_icon");
	int healthIcon2ID = Play::CreateGameObject(typeHealthIcon, { displayWidth / 2, displayHeight - 65 }, 0, "health_icon");
	int healthIcon3ID = Play::CreateGameObject(typeHealthIcon, { displayWidth / 2 + 45, displayHeight - 65 }, 0, "health_icon");

	GameObject& healthIcon1Obj = Play::GetGameObject(healthIcon1ID);
	GameObject& healthIcon2Obj = Play::GetGameObject(healthIcon2ID);
	GameObject& healthIcon3Obj = Play::GetGameObject(healthIcon3ID);

	healthIcon1Obj.scale = healthIcon2Obj.scale = healthIcon3Obj.scale = 1.5;

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

//-------------------------------------------------------------------------------------------------------------------------------
//												DEBUG FUNCTIONS	

void ShowDebugUI() 
{
	GameObject& playerObj = Play::GetGameObjectByType(typePlayer);
	GameObject& hammerObj = Play::GetGameObjectByType(typeHammer);
	GameObject& bowserObj = Play::GetGameObjectByType(typeBowser);
	GameObject& dryBonesObj = Play::GetGameObjectByType(typeDryBones);
	GameObject& magiKoopaObj = Play::GetGameObjectByType(typeMagiKoopa);

	std::vector<int> vGoombas = Play::CollectGameObjectIDsByType(typeGoomba);
	std::vector<int> vBobBombAlights = Play::CollectGameObjectIDsByType(typeBobBombAlight);
	std::vector<int> vBobBombs = Play::CollectGameObjectIDsByType(typeBobBombNotAlight);
	std::vector<int> vExplosions = Play::CollectGameObjectIDsByType(typeBobBombExplosion);
	
	//		Position Checks
	Play::DrawDebugText(playerObj.pos, "Player Here", Play::cGreen);
	Play::DrawDebugText(hammerObj.pos, "Hammer Here", Play::cGreen);
	//Play::DrawDebugText(bowserObj.pos, "Bowser Here", Play::cGreen);

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
		Play::DrawDebugText({ bobBombObj.pos.x , bobBombObj.pos.y + 10}, distanceAsString.c_str(), Play::cGreen);
	}

	for (int bobBombID : vBobBombAlights)
	{
		GameObject& bobBombObj = Play::GetGameObject(bobBombID);

		float distance = FindDistance(bobBombObj, playerObj);
		std::string distanceAsString = std::to_string(distance);
		Play::DrawDebugText(bobBombObj.pos, "BB_A Here", Play::cGreen);
		Play::DrawDebugText({ bobBombObj.pos.x , bobBombObj.pos.y + 10 }, distanceAsString.c_str(), Play::cGreen);
	}

	float playerToDryBonesAng = atan2f(playerObj.pos.y - dryBonesObj.pos.y, playerObj.pos.x - dryBonesObj.pos.x) +  PLAY_PI / 2;
	std::string playerToDryBonesAngString = std::to_string(playerToDryBonesAng);
	float dryBonesDistance = FindDistance(dryBonesObj, playerObj);
	std::string dryBonesDistanceString = std::to_string(dryBonesDistance);
	Play::DrawDebugText({ dryBonesObj.pos.x, dryBonesObj.pos.y + 20 }, dryBonesDistanceString.c_str(), Play::cGreen);
	Play::DrawDebugText({ dryBonesObj.pos.x, dryBonesObj.pos.y + 10 }, playerToDryBonesAngString.c_str(), Play::cGreen);
	Play::DrawDebugText(dryBonesObj.pos, "DD Here", Play::cGreen);

	float playerToMagiAng = atan2f( magiKoopaObj.pos.y - playerObj.pos.y, magiKoopaObj.pos.x - playerObj.pos.x);
	std::string playerToMagiKoopaAngString = std::to_string(playerToMagiAng);
	Play::DrawDebugText({ magiKoopaObj.pos.x, magiKoopaObj.pos.y + 10 }, playerToMagiKoopaAngString.c_str(), Play::cGreen);
	Play::DrawDebugText(magiKoopaObj.pos, "MK Here", Play::cGreen);
	
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

	for (int bobBombID: vBobBombAlights) 
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

	if (Play::IsColliding(dryBonesObj, playerObj))
	{
		Play::DrawDebugText(dryBonesObj.pos, "DD Colliding", Play::cGreen);
	}
	
	if (Play::IsColliding(magiKoopaObj, playerObj))
	{
		Play::DrawDebugText(magiKoopaObj.pos, "MK Colliding", Play::cGreen);
	}
	
	//Timer Stuff
	
	//frames
	std::string framesPasseddString = std::to_string(gameState.framesPassed);
	Play::DrawDebugText({80 , 45}, framesPasseddString.c_str(), Play::cGreen);
	//seconds
	std::string timePassedString = std::to_string(gameState.seconds);
	Play::DrawDebugText({ 80 , 65 }, timePassedString.c_str(), Play::cGreen);
}
