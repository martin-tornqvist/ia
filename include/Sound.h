#ifndef SOUND_H
#define SOUND_H

#include <string>

#include "CommonData.h"
#include "ItemData.h"
#include "CommonTypes.h"

using namespace std;

class Engine;
class Actor;

class Sound {
public:
  Sound(
    const string& msg, const Sfx_t sfx,
    const bool IS_MSG_IGNORED_IF_PLAYER_SEE_ORIG, const Pos& origin,
    const bool IS_LOUD, const bool IS_ALERTING_MONSTER) :
    msg_(msg), sfx_(sfx),
    isMsgIgnoredIfPlayerSeeOrig_(IS_MSG_IGNORED_IF_PLAYER_SEE_ORIG),
    origin_(origin), isLoud_(IS_LOUD),
    isAlertingMonsters_(IS_ALERTING_MONSTER) {}

  Sound() {}
  ~Sound() {}

  inline const string& getMsg() const {return msg_;}
  inline Sfx_t getSfx() const {return sfx_;}
  inline void clearMsg() {msg_ = "";}

  inline bool getIsMsgIgnoredIfPlayerSeeOrigin() const {
    return isMsgIgnoredIfPlayerSeeOrig_;
  }

  inline Pos getOrigin() const {return origin_;}
  inline int isLoud() const {return isLoud_; }
  inline bool getIsAlertingMonsters() const {return isAlertingMonsters_;}

  void addString(const string str) {
    msg_ += str;
  }

private:
  string msg_;
  Sfx_t sfx_;
  bool isMsgIgnoredIfPlayerSeeOrig_;
  Pos origin_;
  bool isLoud_;
  bool isAlertingMonsters_;
};

class SoundEmitter {
public:
  SoundEmitter(Engine* engine) :
    NR_STEPS_HEARD_NORMAL(10), NR_STEPS_HEARD_LOUD(25), eng(engine) {}

  void emitSound(Sound snd);

  void resetNrSoundMsgPrintedCurTurn() {
    nrSoundMsgPrintedCurTurn_ = 0;
  }

private:
  int nrSoundMsgPrintedCurTurn_;

  Dir_t getPlayerToOriginDir(
    const int FLOOD_VALUE_AT_PLAYER,
    const Pos& origin, int floodFill[MAP_X_CELLS][MAP_Y_CELLS]) const;

  bool isSoundHeardAtRange(const int RANGE, const Sound& snd) const;

  const int NR_STEPS_HEARD_NORMAL;
  const int NR_STEPS_HEARD_LOUD;

  Engine* eng;
};

#endif

