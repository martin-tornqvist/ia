#ifndef SOUND_H
#define SOUND_H

#include <string>

#include "CommonData.h"
#include "ItemData.h"
#include "CommonTypes.h"

using namespace std;

class Engine;
class Actor;

enum class SndVol                 {low, high};
enum class AlertsMonsters         {no, yes};
enum class IgnoreMsgIfOriginSeen  {no, yes};

class Snd {
public:
  Snd(
    const string& msg,
    const SfxId sfx,
    const IgnoreMsgIfOriginSeen ignoreMsgIfOriginSeen,
    const Pos& origin,
    Actor* const actorWhoMadeSound, const SndVol vol,
    const AlertsMonsters alertingMonsters) :
    msg_(msg), sfx_(sfx),
    isMsgIgnoredIfOriginSeen_(ignoreMsgIfOriginSeen),
    origin_(origin), actorWhoMadeSound_(actorWhoMadeSound), vol_(vol),
    isAlertingMonsters_(alertingMonsters) {}

  Snd() {}
  ~Snd() {}

  inline const string& getMsg() const {return msg_;}
  inline SfxId getSfx() const {return sfx_;}
  inline void clearMsg() {msg_ = "";}

  inline bool isMsgIgnoredIfOriginSeen() const {
    return isMsgIgnoredIfOriginSeen_ == IgnoreMsgIfOriginSeen::yes;
  }
  inline bool isAlertingMonsters() const {
    return isAlertingMonsters_ == AlertsMonsters::yes;
  }

  inline Pos getOrigin()                const {return origin_;}
  inline Actor* getActorWhoMadeSound()  const {return actorWhoMadeSound_;}
  inline int isLoud()                   const {return vol_ == SndVol::high;}

  void addString(const string str) {
    msg_ += str;
  }

private:
  string msg_;
  SfxId sfx_;
  IgnoreMsgIfOriginSeen isMsgIgnoredIfOriginSeen_;
  Pos origin_;
  Actor* actorWhoMadeSound_;
  SndVol vol_;
  AlertsMonsters isAlertingMonsters_;
};

namespace SndEmit {

void emitSnd(Snd snd, Engine& eng);

void resetNrSoundMsgPrintedCurTurn();

} //SndEmit

#endif

