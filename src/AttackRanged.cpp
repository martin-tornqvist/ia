#include "Attack.h"

#include "Engine.h"

#include "GameTime.h"
#include "CommonTypes.h"
#include "Item.h"
#include "ItemWeapon.h"
#include "Renderer.h"
#include "Map.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "Postmortem.h"
#include "Knockback.h"
#include "Input.h"
#include "Log.h"
#include "Audio.h"
#include "LineCalc.h"
#include "SdlWrapper.h"

using namespace std;

void Attack::projectileFire(Actor& attacker, Weapon& wpn, const Pos& aimPos) {

  const bool IS_ATTACKER_PLAYER = &attacker == eng.player;

  vector<Projectile*> projectiles;

  const bool IS_MACHINE_GUN = wpn.getData().isMachineGun;

  const unsigned int NR_PROJECTILES = IS_MACHINE_GUN ?
                                      NR_MG_PROJECTILES : 1;

  for(unsigned int i = 0; i < NR_PROJECTILES; i++) {
    Projectile* const p = new Projectile;
    p->setAttackData(new RangedAttackData(
                       attacker, wpn, aimPos, attacker.pos, eng));
    projectiles.push_back(p);
  }

  const ActorSize aimLevel =
    projectiles.at(0)->attackData->intendedAimLevel;

  const int DELAY = Config::getDelayProjectileDraw() / (IS_MACHINE_GUN ? 2 : 1);

  printRangedInitiateMessages(*projectiles.at(0)->attackData);

  const bool stopAtTarget = aimLevel == actorSize_floor;
  const int chebTrvlLim = 30;

  //Get projectile path
  const Pos origin = attacker.pos;
  vector<Pos> projectilePath;
  eng.lineCalc->calcNewLine(origin, aimPos, stopAtTarget, chebTrvlLim, false,
                            projectilePath);

  const SDL_Color projectileColor = wpn.getData().rangedMissileClr;
  char projectileGlyph = wpn.getData().rangedMissileGlyph;
  if(projectileGlyph == '/') {
    const int i = projectilePath.size() > 2 ? 2 : 1;
    if(projectilePath.at(i).y == origin.y)
      projectileGlyph = '-';
    if(projectilePath.at(i).x == origin.x)
      projectileGlyph = '|';
    if(
      (projectilePath.at(i).x > origin.x &&
       projectilePath.at(i).y < origin.y) ||
      (projectilePath.at(i).x < origin.x &&
       projectilePath.at(i).y > origin.y))
      projectileGlyph = '/';
    if(
      (projectilePath.at(i).x > origin.x &&
       projectilePath.at(i).y > origin.y) ||
      (projectilePath.at(i).x < origin.x &&
       projectilePath.at(i).y < origin.y))
      projectileGlyph = '\\';
  }
  TileId projectileTile = wpn.getData().rangedMissileTile;
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
//  const bool LEAVE_TRAIL = weapon->getData().rangedMissileLeavesTrail;

  const unsigned int SIZE_OF_PATH_PLUS_ONE =
    projectilePath.size() + (NR_PROJECTILES - 1) *
    NR_CELL_JUMPS_BETWEEN_MG_PROJECTILES;

  for(unsigned int i = 1; i < SIZE_OF_PATH_PLUS_ONE; i++) {

    for(unsigned int p = 0; p < NR_PROJECTILES; p++) {

      //Current projectile's place in the path is the current global place (i)
      //minus a certain number of elements
      int projectilePathElement =
        i - (p * NR_CELL_JUMPS_BETWEEN_MG_PROJECTILES);

      //Emit sound
      if(projectilePathElement == 1) {
        string sndMsg = wpn.getData().rangedSndMsg;
        const SfxId sfx = wpn.getData().rangedAttackSfx;
        if(sndMsg.empty() == false) {
          if(IS_ATTACKER_PLAYER) sndMsg = "";
          const SndVol vol = wpn.getData().rangedSndVol;
          Snd snd(sndMsg, sfx, IgnoreMsgIfOriginSeen::yes, attacker.pos,
                  &attacker, vol, AlertsMonsters::yes);
          eng.sndEmitter->emitSnd(snd);
        }
      }

      Projectile* const curProj = projectiles.at(p);

      //All the following collision checks etc are only made if the projectiles
      //current path element corresponds to an element in the real path vector
      if(
        projectilePathElement >= 1 &&
        projectilePathElement < int(projectilePath.size()) &&
        curProj->isObstructed == false) {

        curProj->pos = projectilePath.at(projectilePathElement);

        curProj->isVisibleToPlayer =
          eng.map->cells[curProj->pos.x][curProj->pos.y].isSeenByPlayer;

        //Get attack data again for every cell traveled through
        curProj->setAttackData(
          new RangedAttackData(
            attacker, wpn, aimPos, curProj->pos , eng, aimLevel));

        const Pos drawPos(curProj->pos);

        //HIT ACTOR?
        if(
          curProj->attackData->curDefender != NULL &&
          curProj->isObstructed == false &&
          curProj->attackData->isEtherealDefenderMissed == false) {

          const bool IS_ACTOR_AIMED_FOR = curProj->pos == aimPos;

          if(
            curProj->attackData->curDefenderSize >= actorSize_humanoid ||
            IS_ACTOR_AIMED_FOR) {

            if(curProj->attackData->attackResult >= successSmall) {
              //RENDER ACTOR HIT
              if(curProj->isVisibleToPlayer) {
                if(Config::isTilesMode()) {
                  curProj->setTile(tile_blast1, clrRedLgt);
                  Renderer::drawProjectiles(projectiles);
                  SdlWrapper::sleep(DELAY / 2);
                  curProj->setTile(tile_blast2, clrRedLgt);
                  Renderer::drawProjectiles(projectiles);
                  SdlWrapper::sleep(DELAY / 2);
                } else {
                  curProj->setGlyph('*', clrRedLgt);
                  Renderer::drawProjectiles(projectiles);
                  SdlWrapper::sleep(DELAY);
                }

                //MESSAGES FOR ACTOR HIT
                printProjectileAtActorMessages(*curProj->attackData, true);
                //Need to draw again here to show log message
                Renderer::drawProjectiles(projectiles);
              }

              curProj->isDoneRendering = true;
              curProj->isObstructed = true;
              curProj->actorHit = curProj->attackData->curDefender;
              curProj->obstructedInElement = projectilePathElement;

              const bool DIED = curProj->attackData->curDefender->hit(
                                  curProj->attackData->dmg,
                                  wpn.getData().rangedDmgType, true);
              if(DIED == false) {
                // Apply weapon hit properties
                PropHandler& defenderPropHandler =
                  curProj->attackData->curDefender->getPropHandler();
                defenderPropHandler.tryApplyPropFromWpn(wpn, false);

                // Knock-back?
                if(wpn.getData().rangedCausesKnockBack) {
                  const AttackData* const curData = curProj->attackData;
                  if(curData->attackResult >= successSmall) {
                    const bool IS_SPIKE_GUN = wpn.getData().id == item_spikeGun;
                    eng.knockBack->tryKnockBack(
                      *(curData->curDefender), curData->attacker->pos,
                      IS_SPIKE_GUN);
                  }
                }
              }
            }
          }
        }

        //PROJECTILE HIT FEATURE?
        vector<FeatureMob*> featureMobs;
        eng.gameTime->getFeatureMobsAtPos(curProj->pos, featureMobs);
        Feature* featureBlockingShot = NULL;
        for(
          unsigned int featMobIndex = 0;
          featMobIndex < featureMobs.size();
          featMobIndex++) {
          if(featureMobs.at(featMobIndex)->isProjectilePassable() == false) {
            featureBlockingShot = featureMobs.at(featMobIndex);
          }
        }
        FeatureStatic* featureStatic =
          eng.map->cells[curProj->pos.x][curProj->pos.y].featureStatic;
        if(featureStatic->isProjectilePassable() == false) {
          featureBlockingShot = featureStatic;
        }

        if(featureBlockingShot != NULL && curProj->isObstructed == false) {
          curProj->obstructedInElement = projectilePathElement - 1;
          curProj->isObstructed = true;

          if(wpn.getData().rangedMakesRicochetSound) {
            Snd snd("I hear a ricochet.", SfxId::ricochet,
                    IgnoreMsgIfOriginSeen::yes, curProj->pos, NULL,
                    SndVol::low, AlertsMonsters::yes);
            eng.sndEmitter->emitSnd(snd);
          }

          //RENDER FEATURE HIT
          if(curProj->isVisibleToPlayer) {
            if(Config::isTilesMode()) {
              curProj->setTile(tile_blast1, clrYellow);
              Renderer::drawProjectiles(projectiles);
              SdlWrapper::sleep(DELAY / 2);
              curProj->setTile(tile_blast2, clrYellow);
              Renderer::drawProjectiles(projectiles);
              SdlWrapper::sleep(DELAY / 2);
            } else {
              curProj->setGlyph('*', clrYellow);
              Renderer::drawProjectiles(projectiles);
              SdlWrapper::sleep(DELAY);
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

          if(wpn.getData().rangedMakesRicochetSound) {
            Snd snd("I hear a ricochet.", SfxId::ricochet,
                    IgnoreMsgIfOriginSeen::yes, curProj->pos, NULL,
                    SndVol::low, AlertsMonsters::yes);
            eng.sndEmitter->emitSnd(snd);
          }

          //RENDER GROUND HITS
          if(curProj->isVisibleToPlayer) {
            if(Config::isTilesMode()) {
              curProj->setTile(tile_blast1, clrYellow);
              Renderer::drawProjectiles(projectiles);
              SdlWrapper::sleep(DELAY / 2);
              curProj->setTile(tile_blast2, clrYellow);
              Renderer::drawProjectiles(projectiles);
              SdlWrapper::sleep(DELAY / 2);
            } else {
              curProj->setGlyph('*', clrYellow);
              Renderer::drawProjectiles(projectiles);
              SdlWrapper::sleep(DELAY);
            }
          }
        }

        //RENDER FLYING PROJECTILES
        if(curProj->isObstructed == false && curProj->isVisibleToPlayer) {
          if(Config::isTilesMode()) {
            curProj->setTile(projectileTile, projectileColor);
            Renderer::drawProjectiles(projectiles);
          } else {
            curProj->setGlyph(projectileGlyph, projectileColor);
            Renderer::drawProjectiles(projectiles);
          }
        }
      }
    } //End projectile loop

    //If any projectile can be seen and not obstructed, delay
    for(unsigned int nn = 0; nn < projectiles.size(); nn++) {
      const Pos& p = projectiles.at(nn)->pos;
      if(eng.map->cells[p.x][p.y].isSeenByPlayer &&
          projectiles.at(nn)->isObstructed == false) {
        SdlWrapper::sleep(DELAY);
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

  //So far, only projectile 0 can have special obstruction events***
  //Must be changed if something like an assault-incinerator is added
  const Projectile* const projectile = projectiles.at(0);
  if(projectile->isObstructed == false) {
    wpn.weaponSpecific_projectileObstructed(
      aimPos, projectile->actorHit);
  } else {
    const int element = projectile->obstructedInElement;
    const Pos& pos = projectilePath.at(element);
    wpn.weaponSpecific_projectileObstructed(
      pos, projectile->actorHit);
  }
  //Cleanup
  for(unsigned int i = 0; i < projectiles.size(); i++) {
    delete projectiles.at(i);
  }

  Renderer::drawMapAndInterface();
}

bool Attack::ranged(Actor& attacker, Weapon& wpn, const Pos& aimPos) {
  bool didAttack = false;

  const bool WPN_HAS_INF_AMMO = wpn.getData().rangedHasInfiniteAmmo;

  if(wpn.getData().isShotgun) {
    if(wpn.nrAmmoLoaded != 0 || WPN_HAS_INF_AMMO) {

      shotgun(attacker, wpn, aimPos);

      didAttack = true;
      if(WPN_HAS_INF_AMMO == false) {
        wpn.nrAmmoLoaded -= 1;
      }
    }
  } else {
    int nrOfProjectiles = 1;

    if(wpn.getData().isMachineGun) {
      nrOfProjectiles = NR_MG_PROJECTILES;
    }

    if(wpn.nrAmmoLoaded >= nrOfProjectiles || WPN_HAS_INF_AMMO) {
      projectileFire(attacker, wpn, aimPos);

      if(eng.player->deadState == ActorDeadState::alive) {

        didAttack = true;

        if(WPN_HAS_INF_AMMO == false) {
          wpn.nrAmmoLoaded -= nrOfProjectiles;
        }
      } else {
        return true;
      }
    }
  }

  Renderer::drawMapAndInterface();

  if(didAttack) {eng.gameTime->actorDidAct();}

  return didAttack;
}

void Attack::printRangedInitiateMessages(const RangedAttackData& data) const {
  if(data.attacker == eng.player)
    eng.log->addMsg("I " + data.verbPlayerAttacks + ".");
  else {
    const Pos& p = data.attacker->pos;
    if(eng.map->cells[p.x][p.y].isSeenByPlayer) {
      const string attackerName = data.attacker->getNameThe();
      const string attackVerb = data.verbOtherAttacks;
      eng.log->addMsg(attackerName + " " + attackVerb + ".", clrWhite, true);
    }
  }
}

void Attack::printProjectileAtActorMessages(const RangedAttackData& data,
    const bool IS_HIT) const {
  //Only print messages if player can see the cell
  const int defX = data.curDefender->pos.x;
  const int defY = data.curDefender->pos.y;
  if(eng.map->cells[defX][defY].isSeenByPlayer) {

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

      if(data.curDefender == eng.player) {
        eng.log->addMsg("I am hit" + dmgPunctuation, clrMsgBad, true);
      } else {
        string otherName = "It";

        if(eng.map->cells[defX][defY].isSeenByPlayer)
          otherName = data.curDefender->getNameThe();

        eng.log->addMsg(otherName + " is hit" + dmgPunctuation, clrMsgGood);
      }
    }
  }
}

