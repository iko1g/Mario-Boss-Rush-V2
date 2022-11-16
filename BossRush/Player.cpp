#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"
#include "Boss.h"
#include "Player.h"
#include "Enemies.h"
#include "MainGame.h"

//-------------------------------------------------------------------------------------------------------------------------------
//												UPDATE FUNCTIONS

//			PLAYER RELATED:

//used to update player state
void UpdatePlayerState()
{
	GameObject& playerObj = Play::GetGameObjectByType(typePlayer);
	GameObject& hammerObj = Play::GetGameObjectByType(typeHammer);
	GameObject& bowserObj = Play::GetGameObjectByType(typeBowser);

	switch (gameState.playerState)
	{
	case PlayerState::playerDefault:
	{
	}
	case PlayerState::playerAppear:
	{
		Play::SetSprite(playerObj, "mario_walk_e_12", 0.25f);
		playerObj.velocity = { 2.5f,0 };

		if (playerObj.pos.x > 100)
		{
			playerObj.velocity = { 0,0 };
			gameState.playerOrientation = gameState.vOrientations.at(1);
			gameState.playerState = PlayerState::playerPowerUp;
		}
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
		std::vector<int> vHealthIcons = Play::CollectGameObjectIDsByType(typePlayerHealthIcon);

		//Minus 1 player health and move to relevant move state
		if (gameState.playerHP - 1 == 0)
		{
			gameState.playerHP--;

			//Destory first object in list as they will have 1hp so there should only be 1 health icon
			GameObject& frontHealthIconToDelObj = Play::GetGameObject(vHealthIcons.front());
			frontHealthIconToDelObj.type = typeDestroyed;

			gameState.roundState = RoundState::roundLose;
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
		std::vector<int> vHealthIcons = Play::CollectGameObjectIDsByType(typePlayerHealthIcon);
		// Plus 1 to player health and move to relevant move state
		gameState.playerHP++;

		//Get health icon at the back
		GameObject& backHealthIconObj = Play::GetGameObject(vHealthIcons.back());

		//Create new health icon at an offset of the health icon at the back of the vector
		int healthIconToAddID = Play::CreateGameObject(typePlayerHealthIcon, { backHealthIconObj.pos.x + 45, displayHeight - 65 }, 0, "health_icon");
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
	}
	break;
	case PlayerState::playerPowerUp:
	{
		//Handle player movement while in this state
		if (gameState.playerisDebuffed)
		{
			HandleDebuffedControls();
		}
		else
		{
			HandleNotDebuffedControls();
		}

		//if 6 seconds have passed then clear colouring
		//rest counter and move player state to relevant state
		if (gameState.timePassed % 5 == 0)
		{
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
	GameObject& bowserObj = Play::GetGameObjectByType(typeBowser);

	//keeps hammer and player positions the same 
	hammerObj.pos = playerObj.pos;

	//Boss Hammer Interactions
	if (gameState.playerState == PlayerState::playerAttack && Play::IsColliding(hammerObj, bowserObj) && gameState.bossState == BossState::bossIdle)
	{
		gameState.bossState = BossState::bossDamaged;
	}

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
		gameState.playerOrientation = gameState.vOrientations.at(0);
	}
	else if (Play::KeyDown(VK_RIGHT))
	{
		playerObj.velocity = { 2.5f,0 };
		Play::SetSprite(playerObj, "mario_walk_e_12", 0.25f);
		gameState.playerOrientation = gameState.vOrientations.at(1);
	}
	else if (Play::KeyDown(VK_DOWN))
	{
		playerObj.velocity = { 0,2.5f };
		Play::SetSprite(playerObj, "mario_walk_s_12", 0.25f);
		gameState.playerOrientation = gameState.vOrientations.at(2);
	}
	else if (Play::KeyDown(VK_LEFT))
	{
		playerObj.velocity = { -2.5f,0 };
		Play::SetSprite(playerObj, "mario_walk_w_12", 0.25f);
		gameState.playerOrientation = gameState.vOrientations.at(3);
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
			if (gameState.playerOrientation == gameState.vOrientations.at(0))
			{
				Play::SetSprite(playerObj, "mario_idle_n_35", 0.25f);
			}
			else if (gameState.playerOrientation == gameState.vOrientations.at(1))
			{
				Play::SetSprite(playerObj, "mario_idle_e_35", 0.25f);
			}
			else if (gameState.playerOrientation == gameState.vOrientations.at(2))
			{
				Play::SetSprite(playerObj, "mario_idle_s_35", 0.25f);
			}
			else if (gameState.playerOrientation == gameState.vOrientations.at(3))
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
			gameState.playerOrientation = gameState.vOrientations.at(0);
		}
		else if (Play::KeyDown(VK_RIGHT))
		{
			playerObj.velocity = { 1.5f,0 };
			Play::SetSprite(playerObj, "mario_fat_walk_e_14", 0.25f);
			gameState.playerOrientation = gameState.vOrientations.at(1);
		}
		else if (Play::KeyDown(VK_DOWN))
		{
			playerObj.velocity = { 0,1.5f };
			Play::SetSprite(playerObj, "mario_fat_walk_s_14", 0.25f);
			gameState.playerOrientation = gameState.vOrientations.at(2);
		}
		else if (Play::KeyDown(VK_LEFT))
		{
			playerObj.velocity = { -1.5f,0 };
			Play::SetSprite(playerObj, "mario_fat_walk_w_14", 0.25f);
			gameState.playerOrientation = gameState.vOrientations.at(3);
		}
		else
		{
			//Decelerate object and if velocity is certain value play relevant idle animations 
			DecelerateObject(playerObj, 0.2f);
			if (playerObj.velocity == Point2f{ 0,0 })
			{
				if (gameState.playerOrientation == gameState.vOrientations.at(0))
				{
					Play::SetSprite(playerObj, "mario_fat_idle_n_45", 0.25f);
				}
				else if (gameState.playerOrientation == gameState.vOrientations.at(1))
				{
					Play::SetSprite(playerObj, "mario_fat_idle_e_45", 0.25f);
				}
				else if (gameState.playerOrientation == gameState.vOrientations.at(2))
				{
					Play::SetSprite(playerObj, "mario_fat_idle_s_45", 0.25f);
				}
				else if (gameState.playerOrientation == gameState.vOrientations.at(3))
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

	Play::PlayAudio("hammer");
	//Based on value of playerOrientation play right hammer and player attack animation
	//once animation is finished them move to notdebuffed state
	if (gameState.playerOrientation == gameState.vOrientations.at(0))
	{
		Play::SetSprite(playerObj, "mario_hammer_n_3", 0.15f);
		Play::SetSprite(hammerObj, "ham_mario_n_3", 0.10f);


		if (Play::IsAnimationComplete(hammerObj) && Play::IsAnimationComplete(playerObj))
		{
			gameState.playerState = PlayerState::playerNotDebuffed;
		}
	}
	else if (gameState.playerOrientation == gameState.vOrientations.at(1))
	{
		Play::SetSprite(playerObj, "mario_hammer_e_3", 0.15f);
		Play::SetSprite(hammerObj, "ham_mario_w_3", 0.10f);

		if (Play::IsAnimationComplete(hammerObj) && Play::IsAnimationComplete(playerObj))
		{
			gameState.playerState = PlayerState::playerNotDebuffed;
		}
	}
	else if (gameState.playerOrientation == gameState.vOrientations.at(2))
	{
		Play::SetSprite(playerObj, "mario_hammer_s_3", 0.15f);
		Play::SetSprite(hammerObj, "ham_mario_s_3", 0.10f);

		if (Play::IsAnimationComplete(hammerObj) && Play::IsAnimationComplete(playerObj))
		{
			gameState.playerState = PlayerState::playerNotDebuffed;
		}
	}
	else if (gameState.playerOrientation == gameState.vOrientations.at(3))
	{
		Play::SetSprite(playerObj, "mario_hammer_w_3", 0.15f);
		Play::SetSprite(hammerObj, "ham_mario_e_3", 0.10f);

		if (Play::IsAnimationComplete(hammerObj) && Play::IsAnimationComplete(playerObj))
		{
			gameState.playerState = PlayerState::playerNotDebuffed;
		}
	}
}