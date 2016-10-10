#include "properties.hpp"

#include <algorithm>

#include "init.hpp"
#include "actor_player.hpp"
#include "msg_log.hpp"
#include "postmortem.hpp"
#include "io.hpp"
#include "actor_mon.hpp"
#include "inventory.hpp"
#include "map.hpp"
#include "explosion.hpp"
#include "player_bon.hpp"
#include "map_parsing.hpp"
#include "line_calc.hpp"
#include "actor_factory.hpp"
#include "feature_rigid.hpp"
#include "feature_mob.hpp"
#include "item.hpp"
#include "text_format.hpp"
#include "saving.hpp"
#include "game.hpp"
#include "map_travel.hpp"

namespace prop_data
{

PropDataT data[(size_t)PropId::END];

namespace
{

void add_prop_data(PropDataT& d)
{
    data[(int)d.id] = d;
    PropDataT blank;
    d = blank;
}

void init_data_list()
{
    PropDataT d;

    d.id = PropId::rPhys;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Physical Resistance";
    d.name_short = "rPhys";
    d.msg[(size_t)PropMsg::start_player] = "I feel resistant to physical harm.";
    d.msg[(size_t)PropMsg::start_mon] = "is resistant to physical harm.";
    d.msg[(size_t)PropMsg::end_player] = "I feel vulnerable to physical harm.";
    d.msg[(size_t)PropMsg::end_mon] = "is vulnerable to physical harm.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::good;
    add_prop_data(d);

    d.id = PropId::rFire;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Fire resistance";
    d.name_short = "rFire";
    d.msg[(size_t)PropMsg::start_player] = "I feel resistant to fire.";
    d.msg[(size_t)PropMsg::start_mon] = "is resistant to fire.";
    d.msg[(size_t)PropMsg::end_player] = "I feel vulnerable to fire.";
    d.msg[(size_t)PropMsg::end_mon] = "is vulnerable to fire.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::good;
    add_prop_data(d);

    d.id = PropId::rPoison;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Poison resistance";
    d.name_short = "rPoison";
    d.msg[(size_t)PropMsg::start_player] = "I feel resistant to poison.";
    d.msg[(size_t)PropMsg::start_mon] = "is resistant to poison.";
    d.msg[(size_t)PropMsg::end_player] = "I feel vulnerable to poison.";
    d.msg[(size_t)PropMsg::end_mon] = "is vulnerable to poison.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::good;
    add_prop_data(d);

    d.id = PropId::rElec;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Electric resistance";
    d.name_short = "rElec";
    d.msg[(size_t)PropMsg::start_player] = "I feel resistant to electricity.";
    d.msg[(size_t)PropMsg::start_mon] = "is resistant to electricity.";
    d.msg[(size_t)PropMsg::end_player] = "I feel vulnerable to electricity.";
    d.msg[(size_t)PropMsg::end_mon] = "is vulnerable to electricity.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::good;
    add_prop_data(d);

    d.id = PropId::rAcid;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Acid resistance";
    d.name_short = "rAcid";
    d.msg[(size_t)PropMsg::start_player] = "I feel resistant to acid.";
    d.msg[(size_t)PropMsg::start_mon] = "is resistant to acid.";
    d.msg[(size_t)PropMsg::end_player] = "I feel vulnerable to acid.";
    d.msg[(size_t)PropMsg::end_mon] = "is vulnerable to acid.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::good;
    add_prop_data(d);

    d.id = PropId::rSleep;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Sleep resistance";
    d.name_short = "rSleep";
    d.msg[(size_t)PropMsg::start_player] = "I feel resistant to sleep.";
    d.msg[(size_t)PropMsg::start_mon] = "is resistant to sleep.";
    d.msg[(size_t)PropMsg::end_player] = "I feel vulnerable to sleep.";
    d.msg[(size_t)PropMsg::end_mon] = "is vulnerable to sleep.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::good;
    add_prop_data(d);

    d.id = PropId::rFear;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Fear resistance";
    d.name_short = "rFear";
    d.msg[(size_t)PropMsg::start_player] = "I feel resistant to fear.";
    d.msg[(size_t)PropMsg::start_mon] = "is resistant to fear.";
    d.msg[(size_t)PropMsg::end_player] = "I feel vulnerable to fear.";
    d.msg[(size_t)PropMsg::end_mon] = "is vulnerable to fear.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::good;
    add_prop_data(d);

    d.id = PropId::rConf;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Confusion resistance";
    d.name_short = "rConf";
    d.msg[(size_t)PropMsg::start_player] = "I feel resistant to confusion.";
    d.msg[(size_t)PropMsg::start_mon] = "is resistant to confusion.";
    d.msg[(size_t)PropMsg::end_player] = "I feel vulnerable to confusion.";
    d.msg[(size_t)PropMsg::end_mon] = "is vulnerable to confusion.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::good;
    add_prop_data(d);

    d.id = PropId::rDisease;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Disease resistance";
    d.name_short = "rDisease";
    d.msg[(size_t)PropMsg::start_player] = "I feel resistant to disease.";
    d.msg[(size_t)PropMsg::start_mon] = "is resistant to disease.";
    d.msg[(size_t)PropMsg::end_player] = "I feel vulnerable to disease.";
    d.msg[(size_t)PropMsg::end_mon] = "is vulnerable to disease.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::good;
    add_prop_data(d);

    d.id = PropId::rBlind;
    d.name = "";
    d.name_short = "rBlind";
    d.msg[(size_t)PropMsg::start_player] = "";
    d.msg[(size_t)PropMsg::start_mon] = "";
    d.msg[(size_t)PropMsg::end_player] = "";
    d.msg[(size_t)PropMsg::end_mon] = "";
    d.is_making_mon_aware = false;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = false;
    d.alignment = PropAlignment::good;
    add_prop_data(d);

    d.id = PropId::rBreath;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Breath resistance";
    d.name_short = "rBreath";
    d.msg[(size_t)PropMsg::start_player] = "I can breath without harm.";
    d.msg[(size_t)PropMsg::start_mon] = "can breath without harm.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::good;
    add_prop_data(d);

    d.id = PropId::rSpell;
    d.name = "Spell Resistance";
    d.name_short = "rSpell";
    d.msg[(size_t)PropMsg::start_player] = "I defy harmful spells!";
    d.msg[(size_t)PropMsg::start_mon] = "is defying harmful magic spells.";
    d.msg[(size_t)PropMsg::end_player] = "I feel vulnerable to magic spells.";
    d.msg[(size_t)PropMsg::end_mon] = "is vulnerable to magic spells.";
    d.is_making_mon_aware = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = false; //NOTE: This prop is tested anyway
    d.alignment = PropAlignment::good;
    add_prop_data(d);

    d.id = PropId::lgtSens;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Light sensitive";
    d.name_short = "lgtSensitive";
    d.msg[(size_t)PropMsg::start_player] = "I feel vulnerable to light!";
    d.msg[(size_t)PropMsg::start_mon] = "is vulnerable to light.";
    d.msg[(size_t)PropMsg::end_player] = "I no longer feel vulnerable to light.";
    d.msg[(size_t)PropMsg::end_mon] = "no longer is vulnerable to light.";
    d.msg[(size_t)PropMsg::res_player] = "";
    d.msg[(size_t)PropMsg::res_mon] = "";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.alignment = PropAlignment::bad;
    add_prop_data(d);

    d.id = PropId::blind;
    d.std_rnd_turns = Range(20, 30);
    d.name = "Blind";
    d.name_short = "Blind";
    d.msg[(size_t)PropMsg::start_player] = "I am blinded!";
    d.msg[(size_t)PropMsg::start_mon] = "is blinded.";
    d.msg[(size_t)PropMsg::end_player] = "I can see again!";
    d.msg[(size_t)PropMsg::end_mon] = "can see again.";
    d.msg[(size_t)PropMsg::res_player] = "";
    d.msg[(size_t)PropMsg::res_mon] = "";
    d.is_making_mon_aware = true;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::bad;
    add_prop_data(d);

    d.id = PropId::fainted;
    d.std_rnd_turns = Range(100, 200);
    d.name = "Fainted";
    d.name_short = "Fainted";
    d.msg[(size_t)PropMsg::start_player] = "I faint!";
    d.msg[(size_t)PropMsg::start_mon] = "faints.";
    d.msg[(size_t)PropMsg::end_player] = "I am awake.";
    d.msg[(size_t)PropMsg::end_mon] = "wakes up.";
    d.msg[(size_t)PropMsg::res_player] = "I resist fainting.";
    d.msg[(size_t)PropMsg::res_mon] = "resists fainting.";
    d.is_making_mon_aware = true;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::bad;
    add_prop_data(d);

    d.id = PropId::burning;
    d.std_rnd_turns = Range(7, 11);
    d.name = "Burning";
    d.name_short = "Burning";
    d.msg[(size_t)PropMsg::start_player] = "I am Burning!";
    d.msg[(size_t)PropMsg::start_mon] = "is burning.";
    d.msg[(size_t)PropMsg::end_player] = "The flames are put out.";
    d.msg[(size_t)PropMsg::end_mon] = "is no longer burning.";
    d.msg[(size_t)PropMsg::res_player] = "I resist burning.";
    d.msg[(size_t)PropMsg::res_mon] = "resists burning.";
    d.is_making_mon_aware = true;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = true;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::bad;
    add_prop_data(d);

    d.id = PropId::poisoned;
    d.std_rnd_turns = Range(30, 60);
    d.name = "Poisoned";
    d.name_short = "Poisoned";
    d.msg[(size_t)PropMsg::start_player] = "I am poisoned!";
    d.msg[(size_t)PropMsg::start_mon] = "is poisoned.";
    d.msg[(size_t)PropMsg::end_player] = "My body is cleansed from poisoning!";
    d.msg[(size_t)PropMsg::end_mon] = "is cleansed from poisoning.";
    d.msg[(size_t)PropMsg::res_player] = "I resist poisoning.";
    d.msg[(size_t)PropMsg::res_mon] = "resists poisoning.";
    d.is_making_mon_aware = true;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::bad;
    add_prop_data(d);

    d.id = PropId::paralyzed;
    d.std_rnd_turns = Range(7, 9);
    d.name = "Paralyzed";
    d.name_short = "Paralyzed";
    d.msg[(size_t)PropMsg::start_player] = "I am paralyzed!";
    d.msg[(size_t)PropMsg::start_mon] = "is paralyzed.";
    d.msg[(size_t)PropMsg::end_player] = "I can move again!";
    d.msg[(size_t)PropMsg::end_mon] = "can move again.";
    d.msg[(size_t)PropMsg::res_player] = "I resist paralyzation.";
    d.msg[(size_t)PropMsg::res_mon] = "resists paralyzation.";
    d.is_making_mon_aware = true;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = false;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::bad;
    add_prop_data(d);

    d.id = PropId::terrified;
    d.std_rnd_turns = Range(12, 16);
    d.name = "Terrified";
    d.name_short = "Terrified";
    d.msg[(size_t)PropMsg::start_player] = "I am terrified!";
    d.msg[(size_t)PropMsg::start_mon] = "looks terrified.";
    d.msg[(size_t)PropMsg::end_player] = "I am no longer terrified!";
    d.msg[(size_t)PropMsg::end_mon] = "is no longer terrified.";
    d.msg[(size_t)PropMsg::res_player] = "I resist fear.";
    d.msg[(size_t)PropMsg::res_mon] = "resists fear.";
    d.is_making_mon_aware = true;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::bad;
    add_prop_data(d);

    d.id = PropId::confused;
    d.std_rnd_turns = Range(80, 120);
    d.name = "Confused";
    d.name_short = "Confused";
    d.msg[(size_t)PropMsg::start_player] = "I am confused!";
    d.msg[(size_t)PropMsg::start_mon] = "looks confused.";
    d.msg[(size_t)PropMsg::end_player] = "I come to my senses.";
    d.msg[(size_t)PropMsg::end_mon] = "is no longer confused.";
    d.msg[(size_t)PropMsg::res_player] = "I manage to keep my head together.";
    d.msg[(size_t)PropMsg::res_mon] = "resists confusion.";
    d.is_making_mon_aware = true;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::bad;
    add_prop_data(d);

    d.id = PropId::stunned;
    d.std_rnd_turns = Range(5, 9);
    d.name = "Stunned";
    d.name_short = "Stunned";
    d.msg[(size_t)PropMsg::start_player] = "I am stunned!";
    d.msg[(size_t)PropMsg::start_mon] = "is stunned.";
    d.msg[(size_t)PropMsg::end_player] = "I am no longer stunned.";
    d.msg[(size_t)PropMsg::end_mon] = "is no longer stunned.";
    d.msg[(size_t)PropMsg::res_player] = "I resist stunning.";
    d.msg[(size_t)PropMsg::res_mon] = "resists stunning.";
    d.is_making_mon_aware = true;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::bad;
    add_prop_data(d);

    d.id = PropId::slowed;
    d.std_rnd_turns = Range(16, 24);
    d.name = "Slowed";
    d.name_short = "Slowed";
    d.msg[(size_t)PropMsg::start_player] = "Everything around me seems to speed up.";
    d.msg[(size_t)PropMsg::start_mon] = "slows down.";
    d.msg[(size_t)PropMsg::end_player] = "Everything around me seems to slow down.";
    d.msg[(size_t)PropMsg::end_mon] = "speeds up.";
    d.msg[(size_t)PropMsg::res_player] = "I resist slowness.";
    d.msg[(size_t)PropMsg::res_mon] = "resists slowness.";
    d.is_making_mon_aware = true;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::bad;
    add_prop_data(d);

    d.id = PropId::hasted;
    d.std_rnd_turns = Range(12, 16);
    d.name = "Hasted";
    d.name_short = "Hasted";
    d.msg[(size_t)PropMsg::start_player] = "Everything around me seems to slow down.";
    d.msg[(size_t)PropMsg::start_mon] = "speeds up.";
    d.msg[(size_t)PropMsg::end_player] = "Everything around me seems to speed up.";
    d.msg[(size_t)PropMsg::end_mon] = "slows down.";
    d.is_making_mon_aware = true;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::good;
    add_prop_data(d);

    d.id = PropId::flared;
    d.std_rnd_turns = Range(3, 4);
    d.msg[(size_t)PropMsg::start_mon] = "is perforated by a flare!";
    d.is_making_mon_aware = true;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = true;
    d.alignment = PropAlignment::bad;
    add_prop_data(d);

    d.id = PropId::nailed;
    d.name = "Nailed";
    d.msg[(size_t)PropMsg::start_player] = "I am fastened by a spike!";
    d.msg[(size_t)PropMsg::start_mon] = "is fastened by a spike.";
    d.msg[(size_t)PropMsg::end_player] = "I tear free!";
    d.msg[(size_t)PropMsg::end_mon] = "tears free!";
    d.is_making_mon_aware = true;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.alignment = PropAlignment::bad;
    add_prop_data(d);

    d.id = PropId::wound;
    d.name = "Wound";
    d.name_short = "Wound";
    d.msg[(size_t)PropMsg::start_player] = "I am wounded!";
    d.msg[(size_t)PropMsg::res_player] = "I resist wounding!";
    d.is_making_mon_aware = false;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = false;
    d.alignment = PropAlignment::bad;
    add_prop_data(d);

    d.id = PropId::warlock_charged;
    d.std_rnd_turns = Range(1, 1);
    d.name = "Charged";
    d.name_short = "Charged";
    d.msg[(size_t)PropMsg::start_player] = "I am burning with power!";
    d.is_making_mon_aware = false;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.alignment = PropAlignment::good;
    add_prop_data(d);

    d.id = PropId::infected;
    d.std_rnd_turns = Range(100, 200);
    d.name = "Infected";
    d.name_short = "Infected";
    d.msg[(size_t)PropMsg::start_player] = "I am infected!";
    d.msg[(size_t)PropMsg::start_mon] = "is infected.";
    d.msg[(size_t)PropMsg::end_player] = "My infection is cured!";
    d.msg[(size_t)PropMsg::end_mon] = "is no longer infected.";
    d.msg[(size_t)PropMsg::res_player] = "";
    d.msg[(size_t)PropMsg::res_mon] = "";
    d.is_making_mon_aware = true;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::bad;
    add_prop_data(d);

    d.id = PropId::diseased;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Diseased";
    d.name_short = "Diseased";
    d.msg[(size_t)PropMsg::start_player] = "I am diseased!";
    d.msg[(size_t)PropMsg::start_mon] = "is diseased.";
    d.msg[(size_t)PropMsg::end_player] = "My disease is cured!";
    d.msg[(size_t)PropMsg::end_mon] = "is no longer diseased.";
    d.msg[(size_t)PropMsg::res_player] = "I resist disease.";
    d.msg[(size_t)PropMsg::res_mon] = "resists disease.";
    d.is_making_mon_aware = true;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::bad;
    add_prop_data(d);

    d.id = PropId::descend;
    d.std_rnd_turns = Range(20, 30);
    d.name = "Descending";
    d.name_short = "Descend";
    d.msg[(size_t)PropMsg::start_player] = "I feel a sinking sensation.";
    d.msg[(size_t)PropMsg::end_player] = "";
    d.msg[(size_t)PropMsg::res_player] = "";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = false;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = false;
    d.alignment = PropAlignment::neutral;
    add_prop_data(d);

    d.id = PropId::weakened;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Weakened";
    d.name_short = "Weakened";
    d.msg[(size_t)PropMsg::start_player] = "I feel weaker.";
    d.msg[(size_t)PropMsg::start_mon] = "looks weaker.";
    d.msg[(size_t)PropMsg::end_player] = "I feel stronger!";
    d.msg[(size_t)PropMsg::end_mon] = "looks stronger!";
    d.msg[(size_t)PropMsg::res_player] = "I resist weakness.";
    d.msg[(size_t)PropMsg::res_mon] = "resists weakness.";
    d.is_making_mon_aware = true;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::bad;
    add_prop_data(d);

    d.id = PropId::frenzied;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Frenzied";
    d.name_short = "Frenzied";
    d.msg[(size_t)PropMsg::start_player] = "I feel ferocious!!!";
    d.msg[(size_t)PropMsg::start_mon] = "Looks ferocious!";
    d.msg[(size_t)PropMsg::end_player] = "I calm down.";
    d.msg[(size_t)PropMsg::end_mon] = "Calms down a little.";
    d.msg[(size_t)PropMsg::res_player] = "";
    d.msg[(size_t)PropMsg::res_mon] = "";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::neutral;
    add_prop_data(d);

    d.id = PropId::blessed;
    d.std_rnd_turns = Range(400, 600);
    d.name = "Blessed";
    d.name_short = "Blessed";
    d.msg[(size_t)PropMsg::start_player] = "I feel luckier.";
    d.msg[(size_t)PropMsg::end_player] = "I have normal luck.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::good;
    add_prop_data(d);

    d.id = PropId::cursed;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Cursed";
    d.name_short = "Cursed";
    d.msg[(size_t)PropMsg::start_player] = "I feel misfortunate.";
    d.msg[(size_t)PropMsg::end_player] = "I feel more fortunate.";
    d.msg[(size_t)PropMsg::res_player] = "I resist misfortune.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::bad;
    add_prop_data(d);

    d.id = PropId::radiant;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Radiant";
    d.name_short = "Radiant";
    d.msg[(size_t)PropMsg::start_player] = "A bright light shines around me.";
    d.msg[(size_t)PropMsg::end_player] = "It suddenly gets darker.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = true;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::neutral;
    add_prop_data(d);

    d.id = PropId::invis;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Invisible";
    d.name_short = "Invis";
    d.msg[(size_t)PropMsg::start_player] = "I am out of sight!";
    d.msg[(size_t)PropMsg::start_mon] = "is out of sight!";
    d.msg[(size_t)PropMsg::end_player] = "I am visible.";
    d.msg[(size_t)PropMsg::end_mon] = "is visible.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = true;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::good;
    add_prop_data(d);

    d.id = PropId::see_invis;
    d.std_rnd_turns = Range(50, 100);
    d.name = "See Invisible";
    d.name_short = "SeeInvis";
    d.msg[(size_t)PropMsg::start_player] = "My eyes perceive the invisible.";
    d.msg[(size_t)PropMsg::start_mon] = "seems to see very clearly.";
    d.msg[(size_t)PropMsg::end_player] = "My eyes can no longer perceive the invisible.";
    d.msg[(size_t)PropMsg::end_mon] = "seems to see less clearly.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = true;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::good;
    add_prop_data(d);

    d.id = PropId::infravis;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Infravision";
    d.name_short = "Infravision";
    d.msg[(size_t)PropMsg::start_player] = "I have infravision.";
    d.msg[(size_t)PropMsg::end_player] = "I no longer have infravision.";
    d.msg[(size_t)PropMsg::res_player] = "";
    d.msg[(size_t)PropMsg::res_mon] = "";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = true;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::good;
    add_prop_data(d);

    d.id = PropId::tele_ctrl;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Teleport Control";
    d.name_short = "TeleCtrl";
    d.msg[(size_t)PropMsg::start_player] = "I feel in control.";
    d.msg[(size_t)PropMsg::end_player] = "I feel less in control.";
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = false;
    d.alignment = PropAlignment::good;
    add_prop_data(d);

    d.id = PropId::spell_reflect;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Spell Reflection";
    d.name_short = "SpellRefl";
    d.msg[(size_t)PropMsg::start_player] = "Spells reflect off me!";
    d.msg[(size_t)PropMsg::end_player] = "I am vulnerable to spells.";
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::good;
    add_prop_data(d);

    d.id = PropId::aiming;
    d.std_rnd_turns = Range(1, 1);
    d.name = "Aiming";
    d.name_short = "Aiming";
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.alignment = PropAlignment::good;
    add_prop_data(d);

    d.id = PropId::fast_shooting;
    d.std_rnd_turns = Range(1, 1);
    d.name = "";
    d.name_short = "";
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.alignment = PropAlignment::good;
    add_prop_data(d);

    d.id = PropId::strangled;
    d.std_rnd_turns = Range(1, 1);
    d.name = "Strangled";
    d.name_short = "Strangled";
    d.is_making_mon_aware = true;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.alignment = PropAlignment::bad;
    add_prop_data(d);

    d.id = PropId::conflict;
    d.name_short = "Conflict";
    d.std_rnd_turns = Range(10, 20);
    d.msg[(size_t)PropMsg::start_mon] = "Looks conflicted.";
    d.msg[(size_t)PropMsg::end_mon] = "Looks more determined.";
    d.is_making_mon_aware = true;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.alignment = PropAlignment::bad;
    add_prop_data(d);

    d.id = PropId::poss_by_zuul;
    d.is_making_mon_aware = false;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = false;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = PropAlignment::neutral;
    add_prop_data(d);

    d.id = PropId::flying;
    d.name_short = "Flying";
    d.is_making_mon_aware = false;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = false;
    d.alignment = PropAlignment::neutral;
    add_prop_data(d);

    d.id = PropId::ethereal;
    d.name_short = "Ethereal";
    d.is_making_mon_aware = false;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = false;
    d.alignment = PropAlignment::neutral;
    add_prop_data(d);

    d.id = PropId::ooze;
    d.name_short = "Ooze";
    d.is_making_mon_aware = false;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = false;
    d.alignment = PropAlignment::neutral;
    add_prop_data(d);

    d.id = PropId::burrowing;
    d.name_short = "Burrow";
    d.is_making_mon_aware = false;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = false;
    d.alignment = PropAlignment::neutral;
    add_prop_data(d);

    d.id = PropId::waiting;
    d.std_rnd_turns = Range(1, 1);
    d.is_making_mon_aware = false;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.alignment = PropAlignment::neutral;
    add_prop_data(d);

    d.id = PropId::disabled_attack;
    d.std_rnd_turns = Range(1, 1);
    d.is_making_mon_aware = false;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.alignment = PropAlignment::neutral;
    add_prop_data(d);

    d.id = PropId::disabled_melee;
    d.std_rnd_turns = Range(1, 1);
    d.is_making_mon_aware = false;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.alignment = PropAlignment::neutral;
    add_prop_data(d);

    d.id = PropId::disabled_ranged;
    d.std_rnd_turns = Range(1, 1);
    d.is_making_mon_aware = false;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.alignment = PropAlignment::neutral;
    add_prop_data(d);
}

} //namespace

void init()
{
    init_data_list();
}

} //prop_data

//-----------------------------------------------------------------------------
// Property handler
//-----------------------------------------------------------------------------
PropHandler::PropHandler(Actor* owning_actor) :
    owning_actor_(owning_actor)
{
    //Reset the active props info
    for (size_t i = 0; i < size_t(PropId::END); ++i)
    {
        active_props_info_[i] = 0;
    }
}

void PropHandler::init_natural_props()
{
    const ActorDataT& d = owning_actor_->data();

    //Add natural properties
    for (size_t i = 0; i < size_t(PropId::END); ++i)
    {
        active_props_info_[i] = 0;

        if (d.natural_props[i])
        {
            Prop* const prop = mk_prop(PropId(i), PropTurns::indefinite);

            try_add(prop,
                    PropSrc::intr,
                    true,
                    Verbosity::silent);
        }
    }
}

PropHandler::~PropHandler()
{
    for (Prop* prop : props_)
    {
#ifndef NDEBUG
        //For sanity check
        decr_active_props_info(prop->id());
#endif // NDEBUG
        delete prop;
    }

#ifndef NDEBUG
    //Sanity check: all active props info should be exactly zero now
    for (size_t i = 0; i < size_t(PropId::END); ++i)
    {
        if (active_props_info_[i] != 0)
        {
            TRACE << "Active property info at id " << i << " not zero" << std::endl;
            ASSERT(false);
        }
    }
#endif // NDEBUG

    for (Prop* prop : actor_turn_prop_buffer_)
    {
        delete prop;
    }
}

void PropHandler::save() const
{
    //Save intrinsic properties to file

    ASSERT(owning_actor_);

    int nr_intr_props_ = 0;

    for (Prop* prop : props_)
    {
        if (prop->src_ == PropSrc::intr)
        {
            ++nr_intr_props_;
        }
    }

    saving::put_int(nr_intr_props_);

    for (Prop* prop : props_)
    {
        if (prop->src_ == PropSrc::intr)
        {
            saving::put_int(int(prop->id()));
            saving::put_int(prop->nr_turns_left_);

            prop->save();
        }
    }
}

void PropHandler::load()
{
    //Load intrinsic properties from file

    ASSERT(owning_actor_);

    const int nr_props = saving::get_int();

    for (int i = 0; i < nr_props; ++i)
    {
        const auto prop_id = PropId(saving::get_int());

        const int nr_turns = saving::get_int();

        const auto turns_init = nr_turns == -1 ?
                                PropTurns::indefinite : PropTurns::specific;

        Prop* const prop = mk_prop(prop_id, turns_init, nr_turns);

        prop->owning_actor_ = owning_actor_;

        prop->src_ = PropSrc::intr;

        props_.push_back(prop);

        incr_active_props_info(prop_id);

        prop->load();
    }
}

Prop* PropHandler::mk_prop(const PropId id, PropTurns turns_init, const int nr_turns) const
{
    ASSERT(id != PropId::END);

    //Prop turns init type should either be:
    // * "specific", and number of turns specified (> 0), OR
    // * NOT "specific" (i.e. "indefinite" or "std"), and number of turns NOT specified (-1)
    ASSERT((turns_init == PropTurns::specific && nr_turns > 0) ||
           (turns_init != PropTurns::specific && nr_turns == -1));

    switch (id)
    {
    case PropId::nailed:
        return new PropNailed(turns_init, nr_turns);

    case PropId::wound:
        return new PropWound(turns_init, nr_turns);

    case PropId::warlock_charged:
        return new PropWarlockCharged(turns_init, nr_turns);

    case PropId::blind:
        return new PropBlind(turns_init, nr_turns);

    case PropId::burning:
        return new PropBurning(turns_init, nr_turns);

    case PropId::flared:
        return new PropFlared(turns_init, nr_turns);

    case PropId::paralyzed:
        return new PropParalyzed(turns_init, nr_turns);

    case PropId::terrified:
        return new PropTerrified(turns_init, nr_turns);

    case PropId::weakened:
        return new PropWeakened(turns_init, nr_turns);

    case PropId::confused:
        return new PropConfused(turns_init, nr_turns);

    case PropId::stunned:
        return new PropStunned(turns_init, nr_turns);

    case PropId::waiting:
        return new PropWaiting(turns_init, nr_turns);

    case PropId::slowed:
        return new PropSlowed(turns_init, nr_turns);

    case PropId::hasted:
        return new PropHasted(turns_init, nr_turns);

    case PropId::infected:
        return new PropInfected(turns_init, nr_turns);

    case PropId::diseased:
        return new PropDiseased(turns_init, nr_turns);

    case PropId::descend:
        return new PropDescend(turns_init, nr_turns);

    case PropId::poisoned:
        return new PropPoisoned(turns_init, nr_turns);

    case PropId::fainted:
        return new PropFainted(turns_init, nr_turns);

    case PropId::frenzied:
        return new PropFrenzied(turns_init, nr_turns);

    case PropId::aiming:
        return new PropAiming(turns_init, nr_turns);

    case PropId::fast_shooting:
        return new PropFastShooting(turns_init, nr_turns);

    case PropId::disabled_attack:
        return new PropDisabledAttack(turns_init, nr_turns);

    case PropId::disabled_melee:
        return new PropDisabledMelee(turns_init, nr_turns);

    case PropId::disabled_ranged:
        return new PropDisabledRanged(turns_init, nr_turns);

    case PropId::blessed:
        return new PropBlessed(turns_init, nr_turns);

    case PropId::cursed:
        return new PropCursed(turns_init, nr_turns);

    case PropId::rAcid:
        return new PropRAcid(turns_init, nr_turns);

    case PropId::rConf:
        return new PropRConf(turns_init, nr_turns);

    case PropId::rBreath:
        return new PropRBreath(turns_init, nr_turns);

    case PropId::rElec:
        return new PropRElec(turns_init, nr_turns);

    case PropId::rFear:
        return new PropRFear(turns_init, nr_turns);

    case PropId::rPhys:
        return new PropRPhys(turns_init, nr_turns);

    case PropId::rFire:
        return new PropRFire(turns_init, nr_turns);

    case PropId::rSpell:
        return new PropRSpell(turns_init, nr_turns);

    case PropId::rPoison:
        return new PropRPoison(turns_init, nr_turns);

    case PropId::rSleep:
        return new PropRSleep(turns_init, nr_turns);

    case PropId::lgtSens:
        return new PropLgtSens(turns_init, nr_turns);

    case PropId::poss_by_zuul:
        return new PropPossByZuul(turns_init, nr_turns);

    case PropId::flying:
        return new PropFlying(turns_init, nr_turns);

    case PropId::ethereal:
        return new PropEthereal(turns_init, nr_turns);

    case PropId::ooze:
        return new PropOoze(turns_init, nr_turns);

    case PropId::burrowing:
        return new PropBurrowing(turns_init, nr_turns);

    case PropId::radiant:
        return new PropRadiant(turns_init, nr_turns);

    case PropId::infravis:
        return new PropInfravis(turns_init, nr_turns);

    case PropId::rDisease:
        return new PropRDisease(turns_init, nr_turns);

    case PropId::rBlind:
        return new PropRBlind(turns_init, nr_turns);

    case PropId::tele_ctrl:
        return new PropTeleControl(turns_init, nr_turns);

    case PropId::spell_reflect:
        return new PropSpellReflect(turns_init, nr_turns);

    case PropId::strangled:
        return new PropStrangled(turns_init, nr_turns);

    case PropId::conflict:
        return new PropConflict(turns_init, nr_turns);

    case PropId::invis:
        return new PropInvisible(turns_init, nr_turns);

    case PropId::see_invis:
        return new PropSeeInvis(turns_init, nr_turns);

    case PropId::END:
        break;
    }

    return nullptr;
}

void PropHandler::try_add(Prop* const prop,
                           PropSrc src,
                           const bool force_effect,
                           const Verbosity verbosity)
{
    ASSERT(prop);

    //First, if this is a prop that runs on actor turns, check if the actor-turn
    //prop buffer does not already contain the prop:
    //  -   If it doesn't, then just add it to the buffer and return.
    //  -   If the buffer already contains the prop, it means it was requested
    //      to be applied from the buffer to the applied props.
    //This way, this function can be used both for requesting to apply props,
    //and for applying props from the buffer.
    if (prop->turn_mode() == PropTurnMode::actor)
    {
        std::vector<Prop*>& buffer = actor_turn_prop_buffer_;

        if (find(begin(buffer), end(buffer), prop) == end(buffer))
        {
            buffer.push_back(prop);
            return;
        }
    }

    prop->owning_actor_ = owning_actor_;
    prop->src_          = src;

    const bool is_player        = owning_actor_->is_player();
    const bool player_see_owner = map::player->can_see_actor(*owning_actor_);

    //Check if property is resisted
    if (!force_effect)
    {
        if (try_resist_prop(prop->id()))
        {
            //Resist message
            if (verbosity == Verbosity::verbose &&
                owning_actor_->is_alive())
            {
                if (is_player)
                {
                    std::string msg = "";
                    prop->msg(PropMsg::res_player, msg);

                    if (!msg.empty())
                    {
                        msg_log::add(msg, clr_text, true);
                    }
                }
                else //Is a monster
                {
                    if (player_see_owner)
                    {
                        std::string msg = "";
                        prop->msg(PropMsg::res_mon, msg);

                        if (!msg.empty())
                        {
                            const std::string monster_name = owning_actor_->name_the();
                            msg_log::add(monster_name + " " + msg);
                        }
                    }
                }
            }

            delete prop;
            return;
        }
    }

    //This point reached means nothing is blocking the property.

    //Is this an intrinsic property, and actor already has an intrinsic
    //property of the same type? If so, the new property will just be "merged"
    //into the old one ("on_more()").
    if (prop->src_ == PropSrc::intr)
    {
        for (Prop* old_prop : props_)
        {
            if (old_prop->src_ == PropSrc::intr &&
                prop->id() == old_prop->id())
            {
                if (!prop->allow_apply_more_while_active())
                {
                    delete prop;
                    return;
                }

                const int turns_left_old = old_prop->nr_turns_left_;
                const int turns_left_new = prop->nr_turns_left_;

                //Start message
                if (verbosity == Verbosity::verbose &&
                    owning_actor_->is_alive())
                {
                    if (is_player)
                    {
                        std::string msg = "";
                        prop->msg(PropMsg::start_player, msg);

                        if (!msg.empty())
                        {
                            msg_log::add(msg, clr_text, true);
                        }
                    }
                    else //Not player
                    {
                        if (player_see_owner)
                        {
                            std::string msg = "";
                            prop->msg(PropMsg::start_mon, msg);

                            if (!msg.empty())
                            {
                                msg_log::add(owning_actor_->name_the() + " " + msg);
                            }
                        }
                    }
                }

                old_prop->on_more();

                old_prop->nr_turns_left_ =
                    ((turns_left_old < 0) || (turns_left_new < 0)) ?
                    -1 :
                    std::max(turns_left_old, turns_left_new);

                delete prop;
                return;
            }
        }
    }

    //This part reached means the property should be applied on its own

    props_.push_back(prop);

    prop->on_start();

    if (verbosity == Verbosity::verbose &&
        owning_actor_->is_alive())
    {
        // TODO: Check if something like this is really needed, if so -
        //       reimplement somehow.
        /*
        if (prop->need_update_vision_when_start_or_end())
        {
            prop->owning_actor_->update_clr();
            game_time::update_light_map();
            map::player->update_fov();
            io::draw_map_state();
        }
        */

        //Start message
        if (is_player)
        {
            std::string msg = "";
            prop->msg(PropMsg::start_player, msg);

            if (!msg.empty())
            {
                msg_log::add(msg, clr_text, true);
            }
        }
        else //Is monster
        {
            if (player_see_owner)
            {
                std::string msg = "";
                prop->msg(PropMsg::start_mon, msg);

                if (!msg.empty())
                {
                    msg_log::add(owning_actor_->name_the() + " " + msg);
                }
            }
        }
    }

    incr_active_props_info(prop->id());
}

void PropHandler::add_prop_from_equipped_item(
    const Item* const item,
    Prop* const prop,
    const Verbosity verbosity)
{
    prop->item_applying_ = item;

    try_add(prop, PropSrc::inv, true, verbosity);
}

Prop* PropHandler::prop(const PropId id) const
{
    if (has_prop(id))
    {
        for (Prop* const prop : props_)
        {
            if (prop->id() == id)
            {
                return prop;
            }
        }
    }

    return nullptr;
}

void PropHandler::remove_props_for_item(const Item* const item)
{
    for (size_t i = 0; i < props_.size(); /* No increment */)
    {
        Prop* const prop = props_[i];

        if (prop->item_applying_ == item)
        {
            ASSERT(prop->src_ == PropSrc::inv);
            ASSERT(prop->turns_init_type_ == PropTurns::indefinite);

            props_.erase(begin(props_) + i);

            decr_active_props_info(prop->id());

            on_prop_end(prop);
        }
        else //Property was not added by this item
        {
            ++i;
        }
    }
}

void PropHandler::try_add_from_att(const Wpn& wpn, const bool is_melee)
{
    const auto& d = wpn.data();

    const auto* const origin_prop
        = is_melee ?
        d.melee.prop_applied :
        d.ranged.prop_applied;

    if (origin_prop)
    {
        // If weapon damage is resisted by the defender, the property is
        // automatically resisted
        const DmgType dmg_type =
            is_melee ?
            d.melee.dmg_type :
            d.ranged.dmg_type;

        if (!try_resist_dmg(dmg_type, Verbosity::silent))
        {
            const auto turns_init_type = origin_prop->turns_init_type();

            const int nr_turns =
                (turns_init_type == PropTurns::specific) ?
                origin_prop->nr_turns_left_ : -1;

            //Make a copy of the weapon effect
            auto* const prop_cpy = mk_prop(origin_prop->id(),
                                           origin_prop->turns_init_type_,
                                           nr_turns);

            try_add(prop_cpy);
        }
    }
}

void PropHandler::incr_active_props_info(const PropId id)
{
    int& v = active_props_info_[(size_t)id];

#ifndef NDEBUG
    if (v < 0)
    {
        TRACE << "Tried to increment property with current value "
              << v << std::endl;

        ASSERT(false);
    }
#endif // NDEBUG

    ++v;
}

void PropHandler::decr_active_props_info(const PropId id)
{
    int& v = active_props_info_[(size_t)id];

#ifndef NDEBUG
    if (v <= 0)
    {
        TRACE << "Tried to decrement property with current value "
              << v << std::endl;

        ASSERT(false);
    }
#endif // NDEBUG

    --v;
}

void PropHandler::on_prop_end(Prop* const prop)
{
    // TODO: Check if something like this is really needed, if so -
    //       reimplement somehow.
    /*
    if (prop->need_update_vision_when_start_or_end())
    {
        prop->owning_actor_->update_clr();
        game_time::update_light_map();
        map::player->update_fov();
        io::draw_map_state();
    }
    */

    //Print end message if this is the last active property of this type
    if (owning_actor_->state() == ActorState::alive &&
        active_props_info_[(size_t)prop->id_] == 0)
    {
        if (owning_actor_->is_player())
        {
            std::string msg = "";

            prop->msg(PropMsg::end_player, msg);

            if (!msg.empty())
            {
                msg_log::add(msg);
            }
        }
        else //Not player
        {
            if (map::player->can_see_actor(*owning_actor_))
            {
                std::string msg = "";

                prop->msg(PropMsg::end_mon, msg);

                if (!msg.empty())
                {
                    msg_log::add(owning_actor_->name_the() + " " + msg);
                }
            }
        }
    }

    prop->on_end();
}

bool PropHandler::end_prop(const PropId id, const bool run_prop_end_effects)
{
    for (auto it = begin(props_); it != end(props_); ++it)
    {
        Prop* const prop = *it;

        if (prop->id_ == id && prop->src_ == PropSrc::intr)
        {
            props_.erase(it);

            decr_active_props_info(prop->id_);

            if (run_prop_end_effects)
            {
                on_prop_end(prop);
            }

            delete prop;

            return true;
        }
    }

    return false;
}

void PropHandler::apply_actor_turn_prop_buffer()
{
    for (Prop* prop : actor_turn_prop_buffer_)
    {
        try_add(prop);
    }

    actor_turn_prop_buffer_.clear();
}

void PropHandler::tick(const PropTurnMode turn_mode)
{
    for (size_t i = 0; i < props_.size(); /* No increment */)
    {
        Prop* prop = props_[i];

        if (prop->turn_mode() == turn_mode)
        {
            //Aggravates monster?
            if (!owning_actor_->is_player() && prop->is_making_mon_aware())
            {
                auto* mon = static_cast<Mon*>(owning_actor_);

                mon->become_aware_player(false);
            }

            //Count down number of turns
            if (prop->nr_turns_left_ > 0)
            {
                ASSERT(prop->src_ == PropSrc::intr);

                --prop->nr_turns_left_;
            }

            if (prop->is_finished())
            {
                props_.erase(begin(props_) + i);

                decr_active_props_info(prop->id());

                on_prop_end(prop);

                delete prop;
                prop = nullptr;
            }
            else //Not finished
            {
                // NOTE: "prop" may be set to nullptr here (if the property
                //       removed itself)
                prop = prop->on_new_turn();
            }
        }

        if (prop)
        {
            //Property has not been removed
            ++i;
        }
    }
}

void PropHandler::props_interface_line(std::vector<StrAndClr>& line) const
{
    line.clear();

    for (Prop* prop : props_)
    {
        std::string str = prop->name_short();

        if (!str.empty())
        {
            const int turns_left  = prop->nr_turns_left_;

            if (turns_left > 0)
            {
                // Player can see number of turns left on own properties with
                // Self-aware?
                if (owning_actor_->is_player() &&
                    player_bon::traits[(size_t)Trait::self_aware] &&
                    prop->allow_display_turns())
                {
                    str += ":" + to_str(turns_left);
                }
            }
            else //Property is indefinite
            {
                if (prop->src() == PropSrc::intr)
                {
                    // Indefinite intrinsic properties are printed in upper case
                    text_format::all_to_upper(str);
                }
            }

            const PropAlignment alignment = prop->alignment();

            const Clr clr = alignment == PropAlignment::good ? clr_msg_good :
                            alignment == PropAlignment::bad  ? clr_msg_bad :
                            clr_white;

            line.push_back(StrAndClr(str, clr));
        }
    }
}

bool PropHandler::try_resist_prop(const PropId id) const
{
    for (Prop* p : props_)
    {
        if (p->is_resisting_other_prop(id))
        {
            return true;
        }
    }

    return false;
}

bool PropHandler::try_resist_dmg(const DmgType dmg_type, const Verbosity verbosity) const
{
    for (Prop* p : props_)
    {
        if (p->try_resist_dmg(dmg_type, verbosity))
        {
            return true;
        }
    }

    return false;
}

bool PropHandler::allow_see() const
{
    for (Prop* p : props_)
    {
        if (!p->allow_see())
        {
            return false;
        }
    }

    return true;
}

int PropHandler::affect_max_hp(const int hp_max) const
{
    int new_hp_max = hp_max;

    for (Prop* prop : props_)
    {
        new_hp_max = prop->affect_max_hp(new_hp_max);
    }

    return new_hp_max;
}

void PropHandler::affect_move_dir(const P& actor_pos, Dir& dir) const
{
    for (Prop* prop : props_)
    {
        prop->affect_move_dir(actor_pos, dir);
    }
}

bool PropHandler::allow_attack(const Verbosity verbosity) const
{
    for (Prop* prop : props_)
    {
        if (!prop->allow_attack_melee(verbosity) &&
            !prop->allow_attack_ranged(verbosity))
        {
            return false;
        }
    }

    return true;
}

bool PropHandler::allow_attack_melee(const Verbosity verbosity) const
{
    for (Prop* prop : props_)
    {
        if (!prop->allow_attack_melee(verbosity))
        {
            return false;
        }
    }

    return true;
}

bool PropHandler::allow_attack_ranged(const Verbosity verbosity) const
{
    for (Prop* prop : props_)
    {
        if (!prop->allow_attack_ranged(verbosity))
        {
            return false;
        }
    }

    return true;
}

bool PropHandler::allow_move() const
{
    for (Prop* prop : props_)
    {
        if (!prop->allow_move())
        {
            return false;
        }
    }

    return true;
}

bool PropHandler::allow_act() const
{
    for (Prop* prop : props_)
    {
        if (!prop->allow_act())
        {
            return false;
        }
    }

    return true;
}

bool PropHandler::allow_read(const Verbosity verbosity) const
{
    for (auto prop : props_)
    {
        if (!prop->allow_read(verbosity))
        {
            return false;
        }
    }

    return true;
}

bool PropHandler::allow_cast_spell(const Verbosity verbosity) const
{
    for (auto prop : props_)
    {
        if (!prop->allow_cast_spell(verbosity))
        {
            return false;
        }
    }

    return true;
}

bool PropHandler::allow_speak(const Verbosity verbosity) const
{
    for (auto prop : props_)
    {
        if (!prop->allow_speak(verbosity))
        {
            return false;
        }
    }

    return true;
}

bool PropHandler::allow_eat(const Verbosity verbosity) const
{
    for (auto prop : props_)
    {
        if (!prop->allow_eat(verbosity))
        {
            return false;
        }
    }

    return true;
}

void PropHandler::on_hit()
{
    for (Prop* prop : props_)
    {
        prop->on_hit();
    }
}

void PropHandler::on_death(const bool is_player_see_owning_actor)
{
    TRACE_FUNC_BEGIN_VERBOSE;

    for (Prop* prop : props_)
    {
        prop->on_death(is_player_see_owning_actor);
    }

    TRACE_FUNC_END_VERBOSE;
}

int PropHandler::ability_mod(const AbilityId ability) const
{
    int modifier = 0;

    for (Prop* prop : props_)
    {
        modifier += prop->ability_mod(ability);
    }

    return modifier;
}

bool PropHandler::affect_actor_clr(Clr& clr) const
{
    bool did_affect_clr = false;

    for (Prop* prop : props_)
    {
        if (prop->affect_actor_clr(clr))
        {
            did_affect_clr = true;

            // It's probably more likely that a color change due to a bad
            // property is critical information (e.g. burning), so then we stop
            // searching and use this color. If it's a good or neutral property
            // that affected the color, then we keep searching.
            if (prop->alignment() == PropAlignment::bad)
            {
                break;
            }
        }
    }

    return did_affect_clr;
}

//-----------------------------------------------------------------------------
// Properties
//-----------------------------------------------------------------------------
Prop::Prop(PropId id, PropTurns turns_init, int nr_turns) :
    id_                 (id),
    data_               (prop_data::data[(size_t)id]),
    nr_turns_left_      (nr_turns),
    turns_init_type_    (turns_init),
    owning_actor_       (nullptr),
    src_                (PropSrc::END),
    item_applying_      (nullptr)
{
    switch (turns_init)
    {
    case PropTurns::std:
#ifndef NDEBUG
        if (nr_turns_left_ != -1)
        {
            TRACE << "Prop turns is \"std\", but " << nr_turns_left_
                  << " turns specified" << std::endl;
            ASSERT(false);
        }
#endif // NDEBUG
        ASSERT(nr_turns_left_ == -1);
        nr_turns_left_ = data_.std_rnd_turns.roll();
        break;

    case PropTurns::indefinite:
#ifndef NDEBUG
        if (nr_turns_left_ != -1)
        {
            TRACE << "Prop turns is \"indefinite\", but " << nr_turns_left_
                  << " turns specified" << std::endl;
            ASSERT(false);
        }
#endif // NDEBUG

        nr_turns_left_ = -1; //Robustness for release builds
        break;

    case PropTurns::specific:
        //Use the number of turns specified in the ctor argument
        ASSERT(nr_turns_left_ > 0);
        break;
    }
}

void PropBlessed::on_start()
{
    owning_actor_->prop_handler().end_prop(PropId::cursed, false);

    bless_adjacent();
}

void PropBlessed::on_more()
{
    bless_adjacent();
}

void PropBlessed::bless_adjacent() const
{
    //"Bless" adjacent fountains
    const P& p = owning_actor_->pos;

    for (const P& d : dir_utils::dir_list_w_center)
    {
        const P p_adj(p + d);

        Cell& cell = map::cells[p_adj.x][p_adj.y];

        Rigid* const rigid = cell.rigid;

        if (rigid->id() == FeatureId::fountain)
        {
            Fountain* const fountain = static_cast<Fountain*>(rigid);

            if (fountain->type() != FountainType::blessed &&
                fountain->has_drinks_left())
            {
                if (cell.is_seen_by_player)
                {
                    std::string name = fountain->name(Article::the);

                    text_format::first_to_lower(name);

                    msg_log::add("The water in " + name + " seems clearer.");
                }

                fountain->set_type(FountainType::blessed);
            }
        }
    }
}

void PropCursed::on_start()
{
    owning_actor_->prop_handler().end_prop(PropId::blessed, false);

    curse_adjacent();

    //If this is a permanent curse, log it as a historic event
    if (owning_actor_->is_player() &&
        turns_init_type_ == PropTurns::indefinite)
    {
        game::add_history_event("A terrible curse was put upon me.");
    }
}

void PropCursed::on_more()
{
    curse_adjacent();
}

void PropCursed::on_end()
{
    //If this was a permanent curse, log it as a historic event
    if (owning_actor_->is_player() &&
        turns_init_type_ == PropTurns::indefinite)
    {
        game::add_history_event("A terrible curse was lifted from me.");
    }
}

void PropCursed::curse_adjacent() const
{
    //"Curse" adjacent fountains
    const P& p = owning_actor_->pos;

    for (const P& d : dir_utils::dir_list_w_center)
    {
        const P p_adj(p + d);

        Cell& cell = map::cells[p_adj.x][p_adj.y];

        Rigid* const rigid = cell.rigid;

        if (rigid->id() == FeatureId::fountain)
        {
            Fountain* const fountain = static_cast<Fountain*>(rigid);

            if (fountain->type() != FountainType::cursed &&
                fountain->has_drinks_left())
            {
                if (cell.is_seen_by_player)
                {
                    std::string name = fountain->name(Article::the);

                    text_format::first_to_lower(name);

                    msg_log::add("The water in " + name + " seems murkier.");
                }

                fountain->set_type(FountainType::cursed);
            }
        }
    }
}

void PropSlowed::on_start()
{
    owning_actor_->prop_handler().end_prop(PropId::hasted, false);
}

void PropHasted::on_start()
{
    owning_actor_->prop_handler().end_prop(PropId::slowed, false);
}

Prop* PropInfected::on_new_turn()
{
#ifndef NDEBUG
    ASSERT(!owning_actor_->prop_handler().has_prop(PropId::diseased));
#endif // NDEBUG

    const int max_turns_left_allow_disease  = 50;
    const int apply_disease_one_in          = nr_turns_left_ - 1;

    if (nr_turns_left_ <= max_turns_left_allow_disease &&
        rnd::one_in(apply_disease_one_in))
    {
        PropHandler& prop_hlr = owning_actor_->prop_handler();

        prop_hlr.try_add(new PropDiseased(PropTurns::indefinite));

        //NOTE: Disease ends infection, this property object is now deleted!

        msg_log::more_prompt();

        return nullptr;
    }

    return this;
}

int PropDiseased::affect_max_hp(const int hp_max) const
{
#ifndef NDEBUG
    ASSERT(!owning_actor_->prop_handler().has_prop(PropId::infected));
#endif // NDEBUG

    return hp_max / 2;
}

void PropDiseased::on_start()
{
    //End infection
    owning_actor_->prop_handler().end_prop(PropId::infected, false);

    //If this is a permanent disease that the player caught, log it as a historic event
    if (owning_actor_->is_player() && turns_init_type_ == PropTurns::indefinite)
    {
        game::add_history_event("Caught a horrible disease.");
    }
}

void PropDiseased::on_end()
{
#ifndef NDEBUG
    ASSERT(!owning_actor_->prop_handler().has_prop(PropId::infected));
#endif // NDEBUG

    //If this is a permanent disease that the player caught, log it as a historic event
    if (owning_actor_->is_player() && turns_init_type_ == PropTurns::indefinite)
    {
        game::add_history_event("My body was cured from a horrible disease.");
    }
}

bool PropDiseased::is_resisting_other_prop(const PropId prop_id) const
{
#ifndef NDEBUG
    ASSERT(!owning_actor_->prop_handler().has_prop(PropId::infected));
#endif // NDEBUG

    //Getting infected while already diseased is just annoying
    return prop_id == PropId::infected;
}

Prop* PropDescend::on_new_turn()
{
    ASSERT(owning_actor_->is_player());

    if (nr_turns_left_ <= 1)
    {
        game_time::is_magic_descend_nxt_std_turn = true;
    }

    return this;
}

void PropPossByZuul::on_death(const bool is_player_see_owning_actor)
{
    if (is_player_see_owning_actor)
    {
        const std::string& name1 = owning_actor_->name_the();
        const std::string& name2 = actor_data::data[int(ActorId::zuul)].name_the;
        msg_log::add(name1 + " was possessed by " + name2 + "!");
    }

    owning_actor_->state_   = ActorState::destroyed;
    const P& pos            = owning_actor_->pos;
    map::mk_gore(pos);
    map::mk_blood(pos);
    actor_factory::summon(pos,
                          std::vector<ActorId> {ActorId::zuul},
                          MakeMonAware::yes);

    //Zuul is now free, allow him to spawn.
    actor_data::data[(size_t)ActorId::zuul].nr_left_allowed_to_spawn = -1;
}

Prop* PropPoisoned::on_new_turn()
{
    if (owning_actor_->is_alive())
    {
        if (game_time::turn() % poison_dmg_n_turn == 0)
        {
            if (owning_actor_->is_player())
            {
                msg_log::add("I am suffering from the poison!", clr_msg_bad, true);
            }
            else //Is monster
            {
                if (map::player->can_see_actor(*owning_actor_))
                {
                    msg_log::add(owning_actor_->name_the() + " suffers from poisoning!");
                }
            }

            owning_actor_->hit(1, DmgType::pure);
        }
    }

    return this;
}

bool PropTerrified::allow_attack_melee(const Verbosity verbosity) const
{
    if (owning_actor_->is_player() && verbosity == Verbosity::verbose)
    {
        msg_log::add("I am too terrified to engage in close combat!");
    }

    return false;
}

bool PropTerrified::allow_attack_ranged(const Verbosity verbosity) const
{
    (void)verbosity;
    return true;
}

void PropTerrified::on_start()
{
    //If this is a monster, we reset its last direction moved. Otherwise it would probably
    //tend to move toward the player even while terrified (the AI would typically use the idle
    //movement algorithm, which favors stepping in the same direction as the last move).

    if (!owning_actor_->is_player())
    {
        Mon* const mon = static_cast<Mon*>(owning_actor_);
        mon->last_dir_moved_ = Dir::center;
    }
}

void PropNailed::affect_move_dir(const P& actor_pos, Dir& dir)
{
    (void)actor_pos;

    if (dir != Dir::center)
    {
        if (owning_actor_->is_player())
        {
            msg_log::add("I struggle to tear out the spike!", clr_msg_bad);
        }
        else //Is monster
        {
            if (map::player->can_see_actor(*owning_actor_))
            {
                msg_log::add(owning_actor_->name_the() +  " struggles in pain!",
                             clr_msg_good);
            }
        }

        owning_actor_->hit(rnd::dice(1, 3), DmgType::physical);

        if (owning_actor_->is_alive())
        {

            //TODO: reimplement something affecting chance of success?

            if (rnd::one_in(4))
            {
                --nr_spikes_;

                if (nr_spikes_ > 0)
                {
                    if (owning_actor_->is_player())
                    {
                        msg_log::add("I rip out a spike from my flesh!");
                    }
                    else //Is monster
                    {
                        if (map::player->can_see_actor(*owning_actor_))
                        {
                            msg_log::add(owning_actor_->name_the() + " tears out a spike!");
                        }
                    }
                }
            }
        }

        dir = Dir::center;
    }
}

void PropWound::save() const
{
    saving::put_int(nr_wounds_);
}

void PropWound::load()
{
    nr_wounds_ = saving::get_int();
}

void PropWound::msg(const PropMsg msg_type, std::string& msg_ref) const
{
    switch (msg_type)
    {
    case PropMsg::start_player:
    case PropMsg::res_player:
        msg_ref = data_.msg[(size_t)msg_type];
        break;

    case PropMsg::end_player:
        msg_ref = (nr_wounds_ > 1) ?
                  "All my wounds are healed!" :
                  "A wound is healed!";
        break;

    case PropMsg::start_mon:
    case PropMsg::end_mon:
    case PropMsg::res_mon:
    case PropMsg::END:
        msg_ref = "";
        break;
    }
}

int PropWound::ability_mod(const AbilityId ability) const
{
    const bool is_survivalist = owning_actor_->is_player() &&
                                player_bon::traits[(size_t)Trait::survivalist];

    const int div = is_survivalist ? 2 : 1;

    const int k = std::min(4, nr_wounds_);

    if (ability == AbilityId::melee)
    {
        return (k * -10) / div;
    }
    else if (ability == AbilityId::ranged)
    {
        return (k *  -5) / div;
    }
    else if (ability == AbilityId::dodge_att)
    {
        return (k * -10) / div;
    }
    else if (ability == AbilityId::dodge_trap)
    {
        return (k * -10) / div;
    }

    return 0;
}

int PropWound::affect_max_hp(const int hp_max) const
{
    const int pen_pct_per_wound = 10;

    int hp_pen_pct = nr_wounds_ * pen_pct_per_wound;

    const bool is_survivalist = owning_actor_->is_player() &&
                                player_bon::traits[(size_t)Trait::survivalist];

    if (is_survivalist)
    {
        hp_pen_pct /= 2;
    }

    //Cap the penalty percentage
    hp_pen_pct = std::min(70, hp_pen_pct);

    return (hp_max * (100 - hp_pen_pct)) / 100;
}

void PropWound::heal_one_wound()
{
    ASSERT(nr_wounds_ > 0);

    --nr_wounds_;

    if (nr_wounds_ > 0)
    {
        msg_log::add("A wound is healed.");
    }
    else //This was the last wound
    {
        //End self
        owning_actor_->prop_handler().end_prop(id());
    }
}

void PropWound::on_more()
{
    const int max_nr_wounds = 5;

    nr_wounds_ = std::min(max_nr_wounds, nr_wounds_ + 1);
}

bool PropConfused::allow_read(const Verbosity verbosity) const
{
    if (owning_actor_->is_player() && verbosity == Verbosity::verbose)
    {
        msg_log::add("I am too confused to read.");
    }

    return false;
}

bool PropConfused::allow_cast_spell(const Verbosity verbosity) const
{
    if (owning_actor_->is_player() && verbosity == Verbosity::verbose)
    {
        msg_log::add("I am too confused to concentrate.");
    }

    return false;
}

bool PropConfused::allow_attack_melee(const Verbosity verbosity) const
{
    (void)verbosity;

    if (owning_actor_ != map::player)
    {
        return rnd::coin_toss();
    }

    return true;
}

bool PropConfused::allow_attack_ranged(const Verbosity verbosity) const
{
    (void)verbosity;

    if (owning_actor_ != map::player)
    {
        return rnd::coin_toss();
    }

    return true;
}

void PropConfused::affect_move_dir(const P& actor_pos, Dir& dir)
{
    if (dir != Dir::center)
    {
        bool blocked[map_w][map_h];

         const R area_check_blocked(actor_pos - P(1, 1), actor_pos + P(1, 1));

        map_parse::run(cell_check::BlocksActor(*owning_actor_, true),
                       blocked,
                       MapParseMode::overwrite,
                       area_check_blocked);

        if (rnd::one_in(8))
        {
            std::vector<P> d_bucket;

            for (const P& d : dir_utils::dir_list)
            {
                const P tgt_p(actor_pos + d);

                if (!blocked[tgt_p.x][tgt_p.y])
                {
                    d_bucket.push_back(d);
                }
            }

            if (!d_bucket.empty())
            {
                const size_t    idx = rnd::range(0, d_bucket.size() - 1);
                const P&        d   = d_bucket[idx];

                dir = dir_utils::dir(d);
            }
        }
    }
}

Prop* PropStrangled::on_new_turn()
{
    const int dmg = rnd::range(3, 4);

    owning_actor_->hit(dmg, DmgType::pure, DmgMethod::forced);

    return this;
}

bool PropStrangled::allow_speak(const Verbosity verbosity) const
{
    if (verbosity == Verbosity::verbose && owning_actor_->is_player())
    {
        msg_log::add("My throat is constricted.");
    }

    return false;
}

bool PropStrangled::allow_eat(const Verbosity verbosity) const
{
    if (verbosity == Verbosity::verbose && owning_actor_->is_player())
    {
        msg_log::add("My throat is constricted.");
    }

    return false;
}

void PropFrenzied::affect_move_dir(const P& actor_pos, Dir& dir)
{
    if (owning_actor_->is_player())
    {
        std::vector<Actor*> seen_foes;
        owning_actor_->seen_foes(seen_foes);

        if (seen_foes.empty())
        {
            return;
        }

        std::vector<P> seen_foes_cells;

        seen_foes_cells.clear();

        for (auto* actor : seen_foes)
        {
            seen_foes_cells.push_back(actor->pos);
        }

        sort(begin(seen_foes_cells), end(seen_foes_cells), IsCloserToPos(actor_pos));

        const P& closest_mon_pos = seen_foes_cells[0];

        bool blocked[map_w][map_h];

        map_parse::run(cell_check::BlocksActor(*owning_actor_, false),
                       blocked);

        std::vector<P> line;

        line_calc::calc_new_line(actor_pos, closest_mon_pos, true, 999, false, line);

        if (line.size() > 1)
        {
            for (P& pos : line)
            {
                if (blocked[pos.x][pos.y])
                {
                    return;
                }
            }

            dir = dir_utils::dir(line[1] - actor_pos);
        }
    }
}

bool PropFrenzied::is_resisting_other_prop(const PropId prop_id) const
{
    return prop_id == PropId::confused || prop_id == PropId::fainted ||
           prop_id == PropId::terrified || prop_id == PropId::weakened;
}

void PropFrenzied::on_start()
{
    owning_actor_->prop_handler().end_prop(PropId::confused);
    owning_actor_->prop_handler().end_prop(PropId::terrified);
    owning_actor_->prop_handler().end_prop(PropId::weakened);
}

void PropFrenzied::on_end()
{
    //Only the player (except for Ghoul background) gets tired after a frenzy
    //(it looks weird for monsters)
    if (owning_actor_->is_player() && player_bon::bg() != Bg::ghoul)
    {
        owning_actor_->prop_handler().try_add(new PropWeakened(PropTurns::std));
    }
}

bool PropFrenzied::allow_read(const Verbosity verbosity) const
{
    if (owning_actor_->is_player() && verbosity == Verbosity::verbose)
    {
        msg_log::add("I am too enraged to read!");
    }

    return false;
}

bool PropFrenzied::allow_cast_spell(const Verbosity verbosity) const
{
    if (owning_actor_->is_player() && verbosity == Verbosity::verbose)
    {
        msg_log::add("I am too enraged to concentrate!");
    }

    return false;
}

Prop* PropBurning::on_new_turn()
{
    if (owning_actor_->is_player())
    {
        msg_log::add("AAAARGH IT BURNS!!!", clr_red_lgt);
    }

    owning_actor_->hit(rnd::dice(1, 2), DmgType::fire);

    return this;
}

bool PropBurning::allow_read(const Verbosity verbosity) const
{
    if (owning_actor_->is_player() && verbosity == Verbosity::verbose)
    {
        msg_log::add("I cannot read while burning.");
    }

    return false;
}

bool PropBurning::allow_cast_spell(const Verbosity verbosity) const
{
    if (owning_actor_->is_player() && verbosity == Verbosity::verbose)
    {
        msg_log::add("I cannot concentrate while burning!");
    }

    return false;
}

bool PropBurning::allow_attack_ranged(const Verbosity verbosity) const
{
    if (owning_actor_->is_player() && verbosity == Verbosity::verbose)
    {
        msg_log::add("Not while burning.");
    }

    return false;
}

bool PropBlind::need_update_vision_when_start_or_end() const
{
    return owning_actor_->is_player();
}

void PropParalyzed::on_start()
{
    auto* const player = map::player;

    if (owning_actor_->is_player())
    {
        auto* const active_explosive = player->active_explosive;

        if (active_explosive)
        {
            active_explosive->on_player_paralyzed();
        }
    }
}

bool PropFainted::need_update_vision_when_start_or_end() const
{
    return owning_actor_->is_player();
}

Prop* PropFlared::on_new_turn()
{
    owning_actor_->hit(1, DmgType::fire);

    if (nr_turns_left_ <= 1)
    {
        owning_actor_->prop_handler().try_add(new PropBurning(PropTurns::std));
        owning_actor_->prop_handler().end_prop(id());

        return nullptr;
    }

    return this;
}

bool PropRAcid::try_resist_dmg(const DmgType dmg_type, const Verbosity verbosity) const
{
    if (dmg_type == DmgType::acid)
    {
        if (verbosity == Verbosity::verbose)
        {
            if (owning_actor_->is_player())
            {
                msg_log::add("I feel a faint burning sensation.");
            }
            else if (map::player->can_see_actor(*owning_actor_))
            {
                msg_log::add(owning_actor_->name_the() + " seems unaffected.");
            }
        }

        return true;
    }

    return false;
}

bool PropRElec::try_resist_dmg(const DmgType dmg_type, const Verbosity verbosity) const
{
    if (dmg_type == DmgType::electric)
    {
        if (verbosity == Verbosity::verbose)
        {
            if (owning_actor_->is_player())
            {
                msg_log::add("I feel a faint tingle.");
            }
            else if (map::player->can_see_actor(*owning_actor_))
            {
                msg_log::add(owning_actor_->name_the() + " seems unaffected.");
            }
        }

        return true;
    }

    return false;
}

bool PropRConf::is_resisting_other_prop(const PropId prop_id) const
{
    return prop_id == PropId::confused;
}

void PropRConf::on_start()
{
    owning_actor_->prop_handler().end_prop(PropId::confused);
}

bool PropRFear::is_resisting_other_prop(const PropId prop_id) const
{
    return prop_id == PropId::terrified;
}

void PropRFear::on_start()
{
    owning_actor_->prop_handler().end_prop(PropId::terrified);

    if (owning_actor_->is_player() && turns_init_type_ == PropTurns::indefinite)
    {
        insanity::on_permanent_rfear();
    }
}

bool PropRPhys::is_resisting_other_prop(const PropId prop_id) const
{
    (void)prop_id;
    return false;
}

void PropRPhys::on_start()
{

}

bool PropRPhys::try_resist_dmg(const DmgType dmg_type, const Verbosity verbosity) const
{
    if (dmg_type == DmgType::physical)
    {
        if (verbosity == Verbosity::verbose)
        {
            if (owning_actor_->is_player())
            {
                msg_log::add("I resist harm.");
            }
            else if (map::player->can_see_actor(*owning_actor_))
            {
                msg_log::add(owning_actor_->name_the() + " seems unaffected.");
            }
        }

        return true;
    }

    return false;
}

bool PropRFire::is_resisting_other_prop(const PropId prop_id) const
{
    return prop_id == PropId::burning;
}

void PropRFire::on_start()
{
    owning_actor_->prop_handler().end_prop(PropId::burning);
}

bool PropRFire::try_resist_dmg(const DmgType dmg_type, const Verbosity verbosity) const
{
    if (dmg_type == DmgType::fire)
    {
        if (verbosity == Verbosity::verbose)
        {
            if (owning_actor_->is_player())
            {
                msg_log::add("I feel hot.");
            }
            else if (map::player->can_see_actor(*owning_actor_))
            {
                msg_log::add(owning_actor_->name_the() + " seems unaffected.");
            }
        }

        return true;
    }

    return false;
}

bool PropRPoison::is_resisting_other_prop(const PropId prop_id) const
{
    return prop_id == PropId::poisoned;
}

void PropRPoison::on_start()
{
    owning_actor_->prop_handler().end_prop(PropId::poisoned);
}

bool PropRSleep::is_resisting_other_prop(const PropId prop_id) const
{
    return prop_id == PropId::fainted;
}

void PropRSleep::on_start()
{
    owning_actor_->prop_handler().end_prop(PropId::fainted);
}

bool PropRDisease::is_resisting_other_prop(const PropId prop_id) const
{
    return prop_id == PropId::diseased || prop_id == PropId::infected;
}

void PropRDisease::on_start()
{
    owning_actor_->prop_handler().end_prop(PropId::diseased);
    owning_actor_->prop_handler().end_prop(PropId::infected);
}

bool PropRBlind::is_resisting_other_prop(const PropId prop_id) const
{
    return prop_id == PropId::blind;
}

void PropRBlind::on_start()
{
    owning_actor_->prop_handler().end_prop(PropId::blind);
}

bool PropSeeInvis::is_resisting_other_prop(const PropId prop_id) const
{
    return prop_id == PropId::blind;
}

void PropSeeInvis::on_start()
{
    owning_actor_->prop_handler().end_prop(PropId::blind);
}

Prop* PropBurrowing::on_new_turn()
{
    const P& p = owning_actor_->pos;
    map::cells[p.x][p.y].rigid->hit(DmgType::physical, DmgMethod::forced);

    return this;
}
