#include "properties.hpp"

#include <algorithm>
#include <cassert>

#include "init.hpp"
#include "actor_player.hpp"
#include "msg_log.hpp"
#include "postmortem.hpp"
#include "render.hpp"
#include "actor_mon.hpp"
#include "inventory.hpp"
#include "map.hpp"
#include "explosion.hpp"
#include "player_bon.hpp"
#include "map_parsing.hpp"
#include "line_calc.hpp"
#include "actor_factory.hpp"
#include "utils.hpp"
#include "feature_rigid.hpp"
#include "feature_mob.hpp"
#include "item.hpp"
#include "text_format.hpp"
#include "save_handling.hpp"
#include "dungeon_master.hpp"

namespace prop_data
{

Prop_data_t data[size_t(Prop_id::END)];

namespace
{

void add_prop_data(Prop_data_t& d)
{
    data[int(d.id)] = d;
    Prop_data_t blank;
    d = blank;
}

void init_data_list()
{
    Prop_data_t d;

    d.id = Prop_id::rPhys;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Physical Resistance";
    d.name_short = "rPhys";
    d.msg[size_t(Prop_msg::start_player)] = "I feel resistant to physical harm.";
    d.msg[size_t(Prop_msg::start_mon)] = "is resistant to physical harm.";
    d.msg[size_t(Prop_msg::end_player)] = "I feel vulnerable to physical harm.";
    d.msg[size_t(Prop_msg::end_mon)] = "is vulnerable to physical harm.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::good;
    add_prop_data(d);

    d.id = Prop_id::rFire;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Fire resistance";
    d.name_short = "rFire";
    d.msg[size_t(Prop_msg::start_player)] = "I feel resistant to fire.";
    d.msg[size_t(Prop_msg::start_mon)] = "is resistant to fire.";
    d.msg[size_t(Prop_msg::end_player)] = "I feel vulnerable to fire.";
    d.msg[size_t(Prop_msg::end_mon)] = "is vulnerable to fire.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::good;
    add_prop_data(d);

    d.id = Prop_id::rCold;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Cold resistance";
    d.name_short = "rCold";
    d.msg[size_t(Prop_msg::start_player)] = "I feel resistant to cold.";
    d.msg[size_t(Prop_msg::start_mon)] = "is resistant to cold.";
    d.msg[size_t(Prop_msg::end_player)] = "I feel vulnerable to cold.";
    d.msg[size_t(Prop_msg::end_mon)] = "is vulnerable to cold.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::good;
    add_prop_data(d);

    d.id = Prop_id::rPoison;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Poison resistance";
    d.name_short = "rPoison";
    d.msg[size_t(Prop_msg::start_player)] = "I feel resistant to poison.";
    d.msg[size_t(Prop_msg::start_mon)] = "is resistant to poison.";
    d.msg[size_t(Prop_msg::end_player)] = "I feel vulnerable to poison.";
    d.msg[size_t(Prop_msg::end_mon)] = "is vulnerable to poison.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::good;
    add_prop_data(d);

    d.id = Prop_id::rElec;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Electric resistance";
    d.name_short = "rElec";
    d.msg[size_t(Prop_msg::start_player)] = "I feel resistant to electricity.";
    d.msg[size_t(Prop_msg::start_mon)] = "is resistant to electricity.";
    d.msg[size_t(Prop_msg::end_player)] = "I feel vulnerable to electricity.";
    d.msg[size_t(Prop_msg::end_mon)] = "is vulnerable to electricity.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::good;
    add_prop_data(d);

    d.id = Prop_id::rAcid;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Acid resistance";
    d.name_short = "rAcid";
    d.msg[size_t(Prop_msg::start_player)] = "I feel resistant to acid.";
    d.msg[size_t(Prop_msg::start_mon)] = "is resistant to acid.";
    d.msg[size_t(Prop_msg::end_player)] = "I feel vulnerable to acid.";
    d.msg[size_t(Prop_msg::end_mon)] = "is vulnerable to acid.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::good;
    add_prop_data(d);

    d.id = Prop_id::rSleep;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Sleep resistance";
    d.name_short = "rSleep";
    d.msg[size_t(Prop_msg::start_player)] = "I feel resistant to sleep.";
    d.msg[size_t(Prop_msg::start_mon)] = "is resistant to sleep.";
    d.msg[size_t(Prop_msg::end_player)] = "I feel vulnerable to sleep.";
    d.msg[size_t(Prop_msg::end_mon)] = "is vulnerable to sleep.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::good;
    add_prop_data(d);

    d.id = Prop_id::rFear;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Fear resistance";
    d.name_short = "rFear";
    d.msg[size_t(Prop_msg::start_player)] = "I feel resistant to fear.";
    d.msg[size_t(Prop_msg::start_mon)] = "is resistant to fear.";
    d.msg[size_t(Prop_msg::end_player)] = "I feel vulnerable to fear.";
    d.msg[size_t(Prop_msg::end_mon)] = "is vulnerable to fear.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::good;
    add_prop_data(d);

    d.id = Prop_id::rConf;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Confusion resistance";
    d.name_short = "rConf";
    d.msg[size_t(Prop_msg::start_player)] = "I feel resistant to confusion.";
    d.msg[size_t(Prop_msg::start_mon)] = "is resistant to confusion.";
    d.msg[size_t(Prop_msg::end_player)] = "I feel vulnerable to confusion.";
    d.msg[size_t(Prop_msg::end_mon)] = "is vulnerable to confusion.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::good;
    add_prop_data(d);

    d.id = Prop_id::rDisease;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Disease resistance";
    d.name_short = "rDisease";
    d.msg[size_t(Prop_msg::start_player)] = "I feel resistant to disease.";
    d.msg[size_t(Prop_msg::start_mon)] = "is resistant to disease.";
    d.msg[size_t(Prop_msg::end_player)] = "I feel vulnerable to disease.";
    d.msg[size_t(Prop_msg::end_mon)] = "is vulnerable to disease.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::good;
    add_prop_data(d);

    d.id = Prop_id::rBreath;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Breath resistance";
    d.name_short = "rBreath";
    d.msg[size_t(Prop_msg::start_player)] = "I can breath without harm.";
    d.msg[size_t(Prop_msg::start_mon)] = "can breath without harm.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::good;
    add_prop_data(d);

    d.id = Prop_id::lgtSens;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Light sensitive";
    d.name_short = "lgtSensitive";
    d.msg[size_t(Prop_msg::start_player)] = "I feel vulnerable to light!";
    d.msg[size_t(Prop_msg::start_mon)] = "is vulnerable to light.";
    d.msg[size_t(Prop_msg::end_player)] = "I no longer feel vulnerable to light.";
    d.msg[size_t(Prop_msg::end_mon)] = "no longer is vulnerable to light.";
    d.msg[size_t(Prop_msg::res_player)] = "";
    d.msg[size_t(Prop_msg::res_mon)] = "";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.alignment = Prop_alignment::bad;
    add_prop_data(d);

    d.id = Prop_id::blind;
    d.std_rnd_turns = Range(20, 30);
    d.name = "Blind";
    d.name_short = "Blind";
    d.msg[size_t(Prop_msg::start_player)] = "I am blinded!";
    d.msg[size_t(Prop_msg::start_mon)] = "is blinded.";
    d.msg[size_t(Prop_msg::end_player)] = "I can see again!";
    d.msg[size_t(Prop_msg::end_mon)] = "can see again.";
    d.msg[size_t(Prop_msg::res_player)] = "I resist blindness.";
    d.msg[size_t(Prop_msg::res_mon)] = "resists blindness.";
    d.is_making_mon_aware = true;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.is_ended_by_magic_healing = true;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::bad;
    add_prop_data(d);

    d.id = Prop_id::fainted;
    d.std_rnd_turns = Range(100, 200);
    d.name = "Fainted";
    d.name_short = "Fainted";
    d.msg[size_t(Prop_msg::start_player)] = "I faint!";
    d.msg[size_t(Prop_msg::start_mon)] = "faints.";
    d.msg[size_t(Prop_msg::end_player)] = "I am awake.";
    d.msg[size_t(Prop_msg::end_mon)] = "wakes up.";
    d.msg[size_t(Prop_msg::res_player)] = "I resist fainting.";
    d.msg[size_t(Prop_msg::res_mon)] = "resists fainting.";
    d.is_making_mon_aware = true;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::bad;
    add_prop_data(d);

    d.id = Prop_id::burning;
    d.std_rnd_turns = Range(5, 10);
    d.name = "Burning";
    d.name_short = "Burning";
    d.msg[size_t(Prop_msg::start_player)] = "I am Burning!";
    d.msg[size_t(Prop_msg::start_mon)] = "is burning.";
    d.msg[size_t(Prop_msg::end_player)] = "The flames are put out.";
    d.msg[size_t(Prop_msg::end_mon)] = "is no longer burning.";
    d.msg[size_t(Prop_msg::res_player)] = "I resist burning.";
    d.msg[size_t(Prop_msg::res_mon)] = "resists burning.";
    d.is_making_mon_aware = true;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = true;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::bad;
    add_prop_data(d);

    d.id = Prop_id::poisoned;
    d.std_rnd_turns = Range(30, 60);
    d.name = "Poisoned";
    d.name_short = "Poisoned";
    d.msg[size_t(Prop_msg::start_player)] = "I am poisoned!";
    d.msg[size_t(Prop_msg::start_mon)] = "is poisoned.";
    d.msg[size_t(Prop_msg::end_player)] = "My body is cleansed from poisoning!";
    d.msg[size_t(Prop_msg::end_mon)] = "is cleansed from poisoning.";
    d.msg[size_t(Prop_msg::res_player)] = "I resist poisoning.";
    d.msg[size_t(Prop_msg::res_mon)] = "resists poisoning.";
    d.is_making_mon_aware = true;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.is_ended_by_magic_healing = true;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::bad;
    add_prop_data(d);

    d.id = Prop_id::paralyzed;
    d.std_rnd_turns = Range(7, 9);
    d.name = "Paralyzed";
    d.name_short = "Paralyzed";
    d.msg[size_t(Prop_msg::start_player)] = "I am paralyzed!";
    d.msg[size_t(Prop_msg::start_mon)] = "is paralyzed.";
    d.msg[size_t(Prop_msg::end_player)] = "I can move again!";
    d.msg[size_t(Prop_msg::end_mon)] = "can move again.";
    d.msg[size_t(Prop_msg::res_player)] = "I resist paralyzation.";
    d.msg[size_t(Prop_msg::res_mon)] = "resists paralyzation.";
    d.is_making_mon_aware = true;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = false;
    d.update_vision_when_start_or_end = false;
    d.is_ended_by_magic_healing = true;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::bad;
    add_prop_data(d);

    d.id = Prop_id::terrified;
    d.std_rnd_turns = Range(5, 12);
    d.name = "Terrified";
    d.name_short = "Terrified";
    d.msg[size_t(Prop_msg::start_player)] = "I am terrified!";
    d.msg[size_t(Prop_msg::start_mon)] = "looks terrified.";
    d.msg[size_t(Prop_msg::end_player)] = "I am no longer terrified!";
    d.msg[size_t(Prop_msg::end_mon)] = "is no longer terrified.";
    d.msg[size_t(Prop_msg::res_player)] = "I resist fear.";
    d.msg[size_t(Prop_msg::res_mon)] = "resists fear.";
    d.is_making_mon_aware = true;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::bad;
    add_prop_data(d);

    d.id = Prop_id::confused;
    d.std_rnd_turns = Range(80, 120);
    d.name = "Confused";
    d.name_short = "Confused";
    d.msg[size_t(Prop_msg::start_player)] = "I am confused!";
    d.msg[size_t(Prop_msg::start_mon)] = "looks confused.";
    d.msg[size_t(Prop_msg::end_player)] = "I am no longer confused.";
    d.msg[size_t(Prop_msg::end_mon)] = "is no longer confused.";
    d.msg[size_t(Prop_msg::res_player)] = "I resist confusion.";
    d.msg[size_t(Prop_msg::res_mon)] = "resists confusion.";
    d.is_making_mon_aware = true;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::bad;
    add_prop_data(d);

    d.id = Prop_id::stunned;
    d.std_rnd_turns = Range(5, 9);
    d.name = "Stunned";
    d.name_short = "Stunned";
    d.msg[size_t(Prop_msg::start_player)] = "I am stunned!";
    d.msg[size_t(Prop_msg::start_mon)] = "is stunned.";
    d.msg[size_t(Prop_msg::end_player)] = "I am no longer stunned.";
    d.msg[size_t(Prop_msg::end_mon)] = "is no longer stunned.";
    d.msg[size_t(Prop_msg::res_player)] = "I resist stunning.";
    d.msg[size_t(Prop_msg::res_mon)] = "resists stunning.";
    d.is_making_mon_aware = true;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.is_ended_by_magic_healing = true;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::bad;
    add_prop_data(d);

    d.id = Prop_id::slowed;
    d.std_rnd_turns = Range(9, 12);
    d.name = "Slowed";
    d.name_short = "Slowed";
    d.msg[size_t(Prop_msg::start_player)] = "Everything around me seems to speed up.";
    d.msg[size_t(Prop_msg::start_mon)] = "slows down.";
    d.msg[size_t(Prop_msg::end_player)] = "Everything around me seems to slow down.";
    d.msg[size_t(Prop_msg::end_mon)] = "speeds up.";
    d.msg[size_t(Prop_msg::res_player)] = "I resist slowness.";
    d.msg[size_t(Prop_msg::res_mon)] = "resists slowness.";
    d.is_making_mon_aware = true;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::bad;
    add_prop_data(d);

    d.id = Prop_id::hasted;
    d.std_rnd_turns = Range(9, 12);
    d.name = "Hasted";
    d.name_short = "Hasted";
    d.msg[size_t(Prop_msg::start_player)] = "Everything around me seems to slow down.";
    d.msg[size_t(Prop_msg::start_mon)] = "speeds up.";
    d.msg[size_t(Prop_msg::end_player)] = "Everything around me seems to speed up.";
    d.msg[size_t(Prop_msg::end_mon)] = "slows down.";
    d.is_making_mon_aware = true;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::good;
    add_prop_data(d);

    d.id = Prop_id::flared;
    d.std_rnd_turns = Range(3, 4);
    d.msg[size_t(Prop_msg::start_mon)] = "is perforated by a flare!";
    d.is_making_mon_aware = true;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = true;
    d.alignment = Prop_alignment::bad;
    add_prop_data(d);

    d.id = Prop_id::nailed;
    d.name = "Nailed";
    d.msg[size_t(Prop_msg::start_player)] = "I am fastened by a spike!";
    d.msg[size_t(Prop_msg::start_mon)] = "is fastened by a spike.";
    d.msg[size_t(Prop_msg::end_player)] = "I tear free!";
    d.msg[size_t(Prop_msg::end_mon)] = "tears free!";
    d.is_making_mon_aware = true;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.alignment = Prop_alignment::bad;
    add_prop_data(d);

    d.id = Prop_id::warlock_charged;
    d.std_rnd_turns = Range(1, 1);
    d.name = "Charged";
    d.name_short = "Charged";
    d.msg[size_t(Prop_msg::start_player)] = "I am burning with power!";
    d.is_making_mon_aware = false;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.alignment = Prop_alignment::good;
    add_prop_data(d);

    d.id = Prop_id::infected;
    d.std_rnd_turns = Range(400, 700);
    d.name = "Infected";
    d.name_short = "Infected";
    d.msg[size_t(Prop_msg::start_player)] = "I am infected!";
    d.msg[size_t(Prop_msg::start_mon)] = "is infected.";
    d.msg[size_t(Prop_msg::end_player)] = "My infection is cured!";
    d.msg[size_t(Prop_msg::end_mon)] = "is no longer infected.";
    d.msg[size_t(Prop_msg::res_player)] = "";
    d.msg[size_t(Prop_msg::res_mon)] = "";
    d.is_making_mon_aware = true;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.is_ended_by_magic_healing = true;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::bad;
    add_prop_data(d);

    d.id = Prop_id::diseased;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Diseased";
    d.name_short = "Diseased";
    d.msg[size_t(Prop_msg::start_player)] = "I am diseased!";
    d.msg[size_t(Prop_msg::start_mon)] = "is diseased.";
    d.msg[size_t(Prop_msg::end_player)] = "My disease is cured!";
    d.msg[size_t(Prop_msg::end_mon)] = "is no longer diseased.";
    d.msg[size_t(Prop_msg::res_player)] = "I resist disease.";
    d.msg[size_t(Prop_msg::res_mon)] = "resists disease.";
    d.is_making_mon_aware = true;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.is_ended_by_magic_healing = true;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::bad;
    add_prop_data(d);

    d.id = Prop_id::weakened;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Weakened";
    d.name_short = "Weakened";
    d.msg[size_t(Prop_msg::start_player)] = "I feel weaker.";
    d.msg[size_t(Prop_msg::start_mon)] = "looks weaker.";
    d.msg[size_t(Prop_msg::end_player)] = "I feel stronger!";
    d.msg[size_t(Prop_msg::end_mon)] = "looks stronger!";
    d.msg[size_t(Prop_msg::res_player)] = "I resist weakness.";
    d.msg[size_t(Prop_msg::res_mon)] = "resists weakness.";
    d.is_making_mon_aware = true;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.is_ended_by_magic_healing = true;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::bad;
    add_prop_data(d);

    d.id = Prop_id::frenzied;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Frenzied";
    d.name_short = "Frenzied";
    d.msg[size_t(Prop_msg::start_player)] = "I feel ferocious!";
    d.msg[size_t(Prop_msg::start_mon)] = "Looks ferocious!";
    d.msg[size_t(Prop_msg::end_player)] = "I feel calmer.";
    d.msg[size_t(Prop_msg::end_mon)] = "Looks calmer.";
    d.msg[size_t(Prop_msg::res_player)] = "";
    d.msg[size_t(Prop_msg::res_mon)] = "";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::neutral;
    add_prop_data(d);

    d.id = Prop_id::blessed;
    d.std_rnd_turns = Range(400, 600);
    d.name = "Blessed";
    d.name_short = "Blessed";
    d.msg[size_t(Prop_msg::start_player)] = "I feel luckier.";
    d.msg[size_t(Prop_msg::end_player)] = "I have normal luck.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::good;
    add_prop_data(d);

    d.id = Prop_id::cursed;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Cursed";
    d.name_short = "Cursed";
    d.msg[size_t(Prop_msg::start_player)] = "I feel misfortunate.";
    d.msg[size_t(Prop_msg::end_player)] = "I feel more fortunate.";
    d.msg[size_t(Prop_msg::res_player)] = "I resist misfortune.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::bad;
    add_prop_data(d);

    d.id = Prop_id::radiant;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Radiant";
    d.name_short = "Radiant";
    d.msg[size_t(Prop_msg::start_player)] = "A bright light shines around me.";
    d.msg[size_t(Prop_msg::end_player)] = "It suddenly seems darker.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = true;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::neutral;
    add_prop_data(d);

    d.id = Prop_id::invis;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Invisible";
    d.name_short = "Invis";
    d.msg[size_t(Prop_msg::start_player)] = "I am out of sight for normal eyes!";
    d.msg[size_t(Prop_msg::start_mon)] = "is out of sight for normal eyes!";
    d.msg[size_t(Prop_msg::end_player)] = "I am visible to normal eyes.";
    d.msg[size_t(Prop_msg::end_mon)] = "is visible to normal eyes.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = true;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::good;
    add_prop_data(d);

    d.id = Prop_id::see_invis;
    d.std_rnd_turns = Range(50, 100);
    d.name = "See Invisible";
    d.name_short = "SeeInvis";
    d.msg[size_t(Prop_msg::start_player)] = "My eyes perceive the invisible.";
    d.msg[size_t(Prop_msg::start_mon)] = "seems to see very clearly.";
    d.msg[size_t(Prop_msg::end_player)] = "My eyes can no longer perceive the invisible.";
    d.msg[size_t(Prop_msg::end_mon)] = "seems to see less clearly.";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = true;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::good;
    add_prop_data(d);

    d.id = Prop_id::infravis;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Infravision";
    d.name_short = "Infravis";
    d.msg[size_t(Prop_msg::start_player)] = "I have infravision.";
    d.msg[size_t(Prop_msg::end_player)] = "I no longer have infravision.";
    d.msg[size_t(Prop_msg::res_player)] = "";
    d.msg[size_t(Prop_msg::res_mon)] = "";
    d.is_making_mon_aware = false;
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = true;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::good;
    add_prop_data(d);

    d.id = Prop_id::tele_ctrl;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Teleport Control";
    d.name_short = "TeleCtrl";
    d.msg[size_t(Prop_msg::start_player)] = "I feel in control.";
    d.msg[size_t(Prop_msg::end_player)] = "I feel less in control.";
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = false;
    d.alignment = Prop_alignment::good;
    add_prop_data(d);

    d.id = Prop_id::spell_reflect;
    d.std_rnd_turns = Range(50, 100);
    d.name = "Spell Reflection";
    d.name_short = "SpellRefl";
    d.msg[size_t(Prop_msg::start_player)] = "Spells reflect off me!";
    d.msg[size_t(Prop_msg::end_player)] = "I am vulnerable to spells.";
    d.allow_display_turns = true;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::good;
    add_prop_data(d);

    d.id = Prop_id::fast_shooting;
    d.std_rnd_turns = Range(1, 1);
    d.name = "";
    d.name_short = "";
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.alignment = Prop_alignment::good;
    add_prop_data(d);

    d.id = Prop_id::strangled;
    d.std_rnd_turns = Range(1, 1);
    d.name = "Strangled";
    d.name_short = "Strangled";
    d.is_making_mon_aware = true;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.alignment = Prop_alignment::bad;
    add_prop_data(d);

    d.id = Prop_id::conflict;
    d.std_rnd_turns = Range(10, 20);
    d.msg[size_t(Prop_msg::start_mon)] = "Looks conflicted.";
    d.msg[size_t(Prop_msg::end_mon)] = "Looks more determined.";
    d.is_making_mon_aware = true;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.alignment = Prop_alignment::bad;
    add_prop_data(d);

    d.id = Prop_id::poss_by_zuul;
    d.is_making_mon_aware = false;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = false;
    d.update_vision_when_start_or_end = true;
    d.allow_test_on_bot = true;
    d.alignment = Prop_alignment::neutral;
    add_prop_data(d);

    d.id = Prop_id::flying;
    d.is_making_mon_aware = false;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = false;
    d.alignment = Prop_alignment::neutral;
    add_prop_data(d);

    d.id = Prop_id::ethereal;
    d.is_making_mon_aware = false;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = false;
    d.alignment = Prop_alignment::neutral;
    add_prop_data(d);

    d.id = Prop_id::ooze;
    d.is_making_mon_aware = false;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = false;
    d.alignment = Prop_alignment::neutral;
    add_prop_data(d);

    d.id = Prop_id::burrowing;
    d.is_making_mon_aware = false;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.allow_test_on_bot = false;
    d.alignment = Prop_alignment::neutral;
    add_prop_data(d);

    d.id = Prop_id::waiting;
    d.std_rnd_turns = Range(1, 1);
    d.is_making_mon_aware = false;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.alignment = Prop_alignment::neutral;
    add_prop_data(d);

    d.id = Prop_id::disabled_attack;
    d.std_rnd_turns = Range(1, 1);
    d.is_making_mon_aware = false;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.alignment = Prop_alignment::neutral;
    add_prop_data(d);

    d.id = Prop_id::disabled_melee;
    d.std_rnd_turns = Range(1, 1);
    d.is_making_mon_aware = false;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.alignment = Prop_alignment::neutral;
    add_prop_data(d);

    d.id = Prop_id::disabled_ranged;
    d.std_rnd_turns = Range(1, 1);
    d.is_making_mon_aware = false;
    d.allow_display_turns = false;
    d.allow_apply_more_while_active = true;
    d.update_vision_when_start_or_end = false;
    d.alignment = Prop_alignment::neutral;
    add_prop_data(d);
}

} //namespace

void init()
{
    init_data_list();
}

} //Prop_data

//-----------------------------------------------------------------------------
// Property handler
//-----------------------------------------------------------------------------
Prop_handler::Prop_handler(Actor* owning_actor) :
    owning_actor_(owning_actor)
{
    //Reset the active props info
    for (size_t i = 0; i < size_t(Prop_id::END); ++i)
    {
        active_props_info_[i] = 0;
    }
}

void Prop_handler::init_natural_props()
{
    const Actor_data_t& d = owning_actor_->data();

    //Add natural properties
    for (size_t i = 0; i < size_t(Prop_id::END); ++i)
    {
        active_props_info_[i] = 0;

        if (d.natural_props[i])
        {
            Prop* const prop = mk_prop(Prop_id(i), Prop_turns::indefinite);
            try_add_prop(prop, Prop_src::intr, true, Verbosity::silent);
        }
    }
}

Prop_handler::~Prop_handler()
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
    for (size_t i = 0; i < size_t(Prop_id::END); ++i)
    {
        if (active_props_info_[i] != 0)
        {
            TRACE << "Active property info at id " << i << " not zero" << std::endl;
            assert(false);
        }
    }
#endif // NDEBUG

    for (Prop* prop : actor_turn_prop_buffer_)
    {
        delete prop;
    }
}

void Prop_handler::save() const
{
    //Save intrinsic properties to file

    int nr_intr_props_ = 0;

    for (Prop* prop : props_)
    {
        if (prop->src_ == Prop_src::intr)
        {
            ++nr_intr_props_;
        }
    }

    save_handling::put_int(nr_intr_props_);

    for (Prop* prop : props_)
    {
        if (prop->src_ == Prop_src::intr)
        {
            save_handling::put_int(int(prop->id()));
            save_handling::put_int(prop->nr_turns_left_);
        }
    }
}

void Prop_handler::load()
{
    //Load intrinsic properties from file

    const int NR_PROPS = save_handling::get_int();

    for (int i = 0; i < NR_PROPS; ++i)
    {
        const auto prop_id = Prop_id(save_handling::get_int());

        const int NR_TURNS = save_handling::get_int();

        const auto turns_init = NR_TURNS == -1 ?
                                Prop_turns::indefinite : Prop_turns::specific;

        Prop* const prop = mk_prop(prop_id, turns_init, NR_TURNS);

        prop->src_ = Prop_src::intr;

        props_.push_back(prop);

        incr_active_props_info(prop_id);
    }
}

Prop* Prop_handler::mk_prop(const Prop_id id, Prop_turns turns_init, const int NR_TURNS) const
{
    assert(id != Prop_id::END);

    //Prop turns init type should either be:
    // * "specific", and number of turns specified (> 0), OR
    // * NOT "specific" (i.e. "indefinite" or "std"), and number of turns NOT specified (-1)
    assert((turns_init == Prop_turns::specific && NR_TURNS > 0) ||
           (turns_init != Prop_turns::specific && NR_TURNS == -1));

    switch (id)
    {
    case Prop_id::nailed:
        return new Prop_nailed(turns_init, NR_TURNS);

    case Prop_id::warlock_charged:
        return new Prop_warlock_charged(turns_init, NR_TURNS);

    case Prop_id::blind:
        return new Prop_blind(turns_init, NR_TURNS);

    case Prop_id::burning:
        return new Prop_burning(turns_init, NR_TURNS);

    case Prop_id::flared:
        return new Prop_flared(turns_init, NR_TURNS);

    case Prop_id::paralyzed:
        return new Prop_paralyzed(turns_init, NR_TURNS);

    case Prop_id::terrified:
        return new Prop_terrified(turns_init, NR_TURNS);

    case Prop_id::weakened:
        return new Prop_weakened(turns_init, NR_TURNS);

    case Prop_id::confused:
        return new Prop_confused(turns_init, NR_TURNS);

    case Prop_id::stunned:
        return new Prop_stunned(turns_init, NR_TURNS);

    case Prop_id::waiting:
        return new Prop_waiting(turns_init, NR_TURNS);

    case Prop_id::slowed:
        return new Prop_slowed(turns_init, NR_TURNS);

    case Prop_id::hasted:
        return new Prop_hasted(turns_init, NR_TURNS);

    case Prop_id::infected:
        return new Prop_infected(turns_init, NR_TURNS);

    case Prop_id::diseased:
        return new Prop_diseased(turns_init, NR_TURNS);

    case Prop_id::poisoned:
        return new Prop_poisoned(turns_init, NR_TURNS);

    case Prop_id::fainted:
        return new Prop_fainted(turns_init, NR_TURNS);

    case Prop_id::frenzied:
        return new Prop_frenzied(turns_init, NR_TURNS);

    case Prop_id::fast_shooting:
        return new Prop_fast_shooting(turns_init, NR_TURNS);

    case Prop_id::disabled_attack:
        return new Prop_disabled_attack(turns_init, NR_TURNS);

    case Prop_id::disabled_melee:
        return new Prop_disabled_melee(turns_init, NR_TURNS);

    case Prop_id::disabled_ranged:
        return new Prop_disabled_ranged(turns_init, NR_TURNS);

    case Prop_id::blessed:
        return new Prop_blessed(turns_init, NR_TURNS);

    case Prop_id::cursed:
        return new Prop_cursed(turns_init, NR_TURNS);

    case Prop_id::rAcid:
        return new Prop_rAcid(turns_init, NR_TURNS);

    case Prop_id::rCold:
        return new Prop_rCold(turns_init, NR_TURNS);

    case Prop_id::rConf:
        return new Prop_rConf(turns_init, NR_TURNS);

    case Prop_id::rBreath:
        return new Prop_rBreath(turns_init, NR_TURNS);

    case Prop_id::rElec:
        return new Prop_rElec(turns_init, NR_TURNS);

    case Prop_id::rFear:
        return new Prop_rFear(turns_init, NR_TURNS);

    case Prop_id::rPhys:
        return new Prop_rPhys(turns_init, NR_TURNS);

    case Prop_id::rFire:
        return new Prop_rFire(turns_init, NR_TURNS);

    case Prop_id::rPoison:
        return new Prop_rPoison(turns_init, NR_TURNS);

    case Prop_id::rSleep:
        return new Prop_rSleep(turns_init, NR_TURNS);

    case Prop_id::lgtSens:
        return new Prop_lgtSens(turns_init, NR_TURNS);

    case Prop_id::poss_by_zuul:
        return new Prop_poss_by_zuul(turns_init, NR_TURNS);

    case Prop_id::flying:
        return new Prop_flying(turns_init, NR_TURNS);

    case Prop_id::ethereal:
        return new Prop_ethereal(turns_init, NR_TURNS);

    case Prop_id::ooze:
        return new Prop_ooze(turns_init, NR_TURNS);

    case Prop_id::burrowing:
        return new Prop_burrowing(turns_init, NR_TURNS);

    case Prop_id::radiant:
        return new Prop_radiant(turns_init, NR_TURNS);

    case Prop_id::infravis:
        return new Prop_infravis(turns_init, NR_TURNS);

    case Prop_id::rDisease:
        return new Prop_rDisease(turns_init, NR_TURNS);

    case Prop_id::tele_ctrl:
        return new Prop_tele_control(turns_init, NR_TURNS);

    case Prop_id::spell_reflect:
        return new Prop_spell_reflect(turns_init, NR_TURNS);

    case Prop_id::strangled:
        return new Prop_strangled(turns_init, NR_TURNS);

    case Prop_id::conflict:
        return new Prop_conflict(turns_init, NR_TURNS);

    case Prop_id::invis:
        return new Prop_invisible(turns_init, NR_TURNS);

    case Prop_id::see_invis:
        return new Prop_see_invis(turns_init, NR_TURNS);

    case Prop_id::END:
        break;
    }

    return nullptr;
}

void Prop_handler::try_add_prop(Prop* const prop,
                                Prop_src src,
                                const bool FORCE_EFFECT,
                                const Verbosity verbosity)
{
    assert(prop);

    //First, if this is a prop that runs on actor turns, check if the actor-turn prop buffer
    //does not already contain the prop:
    // - If it doesn't, then just add it to the buffer and return.
    // - If the buffer already contains the prop, it means it was requested to be applied from the
    //   buffer to the applied props.
    //This way, this function can be used both for requesting to apply props, and for applying
    //props from the buffer.
    if (prop->turn_mode() == Prop_turn_mode::actor)
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

    const bool IS_PLAYER        = owning_actor_->is_player();
    const bool PLAYER_SEE_OWNER = map::player->can_see_actor(*owning_actor_);

    //Check if property is resisted
    if (!FORCE_EFFECT)
    {
        if (try_resist_prop(prop->id()))
        {
            if (verbosity == Verbosity::verbose)
            {
                if (IS_PLAYER)
                {
                    std::string msg = "";
                    prop->msg(Prop_msg::res_player, msg);

                    if (!msg.empty())
                    {
                        msg_log::add(msg, clr_white, true);
                    }
                }
                else //Is a monster
                {
                    if (PLAYER_SEE_OWNER)
                    {
                        std::string msg = "";
                        prop->msg(Prop_msg::res_mon, msg);

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

    //Is this an intrinsic property, and actor already has an intrinsic property of the same type?
    //If so, the new property will just be "merged" into the old one ("on_more()").
    if (prop->src_ == Prop_src::intr)
    {
        for (Prop* old_prop : props_)
        {
            if (old_prop->src_ == Prop_src::intr && prop->id() == old_prop->id())
            {
                if (!prop->allow_apply_more_while_active())
                {
                    delete prop;
                    return;
                }

                const int TURNS_LEFT_OLD = old_prop->nr_turns_left_;
                const int TURNS_LEFT_NEW = prop->nr_turns_left_;

                //Print start message (again)
                if (verbosity == Verbosity::verbose)
                {
                    if (IS_PLAYER)
                    {
                        std::string msg = "";
                        prop->msg(Prop_msg::start_player, msg);

                        if (!msg.empty())
                        {
                            msg_log::add(msg, clr_white, true);
                        }
                    }
                    else //Not player
                    {
                        if (PLAYER_SEE_OWNER)
                        {
                            std::string msg = "";
                            prop->msg(Prop_msg::start_mon, msg);

                            if (!msg.empty())
                            {
                                msg_log::add(owning_actor_->name_the() + " " + msg);
                            }
                        }
                    }
                }

                old_prop->on_more();

                old_prop->nr_turns_left_ = (TURNS_LEFT_OLD < 0 || TURNS_LEFT_NEW < 0) ? -1 :
                                           std::max(TURNS_LEFT_OLD, TURNS_LEFT_NEW);
                delete prop;
                return;
            }
        }
    }

    //This part reached means the property should be applied on its own

    props_.push_back(prop);

    prop->on_start();

    if (verbosity == Verbosity::verbose)
    {
        if (prop->need_update_vision_when_start_or_end())
        {
            prop->owning_actor_->update_clr();
            game_time::update_light_map();
            map::player->update_fov();
            render::draw_map_and_interface();
        }

        if (IS_PLAYER)
        {
            std::string msg = "";
            prop->msg(Prop_msg::start_player, msg);

            if (!msg.empty())
            {
                msg_log::add(msg, clr_white, true);
            }
        }
        else //Is monster
        {
            if (PLAYER_SEE_OWNER)
            {
                std::string msg = "";
                prop->msg(Prop_msg::start_mon, msg);

                if (!msg.empty())
                {
                    msg_log::add(owning_actor_->name_the() + " " + msg);
                }
            }
        }
    }

    incr_active_props_info(prop->id());
}

void Prop_handler::add_prop_from_equipped_item(const Item* const item,
        Prop* const prop,
        const Verbosity verbosity)
{
    prop->item_applying_ = item;

    try_add_prop(prop, Prop_src::inv, true, verbosity);
}

Prop* Prop_handler::prop(const Prop_id id) const
{
    for (Prop* const prop : props_)
    {
        if (prop->id() == id)
        {
            return prop;
        }
    }

    return nullptr;
}

void Prop_handler::remove_props_for_item(const Item* const item)
{
    for (size_t i = 0; i < props_.size(); /* No increment */)
    {
        Prop* const prop = props_[i];

        if (prop->item_applying_ == item)
        {
            assert(prop->src_ == Prop_src::inv);
            assert(prop->turns_init_type_ == Prop_turns::indefinite);

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

void Prop_handler::try_add_prop_from_att(const Wpn& wpn, const bool IS_MELEE)
{
    const auto&         d           = wpn.data();
    const auto* const   origin_prop = IS_MELEE ?
                                      d.melee.prop_applied : d.ranged.prop_applied;

    if (origin_prop)
    {
        //If weapon damage is resisted by the defender, the property is automatically resisted
        const Dmg_type dmg_type = IS_MELEE ?
                                  d.melee.dmg_type : d.ranged.dmg_type;

        if (!try_resist_dmg(dmg_type, Verbosity::silent))
        {
            const auto turns_init_type = origin_prop->turns_init_type();

            const int NR_TURNS = turns_init_type == Prop_turns::specific ?
                                 origin_prop->nr_turns_left_ : -1;

            //Make a copy of the weapon effect
            auto * const prop_cpy = mk_prop(origin_prop->id(),
                                            origin_prop->turns_init_type_,
                                            NR_TURNS);

            try_add_prop(prop_cpy);
        }
    }
}

void Prop_handler::incr_active_props_info(const Prop_id id)
{
    int& v = active_props_info_[size_t(id)];

#ifndef NDEBUG
    if (v < 0)
    {
        TRACE << "Tried to increment property with current value " << v << std::endl;
        assert(false);
    }
#endif // NDEBUG

    ++v;
}

void Prop_handler::decr_active_props_info(const Prop_id id)
{
    int& v = active_props_info_[size_t(id)];

#ifndef NDEBUG
    if (v <= 0)
    {
        TRACE << "Tried to decrement property with current value " << v << std::endl;
        assert(false);
    }
#endif // NDEBUG

    --v;
}

void Prop_handler::on_prop_end(Prop* const prop)
{
    if (prop->need_update_vision_when_start_or_end())
    {
        prop->owning_actor_->update_clr();
        game_time::update_light_map();
        map::player->update_fov();
        render::draw_map_and_interface();
    }

    //Print property end message if this is the last active property of this type
    if (
        owning_actor_->state() == Actor_state::alive &&
        active_props_info_[size_t(prop->id_)] == 0)
    {
        if (owning_actor_->is_player())
        {
            std::string msg = "";

            prop->msg(Prop_msg::end_player, msg);

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

                prop->msg(Prop_msg::end_mon, msg);

                if (!msg.empty())
                {
                    msg_log::add(owning_actor_->name_the() + " " + msg);
                }
            }
        }
    }

    prop->on_end();
}

bool Prop_handler::end_prop(const Prop_id id, const bool RUN_PROP_END_EFFECTS)
{
    int     idx     = -1;
    Prop*   prop    = nullptr;

    for (size_t i = 0; i < props_.size(); ++i)
    {
        prop = props_[i];

        if (prop->id() == id && prop->src_ == Prop_src::intr)
        {
            idx = i;
            break;
        }
    }

    if (idx == -1)
    {
        return false;
    }

    props_.erase(begin(props_) + idx);

    decr_active_props_info(prop->id());

    if (RUN_PROP_END_EFFECTS)
    {
        on_prop_end(prop);
    }

    delete prop;
    return true;
}

bool Prop_handler::end_props_by_magic_healing()
{
    bool is_any_ended = false;

    for (size_t i = 0; i < props_.size(); /* No increment */)
    {
        Prop* const prop = props_[i];

        if (prop->is_ended_by_magic_healing())
        {
            props_.erase(begin(props_) + i);

            decr_active_props_info(prop->id());

            on_prop_end(prop);

            delete prop;

            is_any_ended = true;
        }
        else //Property was not added by this item
        {
            ++i;
        }
    }

    return is_any_ended;
}

void Prop_handler::apply_actor_turn_prop_buffer()
{
    for (Prop* prop : actor_turn_prop_buffer_)
    {
        try_add_prop(prop);
    }

    actor_turn_prop_buffer_.clear();
}

void Prop_handler::tick(const Prop_turn_mode turn_mode)
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

                mon->become_aware(false);
            }

            //Count down number of turns
            if (prop->nr_turns_left_ > 0)
            {
                assert(prop->src_ == Prop_src::intr);

                prop->nr_turns_left_ -= 1;
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
                prop->on_new_turn();
            }
        }

        //Property has not been removed?
        if (prop)
        {
            ++i;
        }
    }
}

void Prop_handler::props_interface_line(std::vector<Str_and_clr>& line) const
{
    line.clear();

    for (Prop* prop : props_)
    {
        std::string str = prop->name_short();

        if (!str.empty())
        {
            const int TURNS_LEFT  = prop->nr_turns_left_;

            if (TURNS_LEFT > 0)
            {
                if (player_bon::traits[size_t(Trait::self_aware)])
                {
                    str += "(" + to_str(TURNS_LEFT) + ")";
                }
            }
            else //Property is indefinite
            {
                if (prop->src() == Prop_src::intr)
                {
                    //Indefinite intrinsic properties are printed in all upper case
                    text_format::all_to_upper(str);
                }
            }

            const Prop_alignment alignment = prop->alignment();

            const Clr clr = alignment == Prop_alignment::good ? clr_msg_good :
                            alignment == Prop_alignment::bad  ? clr_msg_bad :
                            clr_white;

            line.push_back(Str_and_clr(str, clr));
        }
    }
}

bool Prop_handler::try_resist_prop(const Prop_id id) const
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

bool Prop_handler::try_resist_dmg(const Dmg_type dmg_type, const Verbosity verbosity) const
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

bool Prop_handler::allow_see() const
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

int Prop_handler::change_max_hp(const int HP_MAX) const
{
    int new_hp_max = HP_MAX;

    for (Prop* prop : props_)
    {
        new_hp_max = prop->change_max_hp(new_hp_max);
    }

    return new_hp_max;
}

void Prop_handler::change_move_dir(const Pos& actor_pos, Dir& dir) const
{
    for (Prop* prop : props_)
    {
        prop->change_move_dir(actor_pos, dir);
    }
}

bool Prop_handler::allow_attack(const Verbosity verbosity) const
{
    for (Prop* prop : props_)
    {
        if (
            !prop->allow_attack_melee(verbosity) &&
            !prop->allow_attack_ranged(verbosity))
        {
            return false;
        }
    }

    return true;
}

bool Prop_handler::allow_attack_melee(const Verbosity verbosity) const
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

bool Prop_handler::allow_attack_ranged(const Verbosity verbosity) const
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

bool Prop_handler::allow_move() const
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

bool Prop_handler::allow_act() const
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

bool Prop_handler::allow_read(const Verbosity verbosity) const
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

bool Prop_handler::allow_cast_spell(const Verbosity verbosity) const
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

bool Prop_handler::allow_speak(const Verbosity verbosity) const
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

bool Prop_handler::allow_eat(const Verbosity verbosity) const
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

void Prop_handler::on_hit()
{
    for (Prop* prop : props_)
    {
        prop->on_hit();
    }
}

void Prop_handler::on_death(const bool IS_PLAYER_SEE_OWNING_ACTOR)
{
    for (Prop* prop : props_)
    {
        prop->on_death(IS_PLAYER_SEE_OWNING_ACTOR);
    }
}

int Prop_handler::ability_mod(const Ability_id ability) const
{
    int modifier = 0;

    for (Prop* prop : props_)
    {
        modifier += prop->ability_mod(ability);
    }

    return modifier;
}

bool Prop_handler::change_actor_clr(Clr& clr) const
{
    bool did_change_clr = false;

    for (Prop* prop : props_)
    {
        if (prop->change_actor_clr(clr))
        {
            did_change_clr = true;

            //It's probably more likely that a color change due to a bad property is critical
            //information (e.g. burning), so then we stop searching and use this color. If it's
            //a good or neutral property that affected the color, then we keep searching.
            if (prop->alignment() == Prop_alignment::bad)
            {
                break;
            }
        }
    }

    return did_change_clr;
}

//-----------------------------------------------------------------------------
// Properties
//-----------------------------------------------------------------------------
Prop::Prop(Prop_id id, Prop_turns turns_init, int nr_turns) :
    id_                 (id),
    data_               (prop_data::data[size_t(id)]),
    nr_turns_left_      (nr_turns),
    turns_init_type_    (turns_init),
    owning_actor_       (nullptr),
    src_                (Prop_src::END),
    item_applying_      (nullptr)
{
    switch (turns_init)
    {
    case Prop_turns::std:
#ifndef NDEBUG
        if (nr_turns_left_ != -1)
        {
            TRACE << "Prop turns is \"std\", but " << nr_turns_left_ << " turns specified"
                  << std::endl;
            assert(false);
        }
#endif // NDEBUG
        assert(nr_turns_left_ == -1);
        nr_turns_left_ = rnd::range(data_.std_rnd_turns);
        break;

    case Prop_turns::indefinite:
#ifndef NDEBUG
        if (nr_turns_left_ != -1)
        {
            TRACE << "Prop turns is \"indefinite\", but " << nr_turns_left_ << " turns specified"
                  << std::endl;
            assert(false);
        }
#endif // NDEBUG

        nr_turns_left_ = -1; //Robustness for release builds
        break;

    case Prop_turns::specific:
        //Use the number of turns specified in the ctor argument
        assert(nr_turns_left_ > 0);
        break;
    }
}

void Prop_blessed::on_start()
{
    owning_actor_->prop_handler().end_prop(Prop_id::cursed, false);
}

void Prop_cursed::on_start()
{
    owning_actor_->prop_handler().end_prop(Prop_id::blessed, false);

    //If this is a permanent curse that the player caught, log it as a historic event
    if (owning_actor_->is_player() && turns_init_type_ == Prop_turns::indefinite)
    {
        dungeon_master::add_history_event("A terrible curse was put upon me.");
    }
}

void Prop_cursed::on_end()
{
    //If this is a permanent curse that the player caught, log it as a historic event
    if (owning_actor_->is_player() && turns_init_type_ == Prop_turns::indefinite)
    {
        dungeon_master::add_history_event("A terrible curse was lifted from me.");
    }
}

void Prop_slowed::on_start()
{
    owning_actor_->prop_handler().end_prop(Prop_id::hasted, false);
}

void Prop_hasted::on_start()
{
    owning_actor_->prop_handler().end_prop(Prop_id::slowed, false);
}

void Prop_infected::on_new_turn()
{
    if (rnd::one_in(165))
    {
        Prop_handler& prop_hlr = owning_actor_->prop_handler();

        prop_hlr.try_add_prop(new Prop_diseased(Prop_turns::indefinite));

        prop_hlr.end_prop(Prop_id::infected, false);

        msg_log::more_prompt();
    }
}

int Prop_diseased::change_max_hp(const int HP_MAX) const
{
    if (owning_actor_->is_player() && player_bon::traits[size_t(Trait::survivalist)])
    {
        //Survavlist makes you lose only 25% instead of 50%
        return (HP_MAX * 3) / 4;
    }
    else //Not survivalist
    {
        return HP_MAX / 2;
    }
}

void Prop_diseased::on_start()
{
    //Actor::hp_max() will now return a decreased value. Cap current HP to the new (lower) maximum.
    int& hp = owning_actor_->hp_;

    hp = std::min(map::player->hp_max(true), hp);

    //If this is a permanent disease that the player caught, log it as a historic event
    if (owning_actor_->is_player() && turns_init_type_ == Prop_turns::indefinite)
    {
        dungeon_master::add_history_event("Caught a horrible disease.");
    }
}

void Prop_diseased::on_end()
{
    //If this is a permanent disease that the player caught, log it as a historic event
    if (owning_actor_->is_player() && turns_init_type_ == Prop_turns::indefinite)
    {
        dungeon_master::add_history_event("My body was cured from a horrible disease.");
    }
}

bool Prop_diseased::is_resisting_other_prop(const Prop_id prop_id) const
{
    //Getting infected while already diseased is just annoying
    return prop_id == Prop_id::infected;
}

void Prop_poss_by_zuul::on_death(const bool IS_PLAYER_SEE_OWNING_ACTOR)
{
    if (IS_PLAYER_SEE_OWNING_ACTOR)
    {
        const std::string& name1 = owning_actor_->name_the();
        const std::string& name2 = actor_data::data[int(Actor_id::zuul)].name_the;
        msg_log::add(name1 + " was possessed by " + name2 + "!");
    }

    owning_actor_->state_  = Actor_state::destroyed;
    const Pos& pos        = owning_actor_->pos;
    map::mk_gore(pos);
    map::mk_blood(pos);
    actor_factory::summon(pos, std::vector<Actor_id> {Actor_id::zuul}, true);

    //Zuul is now free, allow him to spawn.
    actor_data::data[size_t(Actor_id::zuul)].nr_left_allowed_to_spawn = -1;
}

void Prop_poisoned::on_new_turn()
{
    if (owning_actor_->is_alive())
    {
        if (game_time::turn() % POISON_DMG_N_TURN == 0)
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

            owning_actor_->hit(1, Dmg_type::pure);
        }
    }
}

bool Prop_terrified::allow_attack_melee(const Verbosity verbosity) const
{
    if (owning_actor_->is_player() && verbosity == Verbosity::verbose)
    {
        msg_log::add("I am too terrified to engage in close combat!");
    }

    return false;
}

bool Prop_terrified::allow_attack_ranged(const Verbosity verbosity) const
{
    (void)verbosity;
    return true;
}

void Prop_terrified::on_start()
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

void Prop_nailed::change_move_dir(const Pos& actor_pos, Dir& dir)
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

        owning_actor_->hit(rnd::dice(1, 3), Dmg_type::physical);

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

bool Prop_confused::allow_read(const Verbosity verbosity) const
{
    if (owning_actor_->is_player() && verbosity == Verbosity::verbose)
    {
        msg_log::add("I am too confused to read.");
    }

    return false;
}

bool Prop_confused::allow_cast_spell(const Verbosity verbosity) const
{
    if (owning_actor_->is_player() && verbosity == Verbosity::verbose)
    {
        msg_log::add("I am too confused to concentrate.");
    }

    return false;
}

bool Prop_confused::allow_attack_melee(const Verbosity verbosity) const
{
    (void)verbosity;

    if (owning_actor_ != map::player)
    {
        return rnd::coin_toss();
    }

    return true;
}

bool Prop_confused::allow_attack_ranged(const Verbosity verbosity) const
{
    (void)verbosity;

    if (owning_actor_ != map::player)
    {
        return rnd::coin_toss();
    }

    return true;
}

void Prop_confused::change_move_dir(const Pos& actor_pos, Dir& dir)
{
    if (dir != Dir::center)
    {
        bool blocked[MAP_W][MAP_H];
        map_parse::run(cell_check::Blocks_actor(*owning_actor_, true),
                       blocked);

        if (rnd::one_in(8))
        {
            int tries_left = 100;

            while (tries_left != 0)
            {
                //-1 to 1 for x and y
                const Pos delta(rnd::range(-1, 1), rnd::range(-1, 1));

                if (delta.x != 0 || delta.y != 0)
                {
                    const Pos c = actor_pos + delta;

                    if (!blocked[c.x][c.y])
                    {
                        dir = dir_utils::dir(delta);
                    }
                }

                tries_left--;
            }
        }
    }
}

void Prop_strangled::on_new_turn()
{
    const int DMG = rnd::range(3, 4);

    owning_actor_->hit(DMG, Dmg_type::pure, Dmg_method::forced);
}

bool Prop_strangled::allow_speak(const Verbosity verbosity) const
{
    if (verbosity == Verbosity::verbose && owning_actor_->is_player())
    {
        msg_log::add("My throat is constricted.");
    }

    return false;
}

bool Prop_strangled::allow_eat(const Verbosity verbosity) const
{
    if (verbosity == Verbosity::verbose && owning_actor_->is_player())
    {
        msg_log::add("My throat is constricted.");
    }

    return false;
}

void Prop_frenzied::change_move_dir(const Pos& actor_pos, Dir& dir)
{
    if (owning_actor_->is_player())
    {
        std::vector<Actor*> seen_foes;
        owning_actor_->seen_foes(seen_foes);

        if (seen_foes.empty())
        {
            return;
        }

        std::vector<Pos> seen_foes_cells;

        seen_foes_cells.clear();

        for (auto* actor : seen_foes)
        {
            seen_foes_cells.push_back(actor->pos);
        }

        sort(begin(seen_foes_cells), end(seen_foes_cells), Is_closer_to_pos(actor_pos));

        const Pos& closest_mon_pos = seen_foes_cells[0];

        bool blocked[MAP_W][MAP_H];
        map_parse::run(cell_check::Blocks_actor(*owning_actor_, false), blocked);

        std::vector<Pos> line;
        line_calc::calc_new_line(actor_pos, closest_mon_pos, true, 999, false, line);

        if (line.size() > 1)
        {
            for (Pos& pos : line)
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

bool Prop_frenzied::is_resisting_other_prop(const Prop_id prop_id) const
{
    return prop_id == Prop_id::confused || prop_id == Prop_id::fainted ||
           prop_id == Prop_id::terrified || prop_id == Prop_id::weakened;
}

void Prop_frenzied::on_start()
{
    owning_actor_->prop_handler().end_prop(Prop_id::confused);
    owning_actor_->prop_handler().end_prop(Prop_id::terrified);
    owning_actor_->prop_handler().end_prop(Prop_id::weakened);
}

void Prop_frenzied::on_end()
{
    //Only the player gets tired after a frenzy (it looks weird for monsters)
    if (owning_actor_->is_player())
    {
        owning_actor_->prop_handler().try_add_prop(new Prop_weakened(Prop_turns::std));
    }
}

bool Prop_frenzied::allow_read(const Verbosity verbosity) const
{
    if (owning_actor_->is_player() && verbosity == Verbosity::verbose)
    {
        msg_log::add("I am too enraged to read!");
    }

    return false;
}

bool Prop_frenzied::allow_cast_spell(const Verbosity verbosity) const
{
    if (owning_actor_->is_player() && verbosity == Verbosity::verbose)
    {
        msg_log::add("I am too enraged to concentrate!");
    }

    return false;
}

void Prop_burning::on_new_turn()
{
    if (owning_actor_->is_player())
    {
        msg_log::add("AAAARGH IT BURNS!!!", clr_red_lgt);
    }

    owning_actor_->hit(rnd::dice(1, 2), Dmg_type::fire);
}

bool Prop_burning::allow_read(const Verbosity verbosity) const
{
    if (owning_actor_->is_player() && verbosity == Verbosity::verbose)
    {
        msg_log::add("I cannot read while burning.");
    }

    return false;
}

bool Prop_burning::allow_cast_spell(const Verbosity verbosity) const
{
    if (owning_actor_->is_player() && verbosity == Verbosity::verbose)
    {
        msg_log::add("I cannot concentrate while burning!");
    }

    return false;
}

bool Prop_burning::allow_attack_ranged(const Verbosity verbosity) const
{
    if (owning_actor_->is_player() && verbosity == Verbosity::verbose)
    {
        msg_log::add("Not while burning.");
    }

    return false;
}

bool Prop_blind::need_update_vision_when_start_or_end() const
{
    return owning_actor_->is_player();
}

void Prop_paralyzed::on_start()
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

bool Prop_fainted::need_update_vision_when_start_or_end() const
{
    return owning_actor_->is_player();
}

void Prop_flared::on_new_turn()
{
    owning_actor_->hit(1, Dmg_type::fire);

    if (nr_turns_left_ == 0)
    {
        owning_actor_->prop_handler().try_add_prop(new Prop_burning(Prop_turns::std));
        owning_actor_->prop_handler().end_prop(Prop_id::flared);
    }
}

bool Prop_rAcid::try_resist_dmg(const Dmg_type dmg_type, const Verbosity verbosity) const
{
    if (dmg_type == Dmg_type::acid)
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

bool Prop_rCold::try_resist_dmg(const Dmg_type dmg_type, const Verbosity verbosity) const
{
    if (dmg_type == Dmg_type::cold)
    {
        if (verbosity == Verbosity::verbose)
        {
            if (owning_actor_->is_player())
            {
                msg_log::add("I feel chilly.");
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

bool Prop_rElec::try_resist_dmg(const Dmg_type dmg_type, const Verbosity verbosity) const
{
    if (dmg_type == Dmg_type::electric)
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

bool Prop_rConf::is_resisting_other_prop(const Prop_id prop_id) const
{
    return prop_id == Prop_id::confused;
}

void Prop_rConf::on_start()
{
    owning_actor_->prop_handler().end_prop(Prop_id::confused);
}

bool Prop_rFear::is_resisting_other_prop(const Prop_id prop_id) const
{
    return prop_id == Prop_id::terrified;
}

void Prop_rFear::on_start()
{
    owning_actor_->prop_handler().end_prop(Prop_id::terrified);
}

bool Prop_rPhys::is_resisting_other_prop(const Prop_id prop_id) const
{
    (void)prop_id;
    return false;
}

void Prop_rPhys::on_start()
{

}

bool Prop_rPhys::try_resist_dmg(const Dmg_type dmg_type, const Verbosity verbosity) const
{
    if (dmg_type == Dmg_type::physical)
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

bool Prop_rFire::is_resisting_other_prop(const Prop_id prop_id) const
{
    return prop_id == Prop_id::burning;
}

void Prop_rFire::on_start()
{
    owning_actor_->prop_handler().end_prop(Prop_id::burning);
}

bool Prop_rFire::try_resist_dmg(const Dmg_type dmg_type, const Verbosity verbosity) const
{
    if (dmg_type == Dmg_type::fire)
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

bool Prop_rPoison::is_resisting_other_prop(const Prop_id prop_id) const
{
    return prop_id == Prop_id::poisoned;
}

void Prop_rPoison::on_start()
{
    owning_actor_->prop_handler().end_prop(Prop_id::poisoned);
}

bool Prop_rSleep::is_resisting_other_prop(const Prop_id prop_id) const
{
    return prop_id == Prop_id::fainted;
}

void Prop_rSleep::on_start()
{
    owning_actor_->prop_handler().end_prop(Prop_id::fainted);
}

bool Prop_rDisease::is_resisting_other_prop(const Prop_id prop_id) const
{
    return prop_id == Prop_id::diseased || prop_id == Prop_id::infected;
}

void Prop_rDisease::on_start()
{
    owning_actor_->prop_handler().end_prop(Prop_id::diseased);
    owning_actor_->prop_handler().end_prop(Prop_id::infected);
}

bool Prop_see_invis::is_resisting_other_prop(const Prop_id prop_id) const
{
    return prop_id == Prop_id::blind;
}

void Prop_see_invis::on_start()
{
    owning_actor_->prop_handler().end_prop(Prop_id::blind);
}

void Prop_burrowing::on_new_turn()
{
    const Pos& p = owning_actor_->pos;
    map::cells[p.x][p.y].rigid->hit(Dmg_type::physical, Dmg_method::forced);
}
