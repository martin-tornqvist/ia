#include "Properties.h"

#include <algorithm>
#include <assert.h>

#include "Init.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Postmortem.h"
#include "Render.h"
#include "ActorMon.h"
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
#include "Item.h"

using namespace std;

namespace PropData
{

PropDataT data[size_t(PropId::END)];

namespace
{

void addPropData(PropDataT& d)
{
    data[int(d.id)] = d;
    PropDataT blank;
    d = blank;
}

void initDataList()
{
    PropDataT d;

    d.id = PropId::rPhys;
    d.stdRndTurns = Range(40, 60);
    d.name = "Physical Resistance";
    d.nameShort = "rPhys";
    d.msg[propMsgOnStartPlayer] = "I feel resistant to physical harm.";
    d.msg[propMsgOnStartMon] = "is resistant to physical harm.";
    d.msg[propMsgOnEndPlayer] = "I feel vulnerable to physical harm.";
    d.msg[propMsgOnEndMon] = "is vulnerable to physical harm.";
    d.isMakingMonAware = false;
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentGood;
    addPropData(d);

    d.id = PropId::rFire;
    d.stdRndTurns = Range(40, 60);
    d.name = "Fire resistance";
    d.nameShort = "rFire";
    d.msg[propMsgOnStartPlayer] = "I feel resistant to fire.";
    d.msg[propMsgOnStartMon] = "is resistant to fire.";
    d.msg[propMsgOnEndPlayer] = "I feel vulnerable to fire.";
    d.msg[propMsgOnEndMon] = "is vulnerable to fire.";
    d.isMakingMonAware = false;
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentGood;
    addPropData(d);

    d.id = PropId::rCold;
    d.stdRndTurns = Range(40, 60);
    d.name = "Cold resistance";
    d.nameShort = "rCold";
    d.msg[propMsgOnStartPlayer] = "I feel resistant to cold.";
    d.msg[propMsgOnStartMon] = "is resistant to cold.";
    d.msg[propMsgOnEndPlayer] = "I feel vulnerable to cold.";
    d.msg[propMsgOnEndMon] = "is vulnerable to cold.";
    d.isMakingMonAware = false;
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentGood;
    addPropData(d);

    d.id = PropId::rPoison;
    d.stdRndTurns = Range(40, 60);
    d.name = "Poison resistance";
    d.nameShort = "rPoison";
    d.msg[propMsgOnStartPlayer] = "I feel resistant to poison.";
    d.msg[propMsgOnStartMon] = "is resistant to poison.";
    d.msg[propMsgOnEndPlayer] = "I feel vulnerable to poison.";
    d.msg[propMsgOnEndMon] = "is vulnerable to poison.";
    d.isMakingMonAware = false;
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentGood;
    addPropData(d);

    d.id = PropId::rElec;
    d.stdRndTurns = Range(40, 60);
    d.name = "Electric resistance";
    d.nameShort = "rElec";
    d.msg[propMsgOnStartPlayer] = "I feel resistant to electricity.";
    d.msg[propMsgOnStartMon] = "is resistant to electricity.";
    d.msg[propMsgOnEndPlayer] = "I feel vulnerable to electricity.";
    d.msg[propMsgOnEndMon] = "is vulnerable to electricity.";
    d.isMakingMonAware = false;
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentGood;
    addPropData(d);

    d.id = PropId::rAcid;
    d.stdRndTurns = Range(40, 60);
    d.name = "Acid resistance";
    d.nameShort = "rAcid";
    d.msg[propMsgOnStartPlayer] = "I feel resistant to acid.";
    d.msg[propMsgOnStartMon] = "is resistant to acid.";
    d.msg[propMsgOnEndPlayer] = "I feel vulnerable to acid.";
    d.msg[propMsgOnEndMon] = "is vulnerable to acid.";
    d.isMakingMonAware = false;
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentGood;
    addPropData(d);

    d.id = PropId::rSleep;
    d.stdRndTurns = Range(40, 60);
    d.name = "Sleep resistance";
    d.nameShort = "rSleep";
    d.msg[propMsgOnStartPlayer] = "I feel resistant to sleep.";
    d.msg[propMsgOnStartMon] = "is resistant to sleep.";
    d.msg[propMsgOnEndPlayer] = "I feel vulnerable to sleep.";
    d.msg[propMsgOnEndMon] = "is vulnerable to sleep.";
    d.isMakingMonAware = false;
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentGood;
    addPropData(d);

    d.id = PropId::rFear;
    d.stdRndTurns = Range(40, 60);
    d.name = "Fear resistance";
    d.nameShort = "rFear";
    d.msg[propMsgOnStartPlayer] = "I feel resistant to fear.";
    d.msg[propMsgOnStartMon] = "is resistant to fear.";
    d.msg[propMsgOnEndPlayer] = "I feel vulnerable to fear.";
    d.msg[propMsgOnEndMon] = "is vulnerable to fear.";
    d.isMakingMonAware = false;
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentGood;
    addPropData(d);

    d.id = PropId::rConf;
    d.stdRndTurns = Range(40, 50);
    d.name = "Confusion resistance";
    d.nameShort = "rConf";
    d.msg[propMsgOnStartPlayer] = "I feel resistant to confusion.";
    d.msg[propMsgOnStartMon] = "is resistant to confusion.";
    d.msg[propMsgOnEndPlayer] = "I feel vulnerable to confusion.";
    d.msg[propMsgOnEndMon] = "is vulnerable to confusion.";
    d.isMakingMonAware = false;
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentGood;
    addPropData(d);

    d.id = PropId::rDisease;
    d.stdRndTurns = Range(40, 60);
    d.name = "Disease resistance";
    d.nameShort = "rDisease";
    d.msg[propMsgOnStartPlayer] = "I feel resistant to disease.";
    d.msg[propMsgOnStartMon] = "is resistant to disease.";
    d.msg[propMsgOnEndPlayer] = "I feel vulnerable to disease.";
    d.msg[propMsgOnEndMon] = "is vulnerable to disease.";
    d.isMakingMonAware = false;
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentGood;
    addPropData(d);

    d.id = PropId::rBreath;
    d.stdRndTurns = Range(40, 60);
    d.name = "Breath resistance";
    d.nameShort = "rBreath";
    d.msg[propMsgOnStartPlayer] = "I can breath without harm.";
    d.msg[propMsgOnStartMon] = "can breath without harm.";
    d.isMakingMonAware = false;
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentGood;
    addPropData(d);

    d.id = PropId::lgtSens;
    d.stdRndTurns = Range(40, 60);
    d.name = "Light sensitive";
    d.nameShort = "LgtSensitive";
    d.msg[propMsgOnStartPlayer] = "I feel vulnerable to light!";
    d.msg[propMsgOnStartMon] = "is vulnerable to light.";
    d.msg[propMsgOnEndPlayer] = "I no longer feel vulnerable to light.";
    d.msg[propMsgOnEndMon] = "no longer is vulnerable to light.";
    d.msg[propMsgOnResPlayer] = "";
    d.msg[propMsgOnResMon] = "";
    d.isMakingMonAware = false;
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.alignment = propAlignmentBad;
    addPropData(d);

    d.id = PropId::blind;
    d.stdRndTurns = Range(20, 30);
    d.name = "Blind";
    d.nameShort = "Blind";
    d.msg[propMsgOnStartPlayer] = "I am blinded!";
    d.msg[propMsgOnStartMon] = "is blinded.";
    d.msg[propMsgOnEndPlayer] = "I can see again!";
    d.msg[propMsgOnEndMon] = "can see again.";
    d.msg[propMsgOnResPlayer] = "I resist blindness.";
    d.msg[propMsgOnResMon] = "resists blindness.";
    d.isMakingMonAware = true;
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.isEndedByMagicHealing = true;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentBad;
    addPropData(d);

    d.id = PropId::fainted;
    d.stdRndTurns = Range(100, 200);
    d.name = "Fainted";
    d.nameShort = "Fainted";
    d.msg[propMsgOnStartPlayer] = "I faint!";
    d.msg[propMsgOnStartMon] = "faints.";
    d.msg[propMsgOnEndPlayer] = "I am awake.";
    d.msg[propMsgOnEndMon] = "wakes up.";
    d.msg[propMsgOnResPlayer] = "I resist fainting.";
    d.msg[propMsgOnResMon] = "resists fainting.";
    d.isMakingMonAware = true;
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentBad;
    addPropData(d);

    d.id = PropId::burning;
    d.stdRndTurns = Range(5, 10);
    d.name = "Burning";
    d.nameShort = "Burning";
    d.msg[propMsgOnStartPlayer] = "I am Burning!";
    d.msg[propMsgOnStartMon] = "is burning.";
    d.msg[propMsgOnEndPlayer] = "The flames are put out.";
    d.msg[propMsgOnEndMon] = "is no longer burning.";
    d.msg[propMsgOnResPlayer] = "I resist burning.";
    d.msg[propMsgOnResMon] = "resists burning.";
    d.isMakingMonAware = true;
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = true;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentBad;
    addPropData(d);

    d.id = PropId::poisoned;
    d.stdRndTurns = Range(30, 60);
    d.name = "Poisoned";
    d.nameShort = "Poisoned";
    d.msg[propMsgOnStartPlayer] = "I am poisoned!";
    d.msg[propMsgOnStartMon] = "is poisoned.";
    d.msg[propMsgOnEndPlayer] = "My body is cleansed from poisoning!";
    d.msg[propMsgOnEndMon] = "is cleansed from poisoning.";
    d.msg[propMsgOnResPlayer] = "I resist poisoning.";
    d.msg[propMsgOnResMon] = "resists poisoning.";
    d.isMakingMonAware = true;
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.isEndedByMagicHealing = true;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentBad;
    addPropData(d);

    d.id = PropId::paralyzed;
    d.stdRndTurns = Range(7, 9);
    d.name = "Paralyzed";
    d.nameShort = "Paralyzed";
    d.msg[propMsgOnStartPlayer] = "I am paralyzed!";
    d.msg[propMsgOnStartMon] = "is paralyzed.";
    d.msg[propMsgOnEndPlayer] = "I can move again!";
    d.msg[propMsgOnEndMon] = "can move again.";
    d.msg[propMsgOnResPlayer] = "I resist paralyzation.";
    d.msg[propMsgOnResMon] = "resists paralyzation.";
    d.isMakingMonAware = true;
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = false;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.isEndedByMagicHealing = true;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentBad;
    addPropData(d);

    d.id = PropId::terrified;
    d.stdRndTurns = Range(5, 12);
    d.name = "Terrified";
    d.nameShort = "Terrified";
    d.msg[propMsgOnStartPlayer] = "I am terrified!";
    d.msg[propMsgOnStartMon] = "looks terrified.";
    d.msg[propMsgOnEndPlayer] = "I am no longer terrified!";
    d.msg[propMsgOnEndMon] = "is no longer terrified.";
    d.msg[propMsgOnResPlayer] = "I resist fear.";
    d.msg[propMsgOnResMon] = "resists fear.";
    d.isMakingMonAware = true;
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentBad;
    addPropData(d);

    d.id = PropId::confused;
    d.stdRndTurns = Range(80, 120);
    d.name = "Confused";
    d.nameShort = "Confused";
    d.msg[propMsgOnStartPlayer] = "I am confused!";
    d.msg[propMsgOnStartMon] = "looks confused.";
    d.msg[propMsgOnEndPlayer] = "I am no longer confused.";
    d.msg[propMsgOnEndMon] = "is no longer confused.";
    d.msg[propMsgOnResPlayer] = "I resist confusion.";
    d.msg[propMsgOnResMon] = "resists confusion.";
    d.isMakingMonAware = true;
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentBad;
    addPropData(d);

    d.id = PropId::stunned;
    d.stdRndTurns = Range(5, 9);
    d.name = "Stunned";
    d.nameShort = "Stunned";
    d.msg[propMsgOnStartPlayer] = "I am stunned!";
    d.msg[propMsgOnStartMon] = "is stunned.";
    d.msg[propMsgOnEndPlayer] = "I am no longer stunned.";
    d.msg[propMsgOnEndMon] = "is no longer stunned.";
    d.msg[propMsgOnResPlayer] = "I resist stunning.";
    d.msg[propMsgOnResMon] = "resists stunning.";
    d.isMakingMonAware = true;
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.isEndedByMagicHealing = true;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentBad;
    addPropData(d);

    d.id = PropId::slowed;
    d.stdRndTurns = Range(9, 12);
    d.name = "Slowed";
    d.nameShort = "Slowed";
    d.msg[propMsgOnStartPlayer] = "Everything around me seems to speed up.";
    d.msg[propMsgOnStartMon] = "slows down.";
    d.msg[propMsgOnEndPlayer] = "Everything around me seems to slow down.";
    d.msg[propMsgOnEndMon] = "speeds up.";
    d.msg[propMsgOnResPlayer] = "I resist slowness.";
    d.msg[propMsgOnResMon] = "resists slowness.";
    d.isMakingMonAware = true;
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentBad;
    addPropData(d);

    d.id = PropId::hasted;
    d.stdRndTurns = Range(9, 12);
    d.name = "Hasted";
    d.nameShort = "Hasted";
    d.msg[propMsgOnStartPlayer] = "Everything around me seems to slow down.";
    d.msg[propMsgOnStartMon] = "speeds up.";
    d.msg[propMsgOnEndPlayer] = "Everything around me seems to speed up.";
    d.msg[propMsgOnEndMon] = "slows down.";
    d.isMakingMonAware = true;
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentGood;
    addPropData(d);

    d.id = PropId::flared;
    d.stdRndTurns = Range(3, 4);
    d.msg[propMsgOnStartMon] = "is perforated by a flare!";
    d.isMakingMonAware = true;
    d.allowDisplayTurns = false;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = true;
    d.alignment = propAlignmentBad;
    addPropData(d);

    d.id = PropId::nailed;
    d.name = "Nailed";
    d.msg[propMsgOnStartPlayer] = "I am fastened by a spike!";
    d.msg[propMsgOnStartMon] = "is fastened by a spike.";
    d.msg[propMsgOnEndPlayer] = "I tear free!";
    d.msg[propMsgOnEndMon] = "tears free!";
    d.isMakingMonAware = true;
    d.allowDisplayTurns = false;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.alignment = propAlignmentBad;
    addPropData(d);

    d.id = PropId::warlockCharged;
    d.stdRndTurns = Range(1, 1);
    d.name = "Charged";
    d.nameShort = "Charged";
    d.msg[propMsgOnStartPlayer] = "I am burning with power!";
    d.isMakingMonAware = false;
    d.allowDisplayTurns = false;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.alignment = propAlignmentGood;
    addPropData(d);

    d.id = PropId::infected;
    d.stdRndTurns = Range(400, 700);
    d.name = "Infected";
    d.nameShort = "Infected";
    d.msg[propMsgOnStartPlayer] = "I am infected!";
    d.msg[propMsgOnStartMon] = "is infected.";
    d.msg[propMsgOnEndPlayer] = "My infection is cured!";
    d.msg[propMsgOnEndMon] = "is no longer infected.";
    d.msg[propMsgOnResPlayer] = "";
    d.msg[propMsgOnResMon] = "";
    d.isMakingMonAware = true;
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.isEndedByMagicHealing = true;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentBad;
    addPropData(d);

    d.id = PropId::diseased;
    d.stdRndTurns = Range(400, 600);
    d.name = "Diseased";
    d.nameShort = "Diseased";
    d.msg[propMsgOnStartPlayer] = "I am diseased!";
    d.msg[propMsgOnStartMon] = "is diseased.";
    d.msg[propMsgOnEndPlayer] = "My disease is cured!";
    d.msg[propMsgOnEndMon] = "is no longer diseased.";
    d.msg[propMsgOnResPlayer] = "I resist disease.";
    d.msg[propMsgOnResMon] = "resists disease.";
    d.isMakingMonAware = true;
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.isEndedByMagicHealing = true;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentBad;
    addPropData(d);

    d.id = PropId::weakened;
    d.stdRndTurns = Range(50, 100);
    d.name = "Weakened";
    d.nameShort = "Weakened";
    d.msg[propMsgOnStartPlayer] = "I feel weaker.";
    d.msg[propMsgOnStartMon] = "looks weaker.";
    d.msg[propMsgOnEndPlayer] = "I feel stronger!";
    d.msg[propMsgOnEndMon] = "looks stronger!";
    d.msg[propMsgOnResPlayer] = "I resist weakness.";
    d.msg[propMsgOnResMon] = "resists weakness.";
    d.isMakingMonAware = true;
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.isEndedByMagicHealing = true;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentBad;
    addPropData(d);

    d.id = PropId::frenzied;
    d.stdRndTurns = Range(30, 60);
    d.name = "Frenzied";
    d.nameShort = "Frenzied";
    d.msg[propMsgOnStartPlayer] = "I feel ferocious!";
    d.msg[propMsgOnStartMon] = "Looks ferocious!";
    d.msg[propMsgOnEndPlayer] = "I feel calmer.";
    d.msg[propMsgOnEndMon] = "Looks calmer.";
    d.msg[propMsgOnResPlayer] = "";
    d.msg[propMsgOnResMon] = "";
    d.isMakingMonAware = false;
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentNeutral;
    addPropData(d);

    d.id = PropId::blessed;
    d.stdRndTurns = Range(400, 600);
    d.name = "Blessed";
    d.nameShort = "Blessed";
    d.msg[propMsgOnStartPlayer] = "I feel luckier.";
    d.msg[propMsgOnEndPlayer] = "I have normal luck.";
    d.isMakingMonAware = false;
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentGood;
    addPropData(d);

    d.id = PropId::cursed;
    d.stdRndTurns = Range(400, 600);
    d.name = "Cursed";
    d.nameShort = "Cursed";
    d.msg[propMsgOnStartPlayer] = "I feel misfortunate.";
    d.msg[propMsgOnEndPlayer] = "I feel more fortunate.";
    d.msg[propMsgOnResPlayer] = "I resist misfortune.";
    d.isMakingMonAware = false;
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentBad;
    addPropData(d);

    d.id = PropId::radiant;
    d.stdRndTurns = Range(40, 60);
    d.name = "Radiant";
    d.nameShort = "Radiant";
    d.msg[propMsgOnStartPlayer] = "A bright light shines around me.";
    d.msg[propMsgOnEndPlayer] = "It suddenly seems darker.";
    d.isMakingMonAware = false;
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = true;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentNeutral;
    addPropData(d);

    d.id = PropId::teleCtrl;
    d.stdRndTurns = Range(40, 60);
    d.name = "Teleport Control";
    d.nameShort = "TeleCtrl";
    d.msg[propMsgOnStartPlayer] = "I feel in control.";
    d.msg[propMsgOnEndPlayer] = "I feel less in control.";
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.isEndedByMagicHealing = false;
    d.allowTestOnBot = false;
    d.alignment = propAlignmentGood;
    addPropData(d);

    d.id = PropId::spellReflect;
    d.stdRndTurns = Range(40, 60);
    d.name = "Spell Reflection";
    d.nameShort = "SpellRefl";
    d.msg[propMsgOnStartPlayer] = "Spells reflect off me!";
    d.msg[propMsgOnEndPlayer] = "I am vulnerable to spells.";
    d.allowDisplayTurns = true;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.isEndedByMagicHealing = false;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentGood;
    addPropData(d);

    d.id = PropId::aiming;
    d.stdRndTurns = Range(1, 1);
    d.name = "Aiming";
    d.nameShort = "Aiming";
    d.isMakingMonAware = false;
    d.allowDisplayTurns = false;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.alignment = propAlignmentGood;
    addPropData(d);

    d.id = PropId::strangled;
    d.stdRndTurns = Range(1, 1);
    d.name = "Strangled";
    d.nameShort = "Strangled";
    d.isMakingMonAware = true;
    d.allowDisplayTurns = false;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.alignment = propAlignmentBad;
    addPropData(d);

    d.id = PropId::conflict;
    d.stdRndTurns = Range(10, 20);
    d.msg[propMsgOnStartMon] = "Looks conflicted.";
    d.msg[propMsgOnEndMon] = "Looks more determined.";
    d.isMakingMonAware = true;
    d.allowDisplayTurns = false;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.alignment = propAlignmentBad;
    addPropData(d);

    d.id = PropId::possByZuul;
    d.isMakingMonAware = false;
    d.allowDisplayTurns = false;
    d.allowApplyMoreWhileActive = false;
    d.updatePlayerVisualWhenStartOrEnd = true;
    d.isEndedByMagicHealing = false;
    d.allowTestOnBot = true;
    d.alignment = propAlignmentNeutral;
    addPropData(d);

    d.id = PropId::flying;
    d.isMakingMonAware = false;
    d.allowDisplayTurns = false;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.isEndedByMagicHealing = false;
    d.allowTestOnBot = false;
    d.alignment = propAlignmentNeutral;
    addPropData(d);

    d.id = PropId::ethereal;
    d.isMakingMonAware = false;
    d.allowDisplayTurns = false;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.isEndedByMagicHealing = false;
    d.allowTestOnBot = false;
    d.alignment = propAlignmentNeutral;
    addPropData(d);

    d.id = PropId::ooze;
    d.isMakingMonAware = false;
    d.allowDisplayTurns = false;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.isEndedByMagicHealing = false;
    d.allowTestOnBot = false;
    d.alignment = propAlignmentNeutral;
    addPropData(d);

    d.id = PropId::burrowing;
    d.isMakingMonAware = false;
    d.allowDisplayTurns = false;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.isEndedByMagicHealing = false;
    d.allowTestOnBot = false;
    d.alignment = propAlignmentNeutral;
    addPropData(d);

    d.id = PropId::waiting;
    d.stdRndTurns = Range(1, 1);
    d.isMakingMonAware = false;
    d.allowDisplayTurns = false;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.alignment = propAlignmentNeutral;
    addPropData(d);

    d.id = PropId::disabledAttack;
    d.stdRndTurns = Range(1, 1);
    d.isMakingMonAware = false;
    d.allowDisplayTurns = false;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.alignment = propAlignmentNeutral;
    addPropData(d);

    d.id = PropId::disabledMelee;
    d.stdRndTurns = Range(1, 1);
    d.isMakingMonAware = false;
    d.allowDisplayTurns = false;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.alignment = propAlignmentNeutral;
    addPropData(d);

    d.id = PropId::disabledRanged;
    d.stdRndTurns = Range(1, 1);
    d.isMakingMonAware = false;
    d.allowDisplayTurns = false;
    d.allowApplyMoreWhileActive = true;
    d.updatePlayerVisualWhenStartOrEnd = false;
    d.alignment = propAlignmentNeutral;
    addPropData(d);
}

} //namespace

void init()
{
    initDataList();
}

} //PropData

PropHandler::PropHandler(Actor* owningActor) :
    owningActor_(owningActor)
{
    const ActorDataT& d = owningActor->getData();

    for (size_t i = 0; i < int(PropId::END); ++i)
    {
        if (d.intrProps[i])
        {
            Prop* const prop = mkProp(PropId(i), PropTurns::indefinite);
            tryApplyProp(prop, true, true, true, true);
        }
    }
}

Prop* PropHandler::mkProp(const PropId id, PropTurns turnsInit,
                          const int NR_TURNS) const
{
    assert(id != PropId::END);

    switch (id)
    {
    case PropId::nailed:            return new PropNailed         (turnsInit, NR_TURNS);
    case PropId::warlockCharged:    return new PropWarlockCharged (turnsInit, NR_TURNS);
    case PropId::blind:             return new PropBlind          (turnsInit, NR_TURNS);
    case PropId::burning:           return new PropBurning        (turnsInit, NR_TURNS);
    case PropId::flared:            return new PropFlared         (turnsInit, NR_TURNS);
    case PropId::paralyzed:         return new PropParalyzed      (turnsInit, NR_TURNS);
    case PropId::terrified:         return new PropTerrified      (turnsInit, NR_TURNS);
    case PropId::weakened:          return new PropWeakened       (turnsInit, NR_TURNS);
    case PropId::confused:          return new PropConfused       (turnsInit, NR_TURNS);
    case PropId::stunned:           return new PropStunned        (turnsInit, NR_TURNS);
    case PropId::waiting:           return new PropWaiting        (turnsInit, NR_TURNS);
    case PropId::slowed:            return new PropSlowed         (turnsInit, NR_TURNS);
    case PropId::hasted:            return new PropHasted         (turnsInit, NR_TURNS);
    case PropId::infected:          return new PropInfected       (turnsInit, NR_TURNS);
    case PropId::diseased:          return new PropDiseased       (turnsInit, NR_TURNS);
    case PropId::poisoned:          return new PropPoisoned       (turnsInit, NR_TURNS);
    case PropId::fainted:           return new PropFainted        (turnsInit, NR_TURNS);
    case PropId::frenzied:          return new PropFrenzied       (turnsInit, NR_TURNS);
    case PropId::aiming:            return new PropAiming         (turnsInit, NR_TURNS);
    case PropId::disabledAttack:    return new PropDisabledAttack (turnsInit, NR_TURNS);
    case PropId::disabledMelee:     return new PropDisabledMelee  (turnsInit, NR_TURNS);
    case PropId::disabledRanged:    return new PropDisabledRanged (turnsInit, NR_TURNS);
    case PropId::blessed:           return new PropBlessed        (turnsInit, NR_TURNS);
    case PropId::cursed:            return new PropCursed         (turnsInit, NR_TURNS);
    case PropId::rAcid:             return new PropRAcid          (turnsInit, NR_TURNS);
    case PropId::rCold:             return new PropRCold          (turnsInit, NR_TURNS);
    case PropId::rConf:             return new PropRConfusion     (turnsInit, NR_TURNS);
    case PropId::rBreath:           return new PropRBreath        (turnsInit, NR_TURNS);
    case PropId::rElec:             return new PropRElec          (turnsInit, NR_TURNS);
    case PropId::rFear:             return new PropRFear          (turnsInit, NR_TURNS);
    case PropId::rPhys:             return new PropRPhys          (turnsInit, NR_TURNS);
    case PropId::rFire:             return new PropRFire          (turnsInit, NR_TURNS);
    case PropId::rPoison:           return new PropRPoison        (turnsInit, NR_TURNS);
    case PropId::rSleep:            return new PropRSleep         (turnsInit, NR_TURNS);
    case PropId::lgtSens:           return new PropLgtSens        (turnsInit, NR_TURNS);
    case PropId::possByZuul:        return new PropPossByZuul     (turnsInit, NR_TURNS);
    case PropId::flying:            return new PropFlying         (turnsInit, NR_TURNS);
    case PropId::ethereal:          return new PropEthereal       (turnsInit, NR_TURNS);
    case PropId::ooze:              return new PropOoze           (turnsInit, NR_TURNS);
    case PropId::burrowing:         return new PropBurrowing      (turnsInit, NR_TURNS);
    case PropId::radiant:           return new PropRadiant        (turnsInit, NR_TURNS);
    case PropId::rDisease:          return new PropRDisease       (turnsInit, NR_TURNS);
    case PropId::teleCtrl:          return new PropTeleControl    (turnsInit, NR_TURNS);
    case PropId::spellReflect:      return new PropSpellReflect   (turnsInit, NR_TURNS);
    case PropId::strangled:         return new PropStrangled      (turnsInit, NR_TURNS);
    case PropId::conflict:          return new PropConflict       (turnsInit, NR_TURNS);
    case PropId::END: {} break;
    }
    return nullptr;
}

PropHandler::~PropHandler()
{
    for (Prop* prop : appliedProps_)        {delete prop;}
    for (Prop* prop : actorTurnPropBuffer_) {delete prop;}
}

void PropHandler::getPropsFromSources(
    vector<Prop*>& out, bool sources[int(PropSrc::END)]) const
{
    out.clear();

    //Get from applied properties
    if (sources[int(PropSrc::applied)]) {out = appliedProps_;}

    //Get from inventory if humanoid actor
    if (owningActor_->isHumanoid() && sources[int(PropSrc::inv)])
    {
        const auto& inv = owningActor_->getInv();

        auto addItemProps = [&](const vector<Prop*>& itemPropList)
        {
            for (auto* const prop : itemPropList)
            {
                prop->owningActor_ = owningActor_;
                out.push_back(prop);
            };
        };

        for (const auto& slot : inv.slots_)
        {
            const auto* const item = slot.item;

            if (item)
            {
                addItemProps(item->carrierProps_);
            }
        }

        for (auto* const item : inv.general_)
        {
            addItemProps(item->carrierProps_);
        }
    }
}

void  PropHandler::getPropIdsFromSources(
    bool out[size_t(PropId::END)], bool sources[int(PropSrc::END)]) const
{
    for (int i = 0; i < int(PropId::END); ++i)
    {
        out[i] = 0;
    }

    //Get from applied properties
    if (sources[int(PropSrc::applied)])
    {
        for (const Prop* const prop : appliedProps_)
        {
            out[int(prop->getId())] = true;
        }
    }

    //Get from inventory if humanoid actor
    if (owningActor_->isHumanoid() && sources[int(PropSrc::inv)])
    {
        const auto& inv = owningActor_->getInv();

        auto addItemProps = [&](const vector<Prop*>& itemPropList)
        {
            for (auto* const prop : itemPropList)
            {
                prop->owningActor_      = owningActor_;
                out[int(prop->getId())] = true;
            };
        };

        for (const auto& slot : inv.slots_)
        {
            const auto* const item = slot.item;
            if (item)
            {
                addItemProps(item->carrierProps_);
            }
        }

        for (auto* const item : inv.general_)
        {
            addItemProps(item->carrierProps_);
        }
    }
}

void PropHandler::getPropIds(bool out[size_t(PropId::END)]) const
{
    bool sources[int(PropSrc::END)];
    for (bool& v : sources) {v = true;}
    getPropIdsFromSources(out, sources);
}

bool PropHandler::tryResistProp(const PropId id, const vector<Prop*>& propList) const
{
    for (Prop* p : propList)
    {
        if (p->isResistingOtherProp(id))
        {
            return true;
        }
    }
    return false;
}

bool PropHandler::tryResistDmg(const DmgType dmgType, const bool ALLOW_MSG) const
{
    vector<Prop*> propList;
    bool sources[int(PropSrc::END)];
    for (bool& v : sources) {v = true;}
    getPropsFromSources(propList, sources);

    for (Prop* p : propList)
    {
        if (p->tryResistDmg(dmgType, ALLOW_MSG)) return true;
    }
    return false;
}

bool PropHandler::allowSee() const
{
    vector<Prop*> propList;
    bool sources[int(PropSrc::END)];
    for (bool& v : sources) {v = true;}
    getPropsFromSources(propList, sources);

    for (Prop* p : propList) {if (!p->allowSee()) return false;}
    return true;
}

void PropHandler::tryApplyProp(Prop* const prop, const bool FORCE_EFFECT,
                               const bool NO_MESSAGES,
                               const bool DISABLE_REDRAW,
                               const bool DISABLE_PROP_START_EFFECTS)
{
    assert(prop);

    //First, if this is a prop that runs on actor turns, check if the actor-turn
    //prop buffer does not already contain the prop.
    //-If it doesn't, then just add it to the buffer and return.
    //-If the buffer already contains the prop, it means it was requested to be
    //applied from the buffer to the applied props.
    //This way, this function can be used both for requesting to apply props,
    //and for applying props from the buffer.
    if (prop->getTurnMode() == PropTurnMode::actor)
    {
        vector<Prop*>& buffer = actorTurnPropBuffer_;
        if (find(begin(buffer), end(buffer), prop) == end(buffer))
        {
            buffer.push_back(prop);
            return;
        }
    }

    prop->owningActor_    = owningActor_;

    const bool IS_PLAYER  = owningActor_->isPlayer();
    bool playerSeeOwner   = Map::player->isSeeingActor(*owningActor_, nullptr);

    if (!FORCE_EFFECT)
    {
        vector<Prop*> allProps;

        bool sources[int(PropSrc::END)];
        for (bool& v : sources) {v = true;}

        getPropsFromSources(allProps, sources);

        if (tryResistProp(prop->getId(), allProps))
        {
            if (!NO_MESSAGES)
            {
                if (IS_PLAYER)
                {
                    string msg = "";
                    prop->getMsg(propMsgOnResPlayer, msg);
                    if (!msg.empty()) {Log::addMsg(msg, clrWhite, true);}
                }
                else //Is a monster
                {
                    if (Map::player->isSeeingActor(*owningActor_, nullptr))
                    {
                        string msg = "";
                        prop->getMsg(propMsgOnResMon, msg);
                        if (!msg.empty())
                        {
                            const string monsterName = owningActor_->getNameThe();
                            Log::addMsg(monsterName + " " + msg, clrWhite, true);
                        }
                    }
                }
            }
            delete prop;
            return;
        }
    }

    //This point reached means nothing is blocking the property.

    //Actor already has property applied?
    for (Prop* oldProp : appliedProps_)
    {
        if (prop->getId() == oldProp->getId())
        {
            if (!prop->allowApplyMoreWhileActive())
            {
                delete prop;
                return;
            }

            const int TURNS_LEFT_OLD = oldProp->turnsLeft_;
            const int TURNS_LEFT_NEW = prop->turnsLeft_;

            //Print start message
            if (!NO_MESSAGES)
            {
                if (IS_PLAYER)
                {
                    string msg = "";
                    prop->getMsg(propMsgOnStartPlayer, msg);
                    if (!msg.empty())
                    {
                        Log::addMsg(msg, clrWhite, true);
                    }
                }
                else //Not player
                {
                    if (playerSeeOwner)
                    {
                        string msg = "";
                        prop->getMsg(propMsgOnStartMon, msg);
                        if (!msg.empty())
                        {
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

    if (!DISABLE_PROP_START_EFFECTS) {prop->onStart();}

    if (!DISABLE_REDRAW)
    {
        if (prop->shouldUpdatePlayerVisualWhenStartOrEnd())
        {
            prop->owningActor_->updateClr();
            Map::player->updateFov();
            Render::drawMapAndInterface();
        }
    }

    if (!NO_MESSAGES)
    {
        if (IS_PLAYER)
        {
            string msg = "";
            prop->getMsg(propMsgOnStartPlayer, msg);
            if (!msg.empty())
            {
                Log::addMsg(msg, clrWhite, true);
            }
        }
        else
        {
            if (playerSeeOwner)
            {
                string msg = "";
                prop->getMsg(propMsgOnStartMon, msg);
                if (!msg.empty())
                {
                    Log::addMsg(owningActor_->getNameThe() + " " + msg);
                }
            }
        }
    }
}

void PropHandler::tryApplyPropFromAtt(const Wpn& wpn, const bool IS_MELEE)
{
    const ItemDataT& d            = wpn.getData();
    const auto* const originProp  = IS_MELEE ? d.melee.propApplied :
                                    d.ranged.propApplied;

    if (originProp)
    {
        //If weapon damage type is resisted by the defender, the property is
        //automatically resisted
        DmgType dmgType = IS_MELEE ? d.melee.dmgType : d.ranged.dmgType;
        if (!tryResistDmg(dmgType, false))
        {
            //Make a copy of the weapon effect
            auto* const propCpy =
                mkProp(originProp->getId(), originProp->getTurnsInitType(),
                       originProp->turnsLeft_);
            tryApplyProp(propCpy);
        }
    }
}

bool PropHandler::endAppliedProp(const PropId id, const bool RUN_PROP_END_EFFECTS)
{
    int           idx               = -1;
    Prop*         prop              = nullptr;
    const size_t  NR_APPLIED_PROPS  = appliedProps_.size();

    for (size_t i = 0; i < NR_APPLIED_PROPS; ++i)
    {
        prop = appliedProps_[i];

        if (prop->getId() == id)
        {
            idx = i;
            break;
        }
    }

    if (idx == -1)
    {
        return false;
    }

    appliedProps_.erase(begin(appliedProps_) + idx);

    if (RUN_PROP_END_EFFECTS)
    {
        const bool IS_VISUAL_UPDATE_NEEDED =
            prop->shouldUpdatePlayerVisualWhenStartOrEnd();

        if (IS_VISUAL_UPDATE_NEEDED)
        {
            prop->owningActor_->updateClr();
            Map::player->updateFov();
            Render::drawMapAndInterface();
        }

        bool curPropIds[size_t(PropId::END)];
        getPropIds(curPropIds);

        //Print property end message
        if (!curPropIds[size_t(id)])
        {
            if (owningActor_->isPlayer())
            {
                string msg = "";
                prop->getMsg(propMsgOnEndPlayer, msg);
                if (!msg.empty()) {Log::addMsg(msg, clrWhite);}
            }
            else //Not player
            {
                if (Map::player->isSeeingActor(*owningActor_, nullptr))
                {
                    string msg = "";
                    prop->getMsg(propMsgOnEndMon, msg);
                    if (!msg.empty())
                    {
                        Log::addMsg(owningActor_->getNameThe() + " " + msg);
                    }
                }
            }
        }

        prop->onEnd();
    }

    delete prop;
    return true;
}

void PropHandler::applyActorTurnPropBuffer()
{
    for (Prop* prop : actorTurnPropBuffer_) {tryApplyProp(prop);}
    actorTurnPropBuffer_.clear();
}

void PropHandler::tick(const PropTurnMode turnMode)
{
    //Applied properties
    for (size_t i = 0; i < appliedProps_.size();)
    {
        Prop* const prop = appliedProps_[i];

        //Only tick property if it runs on the given turn mode
        if (prop->getTurnMode() == turnMode)
        {
            if (!owningActor_->isPlayer() && prop->isMakingMonAware())
            {
                auto* mon           = static_cast<Mon*>(owningActor_);
                mon->awareCounter_  = owningActor_->getData().nrTurnsAware;
            }

            if (prop->turnsLeft_ > 0)
            {
                prop->turnsLeft_--;
            }

            if (prop->isFinished())
            {
                endAppliedProp(prop->getId());
            }
            else //Not finished
            {
                prop->onNewTurn();
                i++;
            }
        }
        else //Wrong turn mode
        {
            i++;
        }
    }

    //Inventory properties
    vector<Prop*> invProps;

    bool sources[int(PropSrc::END)];

    for (bool& v : sources) {v = false;}

    sources[int(PropSrc::inv)] = true;

    getPropsFromSources(invProps, sources);

    for (Prop* prop : invProps)
    {
        if (prop->getTurnMode() == turnMode)
        {
            if (!owningActor_->isPlayer() && prop->isMakingMonAware())
            {
                auto* mon           = static_cast<Mon*>(owningActor_);
                mon->awareCounter_  = owningActor_->getData().nrTurnsAware;
            }
            prop->onNewTurn();
        }
    }
}

void PropHandler::getPropsInterfaceLine(vector<StrAndClr>& line) const
{
    line.clear();

    const bool IS_SELF_AWARE = PlayerBon::traits[int(Trait::selfAware)];

    vector<Prop*> propList;
    bool sources[int(PropSrc::END)];
    for (bool& v : sources) {v = true;}
    getPropsFromSources(propList, sources);
    for (Prop* prop : propList)
    {
        const string propName = prop->getNameShort();
        if (!propName.empty())
        {
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

int PropHandler::getChangedMaxHp(const int HP_MAX) const
{
    vector<Prop*> propList;
    bool sources[int(PropSrc::END)];
    for (bool& v : sources) {v = true;}
    getPropsFromSources(propList, sources);
    int newHpMax = HP_MAX;
    for (Prop* prop : propList) {newHpMax = prop->getChangedMaxHp(newHpMax);}
    return newHpMax;
}

void PropHandler::changeMoveDir(const Pos& actorPos, Dir& dir) const
{
    vector<Prop*> propList;
    bool sources[int(PropSrc::END)];
    for (bool& v : sources) {v = true;}
    getPropsFromSources(propList, sources);
    for (Prop* prop : propList) {prop->changeMoveDir(actorPos, dir);}
}

bool PropHandler::allowAttack(const bool ALLOW_MSG) const
{
    vector<Prop*> propList;
    bool sources[int(PropSrc::END)];
    for (bool& v : sources) {v = true;}
    getPropsFromSources(propList, sources);
    for (Prop* prop : propList)
    {
        if (!prop->allowAttackMelee(ALLOW_MSG) && !prop->allowAttackRanged(ALLOW_MSG))
        {
            return false;
        }
    }
    return true;
}

bool PropHandler::allowAttackMelee(const bool ALLOW_MSG) const
{
    vector<Prop*> propList;
    bool sources[int(PropSrc::END)];
    for (bool& v : sources) {v = true;}
    getPropsFromSources(propList, sources);
    for (Prop* prop : propList)
    {
        if (!prop->allowAttackMelee(ALLOW_MSG))
        {
            return false;
        }
    }
    return true;
}

bool PropHandler::allowAttackRanged(const bool ALLOW_MSG) const
{
    vector<Prop*> propList;

    bool sources[int(PropSrc::END)];
    for (bool& v : sources) {v = true;}

    getPropsFromSources(propList, sources);

    for (Prop* prop : propList)
    {
        if (!prop->allowAttackRanged(ALLOW_MSG)) {return false;}
    }
    return true;
}

bool PropHandler::allowMove() const
{
    vector<Prop*> propList;

    bool sources[int(PropSrc::END)];
    for (bool& v : sources) {v = true;}

    getPropsFromSources(propList, sources);

    for (Prop* prop : propList)
    {
        if (!prop->allowMove()) {return false;}
    }
    return true;
}

bool PropHandler::allowAct() const
{
    vector<Prop*> propList;

    bool sources[int(PropSrc::END)];
    for (bool& v : sources) {v = true;}

    getPropsFromSources(propList, sources);

    for (Prop* prop : propList)
    {
        if (!prop->allowAct()) {return false;}
    }
    return true;
}

bool PropHandler::allowRead(const bool ALLOW_MSG) const
{
    TRACE_FUNC_BEGIN_VERBOSE;
    vector<Prop*> propList;

    bool sources[int(PropSrc::END)];

    for (bool& v : sources) {v = true;}
    getPropsFromSources(propList, sources);

    for (auto prop : propList)
    {
        if (!prop->allowRead(ALLOW_MSG))
        {
            TRACE_FUNC_END_VERBOSE;
            return false;
        }
    }

    TRACE_FUNC_END_VERBOSE;
    return true;
}

bool PropHandler::allowCastSpell(const bool ALLOW_MSG) const
{
    vector<Prop*> propList;

    bool sources[int(PropSrc::END)];

    for (bool& v : sources) {v = true;}
    getPropsFromSources(propList, sources);

    for (auto prop : propList)
    {
        if (!prop->allowCastSpell(ALLOW_MSG))
        {
            return false;
        }
    }
    return true;
}

bool PropHandler::allowSpeak(const bool ALLOW_MSG) const
{
    TRACE_FUNC_BEGIN_VERBOSE;
    vector<Prop*> propList;

    bool sources[int(PropSrc::END)];

    for (bool& v : sources) {v = true;}
    getPropsFromSources(propList, sources);

    for (auto prop : propList)
    {
        if (!prop->allowSpeak(ALLOW_MSG))
        {
            TRACE_FUNC_END_VERBOSE;
            return false;
        }
    }

    TRACE_FUNC_END_VERBOSE;
    return true;
}

bool PropHandler::allowEat(const bool ALLOW_MSG) const
{
    TRACE_FUNC_BEGIN_VERBOSE;
    vector<Prop*> propList;

    bool sources[int(PropSrc::END)];

    for (bool& v : sources) {v = true;}
    getPropsFromSources(propList, sources);

    for (auto prop : propList)
    {
        if (!prop->allowEat(ALLOW_MSG))
        {
            TRACE_FUNC_END_VERBOSE;
            return false;
        }
    }

    TRACE_FUNC_END_VERBOSE;
    return true;
}

void PropHandler::onHit()
{
    vector<Prop*> propList;

    bool sources[int(PropSrc::END)];
    for (bool& v : sources) {v = true;}

    getPropsFromSources(propList, sources);

    for (Prop* prop : propList)
    {
        prop->onHit();
    }
}

void PropHandler::onDeath(const bool IS_PLAYER_SEE_OWNING_ACTOR)
{
    vector<Prop*> propList;

    bool sources[int(PropSrc::END)];
    for (bool& v : sources) {v = true;}

    getPropsFromSources(propList, sources);

    for (Prop* prop : propList)
    {
        prop->onDeath(IS_PLAYER_SEE_OWNING_ACTOR);
    }
}

int PropHandler::getAbilityMod(const AbilityId ability) const
{
    vector<Prop*> propList;

    bool sources[int(PropSrc::END)];
    for (bool& v : sources) {v = true;}

    getPropsFromSources(propList, sources);

    int modifier = 0;

    for (Prop* prop : propList)
    {
        modifier += prop->getAbilityMod(ability);
    }
    return modifier;
}

Prop* PropHandler::getProp(const PropId id, const PropSrc source) const
{

    assert(source != PropSrc::END);

    if (source == PropSrc::applied)
    {
        for (Prop* prop : appliedProps_) {if (prop->getId() == id) {return prop;}}
    }
    else if (source == PropSrc::inv)
    {
        vector<Prop*> invProps;

        bool sources[int(PropSrc::END)];
        for (bool& v : sources) {v = false;}
        sources[int(PropSrc::inv)] = true;

        getPropsFromSources(invProps, sources);

        for (Prop* prop : invProps)
        {
            if (prop->getId() == id)
            {
                return prop;
            }
        }
    }
    return nullptr;
}

bool PropHandler::changeActorClr(Clr& clr) const
{
    vector<Prop*> propList;

    bool sources[int(PropSrc::END)];

    for (bool& v : sources) {v = true;}
    getPropsFromSources(propList, sources);

    for (Prop* prop : propList)
    {
        if (prop->changeActorClr(clr))
        {
            return true;
        }
    }
    return false;
}

void PropHandler::endAppliedPropsByMagicHealing()
{
    vector<Prop*> propList;
    bool sources[int(PropSrc::END)];

    for (bool& v : sources) {v = true;}
    getPropsFromSources(propList, sources);

    for (size_t i = 0; i < propList.size(); ++i)
    {
        if (propList[i]->isEndedByMagicHealing())
        {
            endAppliedProp(appliedProps_[i]->getId());
            propList.erase(begin(propList) + i);
            i--;
        }
    }
}

Prop::Prop(PropId id, PropTurns turnsInit, int turns) :
    turnsLeft_      (turns),
    owningActor_    (nullptr),
    id_             (id),
    data_           (&(PropData::data[int(id)])),
    turnsInitType_  (turnsInit)
{
    switch (turnsInit)
    {
    case PropTurns::std:
        turnsLeft_ = Rnd::range(data_->stdRndTurns);
        break;

    case PropTurns::indefinite:
        turnsLeft_ = -1;
        break;

    case PropTurns::specific: {}
        break;
    }
}

void PropBlessed::onStart()
{
    owningActor_->getPropHandler().endAppliedProp(PropId::cursed, false);
}

void PropCursed::onStart()
{
    owningActor_->getPropHandler().endAppliedProp(PropId::blessed, false);
}

void PropSlowed::onStart()
{
    owningActor_->getPropHandler().endAppliedProp(PropId::hasted, false);
}

void PropHasted::onStart()
{
    owningActor_->getPropHandler().endAppliedProp(PropId::slowed, false);
}

void PropInfected::onNewTurn()
{
    if (Rnd::oneIn(150))
    {
        PropHandler& propHlr = owningActor_->getPropHandler();
        propHlr.tryApplyProp(new PropDiseased(PropTurns::std));
        propHlr.endAppliedProp(PropId::infected, false);
        Log::morePrompt();
    }
}

int PropDiseased::getChangedMaxHp(const int HP_MAX) const
{
    if (owningActor_->isPlayer() && PlayerBon::traits[int(Trait::survivalist)])
    {
        return (HP_MAX * 3) / 4; //Survavlist makes you lose only 25% instead of 50%
    }
    else
    {
        return HP_MAX / 2;
    }
}

void PropDiseased::onStart()
{
    //Actor::getHpMax() will now return a decreased value
    //cap current HP to the new, lower, maximum
    int& hp = owningActor_->hp_;
    hp = min(Map::player->getHpMax(true), hp);
}

bool PropDiseased::isResistingOtherProp(const PropId id) const
{
    //Getting infected while already diseased is just annoying
    return id == PropId::infected;
}

void PropPossByZuul::onDeath(const bool IS_PLAYER_SEE_OWNING_ACTOR)
{
    if (IS_PLAYER_SEE_OWNING_ACTOR)
    {
        const string& name1 = owningActor_->getNameThe();
        const string& name2 = ActorData::data[int(ActorId::zuul)].nameThe;
        Log::addMsg(name1 + " was possessed by " + name2 + "!");
    }
    owningActor_->state_  = ActorState::destroyed;
    const Pos& pos        = owningActor_->pos;
    Map::mkGore(pos);
    Map::mkBlood(pos);
    ActorFactory::summonMon(pos, vector<ActorId> {ActorId::zuul}, true);
}

void PropPoisoned::onNewTurn()
{
    if (owningActor_->isAlive())
    {
        if (GameTime::getTurn() % POISON_DMG_N_TURN == 0)
        {
            if (owningActor_->isPlayer())
            {
                Log::addMsg("I am suffering from the poison!", clrMsgBad, true);
            }
            else
            {
                if (Map::player->isSeeingActor(*owningActor_, nullptr))
                {
                    Log::addMsg(owningActor_->getNameThe() + " suffers from poisoning!");
                }
            }
            owningActor_->hit(1, DmgType::pure);
        }
    }
}

bool PropTerrified::allowAttackMelee(const bool ALLOW_MSG) const
{
    if (owningActor_->isPlayer() && ALLOW_MSG)
    {
        Log::addMsg("I am too terrified to engage in close combat!");
    }
    return false;
}

bool PropTerrified::allowAttackRanged(const bool ALLOW_MSG) const
{
    (void)ALLOW_MSG;
    return true;
}

void PropNailed::changeMoveDir(const Pos& actorPos, Dir& dir)
{
    (void)actorPos;

    if (dir != Dir::center)
    {
        if (owningActor_->isPlayer())
        {
            Log::addMsg("I struggle to tear out the spike!", clrMsgBad);
        }
        else
        {
            if (Map::player->isSeeingActor(*owningActor_, nullptr))
            {
                Log::addMsg(owningActor_->getNameThe() +  " struggles in pain!",
                            clrMsgGood);
            }
        }

        owningActor_->hit(Rnd::dice(1, 3), DmgType::physical);

        if (owningActor_->isAlive())
        {

            //TODO: reimplement something affecting chance of success?

            if (Rnd::oneIn(4))
            {
                nrSpikes_--;
                if (nrSpikes_ > 0)
                {
                    if (owningActor_->isPlayer())
                    {
                        Log::addMsg("I rip out a spike from my flesh!");
                    }
                    else
                    {
                        if (Map::player->isSeeingActor(*owningActor_, nullptr))
                        {
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

bool PropConfused::allowRead(const bool ALLOW_MSG) const
{
    if (owningActor_->isPlayer() && ALLOW_MSG)
    {
        Log::addMsg("I am too confused to read.");
    }
    return false;
}

bool PropConfused::allowCastSpell(const bool ALLOW_MSG) const
{
    if (owningActor_->isPlayer() && ALLOW_MSG)
    {
        Log::addMsg("I am too confused to concentrate.");
    }
    return false;
}

bool PropConfused::allowAttackMelee(const bool ALLOW_MSG) const
{
    (void)ALLOW_MSG;

    if (owningActor_ != Map::player)
    {
        return Rnd::coinToss();
    }
    return true;
}

bool PropConfused::allowAttackRanged(const bool ALLOW_MSG) const
{
    (void)ALLOW_MSG;

    if (owningActor_ != Map::player)
    {
        return Rnd::coinToss();
    }
    return true;
}

void PropConfused::changeMoveDir(const Pos& actorPos, Dir& dir)
{
    if (dir != Dir::center)
    {
        bool blocked[MAP_W][MAP_H];
        MapParse::run(CellCheck::BlocksActor(*owningActor_, true),
                      blocked);

        if (Rnd::oneIn(8))
        {
            int triesLeft = 100;
            while (triesLeft != 0)
            {
                //-1 to 1 for x and y
                const Pos delta(Rnd::range(-1, 1), Rnd::range(-1, 1));
                if (delta.x != 0 || delta.y != 0)
                {
                    const Pos c = actorPos + delta;

                    if (!blocked[c.x][c.y])
                    {
                        dir = DirUtils::getDir(delta);
                    }
                }
                triesLeft--;
            }
        }
    }
}

void PropStrangled::onNewTurn()
{
    const int DMG = Rnd::range(3, 4);

    owningActor_->hit(DMG, DmgType::pure, DmgMethod::forced);
}

bool PropStrangled::allowSpeak(const bool ALLOW_MSG) const
{
    if (ALLOW_MSG && owningActor_->isPlayer())
    {
        Log::addMsg("My throat is constricted.");
    }

    return false;
}

bool PropStrangled::allowEat(const bool ALLOW_MSG) const
{
    if (ALLOW_MSG && owningActor_->isPlayer())
    {
        Log::addMsg("My throat is constricted.");
    }

    return false;
}

void PropFrenzied::changeMoveDir(const Pos& actorPos, Dir& dir)
{
    if (owningActor_->isPlayer())
    {
        vector<Actor*> seenFoes;
        owningActor_->getSeenFoes(seenFoes);

        if (seenFoes.empty())
        {
            return;
        }

        vector<Pos> seenFoesCells;

        seenFoesCells.clear();

        for (auto* actor : seenFoes)
        {
            seenFoesCells.push_back(actor->pos);
        }

        sort(begin(seenFoesCells), end(seenFoesCells), IsCloserToPos(actorPos));

        const Pos& closestMonPos = seenFoesCells[0];

        bool blocked[MAP_W][MAP_H];
        MapParse::run(CellCheck::BlocksActor(*owningActor_, false), blocked);

        vector<Pos> line;
        LineCalc::calcNewLine(actorPos, closestMonPos, true, 999, false, line);

        if (line.size() > 1)
        {
            for (Pos& pos : line)
            {
                if (blocked[pos.x][pos.y])
                {
                    return;
                }
            }
            dir = DirUtils::getDir(line[1] - actorPos);
        }
    }
}

bool PropFrenzied::isResistingOtherProp(const PropId id) const
{
    return id == PropId::confused || id == PropId::fainted ||
           id == PropId::terrified || id == PropId::weakened;
}

void PropFrenzied::onStart()
{
    owningActor_->getPropHandler().endAppliedProp(PropId::confused);
    owningActor_->getPropHandler().endAppliedProp(PropId::terrified);
    owningActor_->getPropHandler().endAppliedProp(PropId::weakened);
}

void PropFrenzied::onEnd()
{
    //Only the player gets tired after a frenzy (it looks weird for monsters)
    if (owningActor_->isPlayer())
    {
        owningActor_->getPropHandler().tryApplyProp(new PropWeakened(PropTurns::std));
    }
}

bool PropFrenzied::allowRead(const bool ALLOW_MSG) const
{
    if (owningActor_->isPlayer() && ALLOW_MSG)
    {
        Log::addMsg("I am too enraged to read!");
    }
    return false;
}

bool PropFrenzied::allowCastSpell(const bool ALLOW_MSG) const
{
    if (owningActor_->isPlayer() && ALLOW_MSG)
    {
        Log::addMsg("I am too enraged to concentrate!");
    }
    return false;
}

void PropBurning::onStart()
{
//  owningActor_->addLight(Map::light);
}

void PropBurning::onNewTurn()
{
    if (owningActor_->isPlayer())
    {
        Log::addMsg("AAAARGH IT BURNS!!!", clrRedLgt);
    }
    owningActor_->hit(Rnd::dice(1, 2), DmgType::fire);
}

bool PropBurning::allowRead(const bool ALLOW_MSG) const
{
    if (owningActor_->isPlayer() && ALLOW_MSG)
    {
        Log::addMsg("I cannot read while burning.");
    }
    return false;
}

bool PropBurning::allowCastSpell(const bool ALLOW_MSG) const
{
    if (owningActor_->isPlayer() && ALLOW_MSG)
    {
        Log::addMsg("I cannot concentrate while burning!");
    }
    return false;
}

bool PropBurning::allowAttackRanged(const bool ALLOW_MSG) const
{
    if (owningActor_->isPlayer() && ALLOW_MSG)
    {
        Log::addMsg("Not while burning.");
    }
    return false;
}

bool PropBlind::shouldUpdatePlayerVisualWhenStartOrEnd() const
{
    return owningActor_->isPlayer();
}

void PropParalyzed::onStart()
{
    auto* const player = Map::player;
    if (owningActor_->isPlayer())
    {
        auto* const activeExplosive = player->activeExplosive;
        if (activeExplosive) {activeExplosive->onPlayerParalyzed();}
    }
}

bool PropFainted::shouldUpdatePlayerVisualWhenStartOrEnd() const
{
    return owningActor_->isPlayer();
}

void PropFlared::onNewTurn()
{
    owningActor_->hit(1, DmgType::fire);

    if (turnsLeft_ == 0)
    {
        owningActor_->getPropHandler().tryApplyProp(new PropBurning(PropTurns::std));
        owningActor_->getPropHandler().endAppliedProp(PropId::flared);
    }
}

bool PropRAcid::tryResistDmg(const DmgType dmgType, const bool ALLOW_MSG) const
{
    if (dmgType == DmgType::acid)
    {
        if (ALLOW_MSG)
        {
            if (owningActor_->isPlayer())
            {
                Log::addMsg("I feel a faint burning sensation.");
            }
            else if (Map::player->isSeeingActor(*owningActor_, nullptr))
            {
                Log::addMsg(owningActor_->getNameThe() + " seems unaffected.");
            }
        }
        return true;
    }
    return false;
}

bool PropRCold::tryResistDmg(const DmgType dmgType, const bool ALLOW_MSG) const
{
    if (dmgType == DmgType::cold)
    {
        if (ALLOW_MSG)
        {
            if (owningActor_->isPlayer())
            {
                Log::addMsg("I feel chilly.");
            }
            else if (Map::player->isSeeingActor(*owningActor_, nullptr))
            {
                Log::addMsg(owningActor_->getNameThe() + " seems unaffected.");
            }
        }
        return true;
    }
    return false;
}

bool PropRElec::tryResistDmg(const DmgType dmgType, const bool ALLOW_MSG) const
{
    if (dmgType == DmgType::electric)
    {
        if (ALLOW_MSG)
        {
            if (owningActor_->isPlayer())
            {
                Log::addMsg("I feel a faint tingle.");
            }
            else if (Map::player->isSeeingActor(*owningActor_, nullptr))
            {
                Log::addMsg(owningActor_->getNameThe() + " seems unaffected.");
            }
        }
        return true;
    }
    return false;
}

bool PropRConfusion::isResistingOtherProp(const PropId id) const
{
    return id == PropId::confused;
}

void PropRConfusion::onStart()
{
    owningActor_->getPropHandler().endAppliedProp(PropId::confused);
}

bool PropRFear::isResistingOtherProp(const PropId id) const
{
    return id == PropId::terrified;
}

void PropRFear::onStart()
{
    owningActor_->getPropHandler().endAppliedProp(PropId::terrified);
}

bool PropRPhys::isResistingOtherProp(const PropId id) const
{
    (void)id;
    return false;
}

void PropRPhys::onStart()
{
    return;
}

bool PropRPhys::tryResistDmg(const DmgType dmgType, const bool ALLOW_MSG) const
{
    if (dmgType == DmgType::physical)
    {
        if (ALLOW_MSG)
        {
            if (owningActor_->isPlayer())
            {
                Log::addMsg("I resist harm.");
            }
            else if (Map::player->isSeeingActor(*owningActor_, nullptr))
            {
                Log::addMsg(owningActor_->getNameThe() + " seems unaffected.");
            }
        }
        return true;
    }
    return false;
}

bool PropRFire::isResistingOtherProp(const PropId id) const
{
    return id == PropId::burning;
}

void PropRFire::onStart()
{
    owningActor_->getPropHandler().endAppliedProp(PropId::burning);
}

bool PropRFire::tryResistDmg(const DmgType dmgType, const bool ALLOW_MSG) const
{
    if (dmgType == DmgType::fire)
    {
        if (ALLOW_MSG)
        {
            if (owningActor_->isPlayer())
            {
                Log::addMsg("I feel hot.");
            }
            else if (Map::player->isSeeingActor(*owningActor_, nullptr))
            {
                Log::addMsg(owningActor_->getNameThe() + " seems unaffected.");
            }
        }
        return true;
    }
    return false;
}

bool PropRPoison::isResistingOtherProp(const PropId id) const
{
    return id == PropId::poisoned;
}

void PropRPoison::onStart()
{
    owningActor_->getPropHandler().endAppliedProp(PropId::poisoned);
}

bool PropRSleep::isResistingOtherProp(const PropId id) const
{
    return id == PropId::fainted;
}

void PropRSleep::onStart()
{
    owningActor_->getPropHandler().endAppliedProp(PropId::fainted);
}

bool PropRDisease::isResistingOtherProp(const PropId id) const
{
    return id == PropId::diseased || id == PropId::infected;
}

void PropRDisease::onStart()
{
    owningActor_->getPropHandler().endAppliedProp(PropId::diseased);
    owningActor_->getPropHandler().endAppliedProp(PropId::infected);
}

void PropBurrowing::onNewTurn()
{
    const Pos& p = owningActor_->pos;
    Map::cells[p.x][p.y].rigid->hit(DmgType::physical, DmgMethod::forced);
}
