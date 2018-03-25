#include "actor.hpp"

#include "init.hpp"

#include "io.hpp"
#include "game_time.hpp"
#include "actor_items.hpp"
#include "actor_mon.hpp"
#include "actor_player.hpp"
#include "map.hpp"
#include "fov.hpp"
#include "msg_log.hpp"
#include "feature_trap.hpp"
#include "drop.hpp"
#include "explosion.hpp"
#include "game.hpp"
#include "inventory.hpp"
#include "map_parsing.hpp"
#include "item.hpp"
#include "marker.hpp"
#include "look.hpp"
#include "map_travel.hpp"
#include "popup.hpp"
#include "feature_door.hpp"
#include "text_format.hpp"
#include "property.hpp"
#include "property_data.hpp"
#include "property_handler.hpp"

Actor::Actor() :
    pos(),
    delay_(0),
    state_(ActorState::alive),
    hp_(-1),
    hp_max_(-1),
    spi_(-1),
    spi_max_(-1),
    lair_pos_(),
    properties_(nullptr),
    data_(nullptr),
    inv_(nullptr) {}

Actor::~Actor()
{
    // Free all items owning actors
    for (Item* item : inv_->backpack_)
    {
        item->clear_actor_carrying();
    }

    for (auto& slot : inv_->slots_)
    {
        if (slot.item)
        {
            slot.item->clear_actor_carrying();
        }
    }

    delete inv_;
    delete properties_;
}

bool Actor::has_prop(const PropId id) const
{
    return properties_->has_prop(id);
}

void Actor::apply_prop(Prop* const prop)
{
    properties_->apply(prop);
}

int Actor::ability(const AbilityId id,
                   const bool is_affected_by_props) const
{
    return data_->ability_values.val(id, is_affected_by_props, *this);
}

ActionResult Actor::roll_sneak(const Actor& actor_searching) const
{
    const int sneak_skill = ability(AbilityId::stealth, true);

    const int search_mod =
        actor_searching.is_player() ?
        actor_searching.ability(AbilityId::searching, true) :
        0;

    const int dist = king_dist(pos, actor_searching.pos);

    // Distance  Sneak bonus
    // ----------------------
    // 1         -7
    // 2          0
    // 3          7
    // 4         14
    // 5         21
    // 6         28
    // 7         35
    // 8         42
    const int dist_mod = (dist - 2) * 7;

    const bool is_lit = map::light[pos.x][pos.y];

    const bool is_dark = map::light[pos.x][pos.y];

    const int lgt_mod =
        is_lit ?
        20 : 0;

    const int drk_mod =
        (is_dark && !is_lit) ?
        20 : 0;

    int sneak_tot =
        sneak_skill
        - search_mod
        + dist_mod
        - lgt_mod
        + drk_mod;

    // std::cout << "SNEAKING" << std::endl
    //           << "------------------" << std::endl
    //           << "sneak_skill : " << sneak_skill << std::endl
    //           << "dist_mod    : " << dist_mod << std::endl
    //           << "lgt_mod     : " << lgt_mod << std::endl
    //           << "drk_mod     : " << drk_mod << std::endl
    //           << "sneak_tot   : " << sneak_tot << std::endl;

    // NOTE: There is no need to cap the sneak value, since there's always
    // critical fails

    const auto result = ability_roll::roll(sneak_tot);

    return result;
}

int Actor::hp_max(const bool with_modifiers) const
{
    int result = hp_max_;

    if (with_modifiers)
    {
        result = properties_->affect_max_hp(result);

        result = std::max(1, result);
    }

    return result;
}

int Actor::spi_max() const
{
    int result = spi_max_;

    result = properties_->affect_max_spi(result);

    result = std::max(1, result);

    return result;
}

int Actor::speed_pct() const
{
    // Paralyzed actors always act at normal speed (otherwise paralysis will
    // barely affect super fast monsters at all)
    if (has_prop(PropId::paralyzed))
    {
        return 100;
    }

    int speed = data_->speed_pct;

    // Speed modifications due to properties
    if (properties_->has_prop(PropId::slowed))
    {
        speed -= 50;
    }

    if (properties_->has_prop(PropId::hasted))
    {
        speed += 100;
    }

    if (properties_->has_prop(PropId::frenzied))
    {
        speed += 100;
    }

    if (properties_->has_prop(PropId::clockwork_hasted))
    {
        speed += 2000;
    }

    // Speed bonus from background/traits?
    if (is_player())
    {
        if (player_bon::bg() == Bg::ghoul)
        {
            // NOTE: Keep this the same as the Ghoul monster bonus
            speed += 10;
        }

        if (player_bon::traits[(size_t)Trait::dexterous])
        {
            speed += 10;
        }
    }

    const int min_speed = 10;

    speed = std::max(min_speed, speed);

    return speed;
}

void Actor::init(const P& pos_, ActorData& actor_data)
{
    pos = pos_;

    data_ = &actor_data;

    // NOTE: Cannot compare against global player pointer here, since it may not
    // yet have been set up
    if (data_->id == ActorId::player)
    {
            hp_max_ = data_->hp;
    }
    else // Is monster
    {
            const int hp_max_variation_pct = 50;

            const int hp_range_min = (data_->hp * hp_max_variation_pct) / 100;

            Range hp_range(hp_range_min, data_->hp + hp_range_min);

            hp_range.min = std::max(1, hp_range.min);

            hp_max_ = hp_range.roll();
    }

    hp_ = hp_max_;

    spi_ = spi_max_ = data_->spi;

    lair_pos_ = pos;

    inv_ = new Inventory(this);

    properties_ = new PropHandler(this);

    properties_->apply_natural_props_from_actor_data();

    if (!is_player())
    {
        actor_items::make_for_actor(*this);

        // Monster ghouls start allied to player ghouls
        const bool set_allied =
            data_->is_ghoul &&
            (player_bon::bg() == Bg::ghoul) &&
            // HACK: Do not allow the boss Ghoul to become allied
            (data_->id != ActorId::high_priest_guard_ghoul);

        if (set_allied)
        {
            Mon* const mon = static_cast<Mon*>(this);

            mon->leader_ = map::player;
        }
    }
}

void Actor::on_std_turn_common()
{
    // Do light damage if in lit cell
    if (map::light[pos.x][pos.y])
    {
        hit(1, DmgType::light);
    }

    if (is_alive())
    {
        // Slowly decrease current HP/spirit if above max
        const int decr_above_max_n_turns = 7;

        const bool decr_this_turn =
            (game_time::turn_nr() % decr_above_max_n_turns) == 0;

        if ((hp() > hp_max(true)) && decr_this_turn)
        {
            --hp_;
        }

        if ((spi() > spi_max()) && decr_this_turn)
        {
            --spi_;
        }

        // Regenerate spirit
        int regen_spi_n_turns = 18;

        if (is_player())
        {
            if (player_bon::traits[(size_t)Trait::strong_spirit])
            {
                regen_spi_n_turns -= 8;
            }
        }
        else // Is monster
        {
            // Monsters regen spirit very quickly, so spell casters doesn't
            // suddenly get completely handicapped
            regen_spi_n_turns = 1;
        }

        const bool regen_spi_this_turn =
            (game_time::turn_nr() % regen_spi_n_turns) == 0;

        if (regen_spi_this_turn)
        {
            restore_spi(1, false, Verbosity::silent);
        }
    }

    on_std_turn();
}

void Actor::teleport(const ShouldCtrlTele ctrl_tele)
{
    bool blocks_flood[map_w][map_h];

    map_parsers::BlocksActor(*this, ParseActors::no)
        .run(blocks_flood);

    // Consider all doors, except for metal doors, to be free
    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            const auto* const r = map::cells[x][y].rigid;

            if (r->id() == FeatureId::door)
            {
                const auto* const door = static_cast<const Door*>(r);

                // - For the player, only metal doors block the flood
                // - For monsters, no doors block the flood
                if ((door->type() != DoorType::metal) ||
                    !is_player())
                {
                    blocks_flood[x][y] = false;
                }
            }
        }
    }

    int flood[map_w][map_h];

    floodfill(pos,
              blocks_flood,
              flood);

    bool blocked[map_w][map_h];

    map_parsers::BlocksActor(*this, ParseActors::yes)
        .run(blocked);

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            if (flood[x][y] <= 0)
            {
                blocked[x][y] = true;
            }
        }
    }

    // Make sure we do not teleport to the position we are standing in
    blocked[pos.x][pos.y] = false;

    // Teleport control?
    const bool can_actor_use_tele_ctrl =
        is_player() &&
        properties_->has_prop(PropId::tele_ctrl) &&
        !properties_->has_prop(PropId::confused);

    if ((ctrl_tele == ShouldCtrlTele::always) ||
        ((ctrl_tele == ShouldCtrlTele::if_tele_ctrl_prop) &&
         can_actor_use_tele_ctrl))
    {
        auto tele_ctrl_state = std::unique_ptr<State>(
            new CtrlTele(pos, blocked));

        states::push(std::move(tele_ctrl_state));

        return;
    }

    // Teleport randomly
    const auto pos_bucket = to_vec(blocked, false);

    if (pos_bucket.empty())
    {
        return;
    }

    const P tgt_pos = rnd::element(pos_bucket);

    teleport(tgt_pos, blocked);
}

void Actor::teleport(P p, bool blocked[map_w][map_h])
{
    if (!is_player() &&
        map::player->can_see_actor(*this))
    {
        const std::string actor_name_the =
            text_format::first_to_upper(
                name_the());

        msg_log::add(actor_name_the + mon_disappear_msg);
    }

    if (!is_player())
    {
        static_cast<Mon*>(this)->player_aware_of_me_counter_ = 0;
    }

    properties_->end_prop_silent(PropId::entangled);

    // Hostile void travelers "intercepts" players teleporting, and calls the
    // player to them
    bool is_affected_by_void_traveler = false;

    if (is_player())
    {
        for (Actor* const actor : game_time::actors)
        {
            const auto actor_id = actor->id();

            const bool is_void_traveler =
                (actor_id == ActorId::void_traveler) ||
                (actor_id == ActorId::elder_void_traveler);

            if (is_void_traveler &&
                (actor->state() == ActorState::alive) &&
                actor->properties().allow_act() &&
                !actor->is_actor_my_leader(map::player) &&
                (static_cast<Mon*>(actor)->aware_of_player_counter_ > 0))
            {
                std::vector<P> p_bucket;

                for (const P& d : dir_utils::dir_list)
                {
                    const P adj_p(actor->pos + d);

                    if (!blocked[adj_p.x][adj_p.y])
                    {
                        p_bucket.push_back(adj_p);
                    }
                }

                if (!p_bucket.empty())
                {
                    // Set new teleport destination
                    p = rnd::element(p_bucket);

                    const std::string actor_name_a =
                        text_format::first_to_upper(
                            actor->name_a());

                    msg_log::add(actor_name_a +
                                 " intercepts my teleportation!");

                    static_cast<Mon*>(actor)->become_aware_player(false);

                    is_affected_by_void_traveler = true;

                    break;
                }
            }
        }
    }

    // Update actor position to new position
    pos = p;

    map::update_vision();

    // When the player teleports, remove awareness for all monsters not seeing
    // the player after the teleport
    if (is_player())
    {
        bool blocks_los[map_w][map_h];

        const R r = fov::get_fov_rect(pos);

        map_parsers::BlocksLos()
            .run(blocks_los, MapParseMode::overwrite, r);

        for (auto* const actor : game_time::actors)
        {
            if (actor == map::player)
            {
                continue;
            }

            auto* const mon = static_cast<Mon*>(actor);

            if (!mon->can_see_actor(*this, blocks_los))
            {
                mon->aware_of_player_counter_ = 0;
            }
        }
    }

    const auto player_seen_actors = map::player->seen_actors();

    for (Actor* const actor : player_seen_actors)
    {
        static_cast<Mon*>(actor)->set_player_aware_of_me();
    }

    if (is_player() &&
        (!properties_->has_prop(PropId::tele_ctrl) ||
         properties_->has_prop(PropId::confused) ||
         is_affected_by_void_traveler))
    {
        msg_log::add("I suddenly find myself in a different location!");

        auto prop = new PropConfused();

        prop->set_duration(8);

        properties_->apply(prop);
    }
}

TileId Actor::tile() const
{
    if (is_corpse())
    {
        return TileId::corpse2;
    }

    return data_->tile;
}

char Actor::character() const
{
    if (is_corpse())
    {
        return '&';
    }

    return data_->character;
}

bool Actor::restore_hp(const int hp_restored,
                       const bool is_allowed_above_max,
                       const Verbosity verbosity)
{
    bool is_hp_gained = is_allowed_above_max;

    const int dif_from_max = hp_max(true) - hp_restored;

    // If hp is below limit, but restored hp will push it over the limit, hp is
    // set to max.
    if (!is_allowed_above_max &&
        (hp() > dif_from_max) &&
        (hp() < hp_max(true)))
    {
        hp_ = hp_max(true);
        is_hp_gained = true;
    }

    // If hp is below limit, and restored hp will NOT push it over the limit -
    // restored hp is added to current.
    if (is_allowed_above_max ||
        (hp() <= dif_from_max))
    {
        hp_ += hp_restored;
        is_hp_gained = true;
    }

    if ((verbosity == Verbosity::verbose) &&
        (is_hp_gained))
    {
        if (is_player())
        {
            msg_log::add("I feel healthier!", colors::msg_good());
        }
        else // Is a monster
        {
            if (map::player->can_see_actor(*this))
            {
                const std::string actor_name_the =
                    text_format::first_to_upper(
                        data_->name_the);

                msg_log::add(actor_name_the + " looks healthier.");
            }
        }
    }

    return is_hp_gained;
}

bool Actor::restore_spi(const int spi_restored,
                        const bool is_allowed_above_max,
                        const Verbosity verbosity)
{
    // Maximum allowed level to increase spirit to
    // * If we allow above max, we can raise spirit "infinitely"
    // * Otherwise we cap to max spirit, or current spirit, whichever is higher
    const int limit =
        is_allowed_above_max ?
        INT_MAX :
        std::max(spi_, spi_max());

    const int spi_before = spi_;

    spi_ = std::min(spi_ + spi_restored, limit);

    const bool is_spi_gained = spi_ > spi_before;

    if (verbosity == Verbosity::verbose &&
        is_spi_gained)
    {
        if (is_player())
        {
            msg_log::add("I feel more spirited!", colors::msg_good());
        }
        else
        {
            if (map::player->can_see_actor(*this))
            {
                const std::string actor_name_the =
                    text_format::first_to_upper(
                        data_->name_the);

                msg_log::add(actor_name_the + " looks more spirited.");
            }
        }
    }

    return is_spi_gained;
}

void Actor::set_hp_and_spi_to_max()
{
    hp_ = hp_max(true);
    spi_ = spi_max();
}

void Actor::change_max_hp(const int change, const Verbosity verbosity)
{
    hp_max_ = std::max(1, hp_max_ + change);

    if (verbosity == Verbosity::verbose)
    {
        if (is_player())
        {
            if (change > 0)
            {
                msg_log::add("I feel more vigorous!", colors::msg_good());
            }
            else if (change < 0)
            {
                msg_log::add("I feel frailer!", colors::msg_bad());
            }
        }
        else // Is monster
        {
            if (map::player->can_see_actor(*this))
            {
                const std::string actor_name_the =
                    text_format::first_to_upper(
                        name_the());

                if (change > 0)
                {
                    msg_log::add(actor_name_the + " looks more vigorous.");
                }
                else if (change < 0)
                {
                    msg_log::add(actor_name_the + " looks frailer.");
                }
            }
        }
    }
}

void Actor::change_max_spi(const int change, const Verbosity verbosity)
{
    spi_max_ = std::max(1, spi_max_ + change);

    if (verbosity == Verbosity::verbose)
    {
        if (is_player())
        {
            if (change > 0)
            {
                msg_log::add("My spirit is stronger!", colors::msg_good());
            }
            else if (change < 0)
            {
                msg_log::add("My spirit is weaker!", colors::msg_bad());
            }
        }
        else // Is monster
        {
            if (map::player->can_see_actor(*this))
            {
                const std::string actor_name_the =
                    text_format::first_to_upper(
                        name_the());

                if (change > 0)
                {
                    msg_log::add(actor_name_the +
                                 " appears to grow in spirit.");
                }
                else if (change < 0)
                {
                    msg_log::add(actor_name_the +
                                 " appears to shrink in spirit.");
                }
            }
        }
    }
}

ActorDied Actor::hit(int dmg,
                     const DmgType dmg_type,
                     const DmgMethod method,
                     const AllowWound allow_wound)
{
    const int hp_pct_before = (hp() * 100) / hp_max(true);

    if (state_ == ActorState::destroyed)
    {
        TRACE_FUNC_END_VERBOSE;

        return ActorDied::no;
    }

    // Damage type is "light", and actor is not light sensitive?
    if (dmg_type == DmgType::light &&
        !properties_->has_prop(PropId::light_sensitive))
    {
        return ActorDied::no;
    }

    if (is_player())
    {
        map::player->interrupt_actions();
    }

    // Damage to corpses
    if (is_corpse() && !is_player())
    {
        ASSERT(data_->can_leave_corpse);

        // Chance to destroy is X in Y, where
        // X = damage dealt * 4
        // Y = maximum actor hit points

        const int den = hp_max(true);

        const int num = std::min(dmg * 4, den);

        if (rnd::fraction(num, den))
        {
            if ((method == DmgMethod::kicking) ||
                (method == DmgMethod::blunt) ||
                (method == DmgMethod::slashing) ||
                (method == DmgMethod::piercing))
            {
                Snd snd("*Crack!*",
                        SfxId::hit_corpse_break,
                        IgnoreMsgIfOriginSeen::yes,
                        pos,
                        nullptr,
                        SndVol::low,
                        AlertsMon::yes);

                snd.run();
            }

            destroy();

            if (is_humanoid())
            {
                map::make_gore(pos);
            }

            if (map::cells[pos.x][pos.y].is_seen_by_player)
            {
                msg_log::add(text_format::first_to_upper(corpse_name_the()) +
                             " is destroyed.");
            }
        }
        else // Not destroyed
        {
            if ((method == DmgMethod::kicking) ||
                (method == DmgMethod::blunt) ||
                (method == DmgMethod::slashing) ||
                (method == DmgMethod::piercing))
            {
                const std::string msg =
                    ((method == DmgMethod::blunt) ||
                     (method == DmgMethod::kicking)) ?
                    "*Thud!*" :
                    "*Chop!*";

                Snd snd(msg,
                        SfxId::hit_medium,
                        IgnoreMsgIfOriginSeen::yes,
                        pos,
                        nullptr,
                        SndVol::low,
                        AlertsMon::yes);

                snd.run();
            }
        }

        return ActorDied::no;
    }

    if (dmg_type == DmgType::spirit)
    {
        return hit_spi(dmg);
    }

    // Property resists damage?
    const auto verbosity =
        is_alive() ?
        Verbosity::verbose :
        Verbosity::silent;

    const bool is_dmg_resisted =
        properties_->is_resisting_dmg(dmg_type, verbosity);

    if (is_dmg_resisted)
    {
        return ActorDied::no;
    }

    // TODO: Perhaps allow zero damage?
    dmg = std::max(1, dmg);

    if (dmg_type == DmgType::physical)
    {
        dmg = hit_armor(dmg);
    }

    // Fire/electricity damage reduction from the Resistant trait?
    if (((dmg_type == DmgType::fire) ||
         (dmg_type == DmgType::electric)) &&
        is_player() &&
        player_bon::traits[(size_t)Trait::resistant])
    {
        dmg /= 2;
    }

    // TODO: Perhaps allow zero damage?
    dmg = std::max(1, dmg);

    on_hit(dmg,
           dmg_type,
           method,
           allow_wound);

    properties_->on_hit();

    // TODO: Perhaps allow zero damage?
    dmg = std::max(1, dmg);

    if (!(is_player() && config::is_bot_playing()))
    {
        hp_ -= dmg;
    }

    if (hp() <= 0)
    {
        const bool is_on_bottomless =
            map::cells[pos.x][pos.y].rigid->is_bottomless();

        // Destroy the corpse if the killing blow damage is either:
        //
        // * Above a threshold relative to the actor's maximum hit points, or
        // * Above a fixed value threshold
        //
        // The purpose of the first case is to make it very likely that small
        // creatures like rats are destroyed.
        //
        // The purpose of the second point is that powerful attacks like
        // explosions should always destroy the corpse, even if the creature
        // has a very high pool of hit points.

        const int dmg_threshold_relative = (hp_max(true) * 3) / 2;

        const int dmg_threshold_absolute = 14;

        const bool is_destroyed =
            !data_->can_leave_corpse ||
            is_on_bottomless ||
            has_prop(PropId::summoned) ||
            (dmg >= dmg_threshold_relative) ||
            (dmg >= dmg_threshold_absolute);

        die(is_destroyed,       // Is destroyed
            !is_on_bottomless,  // Allow gore
            !is_on_bottomless); // Allow dropping items

        return ActorDied::yes;
    }

    // HP is greater than 0

    const int hp_pct_after = (hp() * 100) / hp_max(true);

    const int hp_warn_lvl = 25;

    if (is_player() &&
        (hp_pct_before > hp_warn_lvl) &&
        (hp_pct_after <= hp_warn_lvl))
    {
        msg_log::add("-LOW HP WARNING!-",
                     colors::msg_bad(),
                     true,
                     MorePromptOnMsg::yes);
    }

    return ActorDied::no;
}

ActorDied Actor::hit_spi(const int dmg, const Verbosity verbosity)
{
    if (verbosity == Verbosity::verbose)
    {
        if (is_player())
        {
            msg_log::add("My spirit is drained!", colors::msg_bad());
        }
    }

    properties_->on_hit();

    if (!is_player() || !config::is_bot_playing())
    {
        spi_ = std::max(0, spi_ - dmg);
    }

    if (spi() <= 0)
    {
        if (is_player())
        {
            msg_log::add("All my spirit is depleted, I am devoid of life!",
                         colors::msg_bad());
        }
        else // Is monster
        {
            if (map::player->can_see_actor(*this))
            {
                const std::string actor_name_the =
                    text_format::first_to_upper(
                        name_the());

                msg_log::add(actor_name_the + " has no spirit left!");
            }
        }

        const bool is_on_bottomless =
            map::cells[pos.x][pos.y].rigid->is_bottomless();

        const bool is_destroyed =
            !data_->can_leave_corpse ||
            is_on_bottomless ||
            has_prop(PropId::summoned);

        die(is_destroyed, false, true);

        return ActorDied::yes;
    }

    // SP is greater than 0

    return ActorDied::no;
}

int Actor::armor_points() const
{
    int ap = 0;

    // Worn armor
    if (is_humanoid())
    {
        Armor* armor =
            static_cast<Armor*>(inv_->item_in_slot(SlotId::body));

        if (armor)
        {
            ap += armor->armor_points();
        }
    }

    // "Natural armor"
    if (is_player())
    {
        if (player_bon::traits[(size_t)Trait::thick_skinned])
        {
            ++ap;
        }
    }

    return ap;
}

int Actor::hit_armor(int dmg)
{
    //
    // NOTE: We retrieve armor points BEFORE damaging the armor - since it
    //       should reduce damage taken even if it gets damaged or destroyed
    //
    const int ap = armor_points();

    // Danage worn armor
    if (is_humanoid())
    {
        Armor* armor =
            static_cast<Armor*>(inv_->item_in_slot(SlotId::body));

        if (armor)
        {
            TRACE_VERBOSE << "Has armor, running hit on armor" << std::endl;

            armor->hit(dmg);

            if (armor->is_destroyed())
            {
                TRACE << "Armor was destroyed" << std::endl;

                if (is_player())
                {
                    const std::string armor_name =
                        armor->name(ItemRefType::plain, ItemRefInf::none);

                    msg_log::add("My " + armor_name + " is torn apart!",
                                 colors::msg_note());
                }

                inv_->remove_item_in_slot(SlotId::body, true);

                armor = nullptr;
            }
        }
    }

    // Reduce damage by the total ap value - the new damage value may be
    // negative, this is the callers resonsibility to handle
    dmg -= ap;

    return dmg;
}

void Actor::die(const bool is_destroyed,
                const bool allow_gore,
                const bool allow_drop_items)
{
    TRACE_FUNC_BEGIN_VERBOSE;

    // Save player with Talisman of Resurrection? (If died by physical damage)
    if (is_player() &&
        (map::player->ins() < 100) &&
        (spi() > 0) &&
        inv_->has_item_in_backpack(ItemId::resurrect_talisman))
    {
        inv_->decr_item_type_in_backpack(ItemId::resurrect_talisman);

        msg_log::more_prompt();

        io::clear_screen();

        io::update_screen();

        const std::string msg =
            "Strange emptiness surrounds me. An eternity passes as I lay "
            "frozen in a world of shadows. Suddenly I awake!";

        popup::msg(msg, "Dead");

        restore_hp(999,
                   false,
                   Verbosity::silent);

        properties_->end_prop_silent(PropId::wound);

        // If player died due to falling down a chasm, go to next level
        if (map::cells[pos.x][pos.y].rigid->is_bottomless())
        {
            map_travel::go_to_nxt();
        }

        msg_log::add("I LIVE AGAIN!");

        game::add_history_event("I was brought back from the dead.");

        map::player->incr_shock(ShockLvl::mind_shattering,
                                ShockSrc::misc);

        return;
    }

    ASSERT(data_->can_leave_corpse || is_destroyed);

    // Check all monsters and unset this actor as leader
    for (Actor* other : game_time::actors)
    {
        if (other != this &&
            !other->is_player() &&
            is_leader_of(other))
        {
            static_cast<Mon*>(other)->leader_ = nullptr;
        }
    }

    if (!is_player())
    {
        // If this monster is player's target, unset the target
        if (map::player->tgt_ == this)
        {
            map::player->tgt_ = nullptr;
        }

        TRACE_VERBOSE << "Printing death message" << std::endl;

        if (map::player->can_see_actor(*this))
        {
            const std::string msg = death_msg();

            if (!msg.empty())
            {
                msg_log::add(msg);
            }
        }
    }

    if (is_destroyed)
    {
        destroy();
    }
    else // Not destroyed
    {
        state_ = ActorState::corpse;
    }

    if (!is_player())
    {
        // This is a monster

        if (is_humanoid())
        {
            TRACE_VERBOSE << "Emitting death sound" << std::endl;

            Snd snd("I hear agonized screaming.",
                    SfxId::END,
                    IgnoreMsgIfOriginSeen::yes,
                    pos,
                    this,
                    SndVol::high,
                    AlertsMon::no);

            snd.run();
        }
    }

    if (is_destroyed)
    {
        if (data_->can_bleed && allow_gore)
        {
            map::make_gore(pos);
            map::make_blood(pos);
        }
    }
    else // Not destroyed
    {
        if (!is_player())
        {
            P new_pos;

            auto* feature_here = map::cells[pos.x][pos.y].rigid;

            // TODO: this should be decided with a floodfill instead
            if (!feature_here->can_have_corpse())
            {
                for (int dx = -1; dx <= 1; ++dx)
                {
                    for (int dy = -1; dy <= 1; ++dy)
                    {
                        new_pos = pos + P(dx, dy);
                        feature_here = map::cells[pos.x + dx][pos.y + dy].rigid;

                        if (feature_here->can_have_corpse())
                        {
                            pos.set(new_pos);
                            dx = 9999;
                            dy = 9999;
                        }
                    }
                }
            }
        }
    }

    on_death();

    properties_->on_death();

    if (!is_player())
    {
        if (allow_drop_items)
        {
            item_drop::drop_all_characters_items(*this);
        }

        game::on_mon_killed(*this);

        static_cast<Mon*>(this)->leader_ = nullptr;
    }

    TRACE_FUNC_END_VERBOSE;
}

void Actor::destroy()
{
    state_ = ActorState::destroyed;

    properties_->on_destroyed();
}

std::string Actor::death_msg() const
{
    const std::string actor_name_the =
        text_format::first_to_upper(
            name_the());

    std::string msg_end = "";

    if (data_->death_msg_override.empty())
    {
            msg_end = "dies.";
    }
    else
    {
            msg_end = data_->death_msg_override;
    }

    return actor_name_the + " " + msg_end;
}

DidAction Actor::try_eat_corpse()
{
    const bool actor_is_player = is_player();

    PropWound* wound = nullptr;

    if (actor_is_player)
    {
        Prop* prop = properties_->prop(PropId::wound);

        if (prop)
        {
            wound = static_cast<PropWound*>(prop);
        }
    }

    if ((hp() >= hp_max(true)) && !wound)
    {
        // Not "hungry"
        return DidAction::no;
    }

    Actor* corpse = nullptr;

    // Check all corpses here, if this is the player eating, stop at any corpse
    // which is prioritized for bashing (Zombies)
    for (Actor* const actor : game_time::actors)
    {
        if ((actor->pos == pos) &&
            (actor->state() == ActorState::corpse))
        {
            corpse = actor;

            if (actor_is_player &&
                actor->data().prio_corpse_bash)
            {
                break;
            }
        }
    }

    if (corpse)
    {
        const int corpse_max_hp = corpse->hp_max(false);

        const int destr_one_in_n = constr_in_range(1, corpse_max_hp / 4, 8);

        const bool is_destroyed = rnd::one_in(destr_one_in_n);

        const std::string corpse_name_the = corpse->corpse_name_the();

        Snd snd("I hear ripping and chewing.",
                SfxId::bite,
                IgnoreMsgIfOriginSeen::yes,
                pos,
                this,
                SndVol::low,
                AlertsMon::no,
                MorePromptOnMsg::no);

        snd.run();

        if (actor_is_player)
        {
            msg_log::add("I feed on " + corpse_name_the + ".");
        }
        else // Is monster
        {
            if (map::player->can_see_actor(*this))
            {
                const std::string actor_name_the =
                    text_format::first_to_upper(
                        name_the());

                msg_log::add(actor_name_the +
                             " feeds on " +
                             corpse_name_the +
                             ".");
            }
        }

        if (is_destroyed)
        {
            corpse->destroy();

            map::make_gore(pos);
            map::make_blood(pos);
        }

        if (actor_is_player && is_destroyed)
        {
            msg_log::add(text_format::first_to_upper(corpse_name_the) +
                         " is completely devoured.");

            std::vector<Actor*> corpses_here;

            for (auto* const actor : game_time::actors)
            {
                if ((actor->pos == pos) &&
                    (actor->state() == ActorState::corpse))
                {
                    corpses_here.push_back(actor);
                }
            }

            if (!corpses_here.empty())
            {
                msg_log::more_prompt();

                for (auto* const corpse : corpses_here)
                {
                    const std::string name =
                        text_format::first_to_upper(
                            corpse->corpse_name_a());

                    msg_log::add(name + ".");
                }
            }
        }

        // Heal
        on_feed();

        return DidAction::yes;
    }

    return DidAction::no;
}

void Actor::on_feed()
{
    const int hp_restored = rnd::range(3, 5);

    restore_hp(hp_restored, false, Verbosity::silent);

    if (is_player())
    {
        Prop* const prop = properties_->prop(PropId::wound);

        if (prop && rnd::one_in(6))
        {
            PropWound* const wound = static_cast<PropWound*>(prop);

            wound->heal_one_wound();
        }
    }
}

void Actor::add_light(bool light_map[map_w][map_h]) const
{
    if (state_ == ActorState::alive &&
        properties_->has_prop(PropId::radiant))
    {
        // TODO: Much of the code below is duplicated from
        // ActorPlayer::add_light_hook(), some refactoring is needed.

        bool hard_blocked[map_w][map_h];

         const R fov_lmt = fov::get_fov_rect(pos);

        map_parsers::BlocksLos()
            .run(hard_blocked,
                 MapParseMode::overwrite,
                 fov_lmt);

        LosResult fov[map_w][map_h];

        fov::run(pos, hard_blocked, fov);

        for (int y = fov_lmt.p0.y; y <= fov_lmt.p1.y; ++y)
        {
            for (int x = fov_lmt.p0.x; x <= fov_lmt.p1.x; ++x)
            {
                if (!fov[x][y].is_blocked_hard)
                {
                    light_map[x][y] = true;
                }
            }
        }
    }
    else if (properties_->has_prop(PropId::burning))
    {
        for (int dx = -1; dx <= 1; ++dx)
        {
            for (int dy = -1; dy <= 1; ++dy)
            {
                light_map[pos.x + dx][pos.y + dy] = true;
            }
        }
    }

    add_light_hook(light_map);
}

bool Actor::is_player() const
{
    return this == map::player;
}
