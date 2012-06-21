#include "Attack.h"

#include "Engine.h"

#include "Item.h"
#include "ItemWeapon.h"
#include "GameTime.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "Map.h"
#include "Postmortem.h"
#include "Timer.h"

struct ShotGunProjectile {
	ShotGunProjectile() {
		path.resize(0);
	}
	vector<coord> path;

	void init(double xIncr_, double yIncr_) {
		xIncr = xIncr_;
		yIncr = yIncr_;
	}

	double xIncr, yIncr;
};

bool Attack::isCellOnLine(vector<coord> line, int x, int y) {
	for(unsigned int i = 0; i < line.size(); i++) {
		if(line.at(i).x == x && line.at(i).y == y) {
			return true;
		}
	}

	return false;
}

void Attack::shotgun(const coord origin, const coord target, Weapon* const weapon) {
	AttackData data;
	getAttackData(data, target, origin, weapon, false);
	printRangedInitiateMessages(data);

	//	const Audio_t rangedAudio = weapon->getInstanceDefinition().rangedAudio;
	//	if(rangedAudio != audio_none) {
	//		eng->audio->playSound(rangedAudio);
	//	}

	const int CHEB_TO_TARGET = eng->basicUtils->chebyshevDistance(origin, target);

	const double deltaX = (static_cast<double> (target.x) - static_cast<double> (origin.x));
	const double deltaY = (static_cast<double> (target.y) - static_cast<double> (origin.y));

	const double hypot = sqrt((deltaX * deltaX) + (deltaY * deltaY));
	const double baseAngle = asin(deltaY / hypot);

	//This value is -1 or 1
	const int xDir = -1 + (target.x - origin.x >= 0) * 2;

	vector<ShotGunProjectile> projectiles;
	projectiles.resize(0);

	ShotGunProjectile p;
	double xIncr, yIncr;

	//Initialize vectors
	for(double i = -shotgunSpreadAngleHalf; i <= shotgunSpreadAngleHalf + 0.004; i += 0.004) {
		xIncr = cos(baseAngle + i) * double(xDir);
		yIncr = sin(baseAngle + i);
		p.init(xIncr, yIncr);
		projectiles.push_back(p);
	}

	//Also add precise base angle-vector so it can shoot past diagonal obstacles
	xIncr = cos(baseAngle) * double(xDir);
	yIncr = sin(baseAngle);
	p.init(xIncr, yIncr);
	projectiles.push_back(p);

	double curX_prec, curY_prec;
	int curX, curY;
	coord c;
	bool hitArray[MAP_X_CELLS][MAP_Y_CELLS] = { };

	//Array of blocking cells
	// -1 = cell is passable and will always be passable
	//  0 = cell is passable, but may be set to -1 or 1
	//  1 = cell is not passable, and will never be passable
	int blockingArray[MAP_X_CELLS][MAP_Y_CELLS] = { };

	//Add blocking features to array
	bool featureBlockers[MAP_X_CELLS][MAP_Y_CELLS];
	eng->mapTests->makeShootBlockerFeaturesArray(featureBlockers);
	for(int x = 0; x < MAP_X_CELLS; x++) {
		for(int y = 0; y < MAP_Y_CELLS; y++) {
			if(featureBlockers[x][y]) {
				blockingArray[x][y] = 1;
			}
		}
	}

	//An array of actors - so that each projectile doesn't have to check the time loop each step
	Actor* actorArray[MAP_X_CELLS][MAP_Y_CELLS] = { };
	Actor* curActor = NULL;
	const unsigned int ENTITY_LOOP_SIZE = eng->gameTime->getLoopSize();
	for(unsigned int i = 0; i < ENTITY_LOOP_SIZE; i++) {
		curActor = eng->gameTime->getActorAt(i);
		if(curActor->deadState == actorDeadState_alive) {
			actorArray[curActor->pos.x][curActor->pos.y] = curActor;
		}
	}

	//Get main line - only hits on this path has full damage potential.
	vector<coord> mainLine = eng->mapTests->getLine(origin.x, origin.y, target.x, target.y, false, 99999);

	//% Chance for stray hit
	const int probabStrayHit = 50; //Perhaps this should depend on skill?
	const int strayShotDivide = 2;
	bool canGetCleanHit = true;
	bool projectileActive = true;

	eng->renderer->drawMapAndInterface();

	//Get hit locations
	for(unsigned pElement = 0; pElement < projectiles.size(); pElement++) {

		curX_prec = static_cast<double> (origin.x) + 0.5;
		curY_prec = static_cast<double> (origin.y) + 0.5;
		curX_prec += projectiles.at(pElement).xIncr;
		curY_prec += projectiles.at(pElement).yIncr;

		projectileActive = true;

		for(double i = 0; i <= FOV_RADI; i += 0.004) {
			curX_prec += projectiles.at(pElement).xIncr * 0.004;
			curY_prec += projectiles.at(pElement).yIncr * 0.004;
			curX = int(curX_prec);
			curY = int(curY_prec);

			if(blockingArray[curX][curY] == 1) {
				hitArray[curX][curY] = true;
				projectileActive = false;
				i = 99999;
			}

			//If cell hasn't been considered before
			if(blockingArray[curX][curY] == 0) {
				getAttackData(data, target, coord(curX, curY), weapon, false);
				//If there is a defender in current cell, and current defender is at least equal to aim-level
				if(actorArray[curX][curY] != NULL && (data.currentDefenderSize >= data.aimLevel) == true) {
					//If both aim level and current defender is on floor level, it must be within 1 cell of target
					if((data.aimLevel == actorSize_floor && data.currentDefenderSize == actorSize_floor && eng->basicUtils->chebyshevDistance(target,
							coord(curX, curY)) > 1) == false) {

						//If cell is on main line, and distance is within effective weapon distance,
						//a chance for a clean hit is given
						if(isCellOnLine(mainLine, curX, curY) == true && canGetCleanHit == true) {
							if(eng->basicUtils->chebyshevDistance(origin, coord(curX, curY)) <= weapon->effectiveRangeLimit) {
								if(data.attackResult >= successSmall) {

									if(eng->map->playerVision[curX][curY] == true) {
										if(eng->config->USE_TILE_SET) {
											eng->renderer->drawTileInMap(tile_blastAnimation2, curX, curY, clrRedLight);
										} else {
											eng->renderer->drawCharacter('*', renderArea_mainScreen, curX, curY, clrRedLight);
										}
									}

									//Messages
									printProjectileAtActorMessages(data, projectileHitType_cleanHit);

									//Damage
									const bool DIED = data.currentDefender->hit(data.dmg, weapon->getInstanceDefinition().rangedDamageType);
									if(DIED == true) {
										if(data.currentDefender == eng->player) {
											eng->postmortem->setCauseOfDeath(weapon->getInstanceDefinition().causeOfDeathMessage);
										}
									}

									blockingArray[curX][curY] = 1;

									projectileActive = false;
									i = 99999;
								}
							}

							canGetCleanHit = false;
						}

						if(projectileActive == true) {
							//If this point in the code is reached - that means either -
							//A - Actor was not on main line,
							//B - Actor was not within effective range (Chebyshev), or
							//C - Skill roll failed.
							//(It also means that the cell has not been considered before)

							//In any and all of these cases a chance for a stray hit is given

							getAttackData(data, target, coord(curX, curY), weapon, false);

							if(eng->dice(1, 100) < probabStrayHit) {

								if(eng->map->playerVision[curX][curY] == true) {
									if(eng->config->USE_TILE_SET) {
										eng->renderer->drawTileInMap(tile_blastAnimation2, curX, curY, clrRedLight);
									} else {
										eng->renderer->drawCharacter('*', renderArea_mainScreen, curX, curY, clrRedLight);
									}
								}

								data.attackResult = successSmall;

								//Messages
								printProjectileAtActorMessages(data, projectileHitType_strayHit);

								//Damage
								data.currentDefender->hit(data.dmg / strayShotDivide, weapon->getInstanceDefinition().rangedDamageType);

								blockingArray[curX][curY] = 1;

								projectileActive = false;
								i = 99999;
							} else {
								data.attackResult = failNormal;

								if(data.aimLevel == actorSize_floor) {
									if(eng->map->playerVision[curX][curY] == true) {
										if(eng->config->USE_TILE_SET) {
											eng->renderer->drawTileInMap(tile_blastAnimation2, curX, curY, clrRedLight);
										} else {
											eng->renderer->drawCharacter('*', renderArea_mainScreen, curX, curY, clrRedLight);
										}
									}
								}

								blockingArray[curX][curY] = -1;
								projectileActive = false;

								printProjectileAtActorMessages(data, projectileHitType_miss);
							}
						}
					} else {
						blockingArray[curX][curY] = -1;
					}
				} else {
					blockingArray[curX][curY] = -1;
				}
			}

			//Floor hit?
			if(data.aimLevel == actorSize_floor && actorArray[curX][curY] == NULL && (eng->basicUtils->chebyshevDistance(origin, coord(curX, curY))
					== CHEB_TO_TARGET + 1 || (curX == target.x && curY == target.y))) {

				hitArray[curX][curY] = true;
				blockingArray[curX][curY] = 1;
				projectileActive = false;
			}
		}
	}

	//Draw projectiles hitting walls and floor
	for(int x = 0; x < MAP_X_CELLS; x++) {
		for(int y = 0; y < MAP_Y_CELLS; y++) {
			//Cell hit with no actor there means it's a wall or something...
			if(hitArray[x][y] == true && actorArray[x][y] == NULL) {
				if(eng->map->playerVision[x][y] == true) {
					if(eng->config->USE_TILE_SET) {
						eng->renderer->drawTileInMap(tile_blastAnimation2, x, y, clrYellow);
					} else {
						eng->renderer->drawCharacter('*', renderArea_mainScreen, x, y, clrYellow);
					}
				}
			}
		}
	}

	eng->renderer->flip();

	Timer t;
	t.start();
	while(t.get_ticks() < eng->config->DELAY_SHOTGUN) {
	}

	eng->renderer->drawMapAndInterface();
}

