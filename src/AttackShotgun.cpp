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
#include "SdlWrapper.h"

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
    if(IS_ATTACKER_PLAYER) {sndMsg = "";}
    const SndVol vol = wpn.getData().rangedSndVol;
    const SfxId sfx = wpn.getData().rangedAttackSfx;
    Snd snd(sndMsg, sfx, IgnoreMsgIfOriginSeen::yes, attacker.pos, &attacker,
            vol, AlertsMonsters::yes);
    SndEmit::emitSnd(snd, eng);
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
            Renderer::drawMapAndInterface(false);
            Renderer::coverCellInMap(curPos);
            if(Config::isTilesMode()) {
              Renderer::drawTile(tile_blast2, Panel::map, curPos, clrRedLgt);
            } else {
              Renderer::drawGlyph('*', Panel::map, curPos, clrRedLgt);
            }
            Renderer::updateScreen();
            SdlWrapper::sleep(Config::getDelayShotgun());
          }

          //Messages
          printProjectileAtActorMessages(*data, true);

          //Damage
          data->curDefender->hit(
            data->dmg, wpn.getData().rangedDmgType, true);

          nrActorsHit++;

          Renderer::drawMapAndInterface();

          //Special shotgun behavior:
          //If current defender was killed, and player aimed at humanoid level,
          //or at floor level but beyond the current position, the shot will
          //continue one cell.
          const bool IS_TARGET_KILLED =
            data->curDefender->deadState != ActorDeadState::alive;
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
      Snd snd("I hear a ricochet.", SfxId::ricochet, IgnoreMsgIfOriginSeen::yes,
              curPos, NULL, SndVol::low, AlertsMonsters::yes);
      SndEmit::emitSnd(snd, eng);

      if(eng.map->cells[curPos.x][curPos.y].isSeenByPlayer) {
        Renderer::drawMapAndInterface(false);
        Renderer::coverCellInMap(curPos);
        if(Config::isTilesMode()) {
          Renderer::drawTile(tile_blast2, Panel::map, curPos, clrYellow);
        } else {
          Renderer::drawGlyph('*', Panel::map, curPos, clrYellow);
        }
        Renderer::updateScreen();
        SdlWrapper::sleep(Config::getDelayShotgun());
        Renderer::drawMapAndInterface();
      }
      break;
    }

    //Floor hit?
    if(intendedAimLevel == actorSize_floor && curPos == aimPos) {
      Snd snd("I hear a ricochet.", SfxId::ricochet, IgnoreMsgIfOriginSeen::yes,
              curPos, NULL, SndVol::low, AlertsMonsters::yes);
      SndEmit::emitSnd(snd, eng);

      if(eng.map->cells[curPos.x][curPos.y].isSeenByPlayer) {
        Renderer::drawMapAndInterface(false);
        Renderer::coverCellInMap(curPos);
        if(Config::isTilesMode()) {
          Renderer::drawTile(tile_blast2, Panel::map, curPos, clrYellow);
        } else {
          Renderer::drawGlyph('*', Panel::map, curPos, clrYellow);
        }
        Renderer::updateScreen();
        SdlWrapper::sleep(Config::getDelayShotgun());
        Renderer::drawMapAndInterface();
      }
      break;
    }
  }
}
