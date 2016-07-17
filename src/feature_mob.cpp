#include "init.hpp"

#include <algorithm>
#include <vector>

#include "feature_mob.hpp"
#include "game_time.hpp"
#include "player_bon.hpp"
#include "explosion.hpp"
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
    auto* actor = map::actor_at_pos(pos_);

    if (actor)
    {
        const bool is_player = actor == map::player;

        //TODO: There needs to be some criteria here, so that e.g. a statue-monster or a
        //very alien monster can't get blinded by smoke (but do not use is_humanoid - rats,
        //wolves etc should definitely be blinded by smoke).

        //Perhaps add some variable like "has_eyes"?

        bool is_blind_prot = false;

        if (is_player)
        {
            auto&       inv                 = map::player->inv();
            auto* const player_head_item    = inv.slots_[int(Slot_id::head)].item;
            auto* const player_body_item    = inv.slots_[int(Slot_id::body)].item;

            if (player_head_item && player_head_item->data().id == Item_id::gas_mask)
            {
                is_blind_prot = true;

                //This may destroy the gasmask
                static_cast<Gas_mask*>(player_head_item)->decr_turns_left(inv);
            }

            if (player_body_item && player_body_item->data().id == Item_id::armor_asb_suit)
            {
                is_blind_prot = true;
            }
        }

        //Blinded?
        if (!is_blind_prot && rnd::one_in(4))
        {
            if (is_player)
            {
                msg_log::add("I am getting smoke in my eyes.");
            }

            actor->prop_handler().try_add(
                new Prop_blind(Prop_turns::specific, rnd::range(1, 3)));
        }

        //Coughing?
        if (rnd::one_in(4) && !actor->has_prop(Prop_id::rBreath))
        {
            std::string snd_msg = "";

            if (is_player)
            {
                msg_log::add("I cough.", clr_msg_bad);
            }
            else //Is monster
            {
                if (actor->is_humanoid())
                {
                    snd_msg = "I hear coughing.";
                }
            }

            const auto alerts = is_player ? Alerts_mon::yes : Alerts_mon::no;

            snd_emit::run(Snd(snd_msg,
                              Sfx_id::END,
                              Ignore_msg_if_origin_seen::yes,
                              actor->pos,
                              actor,
                              Snd_vol::low,
                              alerts));
        }
    }

    //If not permanent, count down turns left and possibly erase self
    if (nr_turns_left_ > -1)
    {
        --nr_turns_left_;

        if (nr_turns_left_ <= 0)
        {
            game_time::erase_mob(this, true);
        }
    }
}

std::string Smoke::name(const Article article)  const
{
    std::string ret = "";

    if (article == Article::the)
    {
        ret += "the ";
    }

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
        const int d = player_bon::traits[(size_t)Trait::dem_expert] ? 1 : 0;

        const P p(pos_);

        //Removing the dynamite before the explosion, so it can't be rendered
        //after the explosion (e.g. if there are "more" prompts).
        game_time::erase_mob(this, true);

        //NOTE: This object is now deleted! Do not use member variables!

        explosion::run(p,
                       Expl_type::expl,
                       Expl_src::misc,
                       Emit_expl_snd::yes,
                       d);
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
    --nr_turns_left_;

    if (nr_turns_left_ <= 0)
    {
        game_time::erase_mob(this, true);
    }
}

void Lit_flare::add_light(bool light[map_w][map_h]) const
{
    const int radi = fov_std_radi_int; //light_radius();

    P p0(std::max(0, pos_.x - radi),
         std::max(0, pos_.y - radi));

    P p1(std::min(map_w - 1, pos_.x + radi),
         std::min(map_h - 1, pos_.y + radi));

    bool hard_blocked[map_w][map_h];

    map_parse::run(cell_check::Blocks_los(),
                   hard_blocked,
                   Map_parse_mode::overwrite,
                   R(p0, p1));

    Los_result fov[map_w][map_h];

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
