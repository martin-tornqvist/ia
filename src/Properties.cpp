#include "Properties.h"

#include <algorithm>
#include <assert.h>

#include "Init.h"
#include "ItemArmor.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Postmortem.h"
#include "Renderer.h"
#include "ItemWeapon.h"
#include "ActorMonster.h"
#include "Inventory.h"
#include "Map.h"
#include "Explosion.h"
#include "PlayerBon.h"
#include "MapParsing.h"
#include "LineCalc.h"
#include "ActorFactory.h"
#include "Utils.h"
#include "FeatureRigid.h"
#include "FeatureMob.h"
#include "ItemExplosive.h"

using namespace std;

namespace PropData {

PropDataT data[endOfPropIds];

namespace {

void addPropData(PropDataT& d) {
  data[d.id] = d;
  PropDataT blank;
  d = blank;
}

void initDataList() {
  PropDataT d;

  d.id = propRPhys;
  d.stdRndTurns = Range(40, 60);
  d.name = "Physical Resistance";
  d.nameShort = "RPhys";
  d.msg[propMsgOnStartPlayer] = "I feel resistant to physical harm.";
  d.msg[propMsgOnStartMonster] = "is resistant to physical harm.";
  d.msg[propMsgOnEndPlayer] = "I feel vulnerable to physical harm.";
  d.msg[propMsgOnEndMonster] = "is vulnerable to physical harm.";
  d.msg[propMsgOnMorePlayer] = "I feel more resistant to physical harm.";
  d.msg[propMsgOnMoreMonster] = "is more resistant to physical harm.";
  d.msg[propMsgOnResPlayer] = "";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.allowTestOnBot = true;
  d.alignment = propAlignmentGood;
  addPropData(d);

  d.id = propRFire;
  d.stdRndTurns = Range(40, 60);
  d.name = "Fire resistance";
  d.nameShort = "RFire";
  d.msg[propMsgOnStartPlayer] = "I feel resistant to fire.";
  d.msg[propMsgOnStartMonster] = "is resistant to fire.";
  d.msg[propMsgOnEndPlayer] = "I feel vulnerable to fire.";
  d.msg[propMsgOnEndMonster] = "is vulnerable to fire.";
  d.msg[propMsgOnMorePlayer] = "I feel more resistant to fire.";
  d.msg[propMsgOnMoreMonster] = "is more resistant to fire.";
  d.msg[propMsgOnResPlayer] = "";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.allowTestOnBot = true;
  d.alignment = propAlignmentGood;
  addPropData(d);

  d.id = propRCold;
  d.stdRndTurns = Range(40, 60);
  d.name = "Cold resistance";
  d.nameShort = "RCold";
  d.msg[propMsgOnStartPlayer] = "I feel resistant to cold.";
  d.msg[propMsgOnStartMonster] = "is resistant to cold.";
  d.msg[propMsgOnEndPlayer] = "I feel vulnerable to cold.";
  d.msg[propMsgOnEndMonster] = "is vulnerable to cold.";
  d.msg[propMsgOnMorePlayer] = "I feel more resistant to cold.";
  d.msg[propMsgOnMoreMonster] = "is more resistant to cold.";
  d.msg[propMsgOnResPlayer] = "";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.allowTestOnBot = true;
  d.alignment = propAlignmentGood;
  addPropData(d);

  d.id = propRPoison;
  d.stdRndTurns = Range(40, 60);
  d.name = "Poison resistance";
  d.nameShort = "RPoison";
  d.msg[propMsgOnStartPlayer] = "I feel resistant to poison.";
  d.msg[propMsgOnStartMonster] = "is resistant to poison.";
  d.msg[propMsgOnEndPlayer] = "I feel vulnerable to poison.";
  d.msg[propMsgOnEndMonster] = "is vulnerable to poison.";
  d.msg[propMsgOnMorePlayer] = "I feel more resistant to poison.";
  d.msg[propMsgOnMoreMonster] = "is more resistant to poison.";
  d.msg[propMsgOnResPlayer] = "";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.allowTestOnBot = true;
  d.alignment = propAlignmentGood;
  addPropData(d);

  d.id = propRElec;
  d.stdRndTurns = Range(40, 60);
  d.name = "Electric resistance";
  d.nameShort = "RElec";
  d.msg[propMsgOnStartPlayer] = "I feel resistant to electricity.";
  d.msg[propMsgOnStartMonster] = "is resistant to electricity.";
  d.msg[propMsgOnEndPlayer] = "I feel vulnerable to electricity.";
  d.msg[propMsgOnEndMonster] = "is vulnerable to electricity.";
  d.msg[propMsgOnMorePlayer] = "I feel more resistant to electricity.";
  d.msg[propMsgOnMoreMonster] = "is more resistant to electricity.";
  d.msg[propMsgOnResPlayer] = "";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.allowTestOnBot = true;
  d.alignment = propAlignmentGood;
  addPropData(d);

  d.id = propRAcid;
  d.stdRndTurns = Range(40, 60);
  d.name = "Acid resistance";
  d.nameShort = "RAcid";
  d.msg[propMsgOnStartPlayer] = "I feel resistant to acid.";
  d.msg[propMsgOnStartMonster] = "is resistant to acid.";
  d.msg[propMsgOnEndPlayer] = "I feel vulnerable to acid.";
  d.msg[propMsgOnEndMonster] = "is vulnerable to acid.";
  d.msg[propMsgOnMorePlayer] = "I feel more resistant to acid.";
  d.msg[propMsgOnMoreMonster] = "is more resistant to acid.";
  d.msg[propMsgOnResPlayer] = "";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.allowTestOnBot = true;
  d.alignment = propAlignmentGood;
  addPropData(d);

  d.id = propRSleep;
  d.stdRndTurns = Range(40, 60);
  d.name = "Sleep resistance";
  d.nameShort = "RSleep";
  d.msg[propMsgOnStartPlayer] = "I feel resistant to sleep.";
  d.msg[propMsgOnStartMonster] = "is resistant to sleep.";
  d.msg[propMsgOnEndPlayer] = "I feel vulnerable to sleep.";
  d.msg[propMsgOnEndMonster] = "is vulnerable to sleep.";
  d.msg[propMsgOnMorePlayer] = "I feel more resistant to sleep.";
  d.msg[propMsgOnMoreMonster] = "is more resistant to sleep.";
  d.msg[propMsgOnResPlayer] = "";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.allowTestOnBot = true;
  d.alignment = propAlignmentGood;
  addPropData(d);

  d.id = propRFear;
  d.stdRndTurns = Range(40, 60);
  d.name = "Fear resistance";
  d.nameShort = "RFear";
  d.msg[propMsgOnStartPlayer] = "I feel resistant to fear.";
  d.msg[propMsgOnStartMonster] = "is resistant to fear.";
  d.msg[propMsgOnEndPlayer] = "I feel vulnerable to fear.";
  d.msg[propMsgOnEndMonster] = "is vulnerable to fear.";
  d.msg[propMsgOnMorePlayer] = "I feel more resistant to fear.";
  d.msg[propMsgOnMoreMonster] = "is more resistant to fear.";
  d.msg[propMsgOnResPlayer] = "";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.allowTestOnBot = true;
  d.alignment = propAlignmentGood;
  addPropData(d);

  d.id = propRConfusion;
  d.stdRndTurns = Range(40, 50);
  d.name = "Confusion resistance";
  d.nameShort = "RConfusion";
  d.msg[propMsgOnStartPlayer] = "I feel resistant to confusion.";
  d.msg[propMsgOnStartMonster] = "is resistant to confusion.";
  d.msg[propMsgOnEndPlayer] = "I feel vulnerable to confusion.";
  d.msg[propMsgOnEndMonster] = "is vulnerable to confusion.";
  d.msg[propMsgOnMorePlayer] = "I feel more resistant to confusion.";
  d.msg[propMsgOnMoreMonster] = "is more resistant to confusion.";
  d.msg[propMsgOnResPlayer] = "";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.allowTestOnBot = true;
  d.alignment = propAlignmentGood;
  addPropData(d);

  d.id = propRBreath;
  d.stdRndTurns = Range(40, 60);
  d.name = "Breath resistance";
  d.nameShort = "RBreath";
  d.msg[propMsgOnStartPlayer] = "I can breath without harm.";
  d.msg[propMsgOnStartMonster] = "can breath without harm.";
  d.msg[propMsgOnEndPlayer] = "";
  d.msg[propMsgOnEndMonster] = "";
  d.msg[propMsgOnMorePlayer] = "";
  d.msg[propMsgOnMoreMonster] = "";
  d.msg[propMsgOnResPlayer] = "";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.allowTestOnBot = true;
  d.alignment = propAlignmentGood;
  addPropData(d);

  d.id = propLightSensitive;
  d.stdRndTurns = Range(40, 60);
  d.name = "Light sensitive";
  d.nameShort = "LgtSensitive";
  d.msg[propMsgOnStartPlayer] = "I feel vulnerable to light!";
  d.msg[propMsgOnStartMonster] = "is vulnerable to light.";
  d.msg[propMsgOnEndPlayer] = "I no longer feel vulnerable to light.";
  d.msg[propMsgOnEndMonster] = "no longer is vulnerable to light.";
  d.msg[propMsgOnMorePlayer] = "I feel more vulnerable to light.";
  d.msg[propMsgOnMoreMonster] = "is more vulnerable to light.";
  d.msg[propMsgOnResPlayer] = "";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.alignment = propAlignmentBad;
  addPropData(d);

  d.id = propBlind;
  d.stdRndTurns = Range(20, 30);
  d.name = "Blind";
  d.nameShort = "Blind";
  d.msg[propMsgOnStartPlayer] = "I am blind!";
  d.msg[propMsgOnStartMonster] = "is blinded.";
  d.msg[propMsgOnEndPlayer] = "I can see again!";
  d.msg[propMsgOnEndMonster] = "can see again.";
  d.msg[propMsgOnMorePlayer] = "I am more blind.";
  d.msg[propMsgOnMoreMonster] = "is more blind.";
  d.msg[propMsgOnResPlayer] = "I resist blindness.";
  d.msg[propMsgOnResMonster] = "resists blindness.";
  d.isMakingMonsterAware = true;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.isEndedByMagicHealing = true;
  d.allowTestOnBot = true;
  d.alignment = propAlignmentBad;
  addPropData(d);

  d.id = propFainted;
  d.stdRndTurns = Range(100, 200);
  d.name = "Fainted";
  d.nameShort = "Fainted";
  d.msg[propMsgOnStartPlayer] = "I faint!";
  d.msg[propMsgOnStartMonster] = "faints.";
  d.msg[propMsgOnEndPlayer] = "I am awake.";
  d.msg[propMsgOnEndMonster] = "wakes up.";
  d.msg[propMsgOnMorePlayer] = "I faint deeper.";
  d.msg[propMsgOnMoreMonster] = "faints deeper.";
  d.msg[propMsgOnResPlayer] = "I resist fainting.";
  d.msg[propMsgOnResMonster] = "resists fainting.";
  d.isMakingMonsterAware = true;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.allowTestOnBot = true;
  d.alignment = propAlignmentBad;
  addPropData(d);

  d.id = propBurning;
  d.stdRndTurns = Range(5, 10);
  d.name = "Burning";
  d.nameShort = "Burning";
  d.msg[propMsgOnStartPlayer] = "I am Burning!";
  d.msg[propMsgOnStartMonster] = "is burning.";
  d.msg[propMsgOnEndPlayer] = "The flames are put out.";
  d.msg[propMsgOnEndMonster] = "is no longer burning.";
  d.msg[propMsgOnMorePlayer] = "";
  d.msg[propMsgOnMoreMonster] = "";
  d.msg[propMsgOnResPlayer] = "I resist burning.";
  d.msg[propMsgOnResMonster] = "resists burning.";
  d.isMakingMonsterAware = true;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = true;
  d.allowTestOnBot = true;
  d.alignment = propAlignmentBad;
  addPropData(d);

  d.id = propPoisoned;
  d.stdRndTurns = Range(30, 60);
  d.name = "Poisoned";
  d.nameShort = "Poisoned";
  d.msg[propMsgOnStartPlayer] = "I am poisoned!";
  d.msg[propMsgOnStartMonster] = "is poisoned.";
  d.msg[propMsgOnEndPlayer] = "My body is cleansed from poisoning!";
  d.msg[propMsgOnEndMonster] = "is cleansed from poisoning.";
  d.msg[propMsgOnMorePlayer] = "I am more poisoned.";
  d.msg[propMsgOnMoreMonster] = "is more poisoned.";
  d.msg[propMsgOnResPlayer] = "I resist poisoning.";
  d.msg[propMsgOnResMonster] = "resists poisoning.";
  d.isMakingMonsterAware = true;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.isEndedByMagicHealing = true;
  d.allowTestOnBot = true;
  d.alignment = propAlignmentBad;
  addPropData(d);

  d.id = propParalyzed;
  d.stdRndTurns = Range(7, 9);
  d.name = "Paralyzed";
  d.nameShort = "Paralyzed";
  d.msg[propMsgOnStartPlayer] = "I am paralyzed!";
  d.msg[propMsgOnStartMonster] = "is paralyzed.";
  d.msg[propMsgOnEndPlayer] = "I can move again!";
  d.msg[propMsgOnEndMonster] = "can move again.";
  d.msg[propMsgOnMorePlayer] = "";
  d.msg[propMsgOnMoreMonster] = "";
  d.msg[propMsgOnResPlayer] = "I resist paralyzation.";
  d.msg[propMsgOnResMonster] = "resists paralyzation.";
  d.isMakingMonsterAware = true;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = false;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.isEndedByMagicHealing = true;
  d.allowTestOnBot = true;
  d.alignment = propAlignmentBad;
  addPropData(d);

  d.id = propTerrified;
  d.stdRndTurns = Range(5, 12);
  d.name = "Terrified";
  d.nameShort = "Terrified";
  d.msg[propMsgOnStartPlayer] = "I am terrified!";
  d.msg[propMsgOnStartMonster] = "looks terrified.";
  d.msg[propMsgOnEndPlayer] = "I am no longer terrified!";
  d.msg[propMsgOnEndMonster] = "is no longer terrified.";
  d.msg[propMsgOnMorePlayer] = "I am more terrified.";
  d.msg[propMsgOnMoreMonster] = "looks more terrified.";
  d.msg[propMsgOnResPlayer] = "I resist fear.";
  d.msg[propMsgOnResMonster] = "resists fear.";
  d.isMakingMonsterAware = true;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.allowTestOnBot = true;
  d.alignment = propAlignmentBad;
  addPropData(d);

  d.id = propConfused;
  d.stdRndTurns = Range(80, 120);
  d.name = "Confused";
  d.nameShort = "Confused";
  d.msg[propMsgOnStartPlayer] = "I am confused!";
  d.msg[propMsgOnStartMonster] = "looks confused.";
  d.msg[propMsgOnEndPlayer] = "I am no longer confused";
  d.msg[propMsgOnEndMonster] = "is no longer confused.";
  d.msg[propMsgOnMorePlayer] = "I am more confused.";
  d.msg[propMsgOnMoreMonster] = "looks more confused.";
  d.msg[propMsgOnResPlayer] = "I resist confusion.";
  d.msg[propMsgOnResMonster] = "resists confusion.";
  d.isMakingMonsterAware = true;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.allowTestOnBot = true;
  d.alignment = propAlignmentBad;
  addPropData(d);

  d.id = propStunned;
  d.stdRndTurns = Range(5, 9);
  d.name = "Stunned";
  d.nameShort = "Stunned";
  d.msg[propMsgOnStartPlayer] = "I am stunned!";
  d.msg[propMsgOnStartMonster] = "is stunned.";
  d.msg[propMsgOnEndPlayer] = "I am no longer stunned.";
  d.msg[propMsgOnEndMonster] = "is no longer stunned.";
  d.msg[propMsgOnMorePlayer] = "I am more stunned.";
  d.msg[propMsgOnMoreMonster] = "is more stunned.";
  d.msg[propMsgOnResPlayer] = "I resist stunning.";
  d.msg[propMsgOnResMonster] = "resists stunning.";
  d.isMakingMonsterAware = true;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.isEndedByMagicHealing = true;
  d.allowTestOnBot = true;
  d.alignment = propAlignmentBad;
  addPropData(d);

  d.id = propSlowed;
  d.stdRndTurns = Range(9, 12);
  d.name = "Slowed";
  d.nameShort = "Slowed";
  d.msg[propMsgOnStartPlayer] = "Everything around me seems to speed up.";
  d.msg[propMsgOnStartMonster] = "slows down.";
  d.msg[propMsgOnEndPlayer] = "Everything around me seems to slow down.";
  d.msg[propMsgOnEndMonster] = "speeds up.";
  d.msg[propMsgOnMorePlayer] = "I am more slowed.";
  d.msg[propMsgOnMoreMonster] = "slows down more.";
  d.msg[propMsgOnResPlayer] = "I resist slowness.";
  d.msg[propMsgOnResMonster] = "resists slowness.";
  d.isMakingMonsterAware = true;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.allowTestOnBot = true;
  d.alignment = propAlignmentBad;
  addPropData(d);

  d.id = propFlared;
  d.stdRndTurns = Range(3, 4);
  d.msg[propMsgOnStartMonster] = "is perforated by a flare!";
  d.msg[propMsgOnMoreMonster] = "is perforated by another flare!";
  d.isMakingMonsterAware = true;
  d.allowDisplayTurns = false;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = true;
  d.alignment = propAlignmentBad;
  addPropData(d);

  d.id = propNailed;
  d.name = "Nailed";
  d.msg[propMsgOnStartPlayer] = "I am fastened by a spike!";
  d.msg[propMsgOnStartMonster] = "is fastened by a spike.";
  d.msg[propMsgOnEndPlayer] = "I tear free!";
  d.msg[propMsgOnEndMonster] = "tears free!";
  d.msg[propMsgOnMorePlayer] = "I am fastened by another spike!";
  d.msg[propMsgOnMoreMonster] = "is fastened by another spike.";
  d.isMakingMonsterAware = true;
  d.allowDisplayTurns = false;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.alignment = propAlignmentBad;
  addPropData(d);

  d.id = propWarlockCharged;
  d.stdRndTurns = Range(1, 1);
  d.name = "Charged";
  d.nameShort = "Charged";
  d.msg[propMsgOnStartPlayer] = "I am burning with power!";
  d.msg[propMsgOnStartMonster] = "";
  d.msg[propMsgOnEndPlayer] = "";
  d.msg[propMsgOnEndMonster] = "";
  d.msg[propMsgOnMorePlayer] = "";
  d.msg[propMsgOnMoreMonster] = "";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = false;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.alignment = propAlignmentGood;
  addPropData(d);

  d.id = propInfected;
  d.stdRndTurns = Range(400, 700);
  d.name = "Infected";
  d.nameShort = "Infected";
  d.msg[propMsgOnStartPlayer] = "I am infected!";
  d.msg[propMsgOnStartMonster] = "is infected.";
  d.msg[propMsgOnEndPlayer] = "My infection is cured!";
  d.msg[propMsgOnEndMonster] = "is no longer infected.";
  d.msg[propMsgOnMorePlayer] = "I am more infected!";
  d.msg[propMsgOnMoreMonster] = "is more infected.";
  d.msg[propMsgOnResPlayer] = "";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = true;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.isEndedByMagicHealing = true;
  d.allowTestOnBot = true;
  d.alignment = propAlignmentBad;
  addPropData(d);

  d.id = propDiseased;
  d.stdRndTurns = Range(400, 800);
  d.name = "Diseased";
  d.nameShort = "Diseased";
  d.msg[propMsgOnStartPlayer] = "I am diseased!";
  d.msg[propMsgOnStartMonster] = "is diseased.";
  d.msg[propMsgOnEndPlayer] = "My disease is cured!";
  d.msg[propMsgOnEndMonster] = "is no longer diseased.";
  d.msg[propMsgOnMorePlayer] = "I am more diseased.";
  d.msg[propMsgOnMoreMonster] = "is more diseased.";
  d.msg[propMsgOnResPlayer] = "I resist disease.";
  d.msg[propMsgOnResMonster] = "resists disease.";
  d.isMakingMonsterAware = true;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.isEndedByMagicHealing = true;
  d.allowTestOnBot = true;
  d.alignment = propAlignmentBad;
  addPropData(d);

  d.id = propWeakened;
  d.stdRndTurns = Range(50, 100);
  d.name = "Weakened";
  d.nameShort = "Weakened";
  d.msg[propMsgOnStartPlayer] = "I feel weaker.";
  d.msg[propMsgOnStartMonster] = "looks weaker.";
  d.msg[propMsgOnEndPlayer] = "I feel stronger!";
  d.msg[propMsgOnEndMonster] = "looks stronger!";
  d.msg[propMsgOnMorePlayer] = "I feel weaker.";
  d.msg[propMsgOnMoreMonster] = "looks weaker.";
  d.msg[propMsgOnResPlayer] = "I resist weakness.";
  d.msg[propMsgOnResMonster] = "resists weakness.";
  d.isMakingMonsterAware = true;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.isEndedByMagicHealing = true;
  d.allowTestOnBot = true;
  d.alignment = propAlignmentBad;
  addPropData(d);

  d.id = propFrenzied;
  d.stdRndTurns = Range(30, 60);
  d.name = "Frenzied";
  d.nameShort = "Frenzied";
  d.msg[propMsgOnStartPlayer] = "I feel ferocious!";
  d.msg[propMsgOnStartMonster] = "Looks ferocious!";
  d.msg[propMsgOnEndPlayer] = "I feel calmer.";
  d.msg[propMsgOnEndMonster] = "Looks calmer.";
  d.msg[propMsgOnMorePlayer] = "I feel more ferocious.";
  d.msg[propMsgOnMoreMonster] = "Looks more ferocious.";
  d.msg[propMsgOnResPlayer] = "I resist frenzying.";
  d.msg[propMsgOnResMonster] = "resists frenzying.";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.allowTestOnBot = true;
  d.alignment = propAlignmentNeutral;
  addPropData(d);

  d.id = propBlessed;
  d.stdRndTurns = Range(400, 600);
  d.name = "Blessed";
  d.nameShort = "Blessed";
  d.msg[propMsgOnStartPlayer] = "I feel luckier.";
  d.msg[propMsgOnEndPlayer] = "I have normal luck.";
  d.msg[propMsgOnMorePlayer] = "I feel luckier.";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.allowTestOnBot = true;
  d.alignment = propAlignmentGood;
  addPropData(d);

  d.id = propCursed;
  d.stdRndTurns = Range(400, 600);
  d.name = "Cursed";
  d.nameShort = "Cursed";
  d.msg[propMsgOnStartPlayer] = "I feel misfortunate.";
  d.msg[propMsgOnEndPlayer] = "I feel more fortunate.";
  d.msg[propMsgOnMorePlayer] = "I feel more misfortunate.";
  d.msg[propMsgOnResPlayer] = "I resist misfortune.";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.allowTestOnBot = true;
  d.alignment = propAlignmentBad;
  addPropData(d);

  d.id = propAiming;
  d.stdRndTurns = Range(1, 1);
  d.name = "Aiming";
  d.nameShort = "Aiming";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = false;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.alignment = propAlignmentGood;
  addPropData(d);

  d.id = propPossessedByZuul;
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = false;
  d.allowApplyMoreWhileActive = false;
  d.updatePlayerVisualWhenStartOrEnd = true;
  d.isEndedByMagicHealing = false;
  d.allowTestOnBot = true;
  d.alignment = propAlignmentNeutral;
  addPropData(d);

  d.id = propFlying;
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = false;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.isEndedByMagicHealing = false;
  d.allowTestOnBot = false;
  d.alignment = propAlignmentNeutral;
  addPropData(d);

  d.id = propEthereal;
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = false;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.isEndedByMagicHealing = false;
  d.allowTestOnBot = false;
  d.alignment = propAlignmentNeutral;
  addPropData(d);

  d.id = propOoze;
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = false;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.isEndedByMagicHealing = false;
  d.allowTestOnBot = false;
  d.alignment = propAlignmentNeutral;
  addPropData(d);

  d.id = propBurrowing;
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = false;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.isEndedByMagicHealing = false;
  d.allowTestOnBot = false;
  d.alignment = propAlignmentNeutral;
  addPropData(d);

  d.id = propWaiting;
  d.stdRndTurns = Range(1, 1);
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = false;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.alignment = propAlignmentNeutral;
  addPropData(d);

  d.id = propDisabledAttack;
  d.stdRndTurns = Range(1, 1);
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = false;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.alignment = propAlignmentNeutral;
  addPropData(d);

  d.id = propDisabledMelee;
  d.stdRndTurns = Range(1, 1);
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = false;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.alignment = propAlignmentNeutral;
  addPropData(d);

  d.id = propDisabledRanged;
  d.stdRndTurns = Range(1, 1);
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = false;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.alignment = propAlignmentNeutral;
  addPropData(d);
}

} //namespace

void init() {
  initDataList();
}

} //PropData

PropHandler::PropHandler(Actor* owningActor) :
  owningActor_(owningActor) {
  appliedProps_.resize(0);
  actorTurnPropBuffer_.resize(0);

  const ActorDataT& d = owningActor->getData();

  for(unsigned int i = 0; i < endOfPropIds; ++i) {
    if(d.intrProps[i]) {
      Prop* const prop = mkProp(PropId(i), PropTurns::indefinite);
      tryApplyProp(prop, true, true, true, true);
    }
  }
}

Prop* PropHandler::mkProp(const PropId id, PropTurns turnsInit,
                          const int NR_TURNS) const {
  switch(id) {
    case propNailed:            return new PropNailed(turnsInit,            NR_TURNS);
    case propWarlockCharged:    return new PropWarlockCharged(turnsInit,    NR_TURNS);
    case propBlind:             return new PropBlind(turnsInit,             NR_TURNS);
    case propBurning:           return new PropBurning(turnsInit,           NR_TURNS);
    case propFlared:            return new PropFlared(turnsInit,            NR_TURNS);
    case propParalyzed:         return new PropParalyzed(turnsInit,         NR_TURNS);
    case propTerrified:         return new PropTerrified(turnsInit,         NR_TURNS);
    case propWeakened:          return new PropWeakened(turnsInit,          NR_TURNS);
    case propConfused:          return new PropConfused(turnsInit,          NR_TURNS);
    case propStunned:           return new PropStunned(turnsInit,           NR_TURNS);
    case propWaiting:           return new PropWaiting(turnsInit,           NR_TURNS);
    case propSlowed:            return new PropSlowed(turnsInit,            NR_TURNS);
    case propInfected:          return new PropInfected(turnsInit,          NR_TURNS);
    case propDiseased:          return new PropDiseased(turnsInit,          NR_TURNS);
    case propPoisoned:          return new PropPoisoned(turnsInit,          NR_TURNS);
    case propFainted:           return new PropFainted(turnsInit,           NR_TURNS);
    case propFrenzied:          return new PropFrenzied(turnsInit,          NR_TURNS);
    case propAiming:            return new PropAiming(turnsInit,            NR_TURNS);
    case propDisabledAttack:    return new PropDisabledAttack(turnsInit,    NR_TURNS);
    case propDisabledMelee:     return new PropDisabledMelee(turnsInit,     NR_TURNS);
    case propDisabledRanged:    return new PropDisabledRanged(turnsInit,    NR_TURNS);
    case propBlessed:           return new PropBlessed(turnsInit,           NR_TURNS);
    case propCursed:            return new PropCursed(turnsInit,            NR_TURNS);
    case propRAcid:             return new PropRAcid(turnsInit,             NR_TURNS);
    case propRCold:             return new PropRCold(turnsInit,             NR_TURNS);
    case propRConfusion:        return new PropRConfusion(turnsInit,        NR_TURNS);
    case propRBreath:           return new PropRBreath(turnsInit,           NR_TURNS);
    case propRElec:             return new PropRElec(turnsInit,             NR_TURNS);
    case propRFear:             return new PropRFear(turnsInit,             NR_TURNS);
    case propRPhys:             return new PropRPhys(turnsInit,             NR_TURNS);
    case propRFire:             return new PropRFire(turnsInit,             NR_TURNS);
    case propRPoison:           return new PropRPoison(turnsInit,           NR_TURNS);
    case propRSleep:            return new PropRSleep(turnsInit,            NR_TURNS);
    case propLightSensitive:    return new PropLightSensitive(turnsInit,    NR_TURNS);
    case propPossessedByZuul:   return new PropPossessedByZuul(turnsInit,   NR_TURNS);
    case propFlying:            return new PropFlying(turnsInit,            NR_TURNS);
    case propEthereal:          return new PropEthereal(turnsInit,          NR_TURNS);
    case propOoze:              return new PropOoze(turnsInit,              NR_TURNS);
    case propBurrowing:         return new PropBurrowing(turnsInit,         NR_TURNS);
    case endOfPropIds: {assert(false && "Bad property id");}
  }
  return nullptr;
}

PropHandler::~PropHandler() {
  for(Prop* prop : appliedProps_) {delete prop;}
}

void PropHandler::getPropsFromSources(
  vector<Prop*>& out, bool sources[int(PropSrc::END)]) const {

  out.resize(0);

  //Get from applied properties
  if(sources[int(PropSrc::applied)]) {out = appliedProps_;}

  //Get from inventory if humanoid actor
  if(owningActor_->isHumanoid() && sources[int(PropSrc::inv)]) {
    const auto& inv     = owningActor_->getInv();
    const auto& slots   = inv.slots_;
    auto addItemProps = [&](const vector<Prop*>& itemPropList) {
      for(auto* const prop : itemPropList) {
        prop->owningActor_ = owningActor_;
        out.push_back(prop);
      };
    };
    for(const auto& slot : slots) {
      const auto* const item = slot.item;
      if(item) {
        addItemProps(item->propsEnabledOnCarrier);
      }
    }
    const auto& general = inv.general_;
    for(auto* const item : general) {
      addItemProps(item->propsEnabledOnCarrier);
    }
  }
}

void  PropHandler::getPropIdsFromSources(
  vector<PropId>& out, bool sources[int(PropSrc::END)]) const {

  const size_t NR_APPLIED = appliedProps_.size();
  out.resize(NR_APPLIED);

  //Get from applied properties
  if(sources[int(PropSrc::applied)]) {
    for(size_t i = 0; i < NR_APPLIED; ++i) {
      out[i] = appliedProps_[i]->getId();
    }
  }

  //Get from inventory if humanoid actor
  if(owningActor_->isHumanoid() && sources[int(PropSrc::inv)]) {
    const auto& inv     = owningActor_->getInv();
    const auto& slots   = inv.slots_;
    auto addItemProps = [&](const vector<Prop*>& itemPropList) {
      for(auto* const prop : itemPropList) {
        prop->owningActor_ = owningActor_;
        out.push_back(prop->getId());
      };
    };
    for(const auto& slot : slots) {
      const auto* const item = slot.item;
      if(item) {
        addItemProps(item->propsEnabledOnCarrier);
      }
    }
    const auto& general = inv.general_;
    for(auto* const item : general) {
      addItemProps(item->propsEnabledOnCarrier);
    }
  }
}

void PropHandler::getAllActivePropIds(vector<PropId>& out) const {
  out.resize(0);
  bool sources[int(PropSrc::END)];
  for(bool& v : sources) {v = true;}
  getPropIdsFromSources(out, sources);
}

bool PropHandler::tryResistProp(
  const PropId id, const vector<Prop*>& propList) const {

  for(Prop* p : propList) {if(p->tryResistOtherProp(id)) return true;}
  return false;
}

bool PropHandler::tryResistDmg(const DmgType dmgType,
                               const bool ALLOW_MSG_WHEN_TRUE) const {
  vector<Prop*> propList;
  bool sources[int(PropSrc::END)];
  for(bool& v : sources) {v = true;}
  getPropsFromSources(propList, sources);

  for(Prop* p : propList) {
    if(p->tryResistDmg(dmgType, ALLOW_MSG_WHEN_TRUE)) return true;
  }
  return false;
}

bool PropHandler::allowSee() const {
  vector<Prop*> propList;
  bool sources[int(PropSrc::END)];
  for(bool& v : sources) {v = true;}
  getPropsFromSources(propList, sources);

  for(Prop* p : propList) {if(!p->allowSee()) return false;}
  return true;
}

void PropHandler::tryApplyProp(Prop* const prop, const bool FORCE_EFFECT,
                               const bool NO_MESSAGES,
                               const bool DISABLE_REDRAW,
                               const bool DISABLE_PROP_START_EFFECTS) {

  //First, if this is a prop that runs on actor turns, check if the actor-turn
  //prop buffer does not already contain the prop.
  //-If it doesn't, then just add it to the buffer and return.
  //-If the buffer already contains the prop, it means it was requested to be
  //applied from the buffer to the applied props.
  //This way, this function can be used both for requesting to apply props,
  //and for applying props from the buffer.
  if(prop->getTurnMode() == PropTurnMode::actor) {
    vector<Prop*>& buffer = actorTurnPropBuffer_;
    if(find(begin(buffer), end(buffer), prop) == end(buffer)) {
      buffer.push_back(prop);
      return;
    }
  }

  prop->owningActor_    = owningActor_;

  const bool IS_PLAYER  = owningActor_ == Map::player;
  bool playerSeeOwner   = Map::player->isSeeingActor(*owningActor_, nullptr);

  if(!FORCE_EFFECT) {
    vector<Prop*> allProps;
    bool sources[int(PropSrc::END)];
    for(bool& v : sources) {v = true;}
    getPropsFromSources(allProps, sources);
    if(tryResistProp(prop->getId(), allProps)) {
      if(!NO_MESSAGES) {
        if(IS_PLAYER) {
          string msg = "";
          prop->getMsg(propMsgOnResPlayer, msg);
          if(!msg.empty()) {Log::addMsg(msg, clrWhite, true);}
        } else {
          if(Map::player->isSeeingActor(*owningActor_, nullptr)) {
            string msg = "";
            prop->getMsg(propMsgOnResMonster, msg);
            if(!msg.empty()) {
              const string monsterName = owningActor_->getNameThe();
              Log::addMsg(monsterName + " " + msg, clrWhite, true);
            }
          }
        }
      }
      return;
    }
  }

  //This point reached means nothing is blocking the property.

  //Actor already has property applied?
  for(Prop* oldProp : appliedProps_) {
    if(prop->getId() == oldProp->getId()) {

      if(!prop->allowApplyMoreWhileActive()) {
        delete prop;
        return;
      }

      const int TURNS_LEFT_OLD = oldProp->turnsLeft_;
      const int TURNS_LEFT_NEW = prop->turnsLeft_;

      if(
        TURNS_LEFT_OLD != -1 &&
        TURNS_LEFT_NEW >= TURNS_LEFT_OLD &&
        !NO_MESSAGES) {
        if(IS_PLAYER) {
          string msg = "";
          prop->getMsg(propMsgOnMorePlayer, msg);
          if(!msg.empty()) {
            Log::addMsg(msg, clrWhite, true);
          }
        } else {
          if(playerSeeOwner) {
            string msg = "";
            prop->getMsg(propMsgOnMoreMonster, msg);
            if(!msg.empty()) {
              Log::addMsg(owningActor_->getNameThe() + " " + msg);
            }
          }
        }
      }

      oldProp->onMore();

      oldProp->turnsLeft_ = (TURNS_LEFT_OLD < 0 || TURNS_LEFT_NEW < 0) ? -1 :
                            max(TURNS_LEFT_OLD, TURNS_LEFT_NEW);
      delete prop;
      return;
    }
  }

  //This part reached means the property is new
  appliedProps_.push_back(prop);

  if(!DISABLE_PROP_START_EFFECTS) {prop->onStart();}

  if(!DISABLE_REDRAW) {
    if(prop->shouldUpdatePlayerVisualWhenStartOrEnd()) {
      prop->owningActor_->updateClr();
      Map::player->updateFov();
      Renderer::drawMapAndInterface();
    }
  }

  if(!NO_MESSAGES) {
    if(IS_PLAYER) {
      string msg = "";
      prop->getMsg(propMsgOnStartPlayer, msg);
      if(!msg.empty()) {
        Log::addMsg(msg, clrWhite, true);
      }
    } else {
      if(playerSeeOwner) {
        string msg = "";
        prop->getMsg(propMsgOnStartMonster, msg);
        if(!msg.empty()) {
          Log::addMsg(owningActor_->getNameThe() + " " + msg);
        }
      }
    }
  }
}

void PropHandler::tryApplyPropFromWpn(const Wpn& wpn, const bool IS_MELEE) {
  const ItemDataT& d = wpn.getData();
  auto* prop = IS_MELEE ? d.melee.propApplied : d.ranged.propApplied;

  if(prop) {
    //If weapon damage type is resisted by the defender, the property is
    //automatically resisted
    DmgType dmgType = IS_MELEE ? d.melee.dmgType : d.ranged.dmgType;
    if(!tryResistDmg(dmgType, false)) {
      //Make a copy of the weapon effect
      auto* const propCpy = mkProp(prop->getId(), PropTurns::specific, prop->turnsLeft_);
      tryApplyProp(propCpy);
    }
  }
}

bool PropHandler::endAppliedProp(
  const PropId id,
  const bool visionBlockers[MAP_W][MAP_H],
  const bool RUN_PROP_END_EFFECTS) {

  int index   = -1;
  Prop* prop  = nullptr;
  const unsigned int NR_APPLIED_PROPS = appliedProps_.size();
  for(unsigned int i = 0; i < NR_APPLIED_PROPS; ++i) {
    prop = appliedProps_.at(i);
    if(prop->getId() == id) {
      index = i;
      break;
    }
  }
  if(index == -1) {return false;}

  appliedProps_.erase(begin(appliedProps_) + index);

  if(RUN_PROP_END_EFFECTS) {
    const bool IS_VISUAL_UPDATE_NEEDED =
      prop->shouldUpdatePlayerVisualWhenStartOrEnd();

    if(IS_VISUAL_UPDATE_NEEDED) {
      prop->owningActor_->updateClr();
      Map::player->updateFov();
      Renderer::drawMapAndInterface();
    }

    if(owningActor_ == Map::player) {
      string msg = "";
      prop->getMsg(propMsgOnEndPlayer, msg);
      if(!msg.empty()) {Log::addMsg(msg, clrWhite);}
    } else {
      if(Map::player->isSeeingActor(*owningActor_, visionBlockers)) {
        string msg = "";
        prop->getMsg(propMsgOnEndMonster, msg);
        if(!msg.empty()) {
          Log::addMsg(owningActor_->getNameThe() + " " + msg);
        }
      }
    }
    prop->onEnd();
  }

  delete prop;
  return true;
}

void PropHandler::applyActorTurnPropBuffer() {
  for(Prop* prop : actorTurnPropBuffer_) {tryApplyProp(prop);}
  actorTurnPropBuffer_.resize(0);
}

void PropHandler::tick(const PropTurnMode turnMode,
                       const bool visionBlockers[MAP_W][MAP_H]) {

  for(unsigned int i = 0; i < appliedProps_.size();) {
    Prop* const prop = appliedProps_.at(i);

    //Only tick property if it runs on the given turn mode
    //(standard turns or actor turns)
    if(prop->getTurnMode() == turnMode) {

      if(owningActor_ != Map::player) {
        if(prop->isMakingMonsterAware()) {
          static_cast<Monster*>(owningActor_)->awareOfPlayerCounter_ =
            owningActor_->getData().nrTurnsAwarePlayer;
        }
      }

      if(prop->turnsLeft_ > 0) {
        prop->turnsLeft_--;
      }

      if(prop->isFinnished()) {
        endAppliedProp(prop->getId(), visionBlockers);
      } else {
        prop->onNewTurn();
        i++;
      }
    } else {
      i++;
    }
  }

  vector<Prop*> invProps;
  bool sources[int(PropSrc::END)];
  for(bool& v : sources) {v = false;}
  sources[int(PropSrc::inv)] = true;
  getPropsFromSources(invProps, sources);
  for(Prop* prop : invProps) {
    if(owningActor_ != Map::player) {
      if(prop->isMakingMonsterAware()) {
        static_cast<Monster*>(owningActor_)->awareOfPlayerCounter_ =
          owningActor_->getData().nrTurnsAwarePlayer;
      }
    }
    prop->onNewTurn();
  }
}

void PropHandler::getPropsInterfaceLine(vector<StrAndClr>& line) const {
  line.resize(0);

  const bool IS_SELF_AWARE =
    PlayerBon::hasTrait(Trait::selfAware);

  vector<Prop*> propList;
  bool sources[int(PropSrc::END)];
  for(bool& v : sources) {v = true;}
  getPropsFromSources(propList, sources);
  for(Prop* prop : propList) {
    const string propName = prop->getNameShort();
    if(!propName.empty()) {
      const PropAlignment alignment = prop->getAlignment();
      const int TURNS_LEFT = prop->turnsLeft_;
      const string turnsStr = TURNS_LEFT > 0 && IS_SELF_AWARE ?
                              ("(" + toStr(TURNS_LEFT) + ")") : "";
      line.push_back(
        StrAndClr(propName + turnsStr,
                  alignment == propAlignmentGood ? clrMsgGood :
                  alignment == propAlignmentBad  ? clrMsgBad :
                  clrWhite));
    }
  }
}

int PropHandler::getChangedMaxHp(const int HP_MAX) const {
  vector<Prop*> propList;
  bool sources[int(PropSrc::END)];
  for(bool& v : sources) {v = true;}
  getPropsFromSources(propList, sources);
  int newHpMax = HP_MAX;
  for(Prop* prop : propList) {newHpMax = prop->getChangedMaxHp(newHpMax);}
  return newHpMax;
}

void PropHandler::changeMoveDir(const Pos& actorPos, Dir& dir) const {
  vector<Prop*> propList;
  bool sources[int(PropSrc::END)];
  for(bool& v : sources) {v = true;}
  getPropsFromSources(propList, sources);
  for(Prop* prop : propList) {prop->changeMoveDir(actorPos, dir);}
}

bool PropHandler::allowAttack(const bool ALLOW_MESSAGE_WHEN_FALSE) const {
  vector<Prop*> propList;
  bool sources[int(PropSrc::END)];
  for(bool& v : sources) {v = true;}
  getPropsFromSources(propList, sources);
  for(Prop* prop : propList) {
    if(
      !prop->allowAttackMelee(ALLOW_MESSAGE_WHEN_FALSE) &&
      !prop->allowAttackRanged(ALLOW_MESSAGE_WHEN_FALSE)) {
      return false;
    }
  }
  return true;
}

bool PropHandler::allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE) const {
  vector<Prop*> propList;
  bool sources[int(PropSrc::END)];
  for(bool& v : sources) {v = true;}
  getPropsFromSources(propList, sources);
  for(Prop* prop : propList) {
    if(!prop->allowAttackMelee(ALLOW_MESSAGE_WHEN_FALSE)) {
      return false;
    }
  }
  return true;
}

bool PropHandler::allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) const {
  vector<Prop*> propList;
  bool sources[int(PropSrc::END)];
  for(bool& v : sources) {v = true;}
  getPropsFromSources(propList, sources);
  for(Prop* prop : propList) {
    if(!prop->allowAttackRanged(ALLOW_MESSAGE_WHEN_FALSE)) {return false;}
  }
  return true;
}

bool PropHandler::allowMove() const {
  vector<Prop*> propList;
  bool sources[int(PropSrc::END)];
  for(bool& v : sources) {v = true;}
  getPropsFromSources(propList, sources);
  for(Prop* prop : propList) {
    if(!prop->allowMove()) {return false;}
  }
  return true;
}

bool PropHandler::allowAct() const {
  vector<Prop*> propList;
  bool sources[int(PropSrc::END)];
  for(bool& v : sources) {v = true;}
  getPropsFromSources(propList, sources);
  for(Prop* prop : propList) {
    if(!prop->allowAct()) {return false;}
  }
  return true;
}

bool PropHandler::allowRead(const bool ALLOW_MESSAGE_WHEN_FALSE) const {
  TRACE_FUNC_BEGIN_VERBOSE;
  vector<Prop*> propList;
  bool sources[int(PropSrc::END)];
  for(bool& v : sources) {v = true;}
  getPropsFromSources(propList, sources);
  const unsigned int NR_PROPS = propList.size();
  for(unsigned int i = 0; i < NR_PROPS; ++i) {
    if(!propList.at(i)->allowRead(ALLOW_MESSAGE_WHEN_FALSE)) {
      TRACE_FUNC_END_VERBOSE;
      return false;
    }
  }
  TRACE_FUNC_END_VERBOSE;
  return true;
}

bool PropHandler::allowCastSpells(const bool ALLOW_MESSAGE_WHEN_FALSE) const {
  vector<Prop*> propList;
  bool sources[int(PropSrc::END)];
  for(bool& v : sources) {v = true;}
  getPropsFromSources(propList, sources);
  const unsigned int NR_PROPS = propList.size();
  for(unsigned int i = 0; i < NR_PROPS; ++i) {
    if(!propList.at(i)->allowCastSpells(ALLOW_MESSAGE_WHEN_FALSE)) {
      return false;
    }
  }
  return true;
}

void PropHandler::onHit() {
  vector<Prop*> propList;
  bool sources[int(PropSrc::END)];
  for(bool& v : sources) {v = true;}
  getPropsFromSources(propList, sources);
  for(Prop* prop : propList) {prop->onHit();}
}

void PropHandler::onDeath(const bool IS_PLAYER_SEE_OWNING_ACTOR) {
  vector<Prop*> propList;
  bool sources[int(PropSrc::END)];
  for(bool& v : sources) {v = true;}
  getPropsFromSources(propList, sources);
  for(Prop* prop : propList) {prop->onDeath(IS_PLAYER_SEE_OWNING_ACTOR);}
}

int PropHandler::getAbilityMod(const AbilityId ability) const {
  vector<Prop*> propList;
  bool sources[int(PropSrc::END)];
  for(bool& v : sources) {v = true;}
  getPropsFromSources(propList, sources);
  int modifier = 0;
  for(Prop* prop : propList) {
    modifier += prop->getAbilityMod(ability);
  }
  return modifier;
}

Prop* PropHandler::getProp(const PropId id, const PropSrc source) const {

  assert(source != PropSrc::END);

  if(source == PropSrc::applied) {
    for(Prop* prop : appliedProps_) {if(prop->getId() == id) {return prop;}}
  } else if(source == PropSrc::inv) {
    vector<Prop*> invProps;
    bool sources[int(PropSrc::END)];
    for(bool& v : sources) {v = false;}
    sources[int(PropSrc::inv)] = true;
    getPropsFromSources(invProps, sources);
    for(Prop* prop : invProps) {if(prop->getId() == id) {return prop;}}
  }
  return nullptr;
}

bool PropHandler::changeActorClr(Clr& clr) const {
  vector<Prop*> propList;
  bool sources[int(PropSrc::END)];
  for(bool& v : sources) {v = true;}
  getPropsFromSources(propList, sources);
  for(Prop* prop : propList) {if(prop->changeActorClr(clr)) return true;}
  return false;
}

void PropHandler::endAppliedPropsByMagicHealing() {
  bool visionBlockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(), visionBlockers);
  vector<Prop*> propList;
  bool sources[int(PropSrc::END)];
  for(bool& v : sources) {v = true;}
  getPropsFromSources(propList, sources);
  for(unsigned int i = 0; i < propList.size(); ++i) {
    if(propList.at(i)->isEndedByMagicHealing()) {
      endAppliedProp(appliedProps_.at(i)->getId(), visionBlockers);
      propList.erase(begin(propList) + i);
      i--;
    }
  }
}

Prop::Prop(PropId id, PropTurns turnsInit, int turns) :
  turnsLeft_(turns), owningActor_(nullptr), id_(id),
  data_(&(PropData::data[id])) {

  if(turnsInit == PropTurns::std) {
    turnsLeft_ = Rnd::range(data_->stdRndTurns);
  }
  if(turnsInit == PropTurns::indefinite) {
    turnsLeft_ = -1;
  }
}

void PropBlessed::onStart() {
  bool visionBlockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(), visionBlockers);
  owningActor_->getPropHandler().endAppliedProp(
    propCursed, visionBlockers, false);
}

void PropCursed::onStart() {
  bool visionBlockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(), visionBlockers);
  owningActor_->getPropHandler().endAppliedProp(
    propBlessed, visionBlockers, false);
}

void PropInfected::onNewTurn() {
  if(Rnd::oneIn(250)) {
    PropHandler& propHlr = owningActor_->getPropHandler();
    propHlr.tryApplyProp(new PropDiseased(PropTurns::std));
    bool blocked[MAP_W][MAP_H];
    MapParse::parse(CellPred::BlocksVision(), blocked);
    propHlr.endAppliedProp(propInfected, blocked, false);
  }
}

int PropDiseased::getChangedMaxHp(const int HP_MAX) const {
  if(owningActor_ == Map::player && PlayerBon::hasTrait(Trait::survivalist)) {
    return (HP_MAX * 3) / 4; //Survavlist makes you lose only 25% instead of 50%
  } else {
    return HP_MAX / 2;
  }
}

void PropDiseased::onStart() {
  //Actor::getHpMax() will now return a decreased value
  //cap current HP to the new, lower, maximum
  int& hp = owningActor_->hp_;
  hp = min(Map::player->getHpMax(true), hp);
}

bool PropDiseased::tryResistOtherProp(const PropId id) const {
  //Getting infected while already diseased is just annoying
  return id == propInfected;
}

void PropPossessedByZuul::onDeath(const bool IS_PLAYER_SEE_OWNING_ACTOR) {
  if(IS_PLAYER_SEE_OWNING_ACTOR) {
    const string& name1 = owningActor_->getNameThe();
    const string& name2 = ActorData::data[ActorId::zuul].nameThe;
    Log::addMsg(name1 + " was possessed by " + name2 + "!");
  }
  owningActor_->deadState = ActorDeadState::destroyed;
  const Pos& pos          = owningActor_->pos;
  Map::mkGore(pos);
  Map::mkBlood(pos);
  ActorFactory::summonMonsters(pos, vector<ActorId> {ActorId::zuul}, true);
}

void PropPoisoned::onNewTurn() {
  if(owningActor_->deadState == ActorDeadState::alive) {
    const int DMG_N_TURN = 3;
    const int TURN = GameTime::getTurn();
    if(TURN == (TURN / DMG_N_TURN) * DMG_N_TURN) {

      if(owningActor_ == Map::player) {
        Log::addMsg("I am suffering from the poison!", clrMsgBad, true);
      } else {
        if(Map::player->isSeeingActor(*owningActor_, nullptr)) {
          Log::addMsg( owningActor_->getNameThe() + " suffers from poisoning!");
        }
      }

      owningActor_->hit(1, DmgType::pure);
    }
  }
}

bool PropTerrified::allowAttackMelee(
  const bool ALLOW_MESSAGE_WHEN_FALSE) const {

  if(owningActor_ == Map::player && ALLOW_MESSAGE_WHEN_FALSE) {
    Log::addMsg("I am too terrified to engage in close combat!");
  }
  return false;
}

bool PropTerrified::allowAttackRanged(
  const bool ALLOW_MESSAGE_WHEN_FALSE) const {

  (void)ALLOW_MESSAGE_WHEN_FALSE;
  return true;
}

void PropNailed::changeMoveDir(const Pos& actorPos, Dir& dir) {
  (void)actorPos;

  if(dir != Dir::center) {

    if(owningActor_ == Map::player) {
      Log::addMsg("I struggle to tear out the spike!", clrMsgBad);
    } else {
      if(Map::player->isSeeingActor(*owningActor_, nullptr)) {
        Log::addMsg(owningActor_->getNameThe() +  " struggles in pain!",
                    clrMsgGood);
      }
    }

    owningActor_->hit(Rnd::dice(1, 3), DmgType::physical);

    if(owningActor_->deadState == ActorDeadState::alive) {

      //TODO reimplement something affecting chance of success?

      if(Rnd::oneIn(4)) {
        nrSpikes_--;
        if(nrSpikes_ > 0) {
          if(owningActor_ == Map::player) {
            Log::addMsg("I rip out a spike from my flesh!");
          } else {
            if(Map::player->isSeeingActor(*owningActor_, nullptr)) {
              Log::addMsg(
                owningActor_->getNameThe() + " tears out a spike!");
            }
          }
        }
      }
    }

    dir = Dir::center;
  }
}

bool PropConfused::allowRead(const bool ALLOW_MESSAGE_WHEN_FALSE) const {
  if(owningActor_ == Map::player && ALLOW_MESSAGE_WHEN_FALSE) {
    Log::addMsg("I'm too confused.");
  }
  return false;
}

bool PropConfused::allowAttackMelee(
  const bool ALLOW_MESSAGE_WHEN_FALSE) const {

  (void)ALLOW_MESSAGE_WHEN_FALSE;

  if(owningActor_ != Map::player) {
    return Rnd::coinToss();
  }
  return true;
}

bool PropConfused::allowAttackRanged(
  const bool ALLOW_MESSAGE_WHEN_FALSE) const {

  (void)ALLOW_MESSAGE_WHEN_FALSE;


  if(owningActor_ != Map::player) {
    return Rnd::coinToss();
  }
  return true;
}

void PropConfused::changeMoveDir(const Pos& actorPos, Dir& dir) {
  if(dir != Dir::center) {

    bool blocked[MAP_W][MAP_H];
    MapParse::parse(CellPred::BlocksActor(*owningActor_, true),
                    blocked);

    if(Rnd::oneIn(8)) {
      int triesLeft = 100;
      while(triesLeft != 0) {
        //-1 to 1 for x and y
        const Pos delta(Rnd::range(-1, 1), Rnd::range(-1, 1));
        if(delta.x != 0 || delta.y != 0) {
          const Pos c = actorPos + delta;
          if(!blocked[c.x][c.y]) {dir = DirUtils::getDir(delta);}
        }
        triesLeft--;
      }
    }
  }
}

void PropFrenzied::changeMoveDir(const Pos& actorPos, Dir& dir) {
  if(owningActor_ == Map::player) {
    vector<Actor*> spottedEnemies;
    owningActor_->getSpottedEnemies(spottedEnemies);

    if(spottedEnemies.empty()) {return;}

    vector<Pos> spottedEnemiesPositions;
    spottedEnemiesPositions.resize(0);
    for(unsigned int i = 0; i < spottedEnemies.size(); ++i) {
      spottedEnemiesPositions.push_back(spottedEnemies.at(i)->pos);
    }
    sort(begin(spottedEnemiesPositions), end(spottedEnemiesPositions),
         IsCloserToPos(actorPos));

    const Pos& closestMonPos = spottedEnemiesPositions.at(0);

    bool blocked[MAP_W][MAP_H];
    MapParse::parse(CellPred::BlocksActor(*owningActor_, false), blocked);

    vector<Pos> line;
    LineCalc::calcNewLine(actorPos, closestMonPos, true, 999, false, line);

    if(line.size() > 1) {
      for(Pos& pos : line) {if(blocked[pos.x][pos.y]) {return;}}
      dir = DirUtils::getDir(line.at(1) - actorPos);
    }
  }
}

bool PropFrenzied::tryResistOtherProp(const PropId id) const {
  return id == propConfused || id == propFainted ||
         id == propTerrified || id == propWeakened;
}

void PropFrenzied::onStart() {
  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(), blocked);
  owningActor_->getPropHandler().endAppliedProp(propConfused,  blocked);
  owningActor_->getPropHandler().endAppliedProp(propTerrified, blocked);
  owningActor_->getPropHandler().endAppliedProp(propWeakened,  blocked);
}

void PropFrenzied::onEnd() {
  owningActor_->getPropHandler().tryApplyProp(
    new PropWeakened(PropTurns::std));
}

bool PropFrenzied::allowRead(const bool ALLOW_MESSAGE_WHEN_FALSE) const {
  if(owningActor_ == Map::player && ALLOW_MESSAGE_WHEN_FALSE) {
    Log::addMsg("I'm too enraged to concentrate!");
  }
  return false;
}

bool PropFrenzied::allowCastSpells(const bool ALLOW_MESSAGE_WHEN_FALSE) const {
  if(owningActor_ == Map::player && ALLOW_MESSAGE_WHEN_FALSE) {
    Log::addMsg("I'm too enraged to concentrate!");
  }
  return false;
}

void PropBurning::onStart() {
//  owningActor_->addLight(Map::light);
}

void PropBurning::onNewTurn() {
  if(owningActor_ == Map::player) {
    Log::addMsg("AAAARGH IT BURNS!!!", clrRedLgt);
  }
  owningActor_->hit(Rnd::dice(1, 2), DmgType::fire);
}

bool PropBurning::allowRead(const bool ALLOW_MESSAGE_WHEN_FALSE) const {
  if(owningActor_ == Map::player && ALLOW_MESSAGE_WHEN_FALSE) {
    Log::addMsg("Not while burning.");
  }
  return false;
}

bool PropBurning::allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) const {
  if(owningActor_ == Map::player && ALLOW_MESSAGE_WHEN_FALSE) {
    Log::addMsg("Not while burning.");
  }
  return false;
}

bool PropBlind::shouldUpdatePlayerVisualWhenStartOrEnd() const {
  return owningActor_ == Map::player;
}

void PropParalyzed::onStart() {
  auto* const player = Map::player;
  if(owningActor_ == player) {
    auto* const activeExplosive = player->activeExplosive;
    if(activeExplosive) {activeExplosive->onPlayerParalyzed();}
  }
}

bool PropFainted::shouldUpdatePlayerVisualWhenStartOrEnd() const {
  return owningActor_ == Map::player;
}

void PropFlared::onNewTurn() {
  owningActor_->hit(1, DmgType::fire);

  if(turnsLeft_ == 0) {
    bool visionBlockers[MAP_W][MAP_H];
    MapParse::parse(CellPred::BlocksVision(), visionBlockers);
    owningActor_->getPropHandler().tryApplyProp(
      new PropBurning(PropTurns::std));
    owningActor_->getPropHandler().endAppliedProp(
      propFlared, visionBlockers);
  }
}

bool PropRAcid::tryResistDmg(const DmgType dmgType,
                             const bool ALLOW_MSG_WHEN_TRUE) const {
  if(dmgType == DmgType::acid) {
    if(ALLOW_MSG_WHEN_TRUE) {
      if(owningActor_ == Map::player) {
        Log::addMsg("I feel a faint burning sensation.");
      } else if(Map::player->isSeeingActor(*owningActor_, nullptr)) {
        Log::addMsg(owningActor_->getNameThe() + " seems unaffected.");
      }
    }
    return true;
  }
  return false;
}

bool PropRCold::tryResistDmg(const DmgType dmgType, const bool ALLOW_MSG_WHEN_TRUE) const {
  if(dmgType == DmgType::cold) {
    if(ALLOW_MSG_WHEN_TRUE) {
      if(owningActor_ == Map::player) {
        Log::addMsg("I feel chilly.");
      } else if(Map::player->isSeeingActor(*owningActor_, nullptr)) {
        Log::addMsg(owningActor_->getNameThe() + " seems unaffected.");
      }
    }
    return true;
  }
  return false;
}

bool PropRElec::tryResistDmg(const DmgType dmgType,
                             const bool ALLOW_MSG_WHEN_TRUE) const {
  if(dmgType == DmgType::electric) {
    if(ALLOW_MSG_WHEN_TRUE) {
      if(owningActor_ == Map::player) {
        Log::addMsg("I feel a faint tingle.");
      } else if(Map::player->isSeeingActor(*owningActor_, nullptr)) {
        Log::addMsg(owningActor_->getNameThe() + " seems unaffected.");
      }
    }
    return true;
  }
  return false;
}

bool PropRConfusion::tryResistOtherProp(const PropId id) const {
  return id == propConfused;
}

void PropRConfusion::onStart() {
  bool visionBlockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(), visionBlockers);
  owningActor_->getPropHandler().endAppliedProp(propConfused, visionBlockers);
}

bool PropRFear::tryResistOtherProp(const PropId id) const {
  return id == propTerrified;
}

void PropRFear::onStart() {
  bool visionBlockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(), visionBlockers);
  owningActor_->getPropHandler().endAppliedProp(propTerrified, visionBlockers);
}

bool PropRPhys::tryResistOtherProp(const PropId id) const {
  (void)id;
  return false;
}

void PropRPhys::onStart() {
  return;
}

bool PropRPhys::tryResistDmg(const DmgType dmgType,
                             const bool ALLOW_MSG_WHEN_TRUE) const {
  if(dmgType == DmgType::physical) {
    if(ALLOW_MSG_WHEN_TRUE) {
      if(owningActor_ == Map::player) {
        Log::addMsg("I resist harm.");
      } else if(Map::player->isSeeingActor(*owningActor_, nullptr)) {
        Log::addMsg(owningActor_->getNameThe() + " seems unaffected.");
      }
    }
    return true;
  }
  return false;
}

bool PropRFire::tryResistOtherProp(const PropId id) const {
  return id == propBurning;
}

void PropRFire::onStart() {
  bool visionBlockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(), visionBlockers);
  owningActor_->getPropHandler().endAppliedProp(propBurning, visionBlockers);
}

bool PropRFire::tryResistDmg(const DmgType dmgType,
                             const bool ALLOW_MSG_WHEN_TRUE) const {
  if(dmgType == DmgType::fire) {
    if(ALLOW_MSG_WHEN_TRUE) {
      if(owningActor_ == Map::player) {
        Log::addMsg("I feel hot.");
      } else if(Map::player->isSeeingActor(*owningActor_, nullptr)) {
        Log::addMsg(owningActor_->getNameThe() + " seems unaffected.");
      }
    }
    return true;
  }
  return false;
}

bool PropRPoison::tryResistOtherProp(const PropId id) const {
  return id == propPoisoned;
}

void PropRPoison::onStart() {
  bool visionBlockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(), visionBlockers);
  owningActor_->getPropHandler().endAppliedProp(propPoisoned, visionBlockers);
}

bool PropRSleep::tryResistOtherProp(const PropId id) const {
  return id == propFainted;
}

void PropRSleep::onStart() {
  bool visionBlockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(), visionBlockers);
  owningActor_->getPropHandler().endAppliedProp(propFainted, visionBlockers);
}

void PropBurrowing::onNewTurn() {
  const Pos& p = owningActor_->pos;
  Map::cells[p.x][p.y].rigid->hit(DmgType::physical, DmgMethod::forced);
}
