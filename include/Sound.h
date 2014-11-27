#ifndef SOUND_H
#define SOUND_H

#include <string>

#include "CmnData.h"
#include "ItemData.h"
#include "CmnTypes.h"

class Actor;

enum class SndVol                 {low, high};
enum class AlertsMon         {no, yes};
enum class IgnoreMsgIfOriginSeen  {no, yes};

class Snd
{
public:
  Snd(
    const std::string& msg,
    const SfxId sfx,
    const IgnoreMsgIfOriginSeen ignoreMsgIfOriginSeen,
    const Pos& origin,
    Actor* const actorWhoMadeSound, const SndVol vol,
    const AlertsMon alertingMon) :
    msg_(msg), sfx_(sfx),
    isMsgIgnoredIfOriginSeen_(ignoreMsgIfOriginSeen),
    origin_(origin), actorWhoMadeSound_(actorWhoMadeSound), vol_(vol),
    isAlertingMon_(alertingMon) {}

  Snd() {}
  ~Snd() {}

  const std::string& getMsg() const {return msg_;}
  SfxId getSfx() const {return sfx_;}
  void clearMsg() {msg_ = "";}

  bool isMsgIgnoredIfOriginSeen() const
  {
    return isMsgIgnoredIfOriginSeen_ == IgnoreMsgIfOriginSeen::yes;
  }
  bool isAlertingMon() const
  {
    return isAlertingMon_ == AlertsMon::yes;
  }

  Pos getOrigin()                const {return origin_;}
  Actor* getActorWhoMadeSound()  const {return actorWhoMadeSound_;}
  int isLoud()                   const {return vol_ == SndVol::high;}

  void addString(const std::string& str)
  {
    msg_ += str;
  }

private:
  std::string msg_;
  SfxId sfx_;
  IgnoreMsgIfOriginSeen isMsgIgnoredIfOriginSeen_;
  Pos origin_;
  Actor* actorWhoMadeSound_;
  SndVol vol_;
  AlertsMon isAlertingMon_;
};

namespace SndEmit
{

void emitSnd(Snd snd);

void resetNrSndMsgPrintedCurTurn();

} //SndEmit

#endif

