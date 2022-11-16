#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"
#include "Boss.h"
#include "Player.h"
#include "Enemies.h"
#include "MainGame.h"

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
			if (Play::IsColliding(goombaObj, hammerObj))
			{
				int deathParticleID = Play::CreateGameObject(typeEnemyDeathParticle, goombaObj.pos, 0, "death_particle_13");
				GameObject& deathParticleObj = Play::GetGameObject(deathParticleID);
				deathParticleObj.scale = 0.7;
				deathParticleObj.animSpeed = 0.55f;
				Play::PlayAudio("coin");
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

	for (int dryBoneID : vdryBones)
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
	for (int dryBoneHeadID : vdryBonesHeads)
	{
		GameObject& dryBoneHeadObj = Play::GetGameObject(dryBoneHeadID);
		bool userHasCollided = false;
		bool hammerHasCollided = false;

		//if player collides with head then move state to player Damaged
		if (Play::IsColliding(dryBoneHeadObj, playerObj) && (gameState.playerState == PlayerState::playerDebuffed || gameState.playerState == PlayerState::playerNotDebuffed))
		{
			userHasCollided = true;
			gameState.playerState = PlayerState::playerDamaged;
		}
		else
		{
			//have head follow player
			Play::PointGameObject(dryBoneHeadObj, 1, playerObj.pos.x, playerObj.pos.y);

			//if 7 seconds have paassed and the head hasnt collided with the player then destroy the head and body
			if (gameState.timePassed % 6 == 0)
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
			int deathParticleID = Play::CreateGameObject(typeEnemyDeathParticle, dryBoneHeadObj.pos, 0, "death_particle_13");
			GameObject& deathParticleObj = Play::GetGameObject(deathParticleID);
			deathParticleObj.scale = 0.7;
			deathParticleObj.animSpeed = 0.55f;
			Play::PlayAudio("coin");
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
			int deathParticleID = Play::CreateGameObject(typeEnemyDeathParticle, dryBoneHeadObj.pos, 0, "death_particle_13");
			GameObject& deathParticleObj = Play::GetGameObject(deathParticleID);
			deathParticleObj.scale = 0.7;
			deathParticleObj.animSpeed = 0.55f;
			Play::PlayAudio("coin");

			for (int dryBoneBodyID : vdryBonesBodies)
			{
				int deathParticleID = Play::CreateGameObject(typeEnemyDeathParticle, dryBoneHeadObj.pos, 0, "death_particle_13");
				GameObject& deathParticleObj = Play::GetGameObject(deathParticleID);
				deathParticleObj.scale = 0.7;
				deathParticleObj.animSpeed = 0.55f;
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
		if (Play::IsColliding(bobBombObj, playerObj) && (gameState.playerState == PlayerState::playerDebuffed || gameState.playerState == PlayerState::playerNotDebuffed))
		{
			bobBombObj.velocity = { 0,0 };
			Play::DrawObjectTransparent(bobBombObj, 0);

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
			bobBombObj.type = typeDestroyed;
		}
		else
		{
			//if player hasnt collided with bobbomb and 4 seconds have passed
			//set velocity to zero and 'explode'
			if (gameState.timePassed % 5 == 0)
			{
				bobBombObj.velocity = { 0,0 };
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
				bobBombObj.type = typeDestroyed;
			}
		}

		//setting hammer colliding to true so we can delete game oject later
		if (gameState.playerState == PlayerState::playerAttack && Play::IsColliding(bobBombObj, hammerObj))
		{
			isHammerColliding = true;
		}

		if (isHammerColliding)
		{
			int deathParticleID = Play::CreateGameObject(typeEnemyDeathParticle, bobBombObj.pos, 0, "death_particle_13");
			GameObject& deathParticleObj = Play::GetGameObject(deathParticleID);
			deathParticleObj.scale = 0.7;
			deathParticleObj.animSpeed = 0.55f;
			Play::PlayAudio("coin");
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
				gameState.playerState = PlayerState::playerDamaged;
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

	for (int magiKoopaID : vMagiKoopa)
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
		if (playerObj.pos.x < magiKoopaObj.pos.x && IsInBetween(0.087f, -0.087f, playerToMagiAng) /*&& playerObj.pos.x < magiKoopaObj.pos.x*/ && Play::CollectGameObjectIDsByType(typeMagiKoopaProj).size() < 1)
		{
			magiKoopaObj.velocity = { 0,0 };
			Play::SetSprite(magiKoopaObj, "magikoopabattle_w_12", 0.25f);

			//if animation is in frame 3 spawn projectile
			if (magiKoopaObj.frame == 3)
			{
				int projectileID = Play::CreateGameObject(typeMagiKoopaProj, { magiKoopaObj.pos.x - 2, magiKoopaObj.pos.y }, 10, "peachvoice_proj_8");
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
		//if angle between player is between positive 175 degrees and negative 175 degrees and there is only 1 projectile on screen
		else if (playerObj.pos.x > magiKoopaObj.pos.x && IsInBetween(3.054f, -3.054f, playerToMagiAng) /*&& playerObj.pos.x > magiKoopaObj.pos.x*/ && Play::CollectGameObjectIDsByType(typeMagiKoopaProj).size() < 1)
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
			int deathParticleID = Play::CreateGameObject(typeEnemyDeathParticle, magiKoopaObj.pos, 0, "death_particle_13");
			GameObject& deathParticleObj = Play::GetGameObject(deathParticleID);
			deathParticleObj.scale = 0.7;
			deathParticleObj.animSpeed = 0.55f;
			Play::PlayAudio("coin");
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

//Used to update mob spawn particles
void UpdateEnemyParticles()
{
	std::vector<int> vSpawnParticles = Play::CollectGameObjectIDsByType(typeEnemySpawnParticle);
	std::vector<int> vDeathParticles = Play::CollectGameObjectIDsByType(typeEnemyDeathParticle);

	for (int enemySpawnParticleID : vSpawnParticles)
	{
		GameObject& enemySpawnParticleObj = Play::GetGameObject(enemySpawnParticleID);

		if (Play::IsAnimationComplete(enemySpawnParticleObj))
		{
			enemySpawnParticleObj.type = typeDestroyed;
		}

		Play::UpdateGameObject(enemySpawnParticleObj);
		Play::DrawObject(enemySpawnParticleObj);
	}

	for (int enemyDeathParticleID : vDeathParticles)
	{
		GameObject& enemyDeathParticleObj = Play::GetGameObject(enemyDeathParticleID);

		if (Play::IsAnimationComplete(enemyDeathParticleObj))
		{
			enemyDeathParticleObj.type = typeDestroyed;
		}

		Play::UpdateGameObject(enemyDeathParticleObj);
		Play::DrawObject(enemyDeathParticleObj);
	}

}