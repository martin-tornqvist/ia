#include "Sound.h"

#include <iostream>

#include "Engine.h"
#include "Feature.h"
#include "Map.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "GameTime.h"
#include "MapParsing.h"

bool SoundEmitter::isSoundHeardAtRange(const int RANGE,
                                       const Sound& snd) const {
  return snd.isLoud() ?
         (RANGE <= SND_DIST_LOUD) :
         (RANGE <= SND_DIST_NORMAL);
}

void SoundEmitter::emitSound(Sound snd) {
  bool blockers[MAP_W][MAP_H];
  FeatureStatic* f = NULL;
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      f = eng.map->cells[x][y].featureStatic;
      blockers[x][y] = f->isSoundPassable() == false;
    }
  }
  int floodFill[MAP_W][MAP_H];
  const Pos& origin = snd.getOrigin();
  FloodFill::run(origin, blockers, floodFill, 999, Pos(-1, -1), eng);
  floodFill[origin.x][origin.y] = 0;

  for(Actor * actor : eng.gameTime->actors_) {
    const int FLOOD_VALUE_AT_ACTOR = floodFill[actor->pos.x][actor->pos.y];

    const bool IS_ORIGIN_SEEN_BY_PLAYER =
      eng.map->cells[origin.x][origin.y].isSeenByPlayer;

    if(isSoundHeardAtRange(FLOOD_VALUE_AT_ACTOR, snd)) {
      if(actor == eng.player) {

        //Various conditions may clear the sound message
        if(
          nrSoundMsgPrintedCurTurn_ >= 1 ||
          (IS_ORIGIN_SEEN_BY_PLAYER &&
           snd.getIsMsgIgnoredIfPlayerSeeOrigin())) {
          snd.clearMsg();
        }

        const Dir dirToOrigin =
          getPlayerToOriginDir(
            FLOOD_VALUE_AT_ACTOR, origin, floodFill);

        if(snd.getMsg().empty() == false) {
          //Add a direction string to the message (i.e. "(NW)", "(E)" , etc)
          if(dirToOrigin != endOfDirs) {
            string dirStr;
            DirConverter().getCompassDirName(dirToOrigin, dirStr);
            snd.addString("(" + dirStr + ")");
          }
          nrSoundMsgPrintedCurTurn_++;
        }

        const int SND_MAX_DISTANCE =
          snd.isLoud() ? SND_DIST_LOUD : SND_DIST_NORMAL;
        const int PERCENT_DISTANCE =
          (FLOOD_VALUE_AT_ACTOR * 100) / SND_MAX_DISTANCE;

        eng.player->hearSound(snd, IS_ORIGIN_SEEN_BY_PLAYER,
                              dirToOrigin, PERCENT_DISTANCE);
      } else {
        Monster* const monster = dynamic_cast<Monster*>(actor);
        monster->hearSound(snd);
      }
    }
  }
}

Dir SoundEmitter::getPlayerToOriginDir(
  const int FLOOD_VALUE_AT_PLAYER, const Pos& origin,
  int floodFill[MAP_W][MAP_H]) const {

  const Pos& playerPos = eng.player->pos;
  Dir sourceDir = endOfDirs;

  for(int dx = -1; dx <= 1; dx++) {
    for(int dy = -1; dy <= 1; dy++) {

      const Pos offset(dx, dy);
      const Pos checkedPos = playerPos + offset;

      //If player is next to origin, simply return the direction checked in.
      if(checkedPos == origin) {
        return DirConverter().getDir(offset);
      } else {
        //Origin is further away
        const int currentValue = floodFill[checkedPos.x][checkedPos.y];
        //If current value is less than players,
        //this is the direction of the sound.
        if(currentValue < FLOOD_VALUE_AT_PLAYER && currentValue != 0) {
          sourceDir = DirConverter().getDir(offset);
          //If cardinal direction, stop search
          //(To give priority to cardinal directions)
          if(dx == 0 || dy == 0) {
            return sourceDir;
          }
        }
      }
    }
  }
  return sourceDir;
}

