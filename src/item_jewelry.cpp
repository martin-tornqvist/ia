#include "init.hpp"

#include "item_jewelry.hpp"

#include "map.hpp"
#include "utils.hpp"
#include "msg_log.hpp"
#include "game_time.hpp"
#include "render.hpp"
#include "actor_mon.hpp"
#include "text_format.hpp"
#include "actor_factory.hpp"
#include "feature_rigid.hpp"
#include "save_handling.hpp"
#include "dungeon_master.hpp"

namespace
{

Item_id     effect_list_    [size_t(Jewelry_effect_id::END)];
bool        effects_known_  [size_t(Jewelry_effect_id::END)];

Jewelry_effect* mk_effect(const Jewelry_effect_id id, Jewelry* const jewelry)
{
    assert(id != Jewelry_effect_id::END);

    Jewelry_effect* ret = nullptr;

    switch (id)
    {
    case Jewelry_effect_id::rFire:
        ret = new Jewelry_effect_rFire(jewelry);
        break;

    case Jewelry_effect_id::rElec:
        ret = new Jewelry_effect_rElec(jewelry);
        break;

    case Jewelry_effect_id::rPoison:
        ret = new Jewelry_effect_rPoison(jewelry);
        break;

    case Jewelry_effect_id::rDisease:
        ret = new Jewelry_effect_rDisease(jewelry);
        break;

    case Jewelry_effect_id::tele_ctrl:
        ret = new Jewelry_effect_tele_control(jewelry);
        break;

    case Jewelry_effect_id::summon:
        ret = new Jewelry_effect_summon_mon(jewelry);
        break;

    case Jewelry_effect_id::light:
        ret = new Jewelry_effect_light(jewelry);
        break;

    case Jewelry_effect_id::spell_reflect:
        ret = new Jewelry_effect_spell_reflect(jewelry);
        break;

    case Jewelry_effect_id::hp_bon:
        ret = new Jewelry_effect_hp_bon(jewelry);
        break;

    case Jewelry_effect_id::hp_pen:
        ret = new Jewelry_effect_hp_pen(jewelry);
        break;

    case Jewelry_effect_id::spi_bon:
        ret = new Jewelry_effect_spi_bon(jewelry);
        break;

    case Jewelry_effect_id::spi_pen:
        ret = new Jewelry_effect_spi_pen(jewelry);
        break;

    case Jewelry_effect_id::random_tele:
        ret = new Jewelry_effect_random_tele(jewelry);
        break;

    case Jewelry_effect_id::fire:
        ret = new Jewelry_effect_fire(jewelry);
        break;

    case Jewelry_effect_id::conflict:
        ret = new Jewelry_effect_conflict(jewelry);
        break;

    case Jewelry_effect_id::burden:
        ret = new Jewelry_effect_burden(jewelry);
        break;

    case Jewelry_effect_id::hp_regen_bon:
        ret = new Jewelry_effect_hp_regen_bon(jewelry);
        break;

    case Jewelry_effect_id::hp_regen_pen:
        ret = new Jewelry_effect_hp_regen_pen(jewelry);
        break;

    case Jewelry_effect_id::haste:
        ret = new Jewelry_effect_haste(jewelry);
        break;

    case Jewelry_effect_id::shriek:
        ret = new Jewelry_effect_shriek(jewelry);
        break;

    case Jewelry_effect_id::START_OF_SECONDARY_EFFECTS:
    case Jewelry_effect_id::END:
        break;
    }

    assert(ret);
    return ret;
}

} //namespace

//--------------------------------------------------------- JEWELRY PROPERTY EFFECT
void Jewelry_property_effect::on_equip(const Verbosity verbosity)
{
    Prop* const prop = mk_prop();

    assert(prop);

    jewelry_->add_carrier_prop(prop, verbosity);

    jewelry_->effect_noticed(id());
}

Unequip_allowed Jewelry_property_effect::on_unequip()
{
    jewelry_->clear_carrier_props();

    return Unequip_allowed::yes;
}

//--------------------------------------------------------- FIRE RESISTANCE
Prop* Jewelry_effect_rFire::mk_prop() const
{
    return new Prop_rFire(Prop_turns::indefinite);
}

//--------------------------------------------------------- ELEC RESISTANCE
Prop* Jewelry_effect_rElec::mk_prop() const
{
    return new Prop_rElec(Prop_turns::indefinite);
}

//--------------------------------------------------------- POISON RESISTANCE
Prop* Jewelry_effect_rPoison::mk_prop() const
{
    return new Prop_rPoison(Prop_turns::indefinite);
}

//--------------------------------------------------------- DISEASE RESISTANCE
Prop* Jewelry_effect_rDisease::mk_prop() const
{
    return new Prop_rDisease(Prop_turns::indefinite);
}

//--------------------------------------------------------- TELEPORT CONTROL
Prop* Jewelry_effect_tele_control::mk_prop() const
{
    return new Prop_tele_control(Prop_turns::indefinite);
}

//--------------------------------------------------------- LIGHT
Prop* Jewelry_effect_light::mk_prop() const
{
    return new Prop_radiant(Prop_turns::indefinite);
}

//--------------------------------------------------------- SPELL REFLECTION
Prop* Jewelry_effect_spell_reflect::mk_prop() const
{
    return new Prop_spell_reflect(Prop_turns::indefinite);
}

//--------------------------------------------------------- HASTE
Prop* Jewelry_effect_haste::mk_prop() const
{
    return new Prop_hasted(Prop_turns::indefinite);
}

//--------------------------------------------------------- HP BONUS
void Jewelry_effect_hp_bon::on_equip(const Verbosity verbosity)
{
    (void)verbosity;

    jewelry_->actor_carrying()->change_max_hp(4);
    jewelry_->effect_noticed(id());
}

Unequip_allowed Jewelry_effect_hp_bon::on_unequip()
{
    jewelry_->actor_carrying()->change_max_hp(-4);

    return Unequip_allowed::yes;
}

//--------------------------------------------------------- HP PENALTY
void Jewelry_effect_hp_pen::on_equip(const Verbosity verbosity)
{
    (void)verbosity;

    jewelry_->actor_carrying()->change_max_hp(-4);
    jewelry_->effect_noticed(id());
}

Unequip_allowed Jewelry_effect_hp_pen::on_unequip()
{
    jewelry_->actor_carrying()->change_max_hp(4);

    return Unequip_allowed::yes;
}

//--------------------------------------------------------- SPI BONUS
void Jewelry_effect_spi_bon::on_equip(const Verbosity verbosity)
{
    (void)verbosity;

    jewelry_->actor_carrying()->change_max_spi(4);
    jewelry_->effect_noticed(id());
}

Unequip_allowed Jewelry_effect_spi_bon::on_unequip()
{
    jewelry_->actor_carrying()->change_max_spi(-4);

    return Unequip_allowed::yes;
}

//--------------------------------------------------------- SPI PENALTY
void Jewelry_effect_spi_pen::on_equip(const Verbosity verbosity)
{
    (void)verbosity;

    jewelry_->actor_carrying()->change_max_spi(-4);
    jewelry_->effect_noticed(id());
}

Unequip_allowed Jewelry_effect_spi_pen::on_unequip()
{
    jewelry_->actor_carrying()->change_max_spi(4);

    return Unequip_allowed::yes;
}

//--------------------------------------------------------- RANDOM TELEPORTATION
void Jewelry_effect_random_tele::on_std_turn_equipped()
{
    auto& prop_handler = map::player->prop_handler();

    const int TELE_ON_IN_N = 200;

    if (rnd::one_in(TELE_ON_IN_N) && prop_handler.allow_act())
    {
        msg_log::add("I am being teleported...", clr_white, true, More_prompt_on_msg::yes);
        map::player->teleport();
        jewelry_->effect_noticed(id());
    }
}

//--------------------------------------------------------- SUMMON MON
void Jewelry_effect_summon_mon::on_std_turn_equipped()
{
    const int SOUND_ONE_IN_N  = 250;

    if (effects_known_[size_t(id())] && rnd::one_in(SOUND_ONE_IN_N))
    {
        msg_log::add("I hear a faint whistling sound coming nearer...", clr_white, false,
                     More_prompt_on_msg::yes);

        map::player->incr_shock(Shock_lvl::mild, Shock_src::misc);
    }

    const int SUMMON_ONE_IN_N = 1200;

    if (rnd::one_in(SUMMON_ONE_IN_N))
    {
        msg_log::add("There is a loud whistling sound.", clr_white, false,
                     More_prompt_on_msg::yes);

        const P origin = map::player->pos;

        std::vector<Mon*> summoned_mon;

        actor_factory::summon(origin, {Actor_id::greater_polyp}, false, nullptr,
                              &summoned_mon);

        const Mon* const mon = summoned_mon[0];

        if (map::player->can_see_actor(*mon))
        {
            msg_log::add(mon->name_a() + " appears!", clr_white, true, More_prompt_on_msg::yes);
        }

        jewelry_->effect_noticed(id());
    }
}

//--------------------------------------------------------- FIRE
void Jewelry_effect_fire::on_std_turn_equipped()
{
    const int FIRE_ONE_IN_N = 300;

    if (rnd::one_in(FIRE_ONE_IN_N))
    {
        const P& origin   = map::player->pos;
        const int D         = FOV_STD_RADI_INT - 2;
        const int X0        = std::max(1,            origin.x - D);
        const int Y0        = std::max(1,            origin.y - D);
        const int X1        = std::min(MAP_W - 2,    origin.x + D);
        const int Y1        = std::min(MAP_H - 2,    origin.y + D);

        const int FIRE_CELL_ONE_IN_N = 4;

        msg_log::add("The surrounding area suddenly burst into flames!", clr_white,
                     false, More_prompt_on_msg::yes);

        for (int x = X0; x <= X1; ++x)
        {
            for (int y = Y0; y <= Y1; ++y)
            {
                if (rnd::one_in(FIRE_CELL_ONE_IN_N) && P(x, y) != origin)
                {
                    map::cells[x][y].rigid->hit(Dmg_type::fire, Dmg_method::elemental);
                }
            }
        }

        jewelry_->effect_noticed(id());
    }
}

//--------------------------------------------------------- CONFLICT
void Jewelry_effect_conflict::on_std_turn_equipped()
{
    const int CONFLICT_ONE_IN_N = 50;

    if (rnd::one_in(CONFLICT_ONE_IN_N))
    {
        std::vector<Actor*> seen_foes;
        map::player->seen_foes(seen_foes);

        if (!seen_foes.empty())
        {
            std::random_shuffle(begin(seen_foes), end(seen_foes));

            for (Actor* actor : seen_foes)
            {
                if (!actor->data().is_unique)
                {
                    actor->prop_handler().try_add_prop(
                        new Prop_conflict(Prop_turns::std));

                    jewelry_->effect_noticed(id());

                    break;
                }
            }
        }
    }
}

//--------------------------------------------------------- SHRIEK
Jewelry_effect_shriek::Jewelry_effect_shriek(Jewelry* const jewelry) :
    Jewelry_effect(jewelry)
{
    std::string player_name = map::player->name_the();

    text_format::all_to_upper(player_name);

    words_ =
    {
        player_name,
        player_name,
        player_name,
        "BHUUDESCO",
        "STRAGARANA",
        "INFIRMUX",
        "BHAAVA",
        "CRUENTO",
        "PRETIACRUENTO",
        "VILOMAXUS",
        "DEATH",
        "DYING",
        "TAKE",
        "Bl_oOD",
        "END",
        "SACRIFICE",
        "POSSESSED",
        "PROPHECY",
        "SIGNS",
        "OATH",
        "FATE",
        "SUFFER",
        "BEHOLD",
        "BEWARE",
        "WATCHES",
        "LIGHT",
        "DARK",
        "DISAPPEAR",
        "APPEAR",
        "DECAY",
        "IMMORTAL",
        "BOUNDLESS",
        "ETERNAL",
        "ANCIENT",
        "TIME",
        "NEVER-ENDING",
        "DIMENSIONS",
        "EYES",
        "STARS",
        "GAZE",
        "FORBIDDEN",
        "DOMINIONS",
        "RULER",
        "KING",
        "UNKNOWN",
        "ABYSS",
        "BENEATH",
        "BELOW",
        "PASSAGE",
        "PATH",
        "GATE",
        "SERPENT",
        "NYARLATHOTEP",
        "GOL-GOROTH",
        "ABHOLOS",
        "HASTUR",
        "ISTASHA",
        "ITHAQUA",
        "THOG",
        "TSATHOGGUA",
        "YMNAR",
        "XCTHOL",
        "ZATHOG",
        "ZINDARAK",
        "BASATAN",
        "CTHUGHA"
    };
}

void Jewelry_effect_shriek::on_std_turn_equipped()
{
    const int NOISE_ONE_IN_N = 300;

    if (rnd::one_in(NOISE_ONE_IN_N))
    {
        const std::string name = jewelry_->name(Item_ref_type::plain, Item_ref_inf::none);

        msg_log::add("The " + name + " shrieks...", clr_white, false, More_prompt_on_msg::yes);

        const int NR_WORDS = 3;

        std::string phrase = "";

        for (int i = 0; i < NR_WORDS; ++i)
        {
            const int   IDX     = rnd::range(0, words_.size() - 1);
            const auto& word    = words_[size_t(IDX)];

            phrase += word;

            if (i < NR_WORDS - 1)
            {
                phrase += " ... ";
            }
        }

        phrase += "!!!";

        snd_emit::run(Snd(phrase, Sfx_id::END, Ignore_msg_if_origin_seen::no,
                          map::player->pos, map::player, Snd_vol::high, Alerts_mon::yes
                         ));

        map::player->incr_shock(Shock_lvl::mild, Shock_src::misc);

        msg_log::more_prompt();

        jewelry_->effect_noticed(id());
    }
}

//--------------------------------------------------------- BURDEN
void Jewelry_effect_burden::on_equip(const Verbosity verbosity)
{
    if (!effects_known_[size_t(id())])
    {
        if (verbosity == Verbosity::verbose)
        {
            msg_log::add("I suddenly feel more burdened.");
        }

        jewelry_->effect_noticed(id());
    }
}

void Jewelry_effect_burden::change_item_weight(int& weight_ref)
{
    if (effects_known_[size_t(id())])
    {
        //If revealed, this item weighs the average of "heavy" and "medium"
        weight_ref = (int(Item_weight::heavy) + int(Item_weight::medium)) / 2;
    }
}

//--------------------------------------------------------- HP REGEN BONUS
void Jewelry_effect_hp_regen_bon::on_equip(const Verbosity verbosity)
{
    if (verbosity == Verbosity::verbose)
    {
        msg_log::add("I heal faster.");
    }

    jewelry_->effect_noticed(id());
}

Unequip_allowed Jewelry_effect_hp_regen_bon::on_unequip()
{
    msg_log::add("I heal slower.");
    return Unequip_allowed::yes;
}

//--------------------------------------------------------- HP REGEN PENALTY
void Jewelry_effect_hp_regen_pen::on_equip(const Verbosity verbosity)
{
    if (verbosity == Verbosity::verbose)
    {
        msg_log::add("I heal slower.");
    }

    jewelry_->effect_noticed(id());
}

Unequip_allowed Jewelry_effect_hp_regen_pen::on_unequip()
{
    msg_log::add("I heal faster.");
    return Unequip_allowed::yes;
}

//--------------------------------------------------------- JEWELRY
Jewelry::Jewelry(Item_data_t* const item_data) :
    Item(item_data)
{
    for (size_t i = 0; i < int(Jewelry_effect_id::END); ++i)
    {
        if (effect_list_[i] == item_data->id)
        {
            auto* const effect = mk_effect(Jewelry_effect_id(i), this);
            effects_.push_back(effect);
        }
    }

    //Unique item, do not allow spawning more
    data_->allow_spawn = false;
}

Jewelry::~Jewelry()
{
    for (auto* const effect : effects_)
    {
        delete effect;
    }
}

std::vector<std::string> Jewelry::descr() const
{
    std::vector<std::string> ret;

    for (auto* effect : effects_)
    {
        const size_t EFFECT_IDX = size_t(effect->id());

        if (effects_known_[EFFECT_IDX])
        {
            ret.push_back(effect->descr());
        }
    }

    if (data_->is_identified)
    {
        const std::string jewelry_name = name(Item_ref_type::plain, Item_ref_inf::none);

        ret.push_back("All properties of the " + jewelry_name + " are known to me.");
    }

    return ret;
}

std::string Jewelry::name_inf() const
{
    return data_->is_identified ? "{Known}" : "";
}

void Jewelry::on_equip_hook(const Verbosity verbosity)
{
    for (auto* const effect : effects_)
    {
        //This may cause the effect to set up carrier properties (e.g. fire resistance)
        effect->on_equip(verbosity);
    }

    if (verbosity == Verbosity::verbose)
    {
        msg_log::more_prompt();
    }
}

Unequip_allowed Jewelry::on_unequip_hook()
{
    auto unequip_allowed = Unequip_allowed::yes;

    for (auto* const effect : effects_)
    {
        if (effect->on_unequip() == Unequip_allowed::no)
        {
            unequip_allowed = Unequip_allowed::no;
        }
    }

    msg_log::more_prompt();

    return unequip_allowed;
}

void Jewelry::on_std_turn_in_inv(const Inv_type inv_type)
{
    if (inv_type == Inv_type::slots)
    {
        for (auto* const effect : effects_)
        {
            effect->on_std_turn_equipped();
        }
    }
}

void Jewelry::on_actor_turn_in_inv(const Inv_type inv_type)
{
    if (inv_type == Inv_type::slots)
    {
        for (auto* const effect : effects_)
        {
            effect->on_actor_turn_equipped();
        }
    }
}

void Jewelry::identify(const Verbosity verbosity)
{
    if (!data_->is_identified)
    {
        data_->is_identified = true;

        for (auto* effect : effects_)
        {
            const size_t EFFECT_IDX = size_t(effect->id());

            effects_known_[EFFECT_IDX] = true;
        }

        if (verbosity == Verbosity::verbose)
        {
            const std::string name_plain = name(Item_ref_type::plain, Item_ref_inf::none);

            msg_log::add("I feel like all properties of the " + name_plain +
                         " are known to me.", clr_white, false, More_prompt_on_msg::yes);

            const std::string name_a = name(Item_ref_type::a, Item_ref_inf::none);

            dungeon_master::add_history_event("Learned all the properties of " + name_a + ".");
        }
    }
}

int Jewelry::weight() const
{
    int weight = Item::weight();

    for (auto* effect : effects_)
    {
        effect->change_item_weight(weight);
    }

    return weight;
}

int Jewelry::hp_regen_change(const Inv_type inv_type) const
{
    int change = 0;

    if (inv_type == Inv_type::slots)
    {
        for (auto* effect : effects_)
        {
            change += effect->hp_regen_change();
        }
    }

    return change;
}

void Jewelry::effect_noticed(const Jewelry_effect_id effect_id)
{
    const size_t EFFECT_IDX = size_t(effect_id);

    if (!effects_known_[EFFECT_IDX])
    {
        effects_known_[EFFECT_IDX] = true;

        const int   MAX_NR_EFFECTS_ON_ITEM  = 2;
        int         nr_effects_known_this_item  = 0;

        for (auto* effect : effects_)
        {
            const size_t CHECK_EFFECT_IDX = size_t(effect->id());

            if (effects_known_[CHECK_EFFECT_IDX])
            {
                ++nr_effects_known_this_item;
            }
        }

        assert(nr_effects_known_this_item <= MAX_NR_EFFECTS_ON_ITEM);

        const std::string name_plain = name(Item_ref_type::plain, Item_ref_inf::none);

        msg_log::add("I gained new knowledge about the " + name_plain + ".", clr_white,
                     false, More_prompt_on_msg::yes);

        if (nr_effects_known_this_item == MAX_NR_EFFECTS_ON_ITEM)
        {
            //Jewelry is completely identified - print id message and set id status in item data
            identify(Verbosity::verbose);
        }
        else //Not all properties are known
        {
            //NOTE: The identify method adds a history event about learning *all* properties,
            //so we only add an event about learning a property if it is not yet fully identified
            //(i.e. when we don't call the identify method) - otherwise the history would contain
            //something like "Learned a property" immediately followed by "Learned all properties",
            //which would look weird and redundant.
            const std::string name_a = name(Item_ref_type::a, Item_ref_inf::none);

            dungeon_master::add_history_event("Learned a new property of " + name_a + ".");
        }
    }
}

//--------------------------------------------------------- JEWELRY HANDLING
namespace
{

bool can_effects_be_combined(const Jewelry_effect_id id1,
                             const Jewelry_effect_id id2)
{
    typedef Jewelry_effect_id Id;

    assert(id1 != Id::END && id2 != Id::END);

    if (id1 == id2)
    {
        return true;
    }

    switch (id1)
    {
    case Id::hp_bon:
        return id2 != Id::hp_pen;

    case Id::hp_pen:
        return id2 != Id::hp_bon && id2 != Id::rDisease;

    case Id::spi_bon:
        return id2 != Id::spi_pen;

    case Id::spi_pen:
        return id2 != Id::spi_bon;

    case Id::rDisease:
        return id2 != Id::hp_pen;

    case Id::tele_ctrl:
        return id2 != Id::random_tele && id2 != Id::spell_reflect;

    case Id::random_tele:
        return id2 != Id::tele_ctrl;

    case Id::spell_reflect:
        return id2 != Id::tele_ctrl;

    case Id::hp_regen_bon:
        return id2 != Id::hp_regen_pen && id2 != Id::hp_bon && id2 != Id::hp_pen;

    case Id::hp_regen_pen:
        return id2 != Id::hp_regen_bon && id2 != Id::hp_bon && id2 != Id::hp_pen;

    case Id::summon:
    case Id::light:
    case Id::conflict:
    case Id::rFire:
    case Id::rElec:
    case Id::rPoison:
    case Id::burden:
    case Id::shriek:
    case Id::haste:
    case Id::fire:
        return true;

    case Jewelry_effect_id::START_OF_SECONDARY_EFFECTS:
    case Id::END:
        assert(false);
        return false;
    }

    return false;
}

int rnd_item_bucket_idx_for_effect(const Jewelry_effect_id  effect_to_assign,
                                   const std::vector<Item_id>&  item_bucket)
{
    std::vector<int> item_idx_bucket;

    for (size_t item_bucket_idx = 0; item_bucket_idx < item_bucket.size(); ++item_bucket_idx)
    {
        bool can_be_placed_on_item = true;

        //Verify that the effect can be placed on this item by checking if it can
        //be combined with every effect currently assigned to this item.
        for (int i = 0; i < int(Jewelry_effect_id::END); ++i)
        {
            const Jewelry_effect_id cur_effect = Jewelry_effect_id(i);

            if (
                effect_list_[i] == item_bucket[item_bucket_idx] &&
                !can_effects_be_combined(effect_to_assign, cur_effect))
            {
                //Combination with effect already assigned on item not allowed
                can_be_placed_on_item = false;
                break;
            }
        }

        if (can_be_placed_on_item)
        {
            item_idx_bucket.push_back(item_bucket_idx);
        }
    }

    if (item_idx_bucket.empty())
    {
        return -1;
    }

    const int IDX_BUCKET_IDX = rnd::range(0, item_idx_bucket.size() - 1);
    return item_idx_bucket[IDX_BUCKET_IDX];
}

} //namespace

namespace jewelry_handling
{

void init()
{
    //Reset the effect list and knowledge status list
    for (size_t i = 0; i < size_t(Jewelry_effect_id::END); ++i)
    {
        effect_list_   [i] = Item_id::END;
        effects_known_ [i] = false;
    }

    std::vector<Item_id> item_bucket;

    for (auto& data : item_data::data)
    {
        const auto type = data.type;

        if (type == Item_type::amulet || type == Item_type::ring)
        {
            item_bucket.push_back(data.id);
        }
    }

    std::random_shuffle(begin(item_bucket), end(item_bucket));

    std::vector<Jewelry_effect_id> primary_effect_bucket;
    std::vector<Jewelry_effect_id> secondary_effect_bucket;

    for (size_t i = 0; i < size_t(Jewelry_effect_id::END); ++i)
    {
        const auto id = Jewelry_effect_id(i);

        if (id <  Jewelry_effect_id::START_OF_SECONDARY_EFFECTS)
        {
            primary_effect_bucket.push_back(id);
        }
        else if (id >  Jewelry_effect_id::START_OF_SECONDARY_EFFECTS)
        {
            secondary_effect_bucket.push_back(id);
        }
    }

    random_shuffle(begin(primary_effect_bucket),   end(primary_effect_bucket));
    random_shuffle(begin(secondary_effect_bucket), end(secondary_effect_bucket));

    //Assuming there are more jewelry items than primary or secondary effects (if this changes,
    //just add more amulets and rings to the item data)
    assert(item_bucket.size() > primary_effect_bucket.size());
    assert(item_bucket.size() > secondary_effect_bucket.size());

    //Assign primary effects
    for (size_t i = 0; i < item_bucket.size(); ++i)
    {
        const auto item_id = item_bucket[i];

        if (i < primary_effect_bucket.size())
        {
            const auto effect_id = primary_effect_bucket[i];

            effect_list_[size_t(effect_id)] = item_id;
        }
        else //All primary effects are assigned
        {
            item_data::data[size_t(item_id)].allow_spawn = false;
        }
    }

    //Remove all items without a primary effect from the item bucket
    item_bucket.resize(primary_effect_bucket.size());

    //Assign secondary effects
    for (const auto secondary_effect_id : secondary_effect_bucket)
    {
        const int ITEM_IDX = rnd_item_bucket_idx_for_effect(secondary_effect_id, item_bucket);

        if (ITEM_IDX >= 0)
        {
            effect_list_[size_t(secondary_effect_id)] = item_bucket[size_t(ITEM_IDX)];
            item_bucket.erase(begin(item_bucket) + size_t(ITEM_IDX));
        }
    }
}

void save()
{
    for (size_t i = 0; i < size_t(Jewelry_effect_id::END); ++i)
    {
        save_handling::put_int(int(effect_list_[i]));
        save_handling::put_bool(effects_known_[i]);
    }
}

void load()
{
    for (size_t i = 0; i < size_t(Jewelry_effect_id::END); ++i)
    {
        effect_list_[i]     = Item_id(save_handling::get_int());
        effects_known_[i]   = save_handling::get_bool();
    }
}

} //Jewelry_handling
