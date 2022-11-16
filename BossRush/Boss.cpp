#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"
#include "Boss.h"
#include "Player.h"
#include "Enemies.h"
#include "MainGame.h"

//			ENEMY RELATED:

//Used to update boss state
void UpdateBossState()
{
	GameObject& bowserObj = Play::GetGameObjectByType(typeBowser);
	GameObject& playerObj = Play::GetGameObjectByType(typePlayer);
	GameObject& hammerObj = Play::GetGameObjectByType(typeHammer);

	switch (gameState.bossState)
	{
	case BossState::bossDefault:
	{
	}
	case BossState::bossAppear:
	{
		ClearEnemiesAndItems();
		Play::SetSprite(bowserObj, "bowser_walk_w_12", 0.25f);
		bowserObj.velocity = { -2.5f,0 };

		if (bowserObj.pos.x < displayWidth - 150)
		{
			bowserObj.velocity = { 0,0 };
			gameState.bossState = BossState::bossIdle;
		}

		//boss will walk from right
		//once they reach certain x amount move to idle
	}
	break;
	case BossState::bossIdle:
	{
		Play::SetSprite(bowserObj, "bowser_idle_14", 0.25f);
		float distanceFromplayer = FindDistance(bowserObj, playerObj);

		if (gameState.timePassed % 4 == 0)
		{
			//if list of enemies is empty move to spawn mob state
			if (Play::CollectGameObjectIDsByType(typeMagiKoopa).size() + Play::CollectGameObjectIDsByType(typeGoomba).size() == 0)
			{
				gameState.bossState = BossState::bossSummonMob;
			}

			//if distance from player is < 200 then set isBossAtkLngRng to false and move to state boss agro
			else if (distanceFromplayer < 300)
			{
				gameState.isBossAtkLngRng = false;
				gameState.bossState = BossState::bossAgro;
			}
			//if distance from player is > 200 then set isBossAtkLngRng to true and move to state boss agro
			else if (distanceFromplayer > 300)
			{
				gameState.isBossAtkLngRng = true;
				gameState.bossState = BossState::bossAgro;
			}
		}
	}
	break;
	case BossState::bossAgro:
	{
		//Play attack animation
		Play::SetSprite(bowserObj, "bowser_attack_14", 0.25f);

		if (bowserObj.frame == 13)
		{
			if (gameState.isBossAtkLngRng)
			{
				//do long range attack
				//Spawn a fireball at player x pos but off screen
				int fireBall1ID = Play::CreateGameObject(typeBowseLRP, { playerObj.pos.x , -50 }, 20, "bowser_proj_8");
				GameObject& fireBall1Obj = Play::GetGameObject(fireBall1ID);
				fireBall1Obj.velocity = { 0,5 };
				fireBall1Obj.animSpeed = 0.25f;

				//Spawn a fireball at player x pos but off screen
				if (gameState.timePassed % 2 == 0)
				{
					int fireBall2ID = Play::CreateGameObject(typeBowseLRP, { playerObj.pos.x , -50 }, 20, "bowser_proj_8");
					GameObject& longRangeProj2Obj = Play::GetGameObject(fireBall2ID);
					longRangeProj2Obj.velocity = { 0,5 };
					longRangeProj2Obj.animSpeed = 0.25f;
				}
				gameState.bossState = BossState::bossIdle;
			}
			else
			{
				//do short range attack
				//Make shield around boss's front
				//Maybe add it so that based on player position you spawn it N,E,S,W
				if (playerObj.pos.x > bowserObj.pos.x)
				{
					SpawnFireballs(bowserObj, 'e');
				}
				else if (playerObj.pos.x < bowserObj.pos.x)
				{
					SpawnFireballs(bowserObj, 'w');
				}
				gameState.bossState = BossState::bossIdle;
			}
		}
	}
	break;
	case BossState::bossDamaged:
	{

		std::vector<int> vBossHealthIcons = Play::CollectGameObjectIDsByType(typeBowserHealthIcon);
		//playing damage sprite
		Play::SetSprite(bowserObj, "bowser_dead_15", 0.25f);


		if (bowserObj.frame == 5)
		{
			Play::PlayAudio("bowser_damage");
		}

		if (bowserObj.frame > 15)
		{
			//If bosshp -- = 0 then move boss state to bossDead
			//and decrement boss hp
			if (gameState.bossHp - 1 == 0)
			{
				gameState.bossHp--;
				gameState.roundState = RoundState::roundWin;

				//Similar to player hp implementation
				GameObject& frontHealthIconToDelObj = Play::GetGameObject(vBossHealthIcons.front());
				frontHealthIconToDelObj.type = typeDestroyed;
			}
			else
			{
				//else decrement bosshp and move boss state to bossIdle 
				gameState.bossHp--;

				//Similar to player hp implementation
				GameObject& backHealthIconToDelObj = Play::GetGameObject(vBossHealthIcons.back());
				backHealthIconToDelObj.type = typeDestroyed;
				gameState.bossState = BossState::bossIdle;
			}
		}
	}
	break;
	case BossState::bossSummonMob:
	{
		//Play attack animation
		Play::SetSprite(bowserObj, "bowser_attack_14", 0.25f);

		//if animation reaches frame 20 spawn enemies and move back to state idle 
		if (bowserObj.frame == 13)
		{
			//Spawn mobs
			SpawnEnemies();
			gameState.bossState = BossState::bossIdle;
		}
	}
	break;
	case BossState::bossDead:
	{
		//play damage sprite
		Play::SetSprite(bowserObj, "bowser_dead_15", 0.25f);
	}
	break;
	case BossState::bossWin:
	{
		Play::SetSprite(bowserObj, "bowser_win_16", 0.25f);
	}
	break;
	}

	Play::UpdateGameObject(bowserObj);
	Play::DrawObject(bowserObj);
}

//Used to update different boss attack icons
void UpdateBossIcons()
{
	GameObject& bossIconObj = Play::GetGameObjectByType(typeBowserAttackIcon);
	GameObject& bowserObj = Play::GetGameObjectByType(typeBowser);

	bossIconObj.pos = { bowserObj.pos.x,bowserObj.pos.y - 70 };

	//if boss state is agro or summonmob then set the relevant sprite and draw the icon
	if (gameState.bossState == BossState::bossAgro)
	{
		if (gameState.isBossAtkLngRng)
		{
			Play::SetSprite(bossIconObj, "peachvoice_longrange_8", 0.25f);
		}
		else
		{
			Play::SetSprite(bossIconObj, "peachvoice_shortrange_8", 0.25f);
		}
		Play::DrawObject(bossIconObj);
	}
	else if (gameState.bossState == BossState::bossSummonMob)
	{

		Play::SetSprite(bossIconObj, "peachvoice_summon_8", 0.25f);
		Play::DrawObject(bossIconObj);
	}
	Play::UpdateGameObject(bossIconObj);
}

//Used to update boss projectiles
void UpdateBossProjectiles()
{
	GameObject& playerObj = Play::GetGameObjectByType(typePlayer);
	std::vector<int> vBossLProj = Play::CollectGameObjectIDsByType(typeBowseLRP);
	std::vector<int> vBossSProj = Play::CollectGameObjectIDsByType(typeBowseSRP);

	//Long Range Projectiles
	for (int bossProjID : vBossLProj)
	{
		GameObject& bossProjObj = Play::GetGameObject(bossProjID);

		if (Play::IsColliding(bossProjObj, playerObj))
		{
			gameState.playerState = PlayerState::playerDamaged;
			bossProjObj.type = typeDestroyed;
		}

		if (bossProjObj.pos.y > displayHeight + 50)
		{
			bossProjObj.type = typeDestroyed;
		}

		Play::UpdateGameObject(bossProjObj);
		Play::DrawObject(bossProjObj);
	}

	//Short Range Projectiles
	for (int bossProjID : vBossSProj)
	{
		GameObject& bossProjObj = Play::GetGameObject(bossProjID);

		if (Play::IsColliding(bossProjObj, playerObj))
		{
			gameState.playerState = PlayerState::playerDamaged;
			bossProjObj.type = typeDestroyed;
		}

		//If short range has been through 7 cycles of animation then destroy
		//the object
		if (bossProjObj.frame > 28)
		{
			bossProjObj.type = typeDestroyed;
		}
		Play::UpdateGameObject(bossProjObj);
		Play::DrawObject(bossProjObj);
	}
}
