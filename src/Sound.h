#ifndef SOUND_H
#define SOUND_H

#include <string>

#include "ConstTypes.h"
#include "ItemData.h"
#include "ConstDungeonSettings.h"

using namespace std;

class Engine;
class Actor;

class Sound {
public:
  Sound(
    const string& message, const bool IS_MESSAGE_IGNORED_IF_PLAYER_SEE_ORIGIN, const coord& origin,
    const bool IS_LOUD, const bool IS_ALERTING_MONSTER) :
    message_(message), isMessageIgnoredIfPlayerSeeOrigin_(IS_MESSAGE_IGNORED_IF_PLAYER_SEE_ORIGIN),
    origin_(origin), isLoud_(IS_LOUD), isAlertingMonsters_(IS_ALERTING_MONSTER) {}

  Sound() {
  }
  ~Sound() {
  }

  const string& getMessage() const {
    return message_;
  }

  void clearMessage() {
    message_ = "";
  }

  bool getIsMessageIgnoredIfPlayerSeeOrigin() const {
    return isMessageIgnoredIfPlayerSeeOrigin_;
  }

  coord getOrigin() const {
    return origin_;
  }

  int isLoud() const {
    return isLoud_;
  }

  bool getIsAlertingMonsters() const {
    return isAlertingMonsters_;
  }

  void addString(const string str) {
    message_ += str;
  }

private:
  string message_;
  bool isMessageIgnoredIfPlayerSeeOrigin_;
  coord origin_;
  bool isLoud_;
  bool isAlertingMonsters_;
};

class SoundEmitter {
public:
  SoundEmitter(Engine* engine) :
    eng(engine) {
  }

  void emitSound(Sound sound);

  void resetNrSoundsHeardByPlayerCurTurn() {
    nrSoundsHeardByPlayerCurTurn_ = 0;
  }

private:
  int nrSoundsHeardByPlayerCurTurn_;

  string getPlayerToOriginDirectionString(const int FLOOD_VALUE_AT_PLAYER, const coord& origin, int floodFill[MAP_X_CELLS][MAP_Y_CELLS]) const;

  bool isSoundHeardAtRange(const int RANGE, const Sound& sound) const;

  DirectionNames directionNames;

  Engine* eng;
};

#endif

