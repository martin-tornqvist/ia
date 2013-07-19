#include "Attack.h"

#include "Engine.h"

#include "CommonTypes.h"
#include "Item.h"
#include "ItemWeapon.h"
#include "Render.h"
#include "Map.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "Postmortem.h"
#include "Knockback.h"
#include "Input.h"
#include "Log.h"

using namespace std;

void Attack::projectileFire(Actor& attacker, Weapon& wpn, const Pos& aimPos) {
  vector<Projectile*> projectiles;

  const bool IS_MACHINE_GUN = wpn.getDef().isMachineGun;

  const unsigned int NR_PROJECTILES = IS_MACHINE_GUN ?
                                      NUMBER_OF_MACHINEGUN_PROJECTILES_PER_BURST : 1;

  for(unsigned int i = 0; i < NR_PROJECTILES; i++) {
    Projectile* const p = new Projectile;
    p->setAttackData(new RangedAttackData(attacker, wpn, aimPos, attacker.pos, eng));
    projectiles.push_back(p);
  }

  const ActorSizes_t aimLevel =
    projectiles.at(0)->attackData->intendedAimLevel;

  const int DELAY = eng->config->delayProjectileDraw / (IS_MACHINE_GUN ? 2 : 1);

  printRangedInitiateMessages(*projectiles.at(0)->attackData);

//  const Audio_t rangedAudio = weapon->getDef().rangedAudio;
//  if(rangedAudio != audio_none) {
//    eng->audio->playSound(rangedAudio);
//  }

  const bool stopAtTarget = aimLevel == actorSize_floor;
  const int chebTrvlLim = 30;

  //Get projectile path
  const Pos origin = attacker.pos;
  const vector<Pos> projectilePath =
    eng->mapTests->getLine(origin, aimPos, stopAtTarget, chebTrvlLim);

  const SDL_Color projectileColor = wpn.getDef().rangedMissileColor;
  char projectileGlyph = wpn.getDef().rangedMissileGlyph;
  if(projectileGlyph == '/') {
    const int i = projectilePath.size() > 2 ? 2 : 1;
    if(projectilePath.at(i).y == origin.y)
      projectileGlyph = '-';
    if(projectilePath.at(i).x == origin.x)
      projectileGlyph = '|';
    if(
      (projectilePath.at(i).x > origin.x && projectilePath.at(i).y < origin.y) ||
      (projectilePath.at(i).x < origin.x && projectilePath.at(i).y > origin.y))
      projectileGlyph = '/';
    if((
          projectilePath.at(i).x > origin.x && projectilePath.at(i).y > origin.y) ||
        (projectilePath.at(i).x < origin.x && projectilePath.at(i).y < origin.y))
      projectileGlyph = '\\';
  }
  Tile_t projectileTile = wpn.getDef().rangedMissileTile;
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

  //TODO Reimplement projectile trail functionality
//  const bool LEAVE_TRAIL = weapon->getDef().rangedMissileLeavesTrail;

  const unsigned int SIZE_OF_PATH_PLUS_ONE =
    projectilePath.size() + (NR_PROJECTILES - 1) *
    NUMBER_OF_CELLJUMPS_BETWEEN_MACHINEGUN_PROJECTILES;

  for(unsigned int i = 1; i < SIZE_OF_PATH_PLUS_ONE; i++) {

    for(unsigned int p = 0; p < NR_PROJECTILES; p++) {
      //Current projectile's place in the path is the current global place (i)
      //minus a certain number of elements
      int projectilePathElement = i - (p * NUMBER_OF_CELLJUMPS_BETWEEN_MACHINEGUN_PROJECTILES);

      Projectile* const curProj = projectiles.at(p);

      //All the following collision checks etc are only made if the projectiles current path element
      //corresponds to an element in the real path vector
      if(
        projectilePathElement >= 1 &&
        projectilePathElement < int(projectilePath.size()) &&
        curProj->isObstructed == false) {

        curProj->pos = projectilePath.at(projectilePathElement);

        curProj->isVisibleToPlayer =
          eng->map->playerVision[curProj->pos.x][curProj->pos.y];

        //Get attack data again for every cell traveled through
        curProj->setAttackData(
          new RangedAttackData(attacker, wpn, aimPos, curProj->pos , eng, aimLevel));

        const Pos drawPos(curProj->pos);

        //HIT ACTOR?
        if(
          curProj->attackData->currentDefender != NULL &&
          curProj->isObstructed == false) {

          const bool IS_ACTOR_AIMED_FOR = curProj->pos == aimPos;

          if(
            curProj->attackData->currentDefenderSize >= actorSize_humanoid ||
            IS_ACTOR_AIMED_FOR) {

            if(curProj->attackData->attackResult >= successSmall) {
              //RENDER ACTOR HIT
              if(curProj->isVisibleToPlayer) {
                if(eng->config->isTilesMode) {
                  curProj->setTile(tile_blastAnimation1, clrRedLgt);
                  eng->renderer->drawProjectiles(projectiles);
                  eng->sleep(DELAY / 2);
                  curProj->setTile(tile_blastAnimation2, clrRedLgt);
                  eng->renderer->drawProjectiles(projectiles);
                  eng->sleep(DELAY / 2);
                } else {
                  curProj->setGlyph('*', clrRedLgt);
                  eng->renderer->drawProjectiles(projectiles);
                  eng->sleep(DELAY);
                }

                //MESSAGES FOR ACTOR HIT
                printProjectileAtActorMessages(*curProj->attackData, true);
                //Need to draw again here to show log message
                eng->renderer->drawProjectiles(projectiles);
              }

              curProj->isDoneRendering = true;
              curProj->isObstructed = true;
              curProj->actorHit = curProj->attackData->currentDefender;
              curProj->obstructedInElement = projectilePathElement;

              const bool DIED = curProj->attackData->currentDefender->hit(
                                  curProj->attackData->dmg,
                                  wpn.getDef().rangedDmgType);
              if(DIED == false) {
                // Aply weapon hit status effects
                StatusEffectsHandler* const defenderStatusHandler =
                  curProj->attackData->currentDefender->getStatusEffectsHandler();
                defenderStatusHandler->tryAddEffectsFromWeapon(wpn, false);

                // Knock-back?
                if(wpn.getDef().rangedCausesKnockBack) {
                  const AttackData* const curData = curProj->attackData;
                  if(curData->attackResult >= successSmall) {
                    const bool IS_SPIKE_GUN = wpn.getDef().id == item_spikeGun;
                    eng->knockBack->tryKnockBack(curData->currentDefender,
                                                 curData->attacker->pos, IS_SPIKE_GUN);
                  }
                }
              }
            }
          }
        }

        //PROJECTILE HIT FEATURE?
        vector<FeatureMob*> featureMobs = eng->gameTime->getFeatureMobsAtPos(curProj->pos);
        Feature* featureBlockingShot = NULL;
        for(unsigned int featMobIndex = 0; featMobIndex < featureMobs.size(); featMobIndex++) {
          if(featureMobs.at(featMobIndex)->isShootPassable() == false) {
            featureBlockingShot = featureMobs.at(featMobIndex);
          }
        }
        FeatureStatic* featureStatic = eng->map->featuresStatic[curProj->pos.x][curProj->pos.y];
        if(featureStatic->isShootPassable() == false) {
          featureBlockingShot = featureStatic;
        }

        if(featureBlockingShot != NULL && curProj->isObstructed == false) {
          curProj->obstructedInElement = projectilePathElement - 1;
          curProj->isObstructed = true;

          //RENDER FEATURE HIT
          if(curProj->isVisibleToPlayer) {
            if(eng->config->isTilesMode) {
              curProj->setTile(tile_blastAnimation1, clrYellow);
              eng->renderer->drawProjectiles(projectiles);
              eng->sleep(DELAY / 2);
              curProj->setTile(tile_blastAnimation2, clrYellow);
              eng->renderer->drawProjectiles(projectiles);
              eng->sleep(DELAY / 2);
            } else {
              curProj->setGlyph('*', clrYellow);
              eng->renderer->drawProjectiles(projectiles);
              eng->sleep(DELAY);
            }
          }
        }

        //PROJECTILE HIT THE GROUND?
        if(
          curProj->pos == aimPos       &&
          aimLevel == actorSize_floor  &&
          curProj->isObstructed == false) {
          curProj->isObstructed = true;
          curProj->obstructedInElement = projectilePathElement;

          //RENDER GROUND HITS
          if(curProj->isVisibleToPlayer) {
            if(eng->config->isTilesMode) {
              curProj->setTile(tile_blastAnimation1, clrYellow);
              eng->renderer->drawProjectiles(projectiles);
              eng->sleep(DELAY / 2);
              curProj->setTile(tile_blastAnimation2, clrYellow);
              eng->renderer->drawProjectiles(projectiles);
              eng->sleep(DELAY / 2);
            } else {
              curProj->setGlyph('*', clrYellow);
              eng->renderer->drawProjectiles(projectiles);
              eng->sleep(DELAY);
            }
          }
        }

        //RENDER FLYING PROJECTILES
        if(curProj->isObstructed == false && curProj->isVisibleToPlayer) {
          if(eng->config->isTilesMode) {
            curProj->setTile(projectileTile, projectileColor);
            eng->renderer->drawProjectiles(projectiles);
          } else {
            curProj->setGlyph(projectileGlyph, projectileColor);
            eng->renderer->drawProjectiles(projectiles);
          }
        }
      }
    } //End projectile loop

    //If any projectile can be seen and not obstructed, delay
    for(unsigned int nn = 0; nn < projectiles.size(); nn++) {
      if(eng->map->playerVision[projectiles.at(nn)->pos.x][projectiles.at(nn)->pos.y] &&
          projectiles.at(nn)->isObstructed == false) {
        eng->sleep(DELAY);
        nn = 99999;
      }
    }

    //Check if all projectiles obstructed
    bool isAllObstructed = true;
    for(unsigned p = 0; p < projectiles.size(); p++) {
      if(projectiles.at(p)->isObstructed == false) {
        isAllObstructed = false;
      }
    }
    if(isAllObstructed) {
      break;
    }

  } //End path-loop

  /*const bool LEAVE_SMOKE = weapon->getInstanceDefinition().rangedMissileLeavesSmoke;
   if(LEAVE_SMOKE == true) {
   eng->explosionMaker->runSmokeExplosion(projectiles.at(0)->pos, true);
   }*/

  //So far, only projectile 0 can have special obstruction events***
  //Must be changed if something like an assault-incinerator is added
  const Projectile* const projectile = projectiles.at(0);
  if(projectile->isObstructed == false) {
    wpn.weaponSpecific_projectileObstructed(
      aimPos.x, aimPos.y, projectile->actorHit, eng);
  } else {
    const int element = projectile->obstructedInElement;
    const Pos& pos = projectilePath.at(element);
    wpn.weaponSpecific_projectileObstructed(
      pos.x, pos.y, projectile->actorHit, eng);
  }
  //Cleanup
  for(unsigned int i = 0; i < projectiles.size(); i++) {
    delete projectiles.at(i);
  }

  eng->renderer->drawMapAndInterface();
}

bool Attack::ranged(Actor& attacker, Weapon& wpn, const Pos& aimPos) {
  bool didAttack = false;

  const bool IS_ATTACKER_PLAYER = &attacker == eng->player;
  const bool WPN_HAS_INF_AMMO   = wpn.getDef().rangedHasInfiniteAmmo;

  if(wpn.getDef().isShotgun) {
    if(wpn.ammoLoaded != 0 || WPN_HAS_INF_AMMO) {
      shotgun(attacker, wpn, aimPos);

      const string soundMessage = IS_ATTACKER_PLAYER ? "" :
                                  wpn.getDef().rangedSoundMessage;
      if(IS_ATTACKER_PLAYER || soundMessage != "") {
        const bool IS_LOUD = wpn.getDef().rangedSoundIsLoud;
        eng->soundEmitter->emitSound(
          Sound(soundMessage, true, attacker.pos, IS_LOUD, true));
      }

      didAttack = true;
      if(WPN_HAS_INF_AMMO == false) {
        wpn.ammoLoaded -= 1;
      }
    }
  } else {
    int nrOfProjectiles = 1;

    if(wpn.getDef().isMachineGun) {
      nrOfProjectiles = NUMBER_OF_MACHINEGUN_PROJECTILES_PER_BURST;
    }

    if(wpn.ammoLoaded >= nrOfProjectiles || WPN_HAS_INF_AMMO) {
      projectileFire(attacker, wpn, aimPos);

      if(eng->player->deadState == actorDeadState_alive) {
        const string soundMessage = IS_ATTACKER_PLAYER ? "" :
                                    wpn.getDef().rangedSoundMessage;
        if(IS_ATTACKER_PLAYER || soundMessage != "") {
          const bool IS_LOUD = wpn.getDef().rangedSoundIsLoud;
          eng->soundEmitter->emitSound(
            Sound(soundMessage, true, attacker.pos, IS_LOUD, true));
        }

        didAttack = true;

        if(WPN_HAS_INF_AMMO == false) {
          wpn.ammoLoaded -= nrOfProjectiles;
        }
      } else {
        return true;
      }
    }
  }

  eng->renderer->drawMapAndInterface();

  if(didAttack) {
    eng->gameTime->endTurnOfCurrentActor();
  }

  return didAttack;
}

void Attack::printRangedInitiateMessages(const RangedAttackData& data) const {
  if(data.attacker == eng->player)
    eng->log->addMessage("I " + data.verbPlayerAttacks + ".");
  else {
    if(eng->map->playerVision[data.attacker->pos.x][data.attacker->pos.y] == true) {
      const string attackerName = data.attacker->getNameThe();
      const string attackVerb = data.verbOtherAttacks;
      eng->log->addMessage(attackerName + " " + attackVerb + ".", clrWhite, messageInterrupt_force);
    }
  }

  eng->renderer->drawMapAndInterface();
}

void Attack::printProjectileAtActorMessages(const RangedAttackData& data,
    const bool IS_HIT) const {
  //Only print messages if player can see the cell
  const int defX = data.currentDefender->pos.x;
  const int defY = data.currentDefender->pos.y;
  if(eng->map->playerVision[defX][defY]) {
//    if(data.isTargetEthereal == true) {
//      if(data.currentDefender == eng->player) {
//        //Perhaps no text is needed here?
//      } else {
//        eng->log->addMessage("Projectile hits nothing but void.");
//      }
//    } else {
    //Punctuation or exclamation marks depending on attack strength


    if(IS_HIT) {
      string dmgPunctuation = ".";
      const int MAX_DMG_ROLL = data.dmgRolls * data.dmgSides;
      if(MAX_DMG_ROLL >= 4) {
        dmgPunctuation =
          data.dmgRoll > MAX_DMG_ROLL * 5 / 6 ? "!!!" :
          data.dmgRoll > MAX_DMG_ROLL / 2 ? "!" :
          dmgPunctuation;
      }

      if(data.currentDefender == eng->player) {
        eng->log->addMessage("I am hit" + dmgPunctuation, clrMessageBad,
                             messageInterrupt_force);

//          if(data.attackResult == successCritical) {
//            eng->log->addMessage("It was a great hit!", clrMessageBad, messageInterrupt_force);
//          }
      } else {
        string otherName = "It";

        if(eng->map->playerVision[defX][defY] == true)
          otherName = data.currentDefender->getNameThe();

        eng->log->addMessage(otherName + " is hit" + dmgPunctuation, clrMessageGood);

//          if(data.attackResult == successCritical) {
//            eng->log->addMessage("It was a great hit!", clrMessageGood);
//          }
      }
    }
//    }
  }
}

