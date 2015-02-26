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

using namespace std;

//------------------------------------------------------------------- SMOKE
void Smoke::on_new_turn()
{
    auto* actor = utils::get_actor_at_pos(pos_);

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
            auto&       inv             = map::player->get_inv();
            auto* const player_head_item  = inv.slots_[int(Slot_id::head)].item;
            auto* const player_body_item  = inv.slots_[int(Slot_id::body)].item;

            if (player_head_item)
            {
                if (player_head_item->get_data().id == Item_id::gas_mask)
                {
                    is_protected_blindness = true;

                    //This may destroy the gasmask
                    static_cast<Gas_mask*>(player_head_item)->decr_turns_left(inv);
                }
            }

            if (player_body_item)
            {
                if (player_body_item->get_data().id == Item_id::armor_asb_suit)
                {
                    is_protected_blindness = true;
                }
            }
        }

        //Blinded by smoke?
        if (!is_protected_blindness && rnd::one_in(4))
        {
            if (IS_PLAYER) {msg_log::add("I am getting smoke in my eyes.");}

            actor->get_prop_handler().try_apply_prop(
                new Prop_blind(Prop_turns::specific, rnd::range(1, 3)));
        }

        //Choking (this is determined by rBreath)?
        if (rnd::one_in(4))
        {
            bool props[size_t(Prop_id::END)];
            actor->get_prop_handler().get_prop_ids(props);

            if (!props[int(Prop_id::rBreath)])
            {
                string snd_msg = "";

                if (IS_PLAYER)
                {
                    msg_log::add("I am choking!", clr_msg_bad);
                }
                else
                {
                    if (actor->is_humanoid()) {snd_msg = "I hear choking.";}
                }

                const auto alerts = IS_PLAYER ? Alerts_mon::yes : Alerts_mon::no;

                snd_emit::emit_snd(Snd(snd_msg, Sfx_id::END, Ignore_msg_if_origin_seen::yes, actor->pos,
                                       actor, Snd_vol::low, alerts));
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

string Smoke::get_name(const Article article)  const
{
    string ret = "";

    if (article == Article::the) {ret += "the ";}

    return ret + "smoke";
}

Clr Smoke::get_clr() const
{
    return clr_gray;
}

//------------------------------------------------------------------- DYNAMITE
void Lit_dynamite::on_new_turn()
{
    nr_turns_left_--;

    if (nr_turns_left_ <= 0)
    {
        const int D = player_bon::traits[int(Trait::dem_expert)] ? 1 : 0;

        const Pos pos(pos_);

        //Removing the dynamite before the explosion, so it won't be rendered after the
        //explosion (could happen for example if there are "more" prompts).
        game_time::erase_mob(this, true);

        //NOTE: The dynamite is now deleted. Do not use member variable after this point.

        explosion::run_explosion_at(pos, Expl_type::expl, Expl_src::misc, D);
    }
}

string Lit_dynamite::get_name(const Article article)  const
{
    string ret = article == Article::a ? "a " : "the ";
    return ret + "lit stick of dynamite";
}

Clr Lit_dynamite::get_clr() const
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
    bool my_light[MAP_W][MAP_H];
    utils::reset_array(my_light, false);
    const int R = FOV_STD_RADI_INT; //get_light_radius();
    Pos p0(max(0,         pos_.x - R),  max(0,          pos_.y - R));
    Pos p1(min(MAP_W - 1, pos_.x + R),  min(MAP_H - 1,  pos_.y + R));
    bool blocked_los[MAP_W][MAP_H];

    for (int y = p0.y; y <= p1.y; ++y)
    {
        for (int x = p0.x; x <= p1.x; ++x)
        {
            blocked_los[x][y] = !map::cells[x][y].rigid->is_los_passable();
        }
    }

    fov::run_fov_on_array(blocked_los, pos_, my_light, false);

    for (int y = p0.y; y <= p1.y; ++y)
    {
        for (int x = p0.x; x <= p1.x; ++x)
        {
            if (my_light[x][y]) {light[x][y] = true;}
        }
    }
}

string Lit_flare::get_name(const Article article)  const
{
    string ret = article == Article::a ? "a " : "the ";
    return ret + "lit flare";
}

Clr Lit_flare::get_clr() const
{
    return clr_yellow;
}
