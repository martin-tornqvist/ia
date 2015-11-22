#include "actor.hpp"

#include "init.hpp"

#include "render.hpp"
#include "game_time.hpp"
#include "actor_player.hpp"
#include "actor_mon.hpp"
#include "map.hpp"
#include "fov.hpp"
#include "msg_log.hpp"
#include "feature_trap.hpp"
#include "drop.hpp"
#include "explosion.hpp"
#include "dungeon_master.hpp"
#include "inventory.hpp"
#include "map_parsing.hpp"
#include "item.hpp"
#include "utils.hpp"
#include "input.hpp"
#include "marker.hpp"
#include "look.hpp"

Actor::Actor() :
    pos             (),
    state_          (Actor_state::alive),
    clr_            (clr_black),
    glyph_          (' '),
    tile_           (Tile_id::empty),
    hp_             (-1),
    hp_max_         (-1),
    spi_            (-1),
    spi_max_        (-1),
    lair_pos_      (),
    prop_handler_   (nullptr),
    data_           (nullptr),
    inv_            (nullptr) {}

Actor::~Actor()
{
    //Free all items owning actors
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
    delete prop_handler_;
}

bool Actor::has_prop(const Prop_id id) const
{
    return prop_handler_->has_prop(id);
}

int Actor::ability(const Ability_id id, const bool IS_AFFECTED_BY_PROPS) const
{
    return data_->ability_vals.val(id, IS_AFFECTED_BY_PROPS, *this);
}

bool Actor::is_spotting_sneaking_actor(Actor& other)
{
    const P& other_pos = other.pos;

    const int   PLAYER_SEARCH_MOD   = is_player() ?
                                      (ability(Ability_id::searching, true) / 3) : 0;

    const auto& abilities_other     = other.data().ability_vals;

    const int   SNEAK_SKILL         = abilities_other.val(Ability_id::stealth, true, other);

    const int   DIST                = utils::king_dist(pos, other_pos);

    const int   SNEAK_DIST_MOD      = utils::constr_in_range(0, (DIST - 1) * 10, 60);

    const Cell& cell                = map::cells[other_pos.x][other_pos.y];

    const int   SNEAK_LGT_MOD       = cell.is_lit                     ? -40 : 0;
    const int   SNEAK_DRK_MOD       = (cell.is_dark && ! cell.is_lit) ?  40 : 0;

    const int   SNEAK_TOT           = utils::constr_in_range(
                                          0,
                                          SNEAK_SKILL     +
                                          SNEAK_DIST_MOD  +
                                          SNEAK_LGT_MOD   +
                                          SNEAK_DRK_MOD   -
                                          PLAYER_SEARCH_MOD,
                                          99);

    return ability_roll::roll(SNEAK_TOT, &other) <= fail;
}

int Actor::hp_max(const bool WITH_MODIFIERS) const
{
    return WITH_MODIFIERS ? prop_handler_->affect_max_hp(hp_max_) : hp_max_;
}

Actor_speed Actor::speed() const
{
    const auto base_speed = data_->speed;

    int speed_int = int(base_speed);

    //"Slowed" gives speed penalty
    if (prop_handler_->has_prop(Prop_id::slowed) && speed_int > 0)
    {
        --speed_int;
    }

    //"Hasted" or "frenzied" gives speed bonus.
    if (
        (prop_handler_->has_prop(Prop_id::hasted) ||
         prop_handler_->has_prop(Prop_id::frenzied)) &&
        speed_int < int(Actor_speed::END) - 1)
    {
        ++speed_int;
    }

    assert(speed_int >= 0 && speed_int < int(Actor_speed::END));

    return Actor_speed(speed_int);
}

void Actor::seen_foes(std::vector<Actor*>& out)
{
    out.clear();

    bool blocked_los[MAP_W][MAP_H];

    if (!is_player())
    {
        Rect los_rect(std::max(0,         pos.x - FOV_STD_RADI_INT),
                      std::max(0,         pos.y - FOV_STD_RADI_INT),
                      std::min(MAP_W - 1, pos.x + FOV_STD_RADI_INT),
                      std::min(MAP_H - 1, pos.y + FOV_STD_RADI_INT));

        map_parse::run(cell_check::Blocks_los(),
                       blocked_los,
                       Map_parse_mode::overwrite,
                       los_rect);
    }

    for (Actor* actor : game_time::actors)
    {
        if (actor != this && actor->is_alive())
        {
            if (is_player())
            {
                if (map::player->can_see_actor(*actor) && !is_leader_of(actor))
                {
                    out.push_back(actor);
                }
            }
            else //Not player
            {
                const bool IS_HOSTILE_TO_PLAYER = !is_actor_my_leader(map::player);

                const bool IS_OTHER_HOSTILE_TO_PLAYER = actor->is_player() ? false :
                                                        !actor->is_actor_my_leader(map::player);

                const bool IS_ENEMY = IS_HOSTILE_TO_PLAYER != IS_OTHER_HOSTILE_TO_PLAYER;

                const Mon* const mon = static_cast<const Mon*>(this);

                if (IS_ENEMY && mon->can_see_actor(*actor, blocked_los))
                {
                    out.push_back(actor);
                }
            }
        }
    }
}

void Actor::place(const P& pos_, Actor_data_t& actor_data)
{
    pos         = pos_;
    data_       = &actor_data;
    state_      = Actor_state::alive;
    clr_        = data_->color;
    glyph_      = data_->glyph;
    tile_       = data_->tile;
    hp_         = hp_max_  = data_->hp;
    spi_        = spi_max_ = data_->spi;
    lair_pos_   = pos;

    inv_ = new Inventory(this);

    prop_handler_ = new Prop_handler(this);
    prop_handler_->init_natural_props();

    if (data_->id != Actor_id::player)
    {
        mk_start_items();
    }

    place_hook();

    update_clr();
}

void Actor::on_std_turn_common()
{
    //Do light damage if in lit cell
    if (map::cells[pos.x][pos.y].is_lit)
    {
        hit(1, Dmg_type::light);
    }

    if (is_alive())
    {
        //Slowly decrease current HP/spirit if above max
        const int DECR_ABOVE_MAX_N_TURNS = 7;

        if (hp() > hp_max(true) && game_time::turn() % DECR_ABOVE_MAX_N_TURNS == 0)
        {
            --hp_;
        }

        if (spi() > spi_max() && game_time::turn() % DECR_ABOVE_MAX_N_TURNS == 0)
        {
            --spi_;
        }

        //Regenerate spirit
        int regen_spi_n_turns = 15;

        if (is_player())
        {
            if (player_bon::traits[size_t(Trait::stout_spirit)])
            {
                regen_spi_n_turns -= 3;
            }

            if (player_bon::traits[size_t(Trait::strong_spirit)])
            {
                regen_spi_n_turns -= 3;
            }

            if (player_bon::traits[size_t(Trait::mighty_spirit)])
            {
                regen_spi_n_turns -= 3;
            }
        }
        else //Is monster
        {
            //Monsters regen spirit very quickly, so spell casters don't suddenly get
            //completely handicapped
            regen_spi_n_turns = 2;
        }

        if (game_time::turn() % regen_spi_n_turns == 0)
        {
            restore_spi(1, false, Verbosity::silent);
        }
    }

    on_std_turn();
}

void Actor::teleport()
{
    bool blocked[MAP_W][MAP_H];
    map_parse::run(cell_check::Blocks_actor(*this, true), blocked);

    std::vector<P> pos_bucket;
    utils::mk_vector_from_bool_map(false, blocked, pos_bucket);

    if (pos_bucket.empty())
    {
        return;
    }

    if (!is_player() && map::player->can_see_actor(*this))
    {
        msg_log::add(name_the() + " suddenly disappears!");
    }

    P   tgt_pos                   = pos_bucket[rnd::range(0, pos_bucket.size() - 1)];
    bool  player_has_tele_control   = false;

    if (is_player())
    {
        map::player->update_fov();
        render::draw_map_and_interface();
        map::cpy_render_array_to_visual_memory();

        //Teleport control?

        if (
            prop_handler_->has_prop(Prop_id::tele_ctrl) &&
            !prop_handler_->has_prop(Prop_id::confused))
        {
            player_has_tele_control = true;

            auto chance_of_tele_success = [](const P & tgt)
            {
                const int DIST = utils::king_dist(map::player->pos, tgt);
                return utils::constr_in_range(25, 100 - DIST, 95);
            };

            auto on_marker_at_pos = [chance_of_tele_success](const P & p)
            {
                msg_log::clear();
                look::print_location_info_msgs(p);

                const int CHANCE_PCT = chance_of_tele_success(p);

                msg_log::add(to_str(CHANCE_PCT) + "% chance of success.");

                msg_log::add("[enter] to teleport here");
                msg_log::add(cancel_info_str_no_space);
            };

            auto on_key_press = [](const P & p, const Key_data & key_data)
            {
                (void)p;

                if (key_data.sdl_key == SDLK_RETURN)
                {
                    msg_log::clear();
                    return Marker_done::yes;
                }

                return Marker_done::no;
            };

            msg_log::add("I have the power to control teleportation.",
                         clr_white,
                         false,
                         More_prompt_on_msg::yes);

            const P marker_tgt_pos = marker::run(Marker_use_player_tgt::no,
                                                 on_marker_at_pos,
                                                 on_key_press,
                                                 Marker_show_blocked::no);

            if (blocked[marker_tgt_pos.x][marker_tgt_pos.y])
            {
                //Blocked
                msg_log::add("Something is blocking me...",
                             clr_white,
                             false,
                             More_prompt_on_msg::yes);
            }
            else if (rnd::percent(chance_of_tele_success(marker_tgt_pos)))
            {
                //Success
                tgt_pos = marker_tgt_pos;
            }
            else //Distance roll failed
            {
                msg_log::add("I failed to go there...",
                             clr_white,
                             false,
                             More_prompt_on_msg::yes);
            }
        }
    }
    else //Is a monster
    {
        static_cast<Mon*>(this)->player_aware_of_me_counter_ = 0;
    }

    pos = tgt_pos;

    if (is_player())
    {
        map::player->update_fov();

        std::vector<Actor*> player_seen_foes;
        map::player->seen_foes(player_seen_foes);

        for (Actor* const actor : player_seen_foes)
        {
            static_cast<Mon*>(actor)->set_player_aware_of_me();
        }

        render::draw_map_and_interface();

        map::cpy_render_array_to_visual_memory();

        if (!player_has_tele_control)
        {
            msg_log::add("I suddenly find myself in a different location!");
            prop_handler_->try_add_prop(new Prop_confused(Prop_turns::specific, 8));
        }
    }
}

void Actor::update_clr()
{
    if (state_ != Actor_state::alive)
    {
        clr_ = data_->color;
        return;
    }

    if (prop_handler_->affect_actor_clr(clr_))
    {
        return;
    }

    if (is_player() && map::player->active_explosive)
    {
        clr_ = clr_yellow;
        return;
    }

    clr_ = data_->color;
}

bool Actor::restore_hp(const int HP_RESTORED, const bool IS_ALLOWED_ABOVE_MAX,
                       const Verbosity verbosity)
{
    bool        is_hp_gained    = IS_ALLOWED_ABOVE_MAX;
    const int   DIF_FROM_MAX    = hp_max(true) - HP_RESTORED;

    //If hp is below limit, but restored hp will push it over the limit, HP is set to max.
    if (!IS_ALLOWED_ABOVE_MAX && hp() > DIF_FROM_MAX && hp() < hp_max(true))
    {
        hp_             = hp_max(true);
        is_hp_gained    = true;
    }

    //If HP is below limit, and restored hp will NOT push it over the limit -
    //restored hp is added to current.
    if (IS_ALLOWED_ABOVE_MAX || hp() <= DIF_FROM_MAX)
    {
        hp_ += HP_RESTORED;
        is_hp_gained = true;
    }

    update_clr();

    if (verbosity == Verbosity::verbose && is_hp_gained)
    {
        if (is_player())
        {
            msg_log::add("I feel healthier!", clr_msg_good);
        }
        else //Is a monster
        {
            if (map::player->can_see_actor(*this))
            {
                msg_log::add(data_->name_the + " looks healthier.");
            }
        }

        render::draw_map_and_interface();
    }

    return is_hp_gained;
}

bool Actor::restore_spi(const int SPI_RESTORED, const bool IS_ALLOWED_ABOVE_MAX,
                        const Verbosity verbosity)
{
    bool is_spi_gained = IS_ALLOWED_ABOVE_MAX;

    const int DIF_FROM_MAX = spi_max() - SPI_RESTORED;

    //If spi is below limit, but will be pushed over the limit, spi is set to max.
    if (!IS_ALLOWED_ABOVE_MAX && spi() > DIF_FROM_MAX && spi() < spi_max())
    {
        spi_        = spi_max();
        is_spi_gained = true;
    }

    //If spi is below limit, and will not NOT be pushed over the limit - restored spi is
    //added to current.
    if (IS_ALLOWED_ABOVE_MAX || spi() <= DIF_FROM_MAX)
    {
        spi_ += SPI_RESTORED;
        is_spi_gained = true;
    }

    if (verbosity == Verbosity::verbose && is_spi_gained)
    {
        if (is_player())
        {
            msg_log::add("I feel more spirited!", clr_msg_good);
        }
        else
        {
            if (map::player->can_see_actor(*this))
            {
                msg_log::add(data_->name_the + " looks more spirited.");
            }
        }

        render::draw_map_and_interface();
    }

    return is_spi_gained;
}

void Actor::set_hp_and_spi_to_max()
{
    hp_     = hp_max(true);
    spi_    = spi_max();
}

void Actor::change_max_hp(const int CHANGE, const Verbosity verbosity)
{
    hp_max_ = std::max(1, hp_max_ + CHANGE);

    if (verbosity == Verbosity::verbose)
    {
        if (is_player())
        {
            if (CHANGE > 0)
            {
                msg_log::add("I feel more vigorous!", clr_msg_good);
            }
            else if (CHANGE < 0)
            {
                msg_log::add("I feel frailer!", clr_msg_bad);
            }
        }
        else //Is monster
        {
            if (map::player->can_see_actor(*this))
            {
                if (CHANGE > 0)
                {
                    msg_log::add(name_the() + " looks more vigorous.");
                }
                else if (CHANGE < 0)
                {
                    msg_log::add(name_the() + " looks frailer.");
                }
            }
        }
    }
}

void Actor::change_max_spi(const int CHANGE, const Verbosity verbosity)
{
    spi_max_ = std::max(1, spi_max_ + CHANGE);

    if (verbosity == Verbosity::verbose)
    {
        if (is_player())
        {
            if (CHANGE > 0)
            {
                msg_log::add("My spirit is stronger!", clr_msg_good);
            }
            else if (CHANGE < 0)
            {
                msg_log::add("My spirit is weaker!", clr_msg_bad);
            }
        }
        else //Is monster
        {
            if (map::player->can_see_actor(*this))
            {
                if (CHANGE > 0)
                {
                    msg_log::add(name_the() + " appears to grow in spirit.");
                }
                else if (CHANGE < 0)
                {
                    msg_log::add(name_the() + " appears to shrink in spirit.");
                }
            }
        }
    }
}

Actor_died Actor::hit(int dmg,
                      const Dmg_type dmg_type,
                      const Dmg_method method,
                      const Allow_wound allow_wound)
{
    TRACE_FUNC_BEGIN_VERBOSE;

    if (state_ == Actor_state::destroyed)
    {
        TRACE_FUNC_END_VERBOSE;
        return Actor_died::no;
    }

    TRACE_VERBOSE << "Damage from parameter: " << dmg << std::endl;

    //Damage type is "light", and actor is not light sensitive?
    if (dmg_type == Dmg_type::light && !prop_handler_->has_prop(Prop_id::lgtSens))
    {
        return Actor_died::no;
    }

    if (is_player())
    {
        map::player->interrupt_actions();
    }

    //Damage to corpses
    //NOTE: Corpse is automatically destroyed if damage is high enough, otherwise it is
    //destroyed with a random chance
    if (is_corpse() && !is_player())
    {
        assert(data_->can_leave_corpse);

        if (rnd::fraction(3, 4) || dmg >= ((hp_max(true) * 2) / 3))
        {
            if (method == Dmg_method::kick)
            {
                Snd snd("*Crack!*",
                        Sfx_id::hit_corpse_break,
                        Ignore_msg_if_origin_seen::yes,
                        pos,
                        nullptr,
                        Snd_vol::low,
                        Alerts_mon::yes);

                snd_emit::run(snd);
            }

            state_ = Actor_state::destroyed;

            if (is_humanoid())
            {
                map::mk_gore(pos);
            }

            if (map::cells[pos.x][pos.y].is_seen_by_player)
            {
                const std::string corpse_name = corpse_name_the();

                assert(!corpse_name.empty());

                msg_log::add(corpse_name_the() + " is destroyed.");
            }
        }
        else //Not destroyed
        {
            if (method == Dmg_method::kick)
            {
                Snd snd("*Thud*",
                        Sfx_id::hit_medium,
                        Ignore_msg_if_origin_seen::yes,
                        pos,
                        nullptr,
                        Snd_vol::low,
                        Alerts_mon::yes);

                snd_emit::run(snd);
            }
        }

        TRACE_FUNC_END_VERBOSE;
        return Actor_died::no;
    }

    if (dmg_type == Dmg_type::spirit)
    {
        return hit_spi(dmg);
    }

    //Property resists?
    const auto verbosity = is_alive() ? Verbosity::verbose : Verbosity::silent;

    if (prop_handler_->try_resist_dmg(dmg_type, verbosity))
    {
        TRACE_FUNC_END_VERBOSE;
        return Actor_died::no;
    }

    //Filter damage through worn armor
    dmg = std::max(1, dmg);

    if (dmg_type == Dmg_type::physical && is_humanoid())
    {
        Armor* armor = static_cast<Armor*>(inv_->item_in_slot(Slot_id::body));

        if (armor)
        {
            TRACE_VERBOSE << "Has armor, running hit on armor" << std::endl;

            dmg = armor->take_dur_hit_and_get_reduced_dmg(dmg);

            if (armor->is_destroyed())
            {
                TRACE << "Armor was destroyed" << std::endl;

                if (is_player())
                {
                    const std::string armor_name =
                        armor->name(Item_ref_type::plain, Item_ref_inf::none);

                    msg_log::add("My " + armor_name + " is torn apart!", clr_msg_note);
                }

                delete armor;
                armor = nullptr;
                inv_->slots_[int(Slot_id::body)].item = nullptr;
            }
        }
    }

    on_hit(dmg, dmg_type, method, allow_wound);

    TRACE_VERBOSE << "Damage after on_hit(): " << dmg << std::endl;

    prop_handler_->on_hit();

    dmg = std::max(1, dmg);

    if (!is_player() || !config::is_bot_playing())
    {
        hp_ -= dmg;
    }

    if (hp() <= 0)
    {
        const bool IS_ON_BOTTOMLESS = map::cells[pos.x][pos.y].rigid->is_bottomless();

        const bool IS_DMG_ENOUGH_TO_DESTROY = dmg > ((hp_max(true) * 5) / 4);

        const bool IS_DESTROYED = !data_->can_leave_corpse  ||
                                  IS_ON_BOTTOMLESS          ||
                                  IS_DMG_ENOUGH_TO_DESTROY;

        die(IS_DESTROYED, !IS_ON_BOTTOMLESS, !IS_ON_BOTTOMLESS);

        TRACE_FUNC_END_VERBOSE;
        return Actor_died::yes;
    }
    else //HP is greater than 0
    {
        TRACE_FUNC_END_VERBOSE;
        return Actor_died::no;
    }
}

Actor_died Actor::hit_spi(const int DMG, const Verbosity verbosity)
{
    if (verbosity == Verbosity::verbose)
    {
        if (is_player())
        {
            msg_log::add("My spirit is drained!", clr_msg_bad);
        }
    }

    prop_handler_->on_hit();

    if (!is_player() || !config::is_bot_playing())
    {
        spi_ = std::max(0, spi_ - DMG);
    }

    if (spi() <= 0)
    {
        if (is_player())
        {
            msg_log::add("All my spirit is depleted, I am devoid of life!", clr_msg_bad);
        }
        else //Is monster
        {
            if (map::player->can_see_actor(*this))
            {
                msg_log::add(name_the() + " has no spirit left!");
            }
        }

        const bool IS_ON_BOTTOMLESS = map::cells[pos.x][pos.y].rigid->is_bottomless();
        const bool IS_DESTROYED     = !data_->can_leave_corpse || IS_ON_BOTTOMLESS;

        die(IS_DESTROYED, false, true);
        return Actor_died::yes;
    }

    return Actor_died::no;
}

void Actor::die(const bool IS_DESTROYED, const bool ALLOW_GORE, const bool ALLOW_DROP_ITEMS)
{
    assert(data_->can_leave_corpse || IS_DESTROYED);

    //Check all monsters and unset this actor as leader
    for (Actor* actor : game_time::actors)
    {
        if (actor != this && !actor->is_player() && is_leader_of(actor))
        {
            static_cast<Mon*>(actor)->leader_ = nullptr;
        }
    }

    bool is_player_see_dying_actor = true;

    if (!is_player())
    {
        //If this monster is player's target, unset the target
        if (map::player->tgt_ == this)
        {
            map::player->tgt_ = nullptr;
        }

        //Print death messages
        if (map::player->can_see_actor(*this))
        {
            is_player_see_dying_actor = true;

            const std::string msg = death_msg();

            if (!msg.empty())
            {
                msg_log::add(msg);
            }
        }
    }

    if (IS_DESTROYED)
    {
        state_ = Actor_state::destroyed;
    }
    else //Not destroyed
    {
        state_ = Actor_state::corpse;
    }

    if (!is_player() && is_humanoid())
    {
        Snd snd("I hear agonized screaming.",
                Sfx_id::END,
                Ignore_msg_if_origin_seen::yes,
                pos,
                this,
                Snd_vol::low,
                Alerts_mon::no);

        snd_emit::run(snd);
    }

    if (ALLOW_DROP_ITEMS)
    {
        item_drop::drop_all_characters_items(*this);
    }

    if (IS_DESTROYED)
    {
        if (data_->can_bleed && ALLOW_GORE)
        {
            map::mk_gore(pos);
            map::mk_blood(pos);
        }
    }
    else //Not destroyed
    {
        if (!is_player())
        {
            P new_pos;
            auto* feature_here = map::cells[pos.x][pos.y].rigid;

            //TODO: this should be decided with a floodfill instead
            if (!feature_here->can_have_corpse())
            {
                for (int dx = -1; dx <= 1; ++dx)
                {
                    for (int dy = -1; dy <= 1; ++dy)
                    {
                        new_pos      = pos + P(dx, dy);
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

        glyph_ = '&';
        tile_ = Tile_id::corpse2;
    }

    clr_ = clr_red_lgt;

    on_death();

    prop_handler_->on_death(is_player_see_dying_actor);

    if (!is_player())
    {
        dungeon_master::on_mon_killed(*this);
        static_cast<Mon*>(this)->leader_ = nullptr;
    }

    render::draw_map_and_interface();
}

std::string Actor::death_msg() const
{
    return name_the() + " dies.";
}

Did_action Actor::try_eat_corpse()
{
    Prop* const wound_prop = prop_handler_->prop(Prop_id::wound);

    Prop_wound* wound = nullptr;

    if (wound_prop)
    {
        wound = static_cast<Prop_wound*>(wound_prop);
    }

    if (hp() >= hp_max(true) && !wound)
    {
        //Not "hungry"
        msg_log::add("I am satiated.");

        return Did_action::no;
    }

    Actor* corpse = utils::actor_at_pos(pos, Actor_state::corpse);

    if (corpse)
    {
        const bool          IS_PLAYER       = is_player();
        const int           CORPSE_MAX_HP   = corpse->hp_max(false);
        const int           DESTR_ONE_IN_N  = utils::constr_in_range(1, CORPSE_MAX_HP / 4, 20);
        const bool          IS_DESTROYED    = rnd::one_in(DESTR_ONE_IN_N);
        const std::string   corpse_name     = corpse->corpse_name_the();

        const Alerts_mon alerts = IS_PLAYER ? Alerts_mon::yes : Alerts_mon::no;

        const Sfx_id sfx = IS_DESTROYED ?
                           Sfx_id::hit_corpse_break :
                           Sfx_id::END;  //TODO: Make a chewing sound effect

        Snd snd("I hear ripping and chewing.",
                sfx,
                Ignore_msg_if_origin_seen::yes,
                pos,
                this,
                Snd_vol::low,
                alerts,
                More_prompt_on_msg::no);

        snd_emit::run(snd);

        if (is_player())
        {
            msg_log::add("I feed on " + corpse_name + ".");

            if (IS_DESTROYED)
            {
                msg_log::add("There is nothing left to eat.");
            }
        }
        else //Is monster
        {
            if (map::player->can_see_actor(*this))
            {
                const std::string name = name_the();
                msg_log::add(name + " feeds on " + corpse_name + "!");
            }
        }

        if (IS_DESTROYED)
        {
            corpse->state_ = Actor_state::destroyed;
            map::mk_gore(pos);
            map::mk_blood(pos);
        }

        //Heal
        on_feed();

        return Did_action::yes;
    }
    else if (is_player())
    {
        msg_log::add("I find nothing here to feed on.");
    }

    return Did_action::no;
}

void Actor::on_feed()
{
    restore_hp(1, false, Verbosity::silent);

    Prop* const wound_prop = prop_handler_->prop(Prop_id::wound);

    Prop_wound* wound = nullptr;

    if (wound_prop)
    {
        wound = static_cast<Prop_wound*>(wound_prop);
    }

    if (wound && rnd::one_in(10))
    {
        wound->heal_one_wound();
    }
}

void Actor::add_light(bool light_map[MAP_W][MAP_H]) const
{
    if (state_ == Actor_state::alive && prop_handler_->has_prop(Prop_id::radiant))
    {
        //TODO: Much of the code below is duplicated from Actor_player::add_light_hook(), some
        //refactoring is needed.

        bool hard_blocked[MAP_W][MAP_H];

        const Rect fov_lmt = fov::get_fov_rect(pos);

        map_parse::run(cell_check::Blocks_los(),
                       hard_blocked,
                       Map_parse_mode::overwrite,
                       fov_lmt);

        Los_result fov[MAP_W][MAP_H];

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
    else if (prop_handler_->has_prop(Prop_id::burning))
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
