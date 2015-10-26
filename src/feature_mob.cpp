#include "init.hpp"

#include <algorithm>
#include <vector>

#include "feature_mob.hpp"
#include "game_time.hpp"
#include "player_bon.hpp"
#include "explosion.hpp"
#include "utils.hpp"
#include "map.hpp"
#include "feature_rigid.hpp"
#include "fov.hpp"
#include "inventory.hpp"
#include "item.hpp"
#include "msg_log.hpp"
#include "map_parsing.hpp"

//------------------------------------------------------------------- SMOKE
void Smoke::on_new_turn()
{
    auto* actor = utils::actor_at_pos(pos_);

    if (actor)
    {
        const bool IS_PLAYER = actor == map::player;

        //TODO: There needs to be some criteria here, so that e.g. a statue-monster or a
        //very alien monster can't get blinded by smoke (but do not use is_humanoid - rats,
        //wolves etc should definitely be blinded by smoke).

        //Perhaps add some variable like "has_eyes"?

        bool is_protected_blindness = false;

        if (IS_PLAYER)
        {
            auto&       inv             = map::player->inv();
            auto* const player_head_item  = inv.slots_[int(Slot_id::head)].item;
            auto* const player_body_item  = inv.slots_[int(Slot_id::body)].item;

            if (player_head_item)
            {
                if (player_head_item->data().id == Item_id::gas_mask)
                {
                    is_protected_blindness = true;

                    //This may destroy the gasmask
                    static_cast<Gas_mask*>(player_head_item)->decr_turns_left(inv);
                }
            }

            if (player_body_item)
            {
                if (player_body_item->data().id == Item_id::armor_asb_suit)
                {
                    is_protected_blindness = true;
                }
            }
        }

        //Blinded by smoke?
        if (!is_protected_blindness && rnd::one_in(4))
        {
            if (IS_PLAYER)
            {
                msg_log::add("I am getting smoke in my eyes.");
            }

            actor->prop_handler().try_add_prop(
                new Prop_blind(Prop_turns::specific, rnd::range(1, 3)));
        }

        //Choking (this is determined by rBreath)?
        if (rnd::one_in(4))
        {
            if (!actor->has_prop(Prop_id::rBreath))
            {
                std::string snd_msg = "";

                if (IS_PLAYER)
                {
                    msg_log::add("I am choking!", clr_msg_bad);
                }
                else
                {
                    if (actor->is_humanoid()) {snd_msg = "I hear choking.";}
                }

                const auto alerts = IS_PLAYER ? Alerts_mon::yes : Alerts_mon::no;

                snd_emit::run(Snd(snd_msg, Sfx_id::END, Ignore_msg_if_origin_seen::yes,
                                  actor->pos, actor, Snd_vol::low, alerts));

                actor->hit(1, Dmg_type::pure);
            }
        }
    }

    //If not permanent, count down turns left and possibly erase self
    if (nr_turns_left_ > -1)
    {
        if (--nr_turns_left_ <= 0) {game_time::erase_mob(this, true);}
    }
}

std::string Smoke::name(const Article article)  const
{
    std::string ret = "";

    if (article == Article::the) {ret += "the ";}

    return ret + "smoke";
}

Clr Smoke::clr() const
{
    return clr_gray;
}

//------------------------------------------------------------------- DYNAMITE
void Lit_dynamite::on_new_turn()
{
    nr_turns_left_--;

    if (nr_turns_left_ <= 0)
    {
        const int D = player_bon::traits[size_t(Trait::dem_expert)] ? 1 : 0;

        const P p(pos_);

        //Removing the dynamite before the explosion, so it won't be rendered after the
        //explosion (could happen for example if there are "more" prompts).
        game_time::erase_mob(this, true);

        //NOTE: The dynamite is now deleted. Do not use member variable after this point.

        explosion::run(p, Expl_type::expl, Expl_src::misc, Emit_expl_snd::yes, D);
    }
}

std::string Lit_dynamite::name(const Article article)  const
{
    std::string ret = article == Article::a ? "a " : "the ";
    return ret + "lit stick of dynamite";
}

Clr Lit_dynamite::clr() const
{
    return clr_red_lgt;
}

//------------------------------------------------------------------- FLARE
void Lit_flare::on_new_turn()
{
    nr_turns_left_--;

    if (nr_turns_left_ <= 0) {game_time::erase_mob(this, true);}
}

void Lit_flare::add_light(bool light[MAP_W][MAP_H]) const
{
    const int R = FOV_STD_RADI_INT; //light_radius();

    P p0(std::max(0,         pos_.x - R),  std::max(0,          pos_.y - R));
    P p1(std::min(MAP_W - 1, pos_.x + R),  std::min(MAP_H - 1,  pos_.y + R));

    bool hard_blocked[MAP_W][MAP_H];

    map_parse::run(cell_check::Blocks_los(),
                   hard_blocked,
                   Map_parse_mode::overwrite,
                   Rect(p0, p1));

    Los_result fov[MAP_W][MAP_H];

    fov::run(pos_, hard_blocked, fov);

    for (int y = p0.y; y <= p1.y; ++y)
    {
        for (int x = p0.x; x <= p1.x; ++x)
        {
            if (!fov[x][y].is_blocked_hard)
            {
                light[x][y] = true;
            }
        }
    }
}

std::string Lit_flare::name(const Article article)  const
{
    std::string ret = article == Article::a ? "a " : "the ";
    return ret + "lit flare";
}

Clr Lit_flare::clr() const
{
    return clr_yellow;
}
