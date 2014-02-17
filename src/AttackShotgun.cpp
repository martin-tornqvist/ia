#include "Attack.h"

#include "Engine.h"

#include "Renderer.h"
#include "Item.h"
#include "ItemWeapon.h"
#include "GameTime.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "Map.h"
#include "Postmortem.h"
#include "MapParsing.h"
#include "LineCalc.h"
#include "Utils.h"

void Attack::shotgun(Actor& attacker, const Weapon& wpn, const Pos& aimPos) {

  RangedAttackData* data = new RangedAttackData(
    attacker, wpn, aimPos, attacker.pos, eng);

  printRangedInitiateMessages(*data);

  const ActorSize intendedAimLevel = data->intendedAimLevel;

  bool featureBlockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksProjectiles(eng), featureBlockers);

  Actor* actorArray[MAP_W][MAP_H];
  Utils::makeActorArray(actorArray, eng);

  const Pos origin = attacker.pos;
  vector<Pos> path;
  eng.lineCalc->calcNewLine(origin, aimPos, false, 9999, false, path);

  int nrActorsHit = 0;

  int monsterKilledInElement = -1;

  //Emit sound
  const bool IS_ATTACKER_PLAYER = &attacker == eng.player;
  string sndMsg = wpn.getData().rangedSndMsg;
  if(sndMsg.empty() == false) {
    sndMsg = IS_ATTACKER_PLAYER ? "" : sndMsg;
    const SndVol vol = wpn.getData().rangedSndVol;
    const SfxId sfx = wpn.getData().rangedAttackSfx;
    Snd snd(sndMsg, sfx, IgnoreMsgIfOriginSeen::yes, attacker.pos, &attacker,
            vol, AlertsMonsters::yes);
    eng.sndEmitter->emitSnd(snd);
  }

  for(unsigned int i = 1; i < path.size(); i++) {
    //If travelled further than two steps after a killed monster, stop projectile.
    if(monsterKilledInElement != -1) {
      if(i > (unsigned int)(monsterKilledInElement + 1)) {
        break;
      }
    }

    const Pos curPos(path.at(i));

    if(actorArray[curPos.x][curPos.y] != NULL) {

      //Only attempt hit if aiming at a level that would hit the actor
      const ActorSize sizeOfActor =
        actorArray[curPos.x][curPos.y]->getData().actorSize;
      if(sizeOfActor >= actorSize_humanoid || curPos == aimPos) {

        //Actor hit?
        delete data;
        data = new RangedAttackData(
          attacker, wpn, aimPos, curPos, eng, intendedAimLevel);
        const bool IS_WITHIN_RANGE_LMT =
          Utils::chebyshevDist(origin, curPos) <=
          wpn.effectiveRangeLimit;
        if(
          IS_WITHIN_RANGE_LMT &&
          data->attackResult >= successSmall &&
          data->isEtherealDefenderMissed == false) {
          if(eng.map->cells[curPos.x][curPos.y].isSeenByPlayer) {
            eng.renderer->drawMapAndInterface(false);
            eng.renderer->coverCellInMap(curPos);
            if(eng.config->isTilesMode) {
              eng.renderer->drawTile(tile_blast2, panel_map, curPos, clrRedLgt);
            } else {
              eng.renderer->drawGlyph('*', panel_map, curPos, clrRedLgt);
            }
            eng.renderer->updateScreen();
            eng.sleep(eng.config->delayShotgun);
          }

          //Messages
          printProjectileAtActorMessages(*data, true);

          //Damage
          data->curDefender->hit(
            data->dmg, wpn.getData().rangedDmgType, true);

          nrActorsHit++;

          eng.renderer->drawMapAndInterface();

          //Special shotgun behavior:
          //If current defender was killed, and player aimed at humanoid level,
          //or at floor level but beyond the current position, the shot will
          //continue one cell.
          const bool IS_TARGET_KILLED =
            data->curDefender->deadState != actorDeadState_alive;
          if(IS_TARGET_KILLED && monsterKilledInElement == -1) {
            monsterKilledInElement = i;
          }
          if(
            (nrActorsHit >= 2) ||
            (IS_TARGET_KILLED == false) ||
            (intendedAimLevel == actorSize_floor && curPos == aimPos)) {
            break;
          }
        }
      }
    }

    //Wall hit?
    if(featureBlockers[curPos.x][curPos.y]) {
      Snd snd("I hear a ricochet.", sfxRicochet, IgnoreMsgIfOriginSeen::yes,
              curPos, NULL, SndVol::low, AlertsMonsters::yes);
      eng.sndEmitter->emitSnd(snd);

      if(eng.map->cells[curPos.x][curPos.y].isSeenByPlayer) {
        eng.renderer->drawMapAndInterface(false);
        eng.renderer->coverCellInMap(curPos);
        if(eng.config->isTilesMode) {
          eng.renderer->drawTile(tile_blast2, panel_map, curPos, clrYellow);
        } else {
          eng.renderer->drawGlyph('*', panel_map, curPos, clrYellow);
        }
        eng.renderer->updateScreen();
        eng.sleep(eng.config->delayShotgun);
        eng.renderer->drawMapAndInterface();
      }
      break;
    }

    //Floor hit?
    if(intendedAimLevel == actorSize_floor && curPos == aimPos) {
      Snd snd("I hear a ricochet.", sfxRicochet, IgnoreMsgIfOriginSeen::yes,
              curPos, NULL, SndVol::low, AlertsMonsters::yes);
      eng.sndEmitter->emitSnd(snd);

      if(eng.map->cells[curPos.x][curPos.y].isSeenByPlayer) {
        eng.renderer->drawMapAndInterface(false);
        eng.renderer->coverCellInMap(curPos);
        if(eng.config->isTilesMode) {
          eng.renderer->drawTile(tile_blast2, panel_map, curPos, clrYellow);
        } else {
          eng.renderer->drawGlyph('*', panel_map, curPos, clrYellow);
        }
        eng.renderer->updateScreen();
        eng.sleep(eng.config->delayShotgun);
        eng.renderer->drawMapAndInterface();
      }
      break;
    }
  }
}
