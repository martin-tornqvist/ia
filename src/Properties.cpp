#include "Properties.h"

#include <algorithm>

#include "Engine.h"
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
#include "FeatureFactory.h"
#include "PlayerBonuses.h"
#include "MapParsing.h"
#include "LineCalc.h"

using namespace std;

void PropDataHandler::initDataList() {
  PropData d;
  d.reset();

  d.id = propRFire;
  d.stdRndTurns = Range(40, 60);
  d.name = "Fire resistance";
  d.nameShort = "RFire";
  d.msg[propMsgOnStartPlayer] = "I feel resistant to fire.";
  d.msg[propMsgOnStartMonster] = "looks resistant to fire.";
  d.msg[propMsgOnEndPlayer] = "I feel vulnerable to fire.";
  d.msg[propMsgOnEndMonster] = "looks vulnerable to fire.";
  d.msg[propMsgOnMorePlayer] = "I feel more resistant to fire.";
  d.msg[propMsgOnMoreMonster] = "looks more resistant to fire.";
  d.msg[propMsgOnResPlayer] = "";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.allowTestingOnBot = true;
  d.alignment = propAlignmentGood;
  addPropData(d);

  d.id = propRCold;
  d.stdRndTurns = Range(40, 60);
  d.name = "Cold resistance";
  d.nameShort = "RCold";
  d.msg[propMsgOnStartPlayer] = "I feel resistant to cold.";
  d.msg[propMsgOnStartMonster] = "looks resistant to cold.";
  d.msg[propMsgOnEndPlayer] = "I feel vulnerable to cold.";
  d.msg[propMsgOnEndMonster] = "looks vulnerable to cold.";
  d.msg[propMsgOnMorePlayer] = "I feel more resistant to cold.";
  d.msg[propMsgOnMoreMonster] = "looks more resistant to cold.";
  d.msg[propMsgOnResPlayer] = "";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.allowTestingOnBot = true;
  d.alignment = propAlignmentGood;
  addPropData(d);

  d.id = propRPoison;
  d.stdRndTurns = Range(40, 60);
  d.name = "Poison resistance";
  d.nameShort = "RPoison";
  d.msg[propMsgOnStartPlayer] = "I feel resistant to poison.";
  d.msg[propMsgOnStartMonster] = "looks resistant to poison.";
  d.msg[propMsgOnEndPlayer] = "I feel vulnerable to poison.";
  d.msg[propMsgOnEndMonster] = "looks vulnerable to poison.";
  d.msg[propMsgOnMorePlayer] = "I feel more resistant to poison.";
  d.msg[propMsgOnMoreMonster] = "looks more resistant to poison.";
  d.msg[propMsgOnResPlayer] = "";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.isEndedByMagicHealing = true;
  d.allowTestingOnBot = true;
  d.alignment = propAlignmentGood;
  addPropData(d);

  d.id = propRElec;
  d.stdRndTurns = Range(40, 60);
  d.name = "Electric resistance";
  d.nameShort = "RElec";
  d.msg[propMsgOnStartPlayer] = "I feel resistant to electricity.";
  d.msg[propMsgOnStartMonster] = "looks resistant to electricity.";
  d.msg[propMsgOnEndPlayer] = "I feel vulnerable to electricity.";
  d.msg[propMsgOnEndMonster] = "looks vulnerable to electricity.";
  d.msg[propMsgOnMorePlayer] = "I feel more resistant to electricity.";
  d.msg[propMsgOnMoreMonster] = "looks more resistant to electricity.";
  d.msg[propMsgOnResPlayer] = "";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.allowTestingOnBot = true;
  d.alignment = propAlignmentGood;
  addPropData(d);

  d.id = propRAcid;
  d.stdRndTurns = Range(40, 60);
  d.name = "Acid resistance";
  d.nameShort = "RAcid";
  d.msg[propMsgOnStartPlayer] = "I feel resistant to acid.";
  d.msg[propMsgOnStartMonster] = "looks resistant to acid.";
  d.msg[propMsgOnEndPlayer] = "I feel vulnerable to acid.";
  d.msg[propMsgOnEndMonster] = "looks vulnerable to acid.";
  d.msg[propMsgOnMorePlayer] = "I feel more resistant to acid.";
  d.msg[propMsgOnMoreMonster] = "looks more resistant to acid.";
  d.msg[propMsgOnResPlayer] = "";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.allowTestingOnBot = true;
  d.alignment = propAlignmentGood;
  addPropData(d);

  d.id = propRSleep;
  d.stdRndTurns = Range(40, 60);
  d.name = "Sleep resistance";
  d.nameShort = "RSleep";
  d.msg[propMsgOnStartPlayer] = "I feel resistant to sleep.";
  d.msg[propMsgOnStartMonster] = "looks resistant to sleep.";
  d.msg[propMsgOnEndPlayer] = "I feel vulnerable to sleep.";
  d.msg[propMsgOnEndMonster] = "looks vulnerable to sleep.";
  d.msg[propMsgOnMorePlayer] = "I feel more resistant to sleep.";
  d.msg[propMsgOnMoreMonster] = "looks more resistant to sleep.";
  d.msg[propMsgOnResPlayer] = "";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.allowTestingOnBot = true;
  d.alignment = propAlignmentGood;
  addPropData(d);

//  d.id = propFreeAction;
//  d.stdRndTurns = Range(40, 60);
//  d.name = "Free action";
//  d.nameShort = "FreeAction";
//  d.msg[propMsgOnStartPlayer] = "[FreeAction Player start]";
//  d.msg[propMsgOnStartMonster] = "[FreeAction Monster start]";
//  d.msg[propMsgOnEndPlayer] = "[FreeAction Player end]";
//  d.msg[propMsgOnEndMonster] = "[FreeAction Monster end]";
//  d.msg[propMsgOnMorePlayer] = "[FreeAction Player more]";
//  d.msg[propMsgOnMoreMonster] = "[FreeAction Monster more]";
//  d.msg[propMsgOnResPlayer] = "";
//  d.msg[propMsgOnResMonster] = "";
//  d.isMakingMonsterAware = false;
//  d.allowDisplayTurns = true;
//  d.allowApplyMoreWhileActive = true;
//  d.updatePlayerVisualWhenStartOrEnd = false;
//  d.allowTestingOnBot = true;
//  d.alignment = propAlignmentGood;
//  addPropData(d);

  d.id = propRFear;
  d.stdRndTurns = Range(40, 60);
  d.name = "Fear resistance";
  d.nameShort = "RFear";
  d.msg[propMsgOnStartPlayer] = "I feel resistant to fear.";
  d.msg[propMsgOnStartMonster] = "looks resistant to fear.";
  d.msg[propMsgOnEndPlayer] = "I feel vulnerable to fear.";
  d.msg[propMsgOnEndMonster] = "looks vulnerable to fear.";
  d.msg[propMsgOnMorePlayer] = "I feel more resistant to fear.";
  d.msg[propMsgOnMoreMonster] = "looks more resistant to fear.";
  d.msg[propMsgOnResPlayer] = "";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.allowTestingOnBot = true;
  d.alignment = propAlignmentGood;
  addPropData(d);

  d.id = propRConfusion;
  d.stdRndTurns = Range(40, 50);
  d.name = "Confusion resistance";
  d.nameShort = "RConfusion";
  d.msg[propMsgOnStartPlayer] = "I feel resistant to confusion.";
  d.msg[propMsgOnStartMonster] = "looks resistant to confusion.";
  d.msg[propMsgOnEndPlayer] = "I feel vulnerable to confusion.";
  d.msg[propMsgOnEndMonster] = "looks vulnerable to confusion.";
  d.msg[propMsgOnMorePlayer] = "I feel more resistant to confusion.";
  d.msg[propMsgOnMoreMonster] = "looks more resistant to confusion.";
  d.msg[propMsgOnResPlayer] = "";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.allowTestingOnBot = true;
  d.alignment = propAlignmentGood;
  addPropData(d);

  d.id = propLightSensitive;
  d.stdRndTurns = Range(40, 60);
  d.name = "Light sensitive";
  d.nameShort = "LgtSensitive";
  d.msg[propMsgOnStartPlayer] = "I feel vulnerable to light!";
  d.msg[propMsgOnStartMonster] = "looks vulnerable to light.";
  d.msg[propMsgOnEndPlayer] = "I no longer feel vulnerable to light.";
  d.msg[propMsgOnEndMonster] = "no longer looks vulnerable to light.";
  d.msg[propMsgOnMorePlayer] = "I feel more vulnerable to light.";
  d.msg[propMsgOnMoreMonster] = "looks more vulnerable to light.";
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
  d.allowTestingOnBot = true;
  d.alignment = propAlignmentBad;
  addPropData(d);

  d.id = propFainted;
  d.stdRndTurns = Range(50, 75);
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
  d.allowTestingOnBot = true;
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
  d.msg[propMsgOnMorePlayer] = "I am further engulfed by flames!";
  d.msg[propMsgOnMoreMonster] = "is further engulfed by flames.";
  d.msg[propMsgOnResPlayer] = "I resist burning.";
  d.msg[propMsgOnResMonster] = "resists burning.";
  d.isMakingMonsterAware = true;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = true;
  d.allowTestingOnBot = true;
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
  d.allowTestingOnBot = true;
  d.alignment = propAlignmentBad;
  addPropData(d);

  d.id = propParalysed;
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
  d.allowTestingOnBot = true;
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
  d.allowTestingOnBot = true;
  d.alignment = propAlignmentBad;
  addPropData(d);

  d.id = propConfused;
  d.stdRndTurns = Range(100, 200);
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
  d.allowTestingOnBot = true;
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
  d.allowTestingOnBot = true;
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
  d.allowTestingOnBot = true;
  d.alignment = propAlignmentBad;
  addPropData(d);

  d.id = propFlared;
  d.stdRndTurns = Range(3, 4);
  d.name = "";
  d.nameShort = "";
  d.msg[propMsgOnStartPlayer] = "";
  d.msg[propMsgOnStartMonster] = "is perforated by a flare!";
  d.msg[propMsgOnEndPlayer] = "";
  d.msg[propMsgOnEndMonster] = "";
  d.msg[propMsgOnMorePlayer] = "";
  d.msg[propMsgOnMoreMonster] = "is perforated by another flare!";
  d.msg[propMsgOnResPlayer] = "";
  d.msg[propMsgOnResMonster] = "";
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
  d.msg[propMsgOnResPlayer] = "";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = true;
  d.allowDisplayTurns = false;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.alignment = propAlignmentBad;
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
  d.msg[propMsgOnResPlayer] = "I resist infection.";
  d.msg[propMsgOnResMonster] = "resists infection.";
  d.isMakingMonsterAware = true;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.isEndedByMagicHealing = true;
  d.allowTestingOnBot = true;
  d.alignment = propAlignmentBad;
  addPropData(d);

  d.id = propDiseased;
  d.stdRndTurns = Range(2000, 3000);
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
  d.allowTestingOnBot = true;
  d.alignment = propAlignmentBad;
  addPropData(d);

  d.id = propWeakened;
  d.stdRndTurns = Range(100, 200);
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
  d.allowTestingOnBot = true;
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
  d.allowTestingOnBot = true;
  d.alignment = propAlignmentNeutral;
  addPropData(d);

  d.id = propClairvoyant;
  d.stdRndTurns = Range(500, 600);
  d.name = "Clairvoyant";
  d.nameShort = "Clairvoyant";
  d.msg[propMsgOnStartPlayer] = "I see far and beyond!";
  d.msg[propMsgOnStartMonster] = "";
  d.msg[propMsgOnEndPlayer] = "My sight is limited.";
  d.msg[propMsgOnEndMonster] = "";
  d.msg[propMsgOnMorePlayer] = "";
  d.msg[propMsgOnMoreMonster] = "";
  d.msg[propMsgOnResPlayer] = "";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.allowTestingOnBot = true;
  d.alignment = propAlignmentGood;
  addPropData(d);

  d.id = propBlessed;
  d.stdRndTurns = Range(400, 600);
  d.name = "Blessed";
  d.nameShort = "Blessed";
  d.msg[propMsgOnStartPlayer] = "I feel luckier.";
  d.msg[propMsgOnStartMonster] = "";
  d.msg[propMsgOnEndPlayer] = "I have normal luck.";
  d.msg[propMsgOnEndMonster] = "";
  d.msg[propMsgOnMorePlayer] = "I feel luckier.";
  d.msg[propMsgOnMoreMonster] = "";
  d.msg[propMsgOnResPlayer] = "";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.allowTestingOnBot = true;
  d.alignment = propAlignmentGood;
  addPropData(d);

  d.id = propCursed;
  d.stdRndTurns = Range(400, 600);
  d.name = "Cursed";
  d.nameShort = "Cursed";
  d.msg[propMsgOnStartPlayer] = "I feel misfortunate.";
  d.msg[propMsgOnStartMonster] = "";
  d.msg[propMsgOnEndPlayer] = "I feel more fortunate.";
  d.msg[propMsgOnEndMonster] = "";
  d.msg[propMsgOnMorePlayer] = "I feel more misfortunate.";
  d.msg[propMsgOnMoreMonster] = "";
  d.msg[propMsgOnResPlayer] = "I resist misfortune.";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = true;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.allowTestingOnBot = true;
  d.alignment = propAlignmentBad;
  addPropData(d);

  d.id = propStill;
  d.stdRndTurns = Range(1, 1);
  d.name = "Still";
  d.nameShort = "Still";
  d.msg[propMsgOnStartPlayer] = "";
  d.msg[propMsgOnStartMonster] = "";
  d.msg[propMsgOnEndPlayer] = "";
  d.msg[propMsgOnEndMonster] = "";
  d.msg[propMsgOnMorePlayer] = "";
  d.msg[propMsgOnMoreMonster] = "";
  d.msg[propMsgOnResPlayer] = "";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = false;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.alignment = propAlignmentGood;
  addPropData(d);

  d.id = propWound;
  d.name = "Wound";
  d.msg[propMsgOnStartPlayer] = "I am wounded!";
  d.msg[propMsgOnStartMonster] = "";
  d.msg[propMsgOnEndPlayer] = "";
  d.msg[propMsgOnEndMonster] = "";
  d.msg[propMsgOnMorePlayer] = "I am more wounded!";
  d.msg[propMsgOnMoreMonster] = "";
  d.msg[propMsgOnResPlayer] = "I resist wounding!";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = false;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.isEndedByMagicHealing = true;
  d.alignment = propAlignmentBad;
  addPropData(d);

  d.id = propWaiting;
  d.stdRndTurns = Range(1, 1);
  d.name = "";
  d.nameShort = "";
  d.msg[propMsgOnStartPlayer] = "";
  d.msg[propMsgOnStartMonster] = "";
  d.msg[propMsgOnEndPlayer] = "";
  d.msg[propMsgOnEndMonster] = "";
  d.msg[propMsgOnMorePlayer] = "";
  d.msg[propMsgOnMoreMonster] = "";
  d.msg[propMsgOnResPlayer] = "";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = false;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.alignment = propAlignmentNeutral;
  addPropData(d);

  d.id = propDisabledAttack;
  d.stdRndTurns = Range(1, 1);
  d.name = "";
  d.nameShort = "";
  d.msg[propMsgOnStartPlayer] = "";
  d.msg[propMsgOnStartMonster] = "";
  d.msg[propMsgOnEndPlayer] = "";
  d.msg[propMsgOnEndMonster] = "";
  d.msg[propMsgOnMorePlayer] = "";
  d.msg[propMsgOnMoreMonster] = "";
  d.msg[propMsgOnResPlayer] = "";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = false;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.alignment = propAlignmentNeutral;
  addPropData(d);

  d.id = propDisabledMelee;
  d.stdRndTurns = Range(1, 1);
  d.name = "";
  d.nameShort = "";
  d.msg[propMsgOnStartPlayer] = "";
  d.msg[propMsgOnStartMonster] = "";
  d.msg[propMsgOnEndPlayer] = "";
  d.msg[propMsgOnEndMonster] = "";
  d.msg[propMsgOnMorePlayer] = "";
  d.msg[propMsgOnMoreMonster] = "";
  d.msg[propMsgOnResPlayer] = "";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = false;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.alignment = propAlignmentNeutral;
  addPropData(d);

  d.id = propDisabledRanged;
  d.stdRndTurns = Range(1, 1);
  d.name = "";
  d.nameShort = "";
  d.msg[propMsgOnStartPlayer] = "";
  d.msg[propMsgOnStartMonster] = "";
  d.msg[propMsgOnEndPlayer] = "";
  d.msg[propMsgOnEndMonster] = "";
  d.msg[propMsgOnMorePlayer] = "";
  d.msg[propMsgOnMoreMonster] = "";
  d.msg[propMsgOnResPlayer] = "";
  d.msg[propMsgOnResMonster] = "";
  d.isMakingMonsterAware = false;
  d.allowDisplayTurns = false;
  d.allowApplyMoreWhileActive = true;
  d.updatePlayerVisualWhenStartOrEnd = false;
  d.alignment = propAlignmentNeutral;
  addPropData(d);
}

void PropDataHandler::addPropData(PropData& d) {
  dataList[d.id] = d;
  d.reset();
}

PropHandler::PropHandler(Actor* owningActor, Engine* engine) :
  owningActor_(owningActor), eng(engine) {
  appliedProps_.resize(0);

  const ActorData* const actorData = owningActor->getData();

  for(unsigned int i = 0; i < endOfPropIds; i++) {
    if(actorData->intrProps[i]) {
      Prop* const prop = makePropFromId(PropId_t(i), propTurnsIndefinite);
      tryApplyProp(prop, true, true, true, true);
    }
  }
}

Prop* PropHandler::makePropFromId(const PropId_t id, PropTurns_t turnsInit,
                                  const int NR_TURNS) {
  switch(id) {
    case propWound:             return new PropWound(eng, turnsInit, NR_TURNS);
    case propNailed:            return new PropNailed(eng, turnsInit, NR_TURNS);
    case propBlind:             return new PropBlind(eng, turnsInit, NR_TURNS);
    case propBurning:           return new PropBurning(eng, turnsInit, NR_TURNS);
    case propFlared:            return new PropFlared(eng, turnsInit, NR_TURNS);
    case propParalysed:         return new PropParalyzed(eng, turnsInit, NR_TURNS);
//    case propFrozen:            return new PropFrozen(eng, turnsInit, NR_TURNS);
//    case propFreeAction:        return new PropFreeAction(eng, turnsInit, NR_TURNS);
    case propTerrified:         return new PropTerrified(eng, turnsInit, NR_TURNS);
    case propWeakened:          return new PropWeakened(eng, turnsInit, NR_TURNS);
    case propConfused:          return new PropConfused(eng, turnsInit, NR_TURNS);
    case propStunned:           return new PropStunned(eng, turnsInit, NR_TURNS);
    case propWaiting:           return new PropWaiting(eng, turnsInit, NR_TURNS);
    case propSlowed:            return new PropSlowed(eng, turnsInit, NR_TURNS);
    case propInfected:          return new PropInfected(eng, turnsInit, NR_TURNS);
    case propDiseased:          return new PropDiseased(eng, turnsInit, NR_TURNS);
    case propPoisoned:          return new PropPoisoned(eng, turnsInit, NR_TURNS);
    case propFainted:           return new PropFainted(eng, turnsInit, NR_TURNS);
    case propFrenzied:          return new PropFrenzied(eng, turnsInit, NR_TURNS);
//    case propPerfectStealth:    return new PropPerfectStealth(eng, turnsInit, NR_TURNS);
//    case propPerfectFortitude:  return new PropPerfectFortitude(eng, turnsInit, NR_TURNS);
//    case propPerfectToughness:  return new PropPerfectToughness(eng, turnsInit, NR_TURNS);
    case propStill:             return new PropStill(eng, turnsInit, NR_TURNS);
    case propDisabledAttack:    return new PropDisabledAttack(eng, turnsInit, NR_TURNS);
    case propDisabledMelee:     return new PropDisabledMelee(eng, turnsInit, NR_TURNS);
    case propDisabledRanged:    return new PropDisabledRanged(eng, turnsInit, NR_TURNS);
    case propBlessed:           return new PropBlessed(eng, turnsInit, NR_TURNS);
    case propCursed:            return new PropCursed(eng, turnsInit, NR_TURNS);
    case propClairvoyant:       return new PropClairvoyant(eng, turnsInit, NR_TURNS);
    case propRAcid:             return new PropRAcid(eng, turnsInit, NR_TURNS);
    case propRCold:             return new PropRCold(eng, turnsInit, NR_TURNS);
    case propRConfusion:        return new PropRConfusion(eng, turnsInit, NR_TURNS);
    case propRElec:             return new PropRElec(eng, turnsInit, NR_TURNS);
    case propRFear:             return new PropRFear(eng, turnsInit, NR_TURNS);
    case propRFire:             return new PropRFire(eng, turnsInit, NR_TURNS);
    case propRPoison:           return new PropRPoison(eng, turnsInit, NR_TURNS);
    case propRSleep:            return new PropRSleep(eng, turnsInit, NR_TURNS);
    case propLightSensitive:    return new PropLightSensitive(eng, turnsInit, NR_TURNS);
    case endOfPropIds: {
      trace << "[WARNING] Illegal property id, in PropHandler::makePropFromId()" << endl;
      return NULL;
    }
  }
  return NULL;
}

PropHandler::~PropHandler() {
  for(unsigned int i = 0; i < appliedProps_.size(); i++) {
    delete appliedProps_.at(i);
  }
}

void PropHandler::getPropsFromSource(vector<Prop*>& propList,
                                     const PropSrc_t source) const {
  propList.resize(0);

  //Get from applied properties
  if(source == propSrcAppliedAndInv || source == propSrcApplied) {
    propList = appliedProps_;
  }

  //Get from inventory
  if(source == propSrcAppliedAndInv || source == propSrcInv) {
    vector<Item*> items;
    owningActor_->getInventory()->getAllItems(items);
    const int NR_ITEMS = items.size();
    for(int i = 0; i < NR_ITEMS; i++) {
      Item* const item = items.at(i);
      const int NR_PROPS_FROM_ITEM = item->propsEnabledOnCarrier.size();
      for(int ii = 0; ii < NR_PROPS_FROM_ITEM; ii++) {
        Prop* const prop = item->propsEnabledOnCarrier.at(ii);
        prop->owningActor_ = owningActor_;
        propList.push_back(prop);
      }
    }
  }
}

bool PropHandler::hasProp(const PropId_t id) const {
  vector<Prop*> propList;
  getPropsFromSource(propList, propSrcAppliedAndInv);
  return hasProp(id, propList);
}

bool PropHandler::hasProp(const PropId_t id,
                          const vector<Prop*> propList) const {

  const unsigned int NR_PROPS = propList.size();
  for(unsigned int i = 0; i < NR_PROPS; i++) {
    if(propList.at(i)->getId() == id) {
      return true;
    }
  }
  return false;
}

bool PropHandler::tryResistProp(
  const PropId_t id, const vector<Prop*>& propList) {

  const int NR_PROPS = propList.size();
  for(int i = 0; i < NR_PROPS; i++) {
    Prop* const prop = propList.at(i);
    if(prop->tryResistOtherProp(id)) {
      return true;
    }
  }
  return false;
}

bool PropHandler::tryResistDmg(
  const DmgTypes_t dmgType, const bool ALLOW_MESSAGE_WHEN_TRUE) {

  vector<Prop*> propList;
  getPropsFromSource(propList, propSrcAppliedAndInv);
  const int NR_PROPS = propList.size();
  for(int i = 0; i < NR_PROPS; i++) {
    if(propList.at(i)->tryResistDmg(dmgType, ALLOW_MESSAGE_WHEN_TRUE)) {
      return true;
    }
  }
  return false;
}

bool PropHandler::allowSee() {
  vector<Prop*> propList;
  getPropsFromSource(propList, propSrcAppliedAndInv);

  if(hasProp(propClairvoyant, propList)) {
    return true;
  }

  for(unsigned int i = 0; i < propList.size(); i++) {
    if(propList.at(i)->allowSee() == false) {
      return false;
    }
  }
  return true;
}

void PropHandler::tryApplyProp(Prop* const prop, const bool FORCE_EFFECT,
                               const bool NO_MESSAGES,
                               const bool DISABLE_REDRAW,
                               const bool DISABLE_PROP_START_EFFECTS) {

  //---------------------------------------------------------------------------
  //TODO This is used for offsetting game time immediately decreasing all prop
  //turns after they get applied. I.e. being Still (from marksman bonus), or
  //Waiting (from walking through water) for one turn, will not even register
  //for the player, since there will be a standard turn between. When using the
  //code below, one can for example set Still for 1 turn when pressing wait,
  //which sounds sensible. It also sounds sensible that props should end when
  //turnsLeft == 0... it's unclear how this should be handled, but the change
  //below seems to work somewhat well. It doesn't feel like a good long term
  //solution though, and may have some unknown problems. Investigation needed.
  //---------------------------------------------------------------------------
  if(prop->turnsLeft_ > 0) {
    prop->turnsLeft_++;
  }
  //---------------------------------------------------------------------------

  prop->owningActor_ = owningActor_;

  const bool IS_PLAYER = owningActor_ == eng->player;

  bool playerSeeOwner = false;

  if(DISABLE_REDRAW == false) {
    bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
    MapParser::parse(CellPredBlocksVision(eng), blockers);
    playerSeeOwner = eng->player->checkIfSeeActor(*owningActor_, blockers);
  }

  if(FORCE_EFFECT == false) {
    vector<Prop*> allProps;
    getPropsFromSource(allProps, propSrcAppliedAndInv);
    if(tryResistProp(prop->getId(), allProps)) {
      if(NO_MESSAGES == false) {
        if(IS_PLAYER) {
          string msg = "";
          prop->getMsg(propMsgOnResPlayer, msg);
          if(msg.empty() == false) {
            eng->log->addMsg(msg, clrWhite, true);
          }
        } else {
          if(eng->player->checkIfSeeActor(*owningActor_, NULL)) {
            string msg = "";
            prop->getMsg(propMsgOnResMonster, msg);
            if(msg.empty() == false) {
              const string monsterName = owningActor_->getNameThe();
              eng->log->addMsg(monsterName + " " + msg, clrWhite, true);
            }
          }
        }
      }
      return;
    }
  }

  //This point reached means nothing is blocking the property.

  //Actor already has property appplied?
  const unsigned int NR_APPLIED_PROPS = appliedProps_.size();
  for(unsigned int i = 0; i < NR_APPLIED_PROPS; i++) {
    if(prop->getId() == appliedProps_.at(i)->getId()) {

      if(prop->allowApplyMoreWhileActive() == false) {
        delete prop;
        return;
      }

      const int TURNS_LEFT_OLD = appliedProps_.at(i)->turnsLeft_;
      const int TURNS_LEFT_NEW = prop->turnsLeft_;

      if(TURNS_LEFT_NEW >= TURNS_LEFT_OLD && NO_MESSAGES == false) {
        if(IS_PLAYER) {
          string msg = "";
          prop->getMsg(propMsgOnMorePlayer, msg);
          if(msg.empty() == false) {
            eng->log->addMsg(msg, clrWhite, true);
          }
        } else {
          if(playerSeeOwner) {
            string msg = "";
            prop->getMsg(propMsgOnMoreMonster, msg);
            if(msg.empty() == false) {
              eng->log->addMsg(owningActor_->getNameThe() + " " + msg);
            }
          }
        }
      }

      appliedProps_.at(i)->onMore();
      appliedProps_.at(i)->turnsLeft_ = max(TURNS_LEFT_OLD, TURNS_LEFT_NEW);

      delete prop;
      return;
    }
  }

  //This part reached means the applied property is new.
  appliedProps_.push_back(prop);
  if(DISABLE_PROP_START_EFFECTS == false) {
    prop->onStart();
  }

  if(DISABLE_REDRAW == false) {
    if(prop->updatePlayerVisualWhenStartOrEnd()) {
      prop->owningActor_->updateColor();
      eng->player->updateFov();
      eng->renderer->drawMapAndInterface();
    }
  }

  if(NO_MESSAGES == false) {
    if(IS_PLAYER) {
      string msg = "";
      prop->getMsg(propMsgOnStartPlayer, msg);
      if(msg.empty() == false) {
        eng->log->addMsg(msg, clrWhite, true);
      }
    } else {
      if(playerSeeOwner) {
        string msg = "";
        prop->getMsg(propMsgOnStartMonster, msg);
        if(msg.empty() == false) {
          eng->log->addMsg(owningActor_->getNameThe() + " " + msg);
        }
      }
    }
  }
}

void PropHandler::tryApplyPropFromWpn(const Weapon& wpn, const bool IS_MELEE) {
  const ItemData& wpnData = wpn.getData();
  Prop* propAppliedFromWpn =
    IS_MELEE ? wpnData.propAppliedOnMelee : wpnData.propAppliedOnRanged;

  if(propAppliedFromWpn != NULL) {
    //Make a copy of the weapon effect
    Prop* const prop = makePropFromId(
                         propAppliedFromWpn->getId(), propTurnsSpecified,
                         propAppliedFromWpn->turnsLeft_);
    tryApplyProp(prop);
  }
}

bool PropHandler::endAppliedProp(
  const PropId_t id,
  const bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS],
  const bool RUN_PROP_END_EFFECTS) {

  int index   = -1;
  Prop* prop  = NULL;
  const unsigned int NR_APPLIED_PROPS = appliedProps_.size();
  for(unsigned int i = 0; i < NR_APPLIED_PROPS; i++) {
    prop = appliedProps_.at(i);
    if(prop->getId() == id) {
      index = i;
      break;
    }
  }
  if(index == -1) {
    return false;
  }

  appliedProps_.erase(appliedProps_.begin() + index);

  if(RUN_PROP_END_EFFECTS) {
    const bool IS_VISUAL_UPDATE_NEEDED =
      prop->updatePlayerVisualWhenStartOrEnd();

    if(IS_VISUAL_UPDATE_NEEDED) {
      prop->owningActor_->updateColor();
      eng->player->updateFov();
      eng->renderer->drawMapAndInterface();
    }

    if(owningActor_ == eng->player) {
      string msg = "";
      prop->getMsg(propMsgOnEndPlayer, msg);
      eng->log->addMsg(msg, clrWhite);
    } else {
      if(eng->player->checkIfSeeActor(*owningActor_, visionBlockers)) {
        string msg = "";
        prop->getMsg(propMsgOnEndMonster, msg);
        if(msg.empty() == false) {
          eng->log->addMsg(owningActor_->getNameThe() + " " + msg);
        }
      }
    }
    prop->onEnd();
  }

  delete prop;
  return true;
}

void PropHandler::newTurnAllProps(
  const bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS]) {

  for(unsigned int i = 0; i < appliedProps_.size();) {
    Prop* const prop = appliedProps_.at(i);
    if(owningActor_ != eng->player) {
      if(prop->isMakingMonsterAware()) {
        dynamic_cast<Monster*>(owningActor_)->playerAwarenessCounter =
          owningActor_->getData()->nrTurnsAwarePlayer;
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
  }

  vector<Prop*> invProps;
  getPropsFromSource(invProps, propSrcInv);
  const unsigned int NR_INV_PROPS = invProps.size();
  for(unsigned int i = 0; i < NR_INV_PROPS; i++) {
    Prop* const prop = invProps.at(i);
    if(owningActor_ != eng->player) {
      if(prop->isMakingMonsterAware()) {
        dynamic_cast<Monster*>(owningActor_)->playerAwarenessCounter =
          owningActor_->getData()->nrTurnsAwarePlayer;
      }
    }
    prop->onNewTurn();
  }
}

void PropHandler::getPropsInterfaceLine(vector<StringAndClr>& line) {
  line.resize(0);

  const bool IS_SELF_AWARE =
    eng->playerBonHandler->isBonPicked(playerBon_selfAware);

  vector<Prop*> props;
  getPropsFromSource(props, propSrcAppliedAndInv);
  const int NR_PROPS = props.size();
  for(int i = 0; i < NR_PROPS; i++) {
    Prop* const prop = props.at(i);
    const string propName = prop->getNameShort();
    if(propName.empty() == false) {
      const PropAlignment_t alignment = prop->getAlignment();
      const int TURNS_LEFT = prop->turnsLeft_;
      const string turnsStr = TURNS_LEFT > 0 && IS_SELF_AWARE ?
                              ("(" + toString(TURNS_LEFT) + ")") : "";
      line.push_back(
        StringAndClr(propName + turnsStr,
                     alignment == propAlignmentGood ? clrMessageGood :
                     alignment == propAlignmentBad  ? clrMessageBad :
                     clrWhite));
    }
  }
}

void PropHandler::changeMoveDir(const Pos& actorPos, Dir_t& dir) {
  vector<Prop*> propList;
  getPropsFromSource(propList, propSrcAppliedAndInv);
  const unsigned int NR_PROPS = propList.size();
  for(unsigned int i = 0; i < NR_PROPS; i++) {
    propList.at(i)->changeMoveDir(actorPos, dir);
  }
}

bool PropHandler::allowAttack(const bool ALLOW_MESSAGE_WHEN_FALSE) {
  vector<Prop*> propList;
  getPropsFromSource(propList, propSrcAppliedAndInv);
  const unsigned int NR_PROPS = propList.size();
  for(unsigned int i = 0; i < NR_PROPS; i++) {
    if(
      propList.at(i)->allowAttackMelee(ALLOW_MESSAGE_WHEN_FALSE) == false &&
      propList.at(i)->allowAttackRanged(ALLOW_MESSAGE_WHEN_FALSE) == false) {
      return false;
    }
  }
  return true;
}

bool PropHandler::allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE) {
  vector<Prop*> propList;
  getPropsFromSource(propList, propSrcAppliedAndInv);
  const unsigned int NR_PROPS = propList.size();
  for(unsigned int i = 0; i < NR_PROPS; i++) {
    if(propList.at(i)->allowAttackMelee(ALLOW_MESSAGE_WHEN_FALSE) == false) {
      return false;
    }
  }
  return true;
}

bool PropHandler::allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) {
  vector<Prop*> propList;
  getPropsFromSource(propList, propSrcAppliedAndInv);
  const unsigned int NR_PROPS = propList.size();
  for(unsigned int i = 0; i < NR_PROPS; i++) {
    if(propList.at(i)->allowAttackRanged(ALLOW_MESSAGE_WHEN_FALSE) == false) {
      return false;
    }
  }
  return true;
}

bool PropHandler::allowMove() {
  vector<Prop*> propList;
  getPropsFromSource(propList, propSrcAppliedAndInv);
  const unsigned int NR_PROPS = propList.size();
  for(unsigned int i = 0; i < NR_PROPS; i++) {
    if(propList.at(i)->allowMove() == false) {
      return false;
    }
  }
  return true;
}

bool PropHandler::allowAct() {
  vector<Prop*> propList;
  getPropsFromSource(propList, propSrcAppliedAndInv);
  const unsigned int NR_PROPS = propList.size();
  for(unsigned int i = 0; i < NR_PROPS; i++) {
    if(propList.at(i)->allowAct() == false) {
      return false;
    }
  }
  return true;
}

bool PropHandler::allowRead(const bool ALLOW_MESSAGE_WHEN_FALSE) {
  traceVerbose << "PropHandler::allowRead()..." << endl;
  vector<Prop*> propList;
  getPropsFromSource(propList, propSrcAppliedAndInv);
  const unsigned int NR_PROPS = propList.size();
  for(unsigned int i = 0; i < NR_PROPS; i++) {
    if(propList.at(i)->allowRead(ALLOW_MESSAGE_WHEN_FALSE) == false) {
      traceVerbose << "PropHandler::allowRead() [DONE] - return false" << endl;
      return false;
    }
  }
  traceVerbose << "PropHandler::allowRead() [DONE] - return true" << endl;
  return true;
}

bool PropHandler::allowCastSpells(const bool ALLOW_MESSAGE_WHEN_FALSE) {
  vector<Prop*> propList;
  getPropsFromSource(propList, propSrcAppliedAndInv);
  const unsigned int NR_PROPS = propList.size();
  for(unsigned int i = 0; i < NR_PROPS; i++) {
    if(propList.at(i)->allowCastSpells(ALLOW_MESSAGE_WHEN_FALSE) == false) {
      return false;
    }
  }
  return true;
}

void PropHandler::onHit() {
  vector<Prop*> propList;
  getPropsFromSource(propList, propSrcAppliedAndInv);
  const unsigned int NR_PROPS = propList.size();
  for(unsigned int i = 0; i < NR_PROPS; i++) {
    propList.at(i)->onHit();
  }
}

int PropHandler::getAbilityMod(const Abilities_t ability) {
  vector<Prop*> propList;
  getPropsFromSource(propList, propSrcAppliedAndInv);
  const unsigned int NR_PROPS = propList.size();
  int modifier = 0;
  for(unsigned int i = 0; i < NR_PROPS; i++) {
    modifier += propList.at(i)->getAbilityMod(ability);
  }
  return modifier;
}

Prop* PropHandler::getAppliedProp(const PropId_t id) const {
  const unsigned int NR_PROPS = appliedProps_.size();
  for(unsigned int i = 0; i < NR_PROPS; i++) {
    if(appliedProps_.at(i)->getId() == id) {
      return appliedProps_.at(i);
    }
  }
  return NULL;
}

bool PropHandler::changeActorClr(SDL_Color& clr) {
  vector<Prop*> propList;
  getPropsFromSource(propList, propSrcAppliedAndInv);
  const unsigned int NR_PROPS = propList.size();
  for(unsigned int i = 0; i < NR_PROPS; i++) {
    if(propList.at(i)->changeActorClr(clr)) {
      return true;
    }
  }
  return false;
}

void PropHandler::endAppliedPropsByMagicHealing() {
  bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
  MapParser::parse(CellPredBlocksVision(eng), visionBlockers);
  vector<Prop*> propList;
  getPropsFromSource(propList, propSrcAppliedAndInv);
  for(unsigned int i = 0; i < propList.size(); i++) {
    if(propList.at(i)->isEndedByMagicHealing()) {
      endAppliedProp(appliedProps_.at(i)->getId(), visionBlockers);
      propList.erase(propList.begin() + i);
      i--;
    }
  }
}

Prop::Prop(PropId_t id, Engine* engine, PropTurns_t turnsInit, int turns) :
  turnsLeft_(turns), owningActor_(NULL), id_(id), eng(engine),
  data_(&(engine->propDataHandler->dataList[id])) {

  if(turnsInit == propTurnsStandard) {
    turnsLeft_ = eng->dice.range(data_->stdRndTurns);
  }
  if(turnsInit == propTurnsIndefinite) {
    turnsLeft_ = -1;
  }
}

void PropBlessed::onStart() {
  bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
  MapParser::parse(CellPredBlocksVision(eng), visionBlockers);
  owningActor_->getPropHandler()->endAppliedProp(propCursed, visionBlockers);
}

void PropCursed::onStart() {
  bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
  MapParser::parse(CellPredBlocksVision(eng), visionBlockers);
  owningActor_->getPropHandler()->endAppliedProp(propBlessed, visionBlockers);
}

void PropInfected::onNewTurn() {
  if(eng->dice.oneIn(250)) {
    owningActor_->getPropHandler()->tryApplyProp(
      new PropDiseased(eng, propTurnsStandard));
  }
}

void PropDiseased::onStart() {
  //player->getHpMax() will now return a decreased value,
  //cap current HP to the new, lower, maximum
  int& hp = eng->player->hp_;
  hp = min(eng->player->getHpMax(true), hp);
}

void PropPoisoned::onNewTurn() {
  const int DMG_N_TURN = 4;
  const int TURN = eng->gameTime->getTurn();
  if(TURN == (TURN / DMG_N_TURN) * DMG_N_TURN) {

    if(owningActor_ == eng->player) {
      eng->log->addMsg(
        "I am suffering from the poison!", clrMessageBad, true);
    } else {
      if(eng->player->checkIfSeeActor(*owningActor_, NULL)) {
        eng->log->addMsg(
          owningActor_->getNameThe() + " suffers from poisoning!");
      }
    }

    owningActor_->hit(1, dmgType_pure, false);
  }
}

bool PropTerrified::allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE) {
  if(owningActor_ == eng->player && ALLOW_MESSAGE_WHEN_FALSE) {
    eng->log->addMsg(
      "I am too terrified to engage in close combat!");
  }
  return false;
}

bool PropTerrified::allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) {
  (void)ALLOW_MESSAGE_WHEN_FALSE;
  return true;
}

void PropWound::getMsg(const PropMsg_t msgType, string& msgRef) {
  switch(msgType) {
    case propMsgOnStartPlayer:
      msgRef = data_->msg[propMsgOnStartPlayer];
      break;

    case propMsgOnStartMonster:
      msgRef = ""; break;

    case propMsgOnEndPlayer:
      msgRef = nrWounds_ > 1 ?
               "All my wounds are healed!" :
               "A wound is healed!";
      break;

    case propMsgOnEndMonster: msgRef = ""; break;

    case propMsgOnMorePlayer: msgRef = data_->msg[propMsgOnMorePlayer]; break;

    case propMsgOnMoreMonster: msgRef = ""; break;

    case propMsgOnResPlayer: msgRef = data_->msg[propMsgOnResPlayer]; break;

    case propMsgOnResMonster: msgRef = ""; break;
    case endOfPropMsg: msgRef = ""; break;
  }
}

void PropWound::healOneWound() {
  trace << "StatusWound: Nr wounds before healing one: " << nrWounds_ << endl;
  if(--nrWounds_ > 0) {
    eng->log->addMsg("A wound is healed!");
  } else {
    bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
    MapParser::parse(CellPredBlocksVision(eng), visionBlockers);
    owningActor_->getPropHandler()->endAppliedProp(
      propWound, visionBlockers);
  }
}

void PropWound::onMore() {
  nrWounds_++;

  if(nrWounds_ >= 5) {
    if(owningActor_ == eng->player) {
      eng->log->addMsg("I die from my wounds!");
    }
    owningActor_->die(false, false, true);
  }
}

void PropNailed::changeMoveDir(const Pos& actorPos, Dir_t& dir) {
  (void)actorPos;

  if(dir != dirCenter) {

    if(owningActor_ == eng->player) {
      eng->log->addMsg(
        "I struggle to tear out the spike!", clrMessageBad);
    } else {
      if(eng->player->checkIfSeeActor(*owningActor_, NULL)) {
        eng->log->addMsg(
          owningActor_->getNameThe() +  " struggles in pain!", clrMessageGood);
      }
    }

    owningActor_->hit(eng->dice(1, 3), dmgType_physical, false);

    if(owningActor_->deadState == actorDeadState_alive) {

      //TODO reimplement something affecting chance of success?

      if(eng->dice.oneIn(4)) {
        nrSpikes_--;
        if(nrSpikes_ > 0) {
          if(owningActor_ == eng->player) {
            eng->log->addMsg("I rip out a spike from my flesh!");
          } else {
            if(eng->player->checkIfSeeActor(*owningActor_, NULL)) {
              eng->log->addMsg(
                owningActor_->getNameThe() + " tears out a spike!");
            }
          }
        }
      }
    }

    dir = dirCenter;
  }
}

bool PropConfused::allowRead(const bool ALLOW_MESSAGE_WHEN_FALSE) {
  if(owningActor_ == eng->player && ALLOW_MESSAGE_WHEN_FALSE) {
    eng->log->addMsg("I'm too confused.");
  }
  return false;
}

bool PropConfused::allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE) {
  (void)ALLOW_MESSAGE_WHEN_FALSE;
  if(owningActor_ != eng->player) {
    return eng->dice.coinToss();
  }
  return true;
}

bool PropConfused::allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) {
  (void)ALLOW_MESSAGE_WHEN_FALSE;

  if(owningActor_ != eng->player) {
    return eng->dice.coinToss();
  }
  return true;
}

void PropConfused::changeMoveDir(const Pos& actorPos, Dir_t& dir) {
  if(dir != dirCenter) {

    bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
    MapParser::parse(
      CellPredBlocksBodyType(owningActor_->getBodyType(), true, eng),
      blockers);

    if(eng->dice.oneIn(8)) {
      int triesLeft = 100;
      while(triesLeft != 0) {
        //-1 to 1 for x and y
        const Pos delta(eng->dice.range(-1, 1), eng->dice.range(-1, 1));
        if(delta.x != 0 || delta.y != 0) {
          const Pos c = actorPos + delta;
          if(blockers[c.x][c.y] == false) {
            dir = DirConverter(eng).getDir(delta);
          }
        }
        triesLeft--;
      }
    }
  }
}

void PropFrenzied::changeMoveDir(const Pos& actorPos, Dir_t& dir) {
  vector<Actor*> spotedEnemies;
  owningActor_->getSpotedEnemies(spotedEnemies);

  if(spotedEnemies.empty()) {
    return;
  }

  vector<Pos> spotedEnemiesPositions;
  spotedEnemiesPositions.resize(0);
  for(unsigned int i = 0; i < spotedEnemies.size(); i++) {
    spotedEnemiesPositions.push_back(spotedEnemies.at(i)->pos);
  }
  sort(spotedEnemiesPositions.begin(), spotedEnemiesPositions.end(),
       IsCloserToOrigin(actorPos, eng));

  const Pos& closestEnemyPos = spotedEnemiesPositions.at(0);

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  MapParser::parse(
    CellPredBlocksBodyType(owningActor_->getBodyType(), false, eng), blockers);

  vector<Pos> line;
  eng->lineCalc->calcNewLine(actorPos, closestEnemyPos, true, 999, false, line);

  if(line.size() > 1) {
    for(unsigned int i = 0; i < line.size(); i++) {
      const Pos& pos = line.at(i);
      if(blockers[pos.x][pos.y]) {
        return;
      }
    }
    dir = DirConverter(eng).getDir(line.at(1) - actorPos);
  }
}

bool PropFrenzied::tryResistOtherProp(const PropId_t id) {
  return id == propConfused || id == propFainted || id == propTerrified ||
         id == propWeakened;
}

void PropFrenzied::onStart() {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  MapParser::parse(CellPredBlocksVision(eng), blockers);
  owningActor_->getPropHandler()->endAppliedProp(propBurning,   blockers);
  owningActor_->getPropHandler()->endAppliedProp(propConfused,  blockers);
  owningActor_->getPropHandler()->endAppliedProp(propTerrified, blockers);
  owningActor_->getPropHandler()->endAppliedProp(propWeakened,  blockers);
}

void PropFrenzied::onEnd() {
  owningActor_->getPropHandler()->tryApplyProp(
    new PropWeakened(eng, propTurnsStandard));
}

bool PropFrenzied::allowRead(const bool ALLOW_MESSAGE_WHEN_FALSE) {
  if(owningActor_ == eng->player && ALLOW_MESSAGE_WHEN_FALSE) {
    eng->log->addMsg("I'm too enraged to concentrate!");
  }
  return false;
}

bool PropFrenzied::allowCastSpells(const bool ALLOW_MESSAGE_WHEN_FALSE) {
  if(owningActor_ == eng->player && ALLOW_MESSAGE_WHEN_FALSE) {
    eng->log->addMsg("I'm too enraged to concentrate!");
  }
  return false;
}

void PropBurning::onStart() {
//  owningActor_->addLight(eng->map->light);
}

void PropBurning::onNewTurn() {
  if(owningActor_ == eng->player) {
    eng->log->addMsg("AAAARGH IT BURNS!!!", clrRedLgt);
  }
  owningActor_->hit(eng->dice(1, 2), dmgType_fire, false);
}

bool PropBurning::allowRead(const bool ALLOW_MESSAGE_WHEN_FALSE) {
  if(owningActor_ == eng->player && ALLOW_MESSAGE_WHEN_FALSE) {
    eng->log->addMsg("I cannot read while I'm burning.");
  }
  return false;
}

void PropBlind::onStart() {
  bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
  MapParser::parse(CellPredBlocksVision(eng), visionBlockers);
  owningActor_->getPropHandler()->endAppliedProp(
    propClairvoyant, visionBlockers);
}

bool PropBlind::updatePlayerVisualWhenStartOrEnd() {
  return owningActor_ == eng->player;
}

void PropParalyzed::onStart() {
  Player* const player = eng->player;
  if(owningActor_ == player) {
    const Pos& playerPos = player->pos;
    const int DYNAMITE_FUSE = eng->player->dynamiteFuseTurns;
    const int FLARE_FUSE = eng->player->flareFuseTurns;
    const int MOLOTOV_FUSE = eng->player->molotovFuseTurns;

    if(DYNAMITE_FUSE > 0) {
      player->dynamiteFuseTurns = -1;
      player->updateColor();
      eng->log->addMsg("The lit Dynamite stick falls from my hands!");
      Feature* const f =
        eng->map->cells[playerPos.x][playerPos.y].featureStatic;
      if(f->isBottomless() == false) {
        eng->featureFactory->spawnFeatureAt(
          feature_litDynamite, playerPos,
          new DynamiteSpawnData(DYNAMITE_FUSE));
      }
    }
    if(FLARE_FUSE > 0) {
      player->flareFuseTurns = -1;
      player->updateColor();
      eng->log->addMsg("The lit Flare falls from my hands.");
      Feature* const f =
        eng->map->cells[playerPos.x][playerPos.y].featureStatic;
      if(f->isBottomless() == false) {
        eng->featureFactory->spawnFeatureAt(
          feature_litFlare, playerPos, new DynamiteSpawnData(FLARE_FUSE));
      }
      eng->gameTime->updateLightMap();
      player->updateFov();
      eng->renderer->drawMapAndInterface();
    }
    if(MOLOTOV_FUSE > 0) {
      player->molotovFuseTurns = -1;
      player->updateColor();
      eng->log->addMsg("The lit Molotov Cocktail falls from my hands!");
      eng->explosionMaker->runExplosion(
        player->pos, sfxExplosionMolotov, false,
        new PropBurning(eng, propTurnsStandard));
    }
  }
}

void PropFainted::onStart() {
  bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
  MapParser::parse(CellPredBlocksVision(eng), visionBlockers);
  owningActor_->getPropHandler()->endAppliedProp(
    propClairvoyant, visionBlockers);
}

bool PropFainted::updatePlayerVisualWhenStartOrEnd() {
  return owningActor_ == eng->player;
}

bool PropClairvoyant::updatePlayerVisualWhenStartOrEnd() {
  return owningActor_ == eng->player;
}

void PropClairvoyant::onStart() {
  bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
  MapParser::parse(CellPredBlocksVision(eng), visionBlockers);
  owningActor_->getPropHandler()->endAppliedProp(propBlind, visionBlockers);
}

void PropFlared::onNewTurn() {
  owningActor_->hit(1, dmgType_fire, false);

  if(turnsLeft_ == 0) {
    bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
    MapParser::parse(CellPredBlocksVision(eng), visionBlockers);
    owningActor_->getPropHandler()->tryApplyProp(
      new PropBurning(eng, propTurnsStandard));
    owningActor_->getPropHandler()->endAppliedProp(
      propFlared, visionBlockers);
  }
}

bool PropRAcid::tryResistDmg(
  const DmgTypes_t dmgType, const bool ALLOW_MESSAGE_WHEN_TRUE) {
  if(dmgType == dmgType_acid) {
    if(ALLOW_MESSAGE_WHEN_TRUE) {
      if(owningActor_ == eng->player) {
        eng->log->addMsg("I feel a faint burning sensation.");
      } else if(eng->player->checkIfSeeActor(*owningActor_, NULL)) {
        eng->log->addMsg(owningActor_->getNameThe() + " seems unaffected.");
      }
    }
    return true;
  }
  return false;
}

bool PropRCold::tryResistDmg(
  const DmgTypes_t dmgType, const bool ALLOW_MESSAGE_WHEN_TRUE) {
  if(dmgType == dmgType_cold) {
    if(ALLOW_MESSAGE_WHEN_TRUE) {
      if(owningActor_ == eng->player) {
        eng->log->addMsg("I feel chilly.");
      } else if(eng->player->checkIfSeeActor(*owningActor_, NULL)) {
        eng->log->addMsg(owningActor_->getNameThe() + " seems unaffected.");
      }
    }
    return true;
  }
  return false;
}

bool PropRElec::tryResistDmg(
  const DmgTypes_t dmgType, const bool ALLOW_MESSAGE_WHEN_TRUE) {
  if(dmgType == dmgType_electric) {
    if(ALLOW_MESSAGE_WHEN_TRUE) {
      if(owningActor_ == eng->player) {
        eng->log->addMsg("I feel a faint tingle.");
      } else if(eng->player->checkIfSeeActor(*owningActor_, NULL)) {
        eng->log->addMsg(owningActor_->getNameThe() + " seems unaffected.");
      }
    }
    return true;
  }
  return false;
}

bool PropRConfusion::tryResistOtherProp(const PropId_t id) {
  return id == propConfused;
}

void PropRConfusion::onStart() {
  bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
  MapParser::parse(CellPredBlocksVision(eng), visionBlockers);
  owningActor_->getPropHandler()->endAppliedProp(propConfused, visionBlockers);
}

bool PropRFear::tryResistOtherProp(const PropId_t id) {
  return id == propTerrified;
}

void PropRFear::onStart() {
  bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
  MapParser::parse(CellPredBlocksVision(eng), visionBlockers);
  owningActor_->getPropHandler()->endAppliedProp(propTerrified, visionBlockers);
}

bool PropRFire::tryResistOtherProp(const PropId_t id) {
  return id == propBurning;
}

void PropRFire::onStart() {
  bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
  MapParser::parse(CellPredBlocksVision(eng), visionBlockers);
  owningActor_->getPropHandler()->endAppliedProp(propBurning, visionBlockers);
}

bool PropRFire::tryResistDmg(
  const DmgTypes_t dmgType, const bool ALLOW_MESSAGE_WHEN_TRUE) {
  if(dmgType == dmgType_fire) {
    if(ALLOW_MESSAGE_WHEN_TRUE) {
      if(owningActor_ == eng->player) {
        eng->log->addMsg("I feel hot.");
      } else if(eng->player->checkIfSeeActor(*owningActor_, NULL)) {
        eng->log->addMsg(owningActor_->getNameThe() + " seems unaffected.");
      }
    }
    return true;
  }
  return false;
}

bool PropRPoison::tryResistOtherProp(const PropId_t id) {
  return id == propPoisoned;
  return false;
}

void PropRPoison::onStart() {
  bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
  MapParser::parse(CellPredBlocksVision(eng), visionBlockers);
  owningActor_->getPropHandler()->endAppliedProp(propPoisoned, visionBlockers);
}

bool PropRSleep::tryResistOtherProp(const PropId_t id) {
  return id == propFainted;
  return false;
}

void PropRSleep::onStart() {
  bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
  MapParser::parse(CellPredBlocksVision(eng), visionBlockers);
  owningActor_->getPropHandler()->endAppliedProp(propFainted, visionBlockers);
}

