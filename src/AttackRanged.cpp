#include "Attack.h"

#include "Engine.h"

#include "ConstDungeonSettings.h"
#include "Item.h"
#include "ItemWeapon.h"
#include "Render.h"
#include "Map.h"
#include "Timer.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "Postmortem.h"
#include "Knockback.h"
#include "Input.h"

using namespace std;

struct Projectile {
	Projectile() {
		obstructed = false;
		actorHit = NULL;
		obstructedInElement = 0;
	}

	AttackData data;
	bool obstructed;
	Actor* actorHit;
	int obstructedInElement;
	coord pos;
};

void Attack::projectileFire(const coord& origin, coord target, Weapon* const weapon, const unsigned int NR_OF_PROJECTILES) {
	vector<Projectile*> projectiles;

	for(unsigned int i = 0; i < NR_OF_PROJECTILES; i++) {
		Projectile* currentProjectile = new Projectile;
		getAttackData(currentProjectile->data, target, origin, weapon, false);
		projectiles.push_back(currentProjectile);
	}

	printRangedInitiateMessages(projectiles.at(0)->data);

	const Audio_t rangedAudio = weapon->getInstanceDefinition().rangedAudio;
	if(rangedAudio != audio_none) {
		eng->audio->playSound(rangedAudio);
	}

	const bool stopAtTarget = projectiles.at(0)->data.aimLevel == actorSize_floor;
	const int chebTrvlLim = 30;

	//Get projectile path
	const vector<coord> projectilePath = eng->mapTests->getLine(origin.x, origin.y, target.x, target.y, stopAtTarget, chebTrvlLim);

	const SDL_Color projectileColor = weapon->getInstanceDefinition().rangedMissileColor;
	char projectileGlyph = weapon->getInstanceDefinition().rangedMissileGlyph;
	if(projectileGlyph == '/') {
		const int i = projectilePath.size() > 2 ? 2 : 1;
		if(projectilePath.at(i).y == origin.y)
			projectileGlyph = '-';
		if(projectilePath.at(i).x == origin.x)
			projectileGlyph = '|';
		if((projectilePath.at(i).x > origin.x && projectilePath.at(i).y < origin.y) || (projectilePath.at(i).x < origin.x && projectilePath.at(i).y
		      > origin.y))
			projectileGlyph = '/';
		if((projectilePath.at(i).x > origin.x && projectilePath.at(i).y > origin.y) || (projectilePath.at(i).x < origin.x && projectilePath.at(i).y
		      < origin.y))
			projectileGlyph = '\\';
	}
	Tile_t projectileTile = weapon->getInstanceDefinition().rangedMissileTile;
	if(projectileTile == tile_projectileStandardFrontSlash) {
		if(projectileGlyph == '-') {
			projectileTile = tile_projectileStandardDash;
		}
		if(projectileGlyph == '|') {
			projectileTile = tile_projectileStandardVerticalBar;
		}
		if(projectileGlyph == '\\') {
			projectileTile = tile_projectileStandardBackSlash;
		}
	}

	const bool LEAVE_TRAIL = weapon->getInstanceDefinition().rangedMissileLeavesTrail;

	const unsigned int SIZE_OF_PATH_PLUS_ONE = projectilePath.size() + (NR_OF_PROJECTILES - 1) * NUMBER_OF_CELLJUMPS_BETWEEN_MACHINEGUN_PROJECTILES;

	eng->renderer->drawMapAndInterface();

	//Run projectile path. The variable 'i' is the "global path element".
	//The individual projectiles are offset from this global element according
	//to their place in the projectile vector.
	for(unsigned int i = 1; i < SIZE_OF_PATH_PLUS_ONE; i++) {
		if(LEAVE_TRAIL == false) {
			eng->renderer->drawMapAndInterface();
		}

		for(unsigned int p = 0; p < NR_OF_PROJECTILES; p++) {
			//Current projectile's place in the path is the current global place (i)
			//minus a certain number of elements
			int projectilePathElement = i - (p * NUMBER_OF_CELLJUMPS_BETWEEN_MACHINEGUN_PROJECTILES);

			//All the following collision checks etc are only made if the projectiles current path element
			//corresponds to an element in the real path vector
			if(projectilePathElement >= 1 && projectilePathElement < int(projectilePath.size()) && projectiles.at(p)->obstructed == false) {
				projectiles.at(p)->pos = projectilePath.at(projectilePathElement);

				//Get attack data again for every cell traveled through,
				//Aim-level and such are only retrieved the first time
				getAttackData(projectiles.at(p)->data, target, projectiles.at(p)->pos, weapon, false);

				const coord drawPos(projectiles.at(p)->pos.x, projectiles.at(p)->pos.y);

				//HIT ACTOR?
				if(projectiles.at(p)->data.currentDefender != NULL && projectiles.at(p)->obstructed == false) {

					const bool AIMED_FOR_THIS_ACTOR = (projectiles.at(p)->pos == target);

					//Floor-sized actors never obstruct projectiles when not aimed for
					if(projectiles.at(p)->data.currentDefenderSize > actorSize_floor || AIMED_FOR_THIS_ACTOR == true) {

                        ProjectileHitType_t hitType = projectileHitType_miss;

						//If this is the actor aimed for, check skill roll for hit
						if(AIMED_FOR_THIS_ACTOR == true) {
							if(projectiles.at(p)->data.attackResult >= successSmall) {
								hitType = projectileHitType_cleanHit;
							}
						}
						//If clean hit failed (because of not cell aimed for, or failed skill roll),
						//try a stray hit
						if(hitType == projectileHitType_miss) {
							//placeholder value, but it might do the trick?***
							if(eng->dice(1, 100) < 25) {
								hitType = projectileHitType_strayHit;
							}
						}

						if(hitType >= projectileHitType_strayHit) {
							//RENDER ACTOR HIT
							if(eng->map->playerVision[projectiles.at(p)->pos.x][projectiles.at(p)->pos.y] == true) {
								if(eng->config->USE_TILE_SET) {
									eng->renderer->drawBlastAnimationAt(drawPos, clrRedLight, eng->config->DELAY_PROJECTILE_DRAW);
								} else {
									eng->renderer->drawCharacter('*', renderArea_mainScreen, drawPos.x, drawPos.y, clrRedLight);
									eng->renderer->flip();
									Timer t;
									t.start();
									while(t.get_ticks() < eng->config->DELAY_PROJECTILE_DRAW) {
									}
								}
							}

							//MESSAGES FOR ACTOR HIT
							printProjectileAtActorMessages(projectiles.at(p)->data, hitType);

							projectiles.at(p)->obstructed = true;
							projectiles.at(p)->actorHit = projectiles.at(p)->data.currentDefender;
							projectiles.at(p)->obstructedInElement = projectilePathElement;

							if(hitType == projectileHitType_strayHit) {
								projectiles.at(p)->data.dmg = projectiles.at(p)->data.dmg / 2;
							}

							//Damage
							const bool DIED = projectiles.at(p)->data.currentDefender->hit(projectiles.at(p)->data.dmg,
							                  weapon->getInstanceDefinition().rangedDamageType);
							if(DIED) {
								if(projectiles.at(p)->data.currentDefender == eng->player) {
									eng->postmortem->setCauseOfDeath(weapon->getInstanceDefinition().causeOfDeathMessage);
								}
							} else {
								// Aply weapon hit status effects
								projectiles.at(p)->data.currentDefender->getStatusEffectsHandler()->attemptAddEffectsFromWeapon(weapon, false);

								// Knock-back?
								if(weapon->getInstanceDefinition().rangedCausesKnockBack) {
									const AttackData& currentAttData = projectiles.at(p)->data;
									const coord defenderPosBefore = currentAttData.currentDefender->pos;
									if(currentAttData.attackResult >= successSmall) {
										const bool IS_SPIKE_GUN = weapon->getInstanceDefinition().devName == item_spikeGun;
										eng->knockBack->attemptKnockBack(currentAttData.currentDefender, currentAttData.attacker->pos, IS_SPIKE_GUN);
									}
									// If target was knocked back, update target attack cell to defenders new pos
									if(target == defenderPosBefore) {
										target = projectiles.at(p)->data.currentDefender->pos;
									}
								}
							}
						}
					}
				}

				//PROJECTILE HIT FEATURE?
				vector<FeatureMob*> featureMobs = eng->gameTime->getFeatureMobsAtPos(projectiles.at(p)->pos);
				Feature* featureBlockingShot = NULL;
				for(unsigned int featMobIndex = 0; featMobIndex < featureMobs.size(); featMobIndex++) {
					if(featureMobs.at(featMobIndex)->isShootPassable() == false) {
						featureBlockingShot = featureMobs.at(featMobIndex);
					}
				}
				FeatureStatic* featureStatic = eng->map->featuresStatic[projectiles.at(p)->pos.x][projectiles.at(p)->pos.y];
				if(featureStatic->isShootPassable() == false) {
					featureBlockingShot = featureStatic;
				}

				if(featureBlockingShot != NULL && projectiles.at(p)->obstructed == false) {
					projectiles.at(p)->obstructedInElement = projectilePathElement - 1;
					projectiles.at(p)->obstructed = true;

					//RENDER FEATURE HIT
					if(eng->map->playerVision[projectiles.at(p)->pos.x][projectiles.at(p)->pos.y] == true) {
						if(eng->config->USE_TILE_SET) {
							eng->renderer->drawBlastAnimationAt(drawPos, clrYellow, eng->config->DELAY_PROJECTILE_DRAW);
						} else {
							eng->renderer->drawCharacter('*', renderArea_mainScreen, drawPos.x, drawPos.y, clrYellow);
							eng->renderer->flip();
							Timer t;
							t.start();
							while(t.get_ticks() < eng->config->DELAY_PROJECTILE_DRAW) {
							}
						}
					}
				}

				//PROJECTILE HIT THE GROUND?
				if(projectiles.at(p)->pos == target && projectiles.at(p)->data.aimLevel == actorSize_floor && projectiles.at(p)->obstructed == false) {
					projectiles.at(p)->obstructed = true;
					projectiles.at(p)->obstructedInElement = projectilePathElement;

					//RENDER GROUND HITS
					if(eng->map->playerVision[projectiles.at(p)->pos.x][projectiles.at(p)->pos.y] == true) {
						if(eng->config->USE_TILE_SET) {
							eng->renderer->drawBlastAnimationAt(drawPos, clrYellow, eng->config->DELAY_PROJECTILE_DRAW);
						} else {
							eng->renderer->drawCharacter('*', renderArea_mainScreen, drawPos.x, drawPos.y, clrYellow);
							eng->renderer->flip();
							Timer t;
							t.start();
							while(t.get_ticks() < eng->config->DELAY_PROJECTILE_DRAW) {
							}
						}
					}
				}

				//RENDER FLYING PROJECTILES
				if(eng->map->playerVision[projectiles.at(p)->pos.x][projectiles.at(p)->pos.y] == true) {
					const coord projDrawPos(projectiles.at(p)->pos.x, projectiles.at(p)->pos.y);
					if(eng->config->USE_TILE_SET) {
						eng->renderer->drawTileInMap(projectileTile, projDrawPos.x, projDrawPos.y, projectileColor);
					} else {
						eng->renderer->drawCharacter(projectileGlyph, renderArea_mainScreen, projDrawPos.x, projDrawPos.y, projectileColor);
					}
				}
			}
		} //End projectile loop


		eng->renderer->flip();

		//If any projectile can be seen and not obstructed, delay
		bool delayProjectiles = false;
		for(unsigned int nn = 0; nn < projectiles.size(); nn++) {
			if(eng->map->playerVision[projectiles.at(nn)->pos.x][projectiles.at(nn)->pos.y] == true && projectiles.at(nn)->obstructed == false) {
				delayProjectiles = true;
			}
		}

		if(delayProjectiles == true) {
			//If it's a machinegun, delay should be shorter
			if(weapon->getInstanceDefinition().isMachineGun == true) {
				Timer t;
				t.start();
				while(t.get_ticks() < eng->config->DELAY_PROJECTILE_DRAW / 2) {
				}
			} else {
				Timer t;
				t.start();
				while(t.get_ticks() < eng->config->DELAY_PROJECTILE_DRAW) {
				}
			}
		}

		//Check if all projectiles obstructed
		bool allObstructed = true;
		for(unsigned p = 0; p < projectiles.size(); p++) {
			if(projectiles.at(p)->obstructed == false) {
				allObstructed = false;
			}
		}
		if(allObstructed == true) {
			i = 99999;
		}

	} //End path-loop

	/*const bool LEAVE_SMOKE = weapon->getInstanceDefinition().rangedMissileLeavesSmoke;
	 if(LEAVE_SMOKE == true) {
	 eng->explosionMaker->runSmokeExplosion(projectiles.at(0)->pos, true);
	 }*/

	//So far, only projectile 0 can have special obstruction events***
	//Must be changed if something like an assault-incinerator is added
	if(projectiles.at(0)->obstructed == false) {
		weapon->weaponSpecific_projectileObstructed(target.x, target.y, projectiles.at(0)->actorHit, eng);
	} else {
		const int element = projectiles.at(0)->obstructedInElement;
		weapon->weaponSpecific_projectileObstructed(projectilePath.at(element).x, projectilePath.at(element).y, projectiles.at(0)->actorHit, eng);
	}
	//Cleanup
	for(unsigned int p = 0; p < projectiles.size(); p++) {
		delete projectiles.at(p);
	}

	eng->renderer->drawMapAndInterface();
	eng->input->clearKeyEvents();
}

bool Attack::ranged(int attackX, int attackY, Weapon* weapon) {
	bool attacked = false;

	Actor* attacker = eng->gameTime->getCurrentActor();

	const bool infAmmo = weapon->getInstanceDefinition().rangedHasInfiniteAmmo;

	//If it's a shotgun, run shotgun function instead of common projectile
	if(weapon->getInstanceDefinition().isShotgun) {
		if(weapon->ammoLoaded != 0 || infAmmo == true) {
			const string soundMessage = weapon->getInstanceDefinition().rangedSoundMessage;
			if(soundMessage != "") {
            const bool IS_LOUD = weapon->getInstanceDefinition().rangedSoundIsLoud;
            bool IS_ALERTING_MONSTERS = attacker == eng->player;
            eng->soundEmitter->emitSound(Sound(soundMessage, true, attacker->pos, IS_LOUD, IS_ALERTING_MONSTERS));
			}
			shotgun(attacker->pos, coord(attackX, attackY), weapon);
			attacked = true;
			weapon->ammoLoaded -= 1;
		}
	} else {
		int nrOfProjectiles = 1;

		//If weapon is a type of machinegun, several projectiles are fired
		if(weapon->getInstanceDefinition().isMachineGun == true) {
			nrOfProjectiles = NUMBER_OF_MACHINEGUN_PROJECTILES_PER_BURST;
		}

		//If weapon has more ammo loaded than projectiles to be fired -
		if(weapon->ammoLoaded >= nrOfProjectiles || infAmmo == true) {
			const string soundMessage = weapon->getInstanceDefinition().rangedSoundMessage;
			if(soundMessage != "") {
            const bool IS_LOUD = weapon->getInstanceDefinition().rangedSoundIsLoud;
            bool IS_ALERTING_MONSTERS = attacker == eng->player;
            eng->soundEmitter->emitSound(Sound(soundMessage, true, attacker->pos, IS_LOUD, IS_ALERTING_MONSTERS));
			}

			// - Shoot.
			projectileFire(attacker->pos, coord(attackX, attackY), weapon, nrOfProjectiles);

			attacked = true;

			//Subtract current loaded ammo.
			if(infAmmo == false) {
				weapon->ammoLoaded -= nrOfProjectiles;
			}
		}
	}

	//Report that actor has used its action
	if(attacked) {
		eng->gameTime->letNextAct();
	}

	return attacked;
}

