#include "property.hpp"

#include <algorithm>

#include "property_data.hpp"
#include "property_factory.hpp"
#include "actor.hpp"
#include "actor_mon.hpp"
#include "actor_player.hpp"
#include "actor_factory.hpp"
#include "map.hpp"
#include "map_parsing.hpp"
#include "feature_rigid.hpp"
#include "text_format.hpp"
#include "msg_log.hpp"
#include "saving.hpp"
#include "explosion.hpp"
#include "game_time.hpp"
#include "line_calc.hpp"
#include "knockback.hpp"
#include "item_factory.hpp"

// -----------------------------------------------------------------------------
// Property base class
// -----------------------------------------------------------------------------
Prop::Prop(PropId id) :
        id_(id),
        data_(property_data::data[(size_t)id]),
        nr_turns_left_(data_.std_rnd_turns.roll()),
        duration_mode_(PropDurationMode::standard),
        owner_(nullptr),
        src_(PropSrc::END),
        item_applying_(nullptr)
{

}

// -----------------------------------------------------------------------------
// Specific properties
// -----------------------------------------------------------------------------
void PropBlessed::on_applied()
{
        owner_->properties().end_prop_silent(PropId::cursed);

        bless_adjacent();
}

void PropBlessed::on_more(const Prop& new_prop)
{
        (void)new_prop;

        bless_adjacent();
}

void PropBlessed::bless_adjacent() const
{
        // "Bless" adjacent fountains
        const P& p = owner_->pos;

        for (const P& d : dir_utils::dir_list_w_center)
        {
                const P p_adj(p + d);

                Cell& cell = map::cells.at(p_adj);

                Rigid* const rigid = cell.rigid;

                if (rigid->id() != FeatureId::fountain)
                {
                        continue;
                }

                Fountain* const fountain = static_cast<Fountain*>(rigid);

                if (!fountain->has_drinks_left())
                {
                        continue;
                }

                const auto effect = fountain->effect();

                const bool is_bad_effect =
                        effect > FountainEffect::START_OF_BAD_EFFECTS;

                if (!is_bad_effect)
                {
                        continue;
                }

                fountain->set_effect(FountainEffect::refreshing);

                if (cell.is_seen_by_player)
                {
                        const std::string name_the =
                                text_format::first_to_lower(
                                        fountain->name(Article::the));

                        msg_log::add("The water in "
                                     + name_the +
                                     " seems clearer.");
                }
        }
}

void PropCursed::on_applied()
{
        owner_->properties().end_prop_silent(PropId::blessed);

        curse_adjacent();

        // If this is a permanent curse, log it as a historic event
        if (owner_->is_player() &&
            duration_mode_ == PropDurationMode::indefinite)
        {
                game::add_history_event("A terrible curse was put upon me.");
        }
}

void PropCursed::on_more(const Prop& new_prop)
{
        (void)new_prop;

        curse_adjacent();
}

void PropCursed::on_end()
{
        // If this was a permanent curse, log it as a historic event
        if (owner_->is_player() &&
            duration_mode_ == PropDurationMode::indefinite)
        {
                game::add_history_event("A terrible curse was lifted from me.");
        }
}

void PropCursed::curse_adjacent() const
{
        // "Curse" adjacent fountains
        const P& p = owner_->pos;

        for (const P& d : dir_utils::dir_list_w_center)
        {
                const P p_adj(p + d);

                Cell& cell = map::cells.at(p_adj);

                Rigid* const rigid = cell.rigid;

                if (rigid->id() != FeatureId::fountain)
                {
                        continue;
                }

                Fountain* const fountain = static_cast<Fountain*>(rigid);

                if (!fountain->has_drinks_left())
                {
                        continue;
                }

                const auto effect = fountain->effect();

                const bool is_good_effect =
                        effect < FountainEffect::START_OF_BAD_EFFECTS;

                if (!is_good_effect)
                {
                        continue;
                }

                const int min = (int)FountainEffect::START_OF_BAD_EFFECTS + 1;
                const int max = (int)FountainEffect::END - 1;

                fountain->set_effect((FountainEffect)rnd::range(min, max));

                if (cell.is_seen_by_player)
                {
                        std::string name_the =
                                text_format::first_to_lower(
                                        fountain->name(Article::the));

                        msg_log::add("The water in " +
                                     name_the +
                                     " seems murkier.");
                }
        }
}

void PropEntangled::on_applied()
{
        try_player_end_with_machete();
}

PropEnded PropEntangled::affect_move_dir(const P& actor_pos, Dir& dir)
{
        (void)actor_pos;

        if (dir == Dir::center)
        {
                return PropEnded::no;
        }

        if (try_player_end_with_machete())
        {
                return PropEnded::yes;
        }

        dir = Dir::center;

        if (owner_->is_player())
        {
                msg_log::add("I struggle to tear free!",
                             colors::msg_bad());
        }
        else // Is monster
        {
                if (map::player->can_see_actor(*owner_))
                {
                        const std::string actor_name_the =
                                text_format::first_to_upper(
                                        owner_->name_the());

                        msg_log::add(actor_name_the +
                                     " struggles to tear free.",
                                     colors::msg_good());
                }
        }

        if (rnd::one_in(8))
        {
                owner_->properties().end_prop(id());

                return PropEnded::yes;
        }

        return PropEnded::no;
}

bool PropEntangled::try_player_end_with_machete()
{
        if (owner_ != map::player)
        {
                return false;
        }

        Item* item = owner_->inv().item_in_slot(SlotId::wpn);

        if (item && (item->id() == ItemId::machete))
        {
                msg_log::add("I cut myself free with my Machete.");

                owner_->properties().end_prop_silent(id());

                return true;
        }

        return false;
}

void PropSlowed::on_applied()
{
        owner_->properties().end_prop_silent(PropId::hasted);
}

void PropHasted::on_applied()
{
        owner_->properties().end_prop_silent(PropId::slowed);
}

void PropSummoned::on_end()
{
        owner_->set_state(ActorState::destroyed);
}

PropEnded PropInfected::on_tick()
{
#ifndef NDEBUG
        ASSERT(!owner_->properties().has_prop(PropId::diseased));
#endif // NDEBUG

        if (map::player->active_medical_bag_)
        {
                ++nr_turns_left_;

                return PropEnded::no;
        }

        const int allow_disease_below_turns_left = 50;

        const int apply_disease_one_in_n = nr_turns_left_ - 1;

        const bool apply_disease =
                (nr_turns_left_ <= allow_disease_below_turns_left) &&
                ((apply_disease_one_in_n <= 0) ||
                 rnd::one_in(apply_disease_one_in_n));

        if (apply_disease)
        {
                PropHandler& properties = owner_->properties();

                owner_->properties().end_prop_silent(id());

                // NOTE: This property is now deleted

                auto prop_diseased = new PropDiseased();

                prop_diseased->set_indefinite();

                properties.apply(prop_diseased);

                msg_log::more_prompt();

                return PropEnded::yes;
        }

        return PropEnded::no;
}

int PropDiseased::affect_max_hp(const int hp_max) const
{
        return hp_max / 2;
}

void PropDiseased::on_applied()
{
        // End infection
        owner_->properties().end_prop_silent(PropId::infected);

        // If this is a permanent disease that the player caught, log it as a
        // historic event
        if (owner_->is_player() &&
            (duration_mode_ == PropDurationMode::indefinite))
        {
                game::add_history_event("Caught a horrible disease.");
        }
}

void PropDiseased::on_end()
{
#ifndef NDEBUG
        ASSERT(!owner_->properties().has_prop(PropId::infected));
#endif // NDEBUG

        // If this is a permanent disease that the player caught, log it as a
        // historic event
        if (owner_->is_player() &&
            (duration_mode_ == PropDurationMode::indefinite))
        {
                game::add_history_event("I was cured from a horrible disease.");
        }
}

bool PropDiseased::is_resisting_other_prop(const PropId prop_id) const
{
#ifndef NDEBUG
        ASSERT(!owner_->properties().has_prop(PropId::infected));
#endif // NDEBUG

        // Getting infected while already diseased is just annoying
        return prop_id == PropId::infected;
}

PropEnded PropDescend::on_tick()
{
        ASSERT(owner_->is_player());

        if (nr_turns_left_ <= 1)
        {
                game_time::is_magic_descend_nxt_std_turn = true;
        }

        return PropEnded::no;
}

void PropZuulPossessPriest::on_placed()
{
        // If the number left allowed to spawn is zero now, this means that Zuul
        // has been spawned for the first time - hide Zuul inside a priest.
        // Otherwise we do nothing, and just spawn Zuul. When the possessed
        // actor is killed, Zuul will be allowed to spawn infinitely (this is
        // handled elsewhere).

        const int nr_left_allowed = owner_->data().nr_left_allowed_to_spawn;

        ASSERT(nr_left_allowed <= 0);

        const bool should_possess = (nr_left_allowed >= 0);

        if (should_possess)
        {
                owner_->set_state(ActorState::destroyed);

                Actor* actor =
                        actor_factory::make(ActorId::cultist_priest,
                                            owner_->pos);

                auto* prop = new PropPossessedByZuul();

                prop->set_indefinite();

                actor->properties().apply(
                        prop,
                        PropSrc::intr,
                        true,
                        Verbosity::silent);

                actor->restore_hp(999, false, Verbosity::silent);
        }
}

void PropPossessedByZuul::on_death()
{
        // An actor possessed by Zuul has died - release Zuul!

        if (map::player->can_see_actor(*owner_))
        {
                const std::string& name1 =
                        text_format::first_to_upper(
                                owner_->name_the());

                const std::string& name2 =
                        actor_data::data[(size_t)ActorId::zuul].name_the;

                msg_log::add(name1 + " was possessed by " + name2 + "!");
        }

        owner_->set_state(ActorState::destroyed);

        const P& pos = owner_->pos;

        map::make_gore(pos);

        map::make_blood(pos);

        // Zuul is now free, allow it to spawn infinitely
        actor_data::data[(size_t)ActorId::zuul].nr_left_allowed_to_spawn = -1;

        actor_factory::spawn(pos, {ActorId::zuul}, map::rect())
                .make_aware_of_player();
}

PropEnded PropPoisoned::on_tick()
{
        if (owner_->is_alive() &&
            (game_time::turn_nr() % poison_dmg_n_turn) == 0)
        {
                if (owner_->is_player())
                {
                        msg_log::add("I am suffering from the poison!",
                                     colors::msg_bad(),
                                     true);
                }
                else // Is monster
                {
                        if (map::player->can_see_actor(*owner_))
                        {
                                const std::string actor_name_the =
                                        text_format::first_to_upper(
                                                owner_->name_the());

                                msg_log::add(actor_name_the +
                                             " suffers from poisoning!");
                        }
                }

                owner_->hit(1, DmgType::pure);
        }

        return PropEnded::no;
}

void PropAiming::on_hit()
{
        owner_->properties().end_prop(id());
}

bool PropTerrified::allow_attack_melee(const Verbosity verbosity) const
{
        if (owner_->is_player() && verbosity == Verbosity::verbose)
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

void PropTerrified::on_applied()
{
        // If this is a monster, we reset its last direction moved. Otherwise it
        // would probably tend to move toward the player even while terrified
        // (the AI would typically use the idle movement algorithm, which
        // favors stepping in the same direction as the last move).

        if (!owner_->is_player())
        {
                Mon* const mon = static_cast<Mon*>(owner_);

                mon->last_dir_moved_ = Dir::center;
        }
}

PropEnded PropNailed::affect_move_dir(const P& actor_pos, Dir& dir)
{
        (void)actor_pos;

        if (dir == Dir::center)
        {
                return PropEnded::no;
        }

        dir = Dir::center;

        if (owner_->is_player())
        {
                msg_log::add("I struggle to tear out the spike!",
                             colors::msg_bad());
        }
        else // Is monster
        {
                if (map::player->can_see_actor(*owner_))
                {
                        const std::string actor_name_the =
                                text_format::first_to_upper(
                                        owner_->name_the());

                        msg_log::add(actor_name_the + " struggles in pain!",
                                     colors::msg_good());
                }
        }

        owner_->hit(rnd::dice(1, 3), DmgType::physical);

        if (!owner_->is_alive() ||
            !rnd::one_in(4))
        {
                return PropEnded::no;
        }

        --nr_spikes_;

        if (nr_spikes_ > 0)
        {
                if (owner_->is_player())
                {
                        msg_log::add("I rip out a spike from my flesh!");
                }
                else if (map::player->can_see_actor(*owner_))
                {
                        const std::string actor_name_the =
                                text_format::first_to_upper(
                                        owner_->name_the());

                        msg_log::add(actor_name_the + " tears out a spike!");
                }
        }

        return PropEnded::no;
}

void PropWound::save() const
{
        saving::put_int(nr_wounds_);
}

void PropWound::load()
{
        nr_wounds_ = saving::get_int();
}

int PropWound::ability_mod(const AbilityId ability) const
{
        // A player with Survivalist receives no ability penalties
        if (owner_->is_player() &&
            player_bon::traits[(size_t)Trait::survivalist])
        {
                return 0;
        }

        if (ability == AbilityId::melee)
        {
                return (nr_wounds_ * -5);
        }
        else if (ability == AbilityId::dodging)
        {
                return (nr_wounds_ * -5);
        }

        return 0;
}

int PropWound::affect_max_hp(const int hp_max) const
{
        const int pen_pct_per_wound = 10;

        int hp_pen_pct = nr_wounds_ * pen_pct_per_wound;

        // The HP penalty is halved for a player with Survivalist
        if (owner_->is_player() &&
            player_bon::traits[(size_t)Trait::survivalist])
        {
                hp_pen_pct /= 2;
        }

        // Cap the penalty percentage
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
        else // This was the last wound
        {
                // End self
                owner_->properties().end_prop(id());
        }
}

void PropWound::on_more(const Prop& new_prop)
{
        (void)new_prop;

        ++nr_wounds_;

        if (nr_wounds_ >= 5)
        {
                if (owner_ == map::player)
                {
                        msg_log::add("I die from my wounds!");
                }

                owner_->die(false, false, true);
        }
}

PropHpSap::PropHpSap() :
        Prop(PropId::hp_sap),
        nr_drained_(rnd::range(1, 3)) {}

void PropHpSap::save() const
{
        saving::put_int(nr_drained_);
}

void PropHpSap::load()
{
        nr_drained_ = saving::get_int();
}

int PropHpSap::affect_max_hp(const int hp_max) const
{
        return (hp_max - nr_drained_);
}

void PropHpSap::on_more(const Prop& new_prop)
{
        nr_drained_ += static_cast<const PropHpSap*>(&new_prop)->nr_drained_;
}

PropSpiSap::PropSpiSap() :
        Prop(PropId::spi_sap),
        nr_drained_(1) {}

void PropSpiSap::save() const
{
        saving::put_int(nr_drained_);
}

void PropSpiSap::load()
{
        nr_drained_ = saving::get_int();
}

int PropSpiSap::affect_max_spi(const int spi_max) const
{
        return (spi_max - nr_drained_);
}

void PropSpiSap::on_more(const Prop& new_prop)
{
        nr_drained_ += static_cast<const PropSpiSap*>(&new_prop)->nr_drained_;
}

PropMindSap::PropMindSap() :
        Prop(PropId::mind_sap),
        nr_drained_(rnd::range(1, 3)) {}

void PropMindSap::save() const
{
        saving::put_int(nr_drained_);
}

void PropMindSap::load()
{
        nr_drained_ = saving::get_int();
}

int PropMindSap::affect_shock(const int shock) const
{
        return (shock + nr_drained_);
}

void PropMindSap::on_more(const Prop& new_prop)
{
        nr_drained_ += static_cast<const PropMindSap*>(&new_prop)->nr_drained_;
}

bool PropConfused::allow_read_absolute(const Verbosity verbosity) const
{
        if (owner_->is_player() && verbosity == Verbosity::verbose)
        {
                msg_log::add("I am too confused to read.");
        }

        return false;
}

bool PropConfused::allow_cast_intr_spell_absolute(
        const Verbosity verbosity) const
{
        if (owner_->is_player() &&
            (verbosity == Verbosity::verbose))
        {
                msg_log::add("I am too confused to concentrate!");
        }

        return false;
}

bool PropConfused::allow_attack_melee(const Verbosity verbosity) const
{
        (void)verbosity;

        if (owner_ != map::player)
        {
                return rnd::coin_toss();
        }

        return true;
}

bool PropConfused::allow_attack_ranged(const Verbosity verbosity) const
{
        (void)verbosity;

        if (owner_ != map::player)
        {
                return rnd::coin_toss();
        }

        return true;
}

PropEnded PropConfused::affect_move_dir(const P& actor_pos, Dir& dir)
{
        if (dir == Dir::center)
        {
                return PropEnded::no;
        }

        Array2<bool> blocked(map::dims());

        const R area_check_blocked(actor_pos - P(1, 1),
                                   actor_pos + P(1, 1));

        map_parsers::BlocksActor(*owner_, ParseActors::yes)
                .run(blocked,
                     area_check_blocked,
                     MapParseMode::overwrite);

        if (rnd::one_in(8))
        {
                std::vector<P> d_bucket;

                for (const P& d : dir_utils::dir_list)
                {
                        const P tgt_p(actor_pos + d);

                        if (!blocked.at(tgt_p))
                        {
                                d_bucket.push_back(d);
                        }
                }

                if (!d_bucket.empty())
                {
                        const P& d = rnd::element(d_bucket);

                        dir = dir_utils::dir(d);
                }
        }

        return PropEnded::no;
}

PropEnded PropFrenzied::affect_move_dir(const P& actor_pos, Dir& dir)
{
        if (!owner_->is_player() ||
            (dir == Dir::center))
        {
                return PropEnded::no;
        }

        const auto seen_foes = owner_->seen_foes();

        if (seen_foes.empty())
        {
                return PropEnded::no;
        }

        std::vector<P> seen_foes_cells;

        seen_foes_cells.clear();

        for (auto* actor : seen_foes)
        {
                seen_foes_cells.push_back(actor->pos);
        }

        sort(begin(seen_foes_cells),
             end(seen_foes_cells),
             IsCloserToPos(actor_pos));

        const P& closest_mon_pos = seen_foes_cells[0];

        Array2<bool> blocked(map::dims());

        map_parsers::BlocksActor(*owner_, ParseActors::no)
                .run(blocked, blocked.rect());

        const auto line =
                line_calc::calc_new_line(
                        actor_pos,
                        closest_mon_pos,
                        true,
                        999,
                        false);

        if (line.size() > 1)
        {
                for (const P& pos : line)
                {
                        if (blocked.at(pos))
                        {
                                return PropEnded::no;
                        }
                }

                dir = dir_utils::dir(line[1] - actor_pos);
        }

        return PropEnded::no;
}

bool PropFrenzied::is_resisting_other_prop(const PropId prop_id) const
{
        return
                prop_id == PropId::confused ||
                prop_id == PropId::fainted ||
                prop_id == PropId::terrified ||
                prop_id == PropId::weakened;
}

void PropFrenzied::on_applied()
{
        owner_->properties().end_prop_silent(PropId::confused);
        owner_->properties().end_prop_silent(PropId::fainted);
        owner_->properties().end_prop_silent(PropId::terrified);
        owner_->properties().end_prop_silent(PropId::weakened);
}

void PropFrenzied::on_end()
{
        // Only the player (except for Ghoul background) gets tired after a
        // frenzy (it looks weird for monsters)
        if (owner_->is_player() &&
            (player_bon::bg() != Bg::ghoul))
        {
                owner_->apply_prop(new PropWeakened());
        }
}

bool PropFrenzied::allow_read_absolute(const Verbosity verbosity) const
{
        if (owner_->is_player() && verbosity == Verbosity::verbose)
        {
                msg_log::add("I am too enraged to read!");
        }

        return false;
}

bool PropFrenzied::allow_cast_intr_spell_absolute(
        const Verbosity verbosity) const
{
        if (owner_->is_player() &&
            (verbosity == Verbosity::verbose))
        {
                msg_log::add("I am too enraged to concentrate!");
        }

        return false;
}

PropEnded PropBurning::on_tick()
{
        if (owner_->is_player())
        {
                msg_log::add("AAAARGH IT BURNS!!!", colors::light_red());
        }

        owner_->hit(rnd::dice(1, 3), DmgType::fire);

        return PropEnded::no;
}

bool PropBurning::allow_read_chance(const Verbosity verbosity) const
{
        if (!rnd::coin_toss())
        {
                if (owner_->is_player() &&
                    (verbosity == Verbosity::verbose))
                {
                        msg_log::add("I fail to concentrate!");
                }

                return false;
        }

        return true;
}

bool PropBurning::allow_cast_intr_spell_chance(const Verbosity verbosity) const
{
        if (!rnd::coin_toss())
        {
                if (owner_->is_player() &&
                    (verbosity == Verbosity::verbose))
                {
                        msg_log::add("I fail to concentrate!");
                }

                return false;
        }

        return true;
}

bool PropBurning::allow_attack_ranged(const Verbosity verbosity) const
{
        if (owner_->is_player() &&
            (verbosity == Verbosity::verbose))
        {
                msg_log::add("Not while burning.");
        }

        return false;
}

PropActResult PropRecloaks::on_act()
{
        if (owner_->is_alive() &&
            !owner_->has_prop(PropId::cloaked) &&
            rnd::one_in(20))
        {
                auto prop_cloaked = new PropCloaked();

                prop_cloaked->set_indefinite();

                owner_->apply_prop(prop_cloaked);

                game_time::tick();

                return PropActResult(DidAction::yes, PropEnded::no);
        }

        return PropActResult();
}

bool PropBlind::allow_read_absolute(const Verbosity verbosity) const
{
        if (owner_->is_player() &&
            (verbosity == Verbosity::verbose))
        {
                msg_log::add("I cannot read while blind.");
        }

        return false;
}

bool PropBlind::should_update_vision_on_toggled() const
{
        return owner_->is_player();
}

void PropParalyzed::on_applied()
{
        auto* const player = map::player;

        if (owner_->is_player())
        {
                auto* const active_explosive = player->active_explosive_;

                if (active_explosive)
                {
                        active_explosive->on_player_paralyzed();
                }
        }
}

bool PropFainted::should_update_vision_on_toggled() const
{
        return owner_->is_player();
}

PropEnded PropFlared::on_tick()
{
        owner_->hit(1, DmgType::fire);

        if (nr_turns_left_ <= 1)
        {
                owner_->apply_prop(new PropBurning());

                owner_->properties().end_prop(id());

                return PropEnded::yes;
        }

        return PropEnded::no;
}

DmgResistData PropRAcid::is_resisting_dmg(const DmgType dmg_type) const
{
        DmgResistData d;

        d.is_resisted = (dmg_type == DmgType::acid);

        d.msg_resist_player = "I feel a faint burning sensation.";

        d.msg_resist_mon = "seems unaffected.";

        return d;
}

DmgResistData PropRElec::is_resisting_dmg(const DmgType dmg_type) const
{
        DmgResistData d;

        d.is_resisted = (dmg_type == DmgType::electric);

        d.msg_resist_player = "I feel a faint tingle.";

        d.msg_resist_mon = "seems unaffected.";

        return d;
}

bool PropRConf::is_resisting_other_prop(const PropId prop_id) const
{
        return prop_id == PropId::confused;
}

void PropRConf::on_applied()
{
        owner_->properties().end_prop_silent(PropId::confused);
}

bool PropRFear::is_resisting_other_prop(const PropId prop_id) const
{
        return prop_id == PropId::terrified;
}

void PropRFear::on_applied()
{
        owner_->properties().end_prop_silent(PropId::terrified);

        if (owner_->is_player() &&
            duration_mode_ == PropDurationMode::indefinite)
        {
                insanity::on_permanent_rfear();
        }
}

bool PropRSlow::is_resisting_other_prop(const PropId prop_id) const
{
        return prop_id == PropId::slowed;
}

void PropRSlow::on_applied()
{
        owner_->properties().end_prop_silent(PropId::slowed);
}

bool PropRPhys::is_resisting_other_prop(const PropId prop_id) const
{
        (void)prop_id;
        return false;
}

void PropRPhys::on_applied()
{

}

DmgResistData PropRPhys::is_resisting_dmg(const DmgType dmg_type) const
{
        DmgResistData d;

        d.is_resisted = (dmg_type == DmgType::physical);

        d.msg_resist_player = "I resist harm.";

        d.msg_resist_mon = "seems unharmed.";

        return d;
}

bool PropRFire::is_resisting_other_prop(const PropId prop_id) const
{
        return prop_id == PropId::burning;
}

void PropRFire::on_applied()
{
        owner_->properties().end_prop_silent(PropId::burning);
}

DmgResistData PropRFire::is_resisting_dmg(const DmgType dmg_type) const
{
        DmgResistData d;

        d.is_resisted = (dmg_type == DmgType::fire);

        d.msg_resist_player = "I feel warm.";

        d.msg_resist_mon = "seems unaffected.";

        return d;
}

bool PropRPoison::is_resisting_other_prop(const PropId prop_id) const
{
        return prop_id == PropId::poisoned;
}

void PropRPoison::on_applied()
{
        owner_->properties().end_prop_silent(PropId::poisoned);
}

bool PropRSleep::is_resisting_other_prop(const PropId prop_id) const
{
        return prop_id == PropId::fainted;
}

void PropRSleep::on_applied()
{
        owner_->properties().end_prop_silent(PropId::fainted);
}

bool PropRDisease::is_resisting_other_prop(const PropId prop_id) const
{
        return prop_id == PropId::diseased || prop_id == PropId::infected;
}

void PropRDisease::on_applied()
{
        owner_->properties().end_prop_silent(PropId::diseased);
        owner_->properties().end_prop_silent(PropId::infected);
}

bool PropRBlind::is_resisting_other_prop(const PropId prop_id) const
{
        return prop_id == PropId::blind;
}

void PropRBlind::on_applied()
{
        owner_->properties().end_prop_silent(PropId::blind);
}

bool PropRPara::is_resisting_other_prop(const PropId prop_id) const
{
        return prop_id == PropId::paralyzed;
}

void PropRPara::on_applied()
{
        owner_->properties().end_prop_silent(PropId::paralyzed);
}

bool PropSeeInvis::is_resisting_other_prop(const PropId prop_id) const
{
        return prop_id == PropId::blind;
}

void PropSeeInvis::on_applied()
{
        owner_->properties().end_prop_silent(PropId::blind);
}

PropEnded PropBurrowing::on_tick()
{
        const P& p = owner_->pos;

        map::cells.at(p).rigid->hit(1, // Doesn't matter
                                        DmgType::physical,
                                        DmgMethod::forced);

        return PropEnded::no;
}

PropActResult PropVortex::on_act()
{
        TRACE_FUNC_BEGIN;

        // Not supported yet
        if (owner_->is_player())
        {
                return PropActResult();
        }

        if (!owner_->is_alive())
        {
                return PropActResult();
        }

        if (pull_cooldown > 0)
        {
                --pull_cooldown;
        }

        auto* const mon = static_cast<Mon*>(owner_);

        if ((mon->aware_of_player_counter_ <= 0) ||
            (pull_cooldown > 0))
        {
                return PropActResult();
        }

        const P& player_pos = map::player->pos;

        if (is_pos_adj(mon->pos, player_pos, true) ||
            !rnd::coin_toss())
        {
                return PropActResult();
        }

        TRACE << "Monster with vortex property attempting to pull player"
              << std::endl;

        const P delta = player_pos - mon->pos;

        P knockback_from_pos = player_pos;

        if (delta.x >  1)
        {
                ++knockback_from_pos.x;
        }

        if (delta.x < -1)
        {
                --knockback_from_pos.x;
        }

        if (delta.y >  1)
        {
                ++knockback_from_pos.y;
        }

        if (delta.y < -1)
        {
                --knockback_from_pos.y;
        }

        if (knockback_from_pos == player_pos)
        {
                return PropActResult();
        }

        TRACE << "Pos found to knockback player from: "
              << knockback_from_pos.x << ", "
              << knockback_from_pos.y << std::endl;

        TRACE << "Player pos: "
              << player_pos.x << ", " << player_pos.y << std::endl;

        Array2<bool> blocked_los(map::dims());

        const R fov_rect = fov::get_fov_rect(mon->pos);

        map_parsers::BlocksLos()
                .run(blocked_los,
                     fov_rect,
                     MapParseMode::overwrite);

        if (mon->can_see_actor(*(map::player), blocked_los))
        {
                TRACE << "Is seeing player" << std::endl;

                mon->set_player_aware_of_me();

                if (map::player->can_see_actor(*mon))
                {
                        const auto name_the = text_format::first_to_upper(
                                owner_->name_the());

                        msg_log::add(name_the + " pulls me!");
                }
                else
                {
                        msg_log::add("A powerful wind is pulling me!");
                }

                TRACE << "Attempt pull (knockback)" << std::endl;

                // TODO: Add sfx
                knockback::run(
                        *map::player,
                        knockback_from_pos,
                        false,
                        Verbosity::silent);

                pull_cooldown = 2;

                game_time::tick();

                return PropActResult(DidAction::yes, PropEnded::no);
        }

        return PropActResult();
}

void PropExplodesOnDeath::on_death()
{
        TRACE_FUNC_BEGIN;

        explosion::run(owner_->pos, ExplType::expl);

        TRACE_FUNC_END;
}

void PropSplitsOnDeath::on_death()
{
        // Not supported yet
        if (owner_->is_player())
        {
                return;
        }

        const int actor_max_hp = owner_->hp_max(true);

        // Do not allow splitting if HP is reduced to this point (if the monster
        // is killed "hard" enough, it doesn't split)
        const bool is_very_destroyed = owner_->hp() <= (actor_max_hp * (-5));

        const P pos = owner_->pos;

        if (is_very_destroyed ||
            map::cells.at(pos).rigid->is_bottomless() ||
            (game_time::actors.size() >= max_nr_actors_on_map))
        {
                return;
        }

        auto* const leader = static_cast<Mon*>(owner_)->leader_;

        const auto summoned =
                actor_factory::spawn(pos, {2, owner_->id()}, map::rect())
                .make_aware_of_player()
                .set_leader(leader)
                .for_each([this](Mon* const mon)
                {
                        auto prop_waiting = new PropWaiting();

                        prop_waiting->set_duration(1);

                        mon->apply_prop(prop_waiting);

                        // The new actors should usually not also split
                        if (rnd::fraction(4, 5))
                        {
                                mon->properties().end_prop(
                                        PropId::splits_on_death);
                        }

                        // If the original actor is burning, the spawned actors
                        // should too
                        if (owner_->has_prop(PropId::burning))
                        {
                                mon->properties().apply(
                                        new PropBurning(),
                                        PropSrc::intr,
                                        false, // Do not force effect
                                        Verbosity::silent);
                        }
                });

        // If no leader yet, set the first actor as leader of the second
        if (!leader && (summoned.monsters.size() == 2))
        {
                summoned.monsters[1]->leader_ = summoned.monsters[0];
        }
}

PropActResult PropCorpseEater::on_act()
{
        auto did_action = DidAction::no;

        if (owner_->is_alive() &&
            rnd::coin_toss())
        {
                did_action = owner_->try_eat_corpse();

                if (did_action == DidAction::yes)
                {
                        game_time::tick();
                }
        }

        return PropActResult(did_action, PropEnded::no);
}

PropActResult PropTeleports::on_act()
{
        auto did_action = DidAction::no;

        const int teleport_one_in_n = 12;

        if (owner_->is_alive() &&
            rnd::one_in(teleport_one_in_n))
        {
                owner_->teleport();

                game_time::tick();

                did_action = DidAction::yes;
        }

        return PropActResult(did_action, PropEnded::no);
}

PropActResult PropCorruptsEnvColor::on_act()
{
        const auto pos = owner_->pos;

        Rigid* r = map::cells.at(pos).rigid;

        r->corrupt_color();

        return PropActResult();
}

void PropAltersEnv::on_std_turn()
{
        if (!rnd::one_in(4))
        {
                return;
        }

        Array2<bool> blocked(map::dims());

        map_parsers::BlocksMoveCommon(ParseActors::no)
                .run(blocked, blocked.rect());

        // Do not consider doors blocking
        for (size_t i = 0; i < map::nr_cells(); ++i)
        {
                if (map::cells.at(i).rigid->id() == FeatureId::door)
                {
                        blocked.at(i) = false;
                }
        }

        Array2<bool> has_actor(map::dims());

        for (auto actor : game_time::actors)
        {
                has_actor.at(actor->pos) = true;
        }

        const int r = 3;

        const int x0 = std::max(
                1,
                owner_->pos.x - r);

        const int y0 = std::max(
                1,
                owner_->pos.y - r);

        const int x1 = std::min(
                map::w() - 2,
                owner_->pos.x + r);

        const int y1 = std::min(
                map::h() - 2,
                owner_->pos.y + r);

        for (int x = x0; x <= x1; ++x)
        {
                for (int y = y0; y <= y1; ++y)
                {
                        if (has_actor.at(x, y) || !rnd::one_in(6))
                        {
                                continue;
                        }

                        const P pos(x, y);

                        const FeatureId current_id =
                                map::cells.at(x, y).rigid->id();

                        if (current_id == FeatureId::wall)
                        {
                                blocked.at(x, y) = true;

                                if (map_parsers::is_map_connected(blocked))
                                {
                                        map::put(new Floor(pos));
                                }
                                else
                                {
                                        blocked.at(x, y) = false;
                                }
                        }
                        else if (current_id == FeatureId::floor)
                        {
                                blocked.at(x, y) = true;

                                if (map_parsers::is_map_connected(blocked))
                                {
                                        map::put(new Wall(pos));
                                }
                                else
                                {
                                        blocked.at(x, y) = false;
                                }
                        }
                }
        }
}

void PropRegenerates::on_std_turn()
{
        if (owner_->is_alive() &&
            !owner_->has_prop(PropId::burning))
        {
                owner_->restore_hp(2, false, Verbosity::silent);
        }
}

PropActResult PropCorpseRises::on_act()
{
        const int rise_one_in_n = 6;

        if (!owner_->is_corpse() ||
            !rnd::one_in(rise_one_in_n) ||
            map::actor_at_pos(owner_->pos))
        {
                return PropActResult();
        }

        owner_->set_state(ActorState::alive);

        owner_->set_hp(owner_->hp_max(true) / 2);

        --owner_->data().nr_kills;

        if (map::cells.at(owner_->pos).is_seen_by_player)
        {
                const std::string name =
                        text_format::first_to_upper(
                                owner_->corpse_name_the());

                msg_log::add(
                        name +
                        " rises again!!",
                        colors::text(),
                        true);

                map::player->incr_shock(ShockLvl::frightening,
                                        ShockSrc::see_mon);
        }

        static_cast<Mon*>(owner_)->become_aware_player(false);

        game_time::tick();

        has_risen_ = true;

        return PropActResult(DidAction::yes, PropEnded::no);
}

void PropCorpseRises::on_death()
{
        if (has_risen_)
        {
                owner_->destroy();
        }
}

void PropSpawnsZombiePartsOnDestroyed::on_destroyed()
{
        const P& pos = owner_->pos;

        if (map::cells.at(pos).rigid->is_bottomless())
        {
                return;
        }

        // Occasionally make Zombie Dust
        const int make_dust_one_in_n = 7;

        if (rnd::one_in(make_dust_one_in_n))
        {
                item_factory::make_item_on_floor(ItemId::zombie_dust, pos);
        }

        // Spawning zombie part monsters is only allowed if the monster is not
        // destroyed "too hard". This also reward heavy weapons, since they will
        // more often prevent spawning
        const bool is_very_destroyed = (owner_->hp() <= -8);

        const int summon_one_in_n = 4;

        if (is_very_destroyed || !rnd::one_in(summon_one_in_n))
        {
                return;
        }

        ActorId id_to_spawn = ActorId::END;

        const std::vector<int> weights =
                {
                        25,     // Hand
                        25,     // Intestines
                        1       // Floating skull
                };

        const int mon_choice = rnd::weighted_choice(weights);

        const std::string my_name = owner_->name_the();

        std::string spawn_msg = "";

        switch (mon_choice)
        {
        case 0:
                id_to_spawn = ActorId::crawling_hand;

                spawn_msg =
                        "The hand of " +
                        my_name +
                        " comes off and starts crawling around!";
                break;

        case 1:
                id_to_spawn = ActorId::crawling_intestines;

                spawn_msg =
                        "The intestines of " +
                        my_name +
                        " starts crawling around!";
                break;

        case 2:
                id_to_spawn = ActorId::floating_skull;

                spawn_msg =
                        "The head of " +
                        my_name +
                        " starts floating around!";
                break;

        }

        if (map::cells.at(pos).is_seen_by_player)
        {
                ASSERT(!spawn_msg.empty());

                msg_log::add(spawn_msg);

                map::player->incr_shock(ShockLvl::frightening,
                                        ShockSrc::see_mon);
        }

        ASSERT(id_to_spawn != ActorId::END);

        actor_factory::spawn(pos, {id_to_spawn}, map::rect())
                .make_aware_of_player()
                .for_each([](Mon* const mon)
                {
                        auto* waiting = new PropWaiting();

                        waiting->set_duration(1);

                        mon->apply_prop(waiting);
                });
}

void PropBreeds::on_std_turn()
{
        const int spawn_new_one_in_n = 50;

        if (owner_->is_player() ||
            !owner_->is_alive() ||
            owner_->has_prop(PropId::burning) ||
            (game_time::actors.size() >= max_nr_actors_on_map) ||
            !rnd::one_in(spawn_new_one_in_n))
        {
                return;
        }

        auto* const mon = static_cast<Mon*>(owner_);

        Actor* const leader_of_spawned_mon = mon->leader_ ? mon->leader_ : mon;

        const auto area_allowed = R(mon->pos - 1, mon->pos + 1);

        auto summoned =
                actor_factory::spawn_random_position({mon->id()}, area_allowed)
                .set_leader(leader_of_spawned_mon)
                .for_each([](Mon* const mon)
                {
                        auto prop_waiting = new PropWaiting();

                        prop_waiting->set_duration(2);

                        mon->apply_prop(prop_waiting);
                });

        if (mon->aware_of_player_counter_ > 0)
        {
                summoned.make_aware_of_player();
        }
}

void PropConfusesAdjacent::on_std_turn()
{
        if (!owner_->is_alive() ||
            !map::player->can_see_actor(*owner_) ||
            !map::player->pos.is_adjacent(owner_->pos))
        {
                return;
        }

        if (!map::player->properties().has_prop(PropId::confused))
        {
                const std::string msg =
                        text_format::first_to_upper(owner_->name_the()) +
                        " bewilders me.";

                msg_log::add(msg);
        }

        auto prop_confusd = new PropConfused();

        prop_confusd->set_duration(rnd::range(8, 12));

        map::player->apply_prop(prop_confusd);
}

PropActResult PropSpeaksCurses::on_act()
{
        if (owner_->is_player())
        {
                return PropActResult();
        }

        auto* const mon = static_cast<Mon*>(owner_);

        if (!mon->is_alive() ||
            (mon->aware_of_player_counter_ <= 0) ||
            !rnd::one_in(4))
        {
                return PropActResult();
        }

        Array2<bool> blocked_los(map::dims());

        const R fov_rect = fov::get_fov_rect(mon->pos);

        map_parsers::BlocksLos()
                .run(blocked_los,
                     fov_rect,
                     MapParseMode::overwrite);

        if (mon->can_see_actor(*map::player, blocked_los))
        {
                const bool player_see_owner = map::player->can_see_actor(*mon);

                std::string snd_msg =
                        player_see_owner ?
                        text_format::first_to_upper(mon->name_the()) :
                        "Someone";

                snd_msg += " spews forth a litany of curses.";

                Snd snd(snd_msg,
                        SfxId::END,
                        IgnoreMsgIfOriginSeen::no,
                        mon->pos,
                        mon,
                        SndVol::high,
                        AlertsMon::no);

                snd_emit::run(snd);

                map::player->apply_prop(new PropCursed());

                game_time::tick();

                return PropActResult(DidAction::yes, PropEnded::no);
        }

        return PropActResult();
}

PropActResult PropMajorClaphamSummon::on_act()
{
        if (owner_->is_player())
        {
                return PropActResult();
        }

        auto* const mon = static_cast<Mon*>(owner_);

        if (!mon->is_alive() ||
            (mon->aware_of_player_counter_ <= 0))
        {
                return PropActResult();
        }

        Array2<bool> blocked_los(map::dims());

        const R fov_rect = fov::get_fov_rect(mon->pos);

        map_parsers::BlocksLos()
                .run(blocked_los,
                     fov_rect,
                     MapParseMode::overwrite);

        if (!mon->can_see_actor(*(map::player), blocked_los))
        {
                return PropActResult();
        }

        mon->set_player_aware_of_me();

        msg_log::add("Major Clapham Lee calls forth his Tomb-Legions!");

        std::vector<ActorId> ids_to_summon = {ActorId::dean_halsey};

        const int nr_of_extra_spawns = 4;

        const std::vector<ActorId> possible_random_id_choices = {
                ActorId::zombie,
                ActorId::zombie_axe,
                ActorId::bloated_zombie
        };

        for (int i = 0; i < nr_of_extra_spawns; ++i)
        {
                ids_to_summon.push_back(
                        rnd::element(possible_random_id_choices));
        }

        auto spawned =
                actor_factory::spawn(mon->pos, ids_to_summon, map::rect())
                .make_aware_of_player()
                .set_leader(mon)
                .for_each([](Mon* const mon)
                {
                        auto prop_summoned = new PropSummoned();

                        prop_summoned->set_indefinite();

                        mon->apply_prop(prop_summoned);

                        mon->is_player_feeling_msg_allowed_ = false;
                });

        map::player->incr_shock(ShockLvl::terrifying, ShockSrc::misc);

        mon->properties().end_prop(id());

        game_time::tick();

        return PropActResult(DidAction::yes, PropEnded::yes);
}
