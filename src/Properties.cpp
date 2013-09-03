#include "Properties.h"

#include "Engine.h"
#include "ItemArmor.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Postmortem.h"
#include "Render.h"
#include "ItemWeapon.h"
#include "ActorMonster.h"
#include "Inventory.h"
#include "Map.h"
#include "Explosion.h"
#include "FeatureFactory.h"
#include "PlayerBonuses.h"

using namespace std;

void PropDataHandler::initDataList() {
  PropData d;

  d.id = propRFire;
  d.stdRndTurns                       = Range(40, 60);
  d.name                              = "Fire resistance";
  d.nameShort                         = "RFire";
  d.msg[propMsgOnStartPlayer]         = "[RFire Player start]";
  d.msg[propMsgOnStartMonster]        = "[RFire Monster start]";
  d.msg[propMsgOnEndPlayer]           = "[RFire Player end]";
  d.msg[propMsgOnEndMonster]          = "[RFire Monster end]";
  d.msg[propMsgOnMorePlayer]          = "[RFire Player more]";
  d.msg[propMsgOnMoreMonster]         = "[RFire Monster more]";
  d.msg[propMsgOnResistPlayer]        = "";
  d.msg[propMsgOnResistMonster]       = "";
  d.isMakingMonsterAware              = false;
  d.allowDisplayTurns                 = true;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = false;
  d.alignment                         = propAlignmentGood;
  addPropData(d);

  d.id = propRCold;
  d.name                              = "Cold resistance";
  d.nameShort                         = "RCold";
  d.msg[propMsgOnStartPlayer]         = "[RCold Player start]";
  d.msg[propMsgOnStartMonster]        = "[RCold Monster start]";
  d.msg[propMsgOnEndPlayer]           = "[RCold Player end]";
  d.msg[propMsgOnEndMonster]          = "[RCold Monster end]";
  d.msg[propMsgOnMorePlayer]          = "[RCold Player more]";
  d.msg[propMsgOnMoreMonster]         = "[RCold Monster more]";
  d.msg[propMsgOnResistPlayer]        = "";
  d.msg[propMsgOnResistMonster]       = "";
  d.isMakingMonsterAware              = false;
  d.allowDisplayTurns                 = true;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = false;
  d.alignment                         = propAlignmentGood;
  addPropData(d);

  d.id = propRPoison;
  d.name                              = "Poison resistance";
  d.nameShort                         = "RPoison";
  d.msg[propMsgOnStartPlayer]         = "[RPoison Player start]";
  d.msg[propMsgOnStartMonster]        = "[RPoison Monster start]";
  d.msg[propMsgOnEndPlayer]           = "[RPoison Player end]";
  d.msg[propMsgOnEndMonster]          = "[RPoison Monster end]";
  d.msg[propMsgOnMorePlayer]          = "[RPoison Player more]";
  d.msg[propMsgOnMoreMonster]         = "[RPoison Monster more]";
  d.msg[propMsgOnResistPlayer]        = "";
  d.msg[propMsgOnResistMonster]       = "";
  d.isMakingMonsterAware              = false;
  d.allowDisplayTurns                 = true;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = false;
  d.alignment                         = propAlignmentGood;
  addPropData(d);

  d.id = propRElectric;
  d.name                              = "Electric resistance";
  d.nameShort                         = "RElec";
  d.msg[propMsgOnStartPlayer]         = "[RElec Player start]";
  d.msg[propMsgOnStartMonster]        = "[RElec Monster start]";
  d.msg[propMsgOnEndPlayer]           = "[RElec Player end]";
  d.msg[propMsgOnEndMonster]          = "[RElec Monster end]";
  d.msg[propMsgOnMorePlayer]          = "[RElec Player more]";
  d.msg[propMsgOnMoreMonster]         = "[RElec Monster more]";
  d.msg[propMsgOnResistPlayer]        = "";
  d.msg[propMsgOnResistMonster]       = "";
  d.isMakingMonsterAware              = false;
  d.allowDisplayTurns                 = true;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = false;
  d.alignment                         = propAlignmentGood;
  addPropData(d);

  d.id = propRAcid;
  d.name                              = "Acid resistance";
  d.nameShort                         = "RAcid";
  d.msg[propMsgOnStartPlayer]         = "[RAcid Player start]";
  d.msg[propMsgOnStartMonster]        = "[RAcid Monster start]";
  d.msg[propMsgOnEndPlayer]           = "[RAcid Player end]";
  d.msg[propMsgOnEndMonster]          = "[RAcid Monster end]";
  d.msg[propMsgOnMorePlayer]          = "[RAcid Player more]";
  d.msg[propMsgOnMoreMonster]         = "[RAcid Monster more]";
  d.msg[propMsgOnResistPlayer]        = "";
  d.msg[propMsgOnResistMonster]       = "";
  d.isMakingMonsterAware              = false;
  d.allowDisplayTurns                 = true;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = false;
  d.alignment                         = propAlignmentGood;
  addPropData(d);

  d.id = propRSleep;
  d.name                              = "Sleep resistance";
  d.nameShort                         = "RSleep";
  d.msg[propMsgOnStartPlayer]         = "[RSleep Player start]";
  d.msg[propMsgOnStartMonster]        = "[RSleep Monster start]";
  d.msg[propMsgOnEndPlayer]           = "[RSleep Player end]";
  d.msg[propMsgOnEndMonster]          = "[RSleep Monster end]";
  d.msg[propMsgOnMorePlayer]          = "[RSleep Player more]";
  d.msg[propMsgOnMoreMonster]         = "[RSleep Monster more]";
  d.msg[propMsgOnResistPlayer]        = "";
  d.msg[propMsgOnResistMonster]       = "";
  d.isMakingMonsterAware              = false;
  d.allowDisplayTurns                 = true;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = false;
  d.alignment                         = propAlignmentGood;
  addPropData(d);

  d.id = propFreeAction;
  d.name                              = "Free action";
  d.nameShort                         = "FreeAction";
  d.msg[propMsgOnStartPlayer]         = "[FreeAction Player start]";
  d.msg[propMsgOnStartMonster]        = "[FreeAction Monster start]";
  d.msg[propMsgOnEndPlayer]           = "[FreeAction Player end]";
  d.msg[propMsgOnEndMonster]          = "[FreeAction Monster end]";
  d.msg[propMsgOnMorePlayer]          = "[FreeAction Player more]";
  d.msg[propMsgOnMoreMonster]         = "[FreeAction Monster more]";
  d.msg[propMsgOnResistPlayer]        = "";
  d.msg[propMsgOnResistMonster]       = "";
  d.isMakingMonsterAware              = false;
  d.allowDisplayTurns                 = true;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = false;
  d.alignment                         = propAlignmentGood;
  addPropData(d);

  d.id = propRFear;
  d.name                              = "Fear resistance";
  d.nameShort                         = "RFear";
  d.msg[propMsgOnStartPlayer]         = "[RFear Player start]";
  d.msg[propMsgOnStartMonster]        = "[RFear Monster start]";
  d.msg[propMsgOnEndPlayer]           = "[RFear Player end]";
  d.msg[propMsgOnEndMonster]          = "[RFear Monster end]";
  d.msg[propMsgOnMorePlayer]          = "[RFear Player more]";
  d.msg[propMsgOnMoreMonster]         = "[RFear Monster more]";
  d.msg[propMsgOnResistPlayer]        = "";
  d.msg[propMsgOnResistMonster]       = "";
  d.isMakingMonsterAware              = false;
  d.allowDisplayTurns                 = true;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = false;
  d.alignment                         = propAlignmentGood;
  addPropData(d);

  d.id = propRConfusion;
  d.name                              = "Confusion resistance";
  d.nameShort                         = "RConfusion";
  d.msg[propMsgOnStartPlayer]         = "[RConfusion Player start]";
  d.msg[propMsgOnStartMonster]        = "[RConfusion Monster start]";
  d.msg[propMsgOnEndPlayer]           = "[RConfusion Player end]";
  d.msg[propMsgOnEndMonster]          = "[RConfusion Monster end]";
  d.msg[propMsgOnMorePlayer]          = "[RConfusion Player more]";
  d.msg[propMsgOnMoreMonster]         = "[RConfusion Monster more]";
  d.msg[propMsgOnResistPlayer]        = "";
  d.msg[propMsgOnResistMonster]       = "";
  d.isMakingMonsterAware              = false;
  d.allowDisplayTurns                 = true;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = false;
  d.alignment                         = propAlignmentGood;
  addPropData(d);

  d.id = propLightSensitive;
  d.name                              = "Light sensitive";
  d.nameShort                         = "LgtSensitive";
  d.msg[propMsgOnStartPlayer]         = "[LgtSensitive Player start]";
  d.msg[propMsgOnStartMonster]        = "[LgtSensitive Monster start]";
  d.msg[propMsgOnEndPlayer]           = "[LgtSensitive Player end]";
  d.msg[propMsgOnEndMonster]          = "[LgtSensitive Monster end]";
  d.msg[propMsgOnMorePlayer]          = "[LgtSensitive Player more]";
  d.msg[propMsgOnMoreMonster]         = "[LgtSensitive Monster more]";
  d.msg[propMsgOnResistPlayer]        = "";
  d.msg[propMsgOnResistMonster]       = "";
  d.isMakingMonsterAware              = false;
  d.allowDisplayTurns                 = true;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = false;
  d.alignment                         = propAlignmentBad;
  addPropData(d);

  d.id = propBlind;
  d.name                              = "Blind";
  d.nameShort                         = "Blind";
  d.msg[propMsgOnStartPlayer]         = "I am blind!";
  d.msg[propMsgOnStartMonster]        = "is blinded.";
  d.msg[propMsgOnEndPlayer]           = "I can see again!";
  d.msg[propMsgOnEndMonster]          = "can see again.";
  d.msg[propMsgOnMorePlayer]          = "I am more blind.";
  d.msg[propMsgOnMoreMonster]         = "is more blind.";
  d.msg[propMsgOnResistPlayer]        = "I resist blindness.";
  d.msg[propMsgOnResistMonster]       = "resists blindness.";
  d.isMakingMonsterAware              = true;
  d.allowDisplayTurns                 = true;
  d.allowApplyMoreWhileActive         = true;
  d.alignment                         = propAlignmentBad;
  addPropData(d);

  d.id = propFainted;
  d.name                              = "Fainted";
  d.nameShort                         = "Fainted";
  d.msg[propMsgOnStartPlayer]         = "I faint!";
  d.msg[propMsgOnStartMonster]        = "faints.";
  d.msg[propMsgOnEndPlayer]           = "I am awake.";
  d.msg[propMsgOnEndMonster]          = "wakes up.";
  d.msg[propMsgOnMorePlayer]          = "I faint deeper.";
  d.msg[propMsgOnMoreMonster]         = "faints deeper.";
  d.msg[propMsgOnResistPlayer]        = "I resist fainting.";
  d.msg[propMsgOnResistMonster]       = "resists fainting.";
  d.isMakingMonsterAware              = true;
  d.allowDisplayTurns                 = true;
  d.allowApplyMoreWhileActive         = true;
  d.alignment                         = propAlignmentBad;
  addPropData(d);

  d.id = propBurning;
  d.name                              = "Burning";
  d.nameShort                         = "Burning";
  d.msg[propMsgOnStartPlayer]         = "I am Burning!";
  d.msg[propMsgOnStartMonster]        = "is burning.";
  d.msg[propMsgOnEndPlayer]           = "The flames are put out.";
  d.msg[propMsgOnEndMonster]          = "is no longer burning.";
  d.msg[propMsgOnMorePlayer]          = "I am further engulfed by flames!";
  d.msg[propMsgOnMoreMonster]         = "is further engulfed by flames.";
  d.msg[propMsgOnResistPlayer]        = "I resist burning.";
  d.msg[propMsgOnResistMonster]       = "resists burning.";
  d.isMakingMonsterAware              = true;
  d.allowDisplayTurns                 = true;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = true;
  d.alignment                         = propAlignmentBad;
  addPropData(d);

  d.id = propFrozen;
  d.name                              = "Frozen";
  d.nameShort                         = "Frozen";
  d.msg[propMsgOnStartPlayer]         = "[Frozen Player start]";
  d.msg[propMsgOnStartMonster]        = "[Frozen Monster start]";
  d.msg[propMsgOnEndPlayer]           = "[Frozen Player end]";
  d.msg[propMsgOnEndMonster]          = "[Frozen Monster end]";
  d.msg[propMsgOnMorePlayer]          = "[Frozen Player more]";
  d.msg[propMsgOnMoreMonster]         = "[Frozen Monster more]";
  d.msg[propMsgOnResistPlayer]        = "[Frozen Player resist]";
  d.msg[propMsgOnResistMonster]       = "[Frozen Monster resist]";
  d.isMakingMonsterAware              = true;
  d.allowDisplayTurns                 = true;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = true;
  d.alignment                         = propAlignmentBad;
  addPropData(d);

  d.id = propPoisoned;
  d.name                              = "Poisoned";
  d.nameShort                         = "Poisoned";
  d.msg[propMsgOnStartPlayer]         = "I am poisoned!";
  d.msg[propMsgOnStartMonster]        = "is poisoned.";
  d.msg[propMsgOnEndPlayer]           = "My body is cleansed from poisoning!";
  d.msg[propMsgOnEndMonster]          = "is cleansed from poisoning.";
  d.msg[propMsgOnMorePlayer]          = "I am more poisoned.";
  d.msg[propMsgOnMoreMonster]         = "is more poisoned.";
  d.msg[propMsgOnResistPlayer]        = "I resist poisoning.";
  d.msg[propMsgOnResistMonster]       = "resists poisoning.";
  d.isMakingMonsterAware              = true;
  d.allowDisplayTurns                 = true;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = false;
  d.alignment                         = propAlignmentBad;
  addPropData(d);

  d.id = propParalysed;
  d.name                              = "Paralyzed";
  d.nameShort                         = "Paralyzed";
  d.msg[propMsgOnStartPlayer]         = "I am paralyzed!";
  d.msg[propMsgOnStartMonster]        = "is paralyzed.";
  d.msg[propMsgOnEndPlayer]           = "I can move again!";
  d.msg[propMsgOnEndMonster]          = "can move again.";
  d.msg[propMsgOnMorePlayer]          = "";
  d.msg[propMsgOnMoreMonster]         = "";
  d.msg[propMsgOnResistPlayer]        = "I resist paralyzation.";
  d.msg[propMsgOnResistMonster]       = "resists paralyzation.";
  d.isMakingMonsterAware              = true;
  d.allowDisplayTurns                 = true;
  d.allowApplyMoreWhileActive         = false;
  d.updatePlayerVisualWhenStartOrEnd  = false;
  d.alignment                         = propAlignmentBad;
  addPropData(d);

  d.id = propTerrified;
  d.name                              = "Terrified";
  d.nameShort                         = "Terrified";
  d.msg[propMsgOnStartPlayer]         = "I am terrified!";
  d.msg[propMsgOnStartMonster]        = "looks terrified.";
  d.msg[propMsgOnEndPlayer]           = "I am no longer terrified!";
  d.msg[propMsgOnEndMonster]          = "is no longer terrified.";
  d.msg[propMsgOnMorePlayer]          = "I am more terrified.";
  d.msg[propMsgOnMoreMonster]         = "looks more terrified.";
  d.msg[propMsgOnResistPlayer]        = "I resist fear.";
  d.msg[propMsgOnResistMonster]       = "resists fear.";
  d.isMakingMonsterAware              = true;
  d.allowDisplayTurns                 = true;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = false;
  d.alignment                         = propAlignmentBad;
  addPropData(d);

  d.id = propConfused;
  d.name                              = "Confused";
  d.nameShort                         = "Confused";
  d.msg[propMsgOnStartPlayer]         = "I am confused!";
  d.msg[propMsgOnStartMonster]        = "looks confused.";
  d.msg[propMsgOnEndPlayer]           = "I am no longer confused";
  d.msg[propMsgOnEndMonster]          = "is no longer confused.";
  d.msg[propMsgOnMorePlayer]          = "I am more confused.";
  d.msg[propMsgOnMoreMonster]         = "looks more confused.";
  d.msg[propMsgOnResistPlayer]        = "I resist confusion.";
  d.msg[propMsgOnResistMonster]       = "resists confusion.";
  d.isMakingMonsterAware              = true;
  d.allowDisplayTurns                 = true;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = false;
  d.alignment                         = propAlignmentBad;
  addPropData(d);

  d.id = propStunned;
  d.name                              = "Stunned";
  d.nameShort                         = "Stunned";
  d.msg[propMsgOnStartPlayer]         = "[Stunned Player start]";
  d.msg[propMsgOnStartMonster]        = "[Stunned Monster start]";
  d.msg[propMsgOnEndPlayer]           = "[Stunned Player end]";
  d.msg[propMsgOnEndMonster]          = "[Stunned Monster end]";
  d.msg[propMsgOnMorePlayer]          = "[Stunned Player more]";
  d.msg[propMsgOnMoreMonster]         = "[Stunned Monster more]";
  d.msg[propMsgOnResistPlayer]        = "[Stunned Player resist]";
  d.msg[propMsgOnResistMonster]       = "[Stunned Monster resist]";
  d.isMakingMonsterAware              = true;
  d.allowDisplayTurns                 = true;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = false;
  d.alignment                         = propAlignmentBad;
  addPropData(d);

  d.id = propSlowed;
  d.name                              = "Slowed";
  d.nameShort                         = "Slowed";
  d.msg[propMsgOnStartPlayer]         = "Everything around me seems to speed up.";
  d.msg[propMsgOnStartMonster]        = "slows down.";
  d.msg[propMsgOnEndPlayer]           = "Everything around me seems to slow down.";
  d.msg[propMsgOnEndMonster]          = "speeds up.";
  d.msg[propMsgOnMorePlayer]          = "I am more slowed.";
  d.msg[propMsgOnMoreMonster]         = "slows down more.";
  d.msg[propMsgOnResistPlayer]        = "I resist slowness.";
  d.msg[propMsgOnResistMonster]       = "resists slowness.";
  d.isMakingMonsterAware              = true;
  d.allowDisplayTurns                 = true;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = false;
  d.alignment                         = propAlignmentBad;
  addPropData(d);

  d.id = propFlared;
  d.name                              = "";
  d.nameShort                         = "";
  d.msg[propMsgOnStartPlayer]         = "";
  d.msg[propMsgOnStartMonster]        = "is perforated by a flare!";
  d.msg[propMsgOnEndPlayer]           = "";
  d.msg[propMsgOnEndMonster]          = "";
  d.msg[propMsgOnMorePlayer]          = "";
  d.msg[propMsgOnMoreMonster]         = "is perforated by another flare!";
  d.msg[propMsgOnResistPlayer]        = "";
  d.msg[propMsgOnResistMonster]       = "";
  d.isMakingMonsterAware              = true;
  d.allowDisplayTurns                 = false;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = true;
  d.alignment                         = propAlignmentBad;
  addPropData(d);

  d.id = propNailed;
  d.name                              = "Nailed";
  d.msg[propMsgOnStartPlayer]         = "I am fastened by a spike!";
  d.msg[propMsgOnStartMonster]        = "is fastened by a spike.";
  d.msg[propMsgOnEndPlayer]           = "I tear free!";
  d.msg[propMsgOnEndMonster]          = "tears free!";
  d.msg[propMsgOnMorePlayer]          = "I am fastened by another spike!";
  d.msg[propMsgOnMoreMonster]         = "is fastened by another spike.";
  d.msg[propMsgOnResistPlayer]        = "";
  d.msg[propMsgOnResistMonster]       = "";
  d.isMakingMonsterAware              = true;
  d.allowDisplayTurns                 = false;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = false;
  d.alignment                         = propAlignmentBad;
  addPropData(d);

  d.id = propInfected;
  d.name                              = "Infected";
  d.nameShort                         = "Infected";
  d.msg[propMsgOnStartPlayer]         = "I am infected!";
  d.msg[propMsgOnStartMonster]        = "is infected.";
  d.msg[propMsgOnEndPlayer]           = "My infection is cured!";
  d.msg[propMsgOnEndMonster]          = "is no longer infected.";
  d.msg[propMsgOnMorePlayer]          = "I am more infected!";
  d.msg[propMsgOnMoreMonster]         = "is more infected.";
  d.msg[propMsgOnResistPlayer]        = "I resist infection.";
  d.msg[propMsgOnResistMonster]       = "resists infection.";
  d.isMakingMonsterAware              = true;
  d.allowDisplayTurns                 = true;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = false;
  d.alignment                         = propAlignmentBad;
  addPropData(d);

  d.id = propDiseased;
  d.name                              = "Diseased";
  d.nameShort                         = "Diseased";
  d.msg[propMsgOnStartPlayer]         = "I am diseased!";
  d.msg[propMsgOnStartMonster]        = "is diseased.";
  d.msg[propMsgOnEndPlayer]           = "My disease is cured!";
  d.msg[propMsgOnEndMonster]          = "is no longer diseased.";
  d.msg[propMsgOnMorePlayer]          = "I am more diseased.";
  d.msg[propMsgOnMoreMonster]         = "is more diseased.";
  d.msg[propMsgOnResistPlayer]        = "I resist disease.";
  d.msg[propMsgOnResistMonster]       = "resists disease.";
  d.isMakingMonsterAware              = true;
  d.allowDisplayTurns                 = true;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = false;
  d.alignment                         = propAlignmentBad;
  addPropData(d);

  d.id = propWeakened;
  d.name                              = "Weakened";
  d.nameShort                         = "Weakened";
  d.msg[propMsgOnStartPlayer]         = "I feel weaker.";
  d.msg[propMsgOnStartMonster]        = "looks weaker.";
  d.msg[propMsgOnEndPlayer]           = "I feel stronger!";
  d.msg[propMsgOnEndMonster]          = "looks stronger!";
  d.msg[propMsgOnMorePlayer]          = "I feel weaker.";
  d.msg[propMsgOnMoreMonster]         = "looks weaker.";
  d.msg[propMsgOnResistPlayer]        = "I resist weakness.";
  d.msg[propMsgOnResistMonster]       = "resists weakness.";
  d.isMakingMonsterAware              = true;
  d.allowDisplayTurns                 = true;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = false;
  d.alignment                         = propAlignmentBad;
  addPropData(d);

  d.id = propJuggernaut;
  d.name                              = "Juggernaut";
  d.nameShort                         = "Juggernaut";
  d.msg[propMsgOnStartPlayer]         = "[Juggernaut Player start]";
  d.msg[propMsgOnStartMonster]        = "[Juggernaut Monster start]";
  d.msg[propMsgOnEndPlayer]           = "[Juggernaut Player end]";
  d.msg[propMsgOnEndMonster]          = "[Juggernaut Monster end]";
  d.msg[propMsgOnMorePlayer]          = "[Juggernaut Player more]";
  d.msg[propMsgOnMoreMonster]         = "[Juggernaut Monster more]";
  d.msg[propMsgOnResistPlayer]        = "[Juggernaut Player resist]";
  d.msg[propMsgOnResistMonster]       = "[Juggernaut Monster resist]";
  d.isMakingMonsterAware              = false;
  d.allowDisplayTurns                 = true;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = false;
  d.alignment                         = propAlignmentGood;
  addPropData(d);

//  d.id = propHeroic;
//  d.name                              = "Heroic";
//  d.nameShort                         = "Heroic";
//  d.msg[propMsgOnStartPlayer]         = "[RConfusion Player start]";
//  d.msg[propMsgOnStartMonster]        = "[RConfusion Monster start]";
//  d.msg[propMsgOnEndPlayer]           = "[RConfusion Player end]";
//  d.msg[propMsgOnEndMonster]          = "[RConfusion Monster end]";
//  d.msg[propMsgOnMorePlayer]          = "[RConfusion Player more]";
//  d.msg[propMsgOnMoreMonster]         = "[RConfusion Monster more]";
//  d.msg[propMsgOnResistPlayer]        = "[RConfusion Player resist]";
//  d.msg[propMsgOnResistMonster]       = "[RConfusion Monster resist]";
//  d.isMakingMonsterAware              =
//  d.allowDisplayTurns                 =
//  d.allowApplyMoreWhileActive         =
//  d.updatePlayerVisualWhenStartOrEnd  =
//  d.alignment                         =
//  addPropData(d);

  d.id = propClairvoyant;
  d.name                              = "Clairvoyant";
  d.nameShort                         = "Clairvoyant";
  d.msg[propMsgOnStartPlayer]         = "I see far and beyond!";
  d.msg[propMsgOnStartMonster]        = "";
  d.msg[propMsgOnEndPlayer]           = "My sight is limited.";
  d.msg[propMsgOnEndMonster]          = "";
  d.msg[propMsgOnMorePlayer]          = "";
  d.msg[propMsgOnMoreMonster]         = "";
  d.msg[propMsgOnResistPlayer]        = "";
  d.msg[propMsgOnResistMonster]       = "";
  d.isMakingMonsterAware              = false;
  d.allowDisplayTurns                 = true;
  d.allowApplyMoreWhileActive         = true;
  d.alignment                         = propAlignmentGood;
  addPropData(d);

  d.id = propBlessed;
  d.name                              = "Blessed";
  d.nameShort                         = "Blessed";
  d.msg[propMsgOnStartPlayer]         = "I feel luckier.";
  d.msg[propMsgOnStartMonster]        = "";
  d.msg[propMsgOnEndPlayer]           = "I have normal luck.";
  d.msg[propMsgOnEndMonster]          = "";
  d.msg[propMsgOnMorePlayer]          = "I feel luckier.";
  d.msg[propMsgOnMoreMonster]         = "";
  d.msg[propMsgOnResistPlayer]        = "";
  d.msg[propMsgOnResistMonster]       = "";
  d.isMakingMonsterAware              = false;
  d.allowDisplayTurns                 = true;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = false;
  d.alignment                         = propAlignmentGood;
  addPropData(d);

  d.id = propCursed;
  d.name                              = "Cursed";
  d.nameShort                         = "Cursed";
  d.msg[propMsgOnStartPlayer]         = "I feel misfortunate.";
  d.msg[propMsgOnStartMonster]        = "";
  d.msg[propMsgOnEndPlayer]           = "I feel more fortunate.";
  d.msg[propMsgOnEndMonster]          = "";
  d.msg[propMsgOnMorePlayer]          = "I feel more misfortunate.";
  d.msg[propMsgOnMoreMonster]         = "";
  d.msg[propMsgOnResistPlayer]        = "I resist misfortune.";
  d.msg[propMsgOnResistMonster]       = "";
  d.isMakingMonsterAware              = false;
  d.allowDisplayTurns                 = true;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = false;
  d.alignment                         = propAlignmentBad;
  addPropData(d);

  d.id = propStill;
  d.name                              = "Still";
  d.nameShort                         = "Still";
  d.msg[propMsgOnStartPlayer]         = "";
  d.msg[propMsgOnStartMonster]        = "";
  d.msg[propMsgOnEndPlayer]           = "";
  d.msg[propMsgOnEndMonster]          = "";
  d.msg[propMsgOnMorePlayer]          = "";
  d.msg[propMsgOnMoreMonster]         = "";
  d.msg[propMsgOnResistPlayer]        = "";
  d.msg[propMsgOnResistMonster]       = "";
  d.isMakingMonsterAware              = false;
  d.allowDisplayTurns                 = false;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = false;
  d.alignment                         = propAlignmentGood;
  addPropData(d);

  d.id = propWound;
  d.name                              = "Wound";
  d.msg[propMsgOnStartPlayer]         = "I am wounded!";
  d.msg[propMsgOnStartMonster]        = "";
  d.msg[propMsgOnEndPlayer]           = "My wound is healed.";
  d.msg[propMsgOnEndMonster]          = "";
  d.msg[propMsgOnMorePlayer]          = "I am more wounded!";
  d.msg[propMsgOnMoreMonster]         = "";
  d.msg[propMsgOnResistPlayer]        = "I resist wounding!";
  d.msg[propMsgOnResistMonster]       = "";
  d.isMakingMonsterAware              = false;
  d.allowDisplayTurns                 = false;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = false;
  d.alignment                         = propAlignmentBad;
  addPropData(d);

  d.id = propWaiting;
  d.name                              = "";
  d.nameShort                         = "";
  d.msg[propMsgOnStartPlayer]         = "";
  d.msg[propMsgOnStartMonster]        = "";
  d.msg[propMsgOnEndPlayer]           = "";
  d.msg[propMsgOnEndMonster]          = "";
  d.msg[propMsgOnMorePlayer]          = "";
  d.msg[propMsgOnMoreMonster]         = "";
  d.msg[propMsgOnResistPlayer]        = "";
  d.msg[propMsgOnResistMonster]       = "";
  d.isMakingMonsterAware              = false;
  d.allowDisplayTurns                 = false;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = false;
  d.alignment                         = propAlignmentNeutral;
  addPropData(d);

  d.id = propDisabledAttack;
  d.name                              = "";
  d.nameShort                         = "";
  d.msg[propMsgOnStartPlayer]         = "";
  d.msg[propMsgOnStartMonster]        = "";
  d.msg[propMsgOnEndPlayer]           = "";
  d.msg[propMsgOnEndMonster]          = "";
  d.msg[propMsgOnMorePlayer]          = "";
  d.msg[propMsgOnMoreMonster]         = "";
  d.msg[propMsgOnResistPlayer]        = "";
  d.msg[propMsgOnResistMonster]       = "";
  d.isMakingMonsterAware              = false;
  d.allowDisplayTurns                 = false;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = false;
  d.alignment                         = propAlignmentNeutral;
  addPropData(d);

  d.id = propDisabledMelee;
  d.name                              = "";
  d.nameShort                         = "";
  d.msg[propMsgOnStartPlayer]         = "";
  d.msg[propMsgOnStartMonster]        = "";
  d.msg[propMsgOnEndPlayer]           = "";
  d.msg[propMsgOnEndMonster]          = "";
  d.msg[propMsgOnMorePlayer]          = "";
  d.msg[propMsgOnMoreMonster]         = "";
  d.msg[propMsgOnResistPlayer]        = "";
  d.msg[propMsgOnResistMonster]       = "";
  d.isMakingMonsterAware              = false;
  d.allowDisplayTurns                 = false;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = false;
  d.alignment                         = propAlignmentNeutral;
  addPropData(d);

  d.id = propDisabledRanged;
  d.name                              = "";
  d.nameShort                         = "";
  d.msg[propMsgOnStartPlayer]         = "";
  d.msg[propMsgOnStartMonster]        = "";
  d.msg[propMsgOnEndPlayer]           = "";
  d.msg[propMsgOnEndMonster]          = "";
  d.msg[propMsgOnMorePlayer]          = "";
  d.msg[propMsgOnMoreMonster]         = "";
  d.msg[propMsgOnResistPlayer]        = "";
  d.msg[propMsgOnResistMonster]       = "";
  d.isMakingMonsterAware              = false;
  d.allowDisplayTurns                 = false;
  d.allowApplyMoreWhileActive         = true;
  d.updatePlayerVisualWhenStartOrEnd  = false;
  d.alignment                         = propAlignmentNeutral;
  addPropData(d);
}

void PropDataHandler::addPropData(PropData& d) {
  dataList[d.id] = d;
}

PropHandler::PropHandler(Actor* owningActor, Engine* engine) :
  owningActor_(owningActor), eng(engine) {
  appliedProps_.resize(0);

  const ActorData* const actorData = owningActor->getData();

  for(unsigned int i = 0; i < endOfPropIds; i++) {
    if(actorData->intrProps[i]) {
      Prop* const prop = makePropFromId(PropId_t(i), propTurnsIndefinite);
      tryApplyProp(prop, true, true, true);
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
    case propFrozen:            return new PropFrozen(eng, turnsInit, NR_TURNS);
    case propFreeAction:        return new PropFreeAction(eng, turnsInit, NR_TURNS);
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
    case propJuggernaut:        return new PropJuggernaut(eng, turnsInit, NR_TURNS);
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
    case propRElectric:         return new PropRElectric(eng, turnsInit, NR_TURNS);
    case propRFear:             return new PropRFear(eng, turnsInit, NR_TURNS);
    case propRFire:             return new PropRFire(eng, turnsInit, NR_TURNS);
    case propRPoison:           return new PropRPoison(eng, turnsInit, NR_TURNS);
    case propRSleep:            return new PropRSleep(eng, turnsInit, NR_TURNS);
    case propLightSensitive:    return new PropLightSensitive(eng, turnsInit, NR_TURNS);
    case endOfPropIds: {
      tracer << "[WARNING] Illegal property id, in PropHandler::makePropFromId()" << endl;
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
    for(unsigned int i = 0; i < NR_ITEMS; i++) {
      Item* const item = items.at(i);
      const int NR_PROPS_FROM_ITEM = item->propsEnabledOnCarrier.size();
      for(unsigned int ii = 0; ii < NR_PROPS_FROM_ITEM; ii++) {
        propList.push_back(item->propsEnabledOnCarrier.at(ii));
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
                               const bool DISABLE_REDRAW) {
  prop->owningActor_ = owningActor_;

  const bool IS_OWNER_PLAYER = owningActor_ == eng->player;

  bool playerSeeOwner = false;

  if(DISABLE_REDRAW == false) {
    bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
    eng->mapTests->makeVisionBlockerArray(eng->player->pos, blockers);
    const bool playerSeeOwner =
      eng->player->checkIfSeeActor(*owningActor_, blockers);
  }

  if(FORCE_EFFECT == false) {
    //TODO Check for existing properties blocking the applied property

    if(false /*TODO*/) {
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
        if(IS_OWNER_PLAYER) {
          string msg = "";
          prop->getMsg(propMsgOnMorePlayer, msg);
          if(msg != "") {
            eng->log->addMessage(msg);
          }
        } else {
          if(playerSeeOwner) {
            string msg = "";
            prop->getMsg(propMsgOnMoreMonster, msg);
            if(msg != "") {
              eng->log->addMessage(owningActor_->getNameThe() + " " + msg);
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
  prop->onStart();

  if(DISABLE_REDRAW == false) {
    if(prop->updatePlayerVisualWhenStartOrEnd()) {
      prop->owningActor_->updateColor();
      eng->player->updateFov();
      eng->renderer->drawMapAndInterface();
    }
  }

  if(NO_MESSAGES == false) {
    if(IS_OWNER_PLAYER) {
      string msg = "";
      prop->getMsg(propMsgOnStartPlayer, msg);
      if(msg != "") {
        eng->log->addMessage(msg);
      }
    } else {
      if(playerSeeOwner) {
        string msg = "";
        prop->getMsg(propMsgOnStartMonster, msg);
        if(msg != "") {
          eng->log->addMessage(owningActor_->getNameThe() + " " + msg);
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

void PropHandler::endAppliedProp(
  const PropId_t id,
  const bool visionBlockingArray[MAP_X_CELLS][MAP_Y_CELLS],
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
    return;
  }

  appliedProps_.erase(appliedProps_.begin() + index);

  if(RUN_PROP_END_EFFECTS) {
    prop->onEnd();

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
      eng->log->addMessage(msg, clrWhite);
    } else {
      if(eng->player->checkIfSeeActor(*owningActor_, visionBlockingArray)) {
        string msg = "";
        prop->getMsg(propMsgOnEndMonster, msg);
        if(msg != "") {
          eng->log->addMessage(owningActor_->getNameThe() + " " + msg);
        }
      }
    }
  }

  delete prop;
}

void PropHandler::newTurnAllProps(
  const bool visionBlockingArray[MAP_X_CELLS][MAP_Y_CELLS]) {

  for(unsigned int i = 0; i < appliedProps_.size();) {
    Prop* const prop = appliedProps_.at(i);
    tracer << "Prop name: " << prop->getNameShort() << endl;

    if(owningActor_ != eng->player) {
      if(prop->isMakingMonsterAware()) {
        dynamic_cast<Monster*>(owningActor_)->playerAwarenessCounter =
          owningActor_->getData()->nrTurnsAwarePlayer;
      }
    }

    if(prop->isFinnished()) {
      endAppliedProp(prop->getId(), visionBlockingArray);
    } else {
      if(prop->turnsLeft_ > 0) {
        prop->turnsLeft_--;
      }
      prop->onNewTurn();
      i++;
    }
  }

  vector<Prop*> invProps;
  getPropsFromSource(invProps, propSrcInv);
  const unsigned int NR_INV_PROPS = invProps.size();
  for(unsigned int i = 0; i < NR_INV_PROPS;) {
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

void PropHandler::changeMovePos(const Pos& actorPos, Pos& movePos) {
  vector<Prop*> propList;
  getPropsFromSource(propList, propSrcAppliedAndInv);
  const unsigned int NR_PROPS = propList.size();
  for(unsigned int i = 0; i < NR_PROPS; i++) {
    propList.at(i)->changeMovePos(actorPos, movePos);
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
  eng->mapTests->makeVisionBlockerArray(eng->player->pos, visionBlockers);
  owningActor_->getPropHandler()->endAppliedProp(propCursed, visionBlockers);
}

void PropCursed::onStart() {
  bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeVisionBlockerArray(eng->player->pos, visionBlockers);
  owningActor_->getPropHandler()->endAppliedProp(propBlessed, visionBlockers);
}

void PropInfected::onNewTurn() {
  if(eng->dice.percentile() <= 4) {
    owningActor_->getPropHandler()->tryApplyProp(
      new PropDiseased(eng, propTurnsStandard));
    bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];

    eng->mapTests->makeVisionBlockerArray(owningActor_->pos, visionBlockers);
    owningActor_->getPropHandler()->endAppliedProp(
      propInfected, visionBlockers);
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
      eng->log->addMessage(
        "I am suffering from the poison!",
        clrMessageBad, messageInterrupt_force);
    } else {
      if(eng->player->checkIfSeeActor(*owningActor_, NULL)) {
        eng->log->addMessage(
          owningActor_->getNameThe() + " suffers from poisoning!");
      }
    }

    owningActor_->hit(1, dmgType_pure);
  }
}

bool PropTerrified::allowAttackMelee(const bool ALLOW_MESSAGE_WHEN_FALSE) {
  if(ALLOW_MESSAGE_WHEN_FALSE) {
    if(owningActor_ == eng->player) {
      eng->log->addMessage(
        "I am too terrified to engage in close combat!");
    }
  }
  return false;
}

bool PropTerrified::allowAttackRanged(const bool ALLOW_MESSAGE_WHEN_FALSE) {
  (void)ALLOW_MESSAGE_WHEN_FALSE;
  return true;
}

void PropWound::healOneWound() {
  tracer << "StatusWound: Nr wounds before healing one: " << nrWounds_ << endl;
  if(--nrWounds_ > 0) {
    eng->log->addMessage("A wound is healed!", clrMessageGood);
  } else {
    eng->log->addMessage("All my wounds are healed!", clrMessageGood);
    bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
    eng->mapTests->makeVisionBlockerArray(
      eng->player->pos, visionBlockers);
    owningActor_->getPropHandler()->endAppliedProp(
      propWound, visionBlockers);
  }
}

void PropWound::onMore() {
  nrWounds_++;

  if(nrWounds_ >= 5) {
    if(owningActor_ == eng->player) {
      eng->log->addMessage("I die from my wounds!");
    }
    owningActor_->die(false, false, true);
  }
}

void PropNailed::changeMovePos(const Pos& actorPos, Pos& movePos) {
  if(actorPos != movePos) {

    if(owningActor_ == eng->player) {
      eng->log->addMessage(
        "I struggle to tear out the spike!", clrMessageBad);
    } else {
      if(eng->player->checkIfSeeActor(*owningActor_, NULL)) {
        eng->log->addMessage(
          owningActor_->getNameThe() +  " struggles in pain!", clrMessageGood);
      }
    }

    owningActor_->hit(eng->dice(1, 3), dmgType_physical);

    if(owningActor_->deadState == actorDeadState_alive) {

      //TODO reimplement something affecting chance of success?

      if(eng->dice.fraction(1, 4)) {
        nrSpikes_--;
        if(nrSpikes_ > 0) {
          if(owningActor_ == eng->player) {
            eng->log->addMessage("I rip out a spike from my flesh!");
          } else {
            if(eng->player->checkIfSeeActor(*owningActor_, NULL)) {
              eng->log->addMessage(
                owningActor_->getNameThe() + " tears out a spike!");
            }
          }
        }
      }
    }

    movePos = actorPos;
  }
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

void PropConfused::changeMovePos(const Pos& actorPos, Pos& movePos) {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeMoveBlockerArray(owningActor_, blockers);

  if(actorPos != movePos) {
    const int CHANCE_TO_MOVE_WRONG = 40;
    if(eng->dice.percentile() < CHANCE_TO_MOVE_WRONG) {
      int triesLeft = 100;
      while(triesLeft != 0) {
        //-1 to 1 for x and y
        const Pos delta(eng->dice(1, 3) - 2, eng->dice(1, 3) - 2);
        if(delta.x != 0 || delta.y != 0) {
          const Pos c = actorPos + delta;
          if(blockers[c.x][c.y] == false) {
            movePos = actorPos + delta;
          }
        }
        triesLeft--;
      }
    }
  }
}

void PropBurning::onStart() {
  owningActor_->addLight(eng->map->light);
}

void PropBurning::onNewTurn() {
  if(owningActor_ == eng->player) {
    eng->log->addMessage("AAAARGH IT BURNS!!!", clrRedLgt);
  }
  owningActor_->hit(eng->dice(1, 2), dmgType_fire);
}

void PropBlind::onStart() {
  bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeVisionBlockerArray(
    eng->player->pos, visionBlockers);
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
      eng->log->addMessage("The lit Dynamite stick falls from my hands!");
      Feature* const f = eng->map->featuresStatic[playerPos.x][playerPos.y];
      if(f->isBottomless() == false) {
        eng->featureFactory->spawnFeatureAt(
          feature_litDynamite, playerPos,
          new DynamiteSpawnData(DYNAMITE_FUSE));
      }
    }
    if(FLARE_FUSE > 0) {
      player->flareFuseTurns = -1;
      player->updateColor();
      eng->log->addMessage("The lit Flare falls from my hands.");
      Feature* const f = eng->map->featuresStatic[playerPos.x][playerPos.y];
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
      eng->log->addMessage("The lit Molotov Cocktail falls from my hands!");
      eng->explosionMaker->runExplosion(
        player->pos, false, new PropBurning(eng, propTurnsStandard));
    }
  }
}

bool PropFreeAction::tryResistOherProp(const PropId_t otherId) {
  //TODO Add resistance checks for this property
  return false;
}

void PropFainted::onStart() {
  bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeVisionBlockerArray(eng->player->pos, visionBlockers);
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
  eng->mapTests->makeVisionBlockerArray(eng->player->pos, visionBlockers);
  owningActor_->getPropHandler()->endAppliedProp(propBlind, visionBlockers);
}

void PropFlared::onNewTurn() {
  owningActor_->hit(1, dmgType_fire);

  if(turnsLeft_ == 0) {
    bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
    eng->mapTests->makeVisionBlockerArray(
      eng->player->pos, visionBlockers, 99999);
    owningActor_->getPropHandler()->tryApplyProp(
      new PropBurning(eng, propTurnsStandard));
    owningActor_->getPropHandler()->endAppliedProp(
      propFlared, visionBlockers);
  }
}

bool PropRAcid::tryResistOherProp(const PropId_t otherId) {
  //TODO Add resistance checks for this property
  return false;
}

bool PropRCold::tryResistOherProp(const PropId_t otherId) {
  //TODO Add resistance checks for this property
  return false;
}

bool PropRConfusion::tryResistOherProp(const PropId_t otherId) {
  //TODO Add resistance checks for this property
  return false;
}

bool PropRElectric::tryResistOherProp(const PropId_t otherId) {
  //TODO Add resistance checks for this property
  return false;
}

bool PropRFear::tryResistOherProp(const PropId_t otherId) {
  //TODO Add resistance checks for this property
  return false;
}

bool PropRFire::tryResistOherProp(const PropId_t otherId) {
  //TODO Add resistance checks for this property
  return false;
}

bool PropRPoison::tryResistOherProp(const PropId_t otherId) {
  //TODO Add resistance checks for this property
  return false;
}

bool PropRSleep::tryResistOherProp(const PropId_t otherId) {
  //TODO Add resistance checks for this property
  return false;
}
