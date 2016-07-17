#include "init.hpp"

#include "item_amulet.hpp"

#include "map.hpp"
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

ItemId     effect_list_    [size_t(AmuletEffectId::END)];
bool        effects_known_  [size_t(AmuletEffectId::END)];

AmuletEffect* mk_effect(const AmuletEffectId id, Amulet* const amulet)
{
    ASSERT(id != AmuletEffectId::END);

    AmuletEffect* ret = nullptr;

    switch (id)
    {
    case AmuletEffectId::rFire:
        ret = new AmuletEffectRFire(amulet);
        break;

    case AmuletEffectId::rElec:
        ret = new AmuletEffectRElec(amulet);
        break;

    case AmuletEffectId::rPoison:
        ret = new AmuletEffectRPoison(amulet);
        break;

    case AmuletEffectId::rDisease:
        ret = new AmuletEffectRDisease(amulet);
        break;

    case AmuletEffectId::tele_ctrl:
        ret = new AmuletEffectTeleControl(amulet);
        break;

    case AmuletEffectId::summon:
        ret = new AmuletEffectSummonMon(amulet);
        break;

    case AmuletEffectId::light:
        ret = new AmuletEffectLight(amulet);
        break;

    case AmuletEffectId::spell_reflect:
        ret = new AmuletEffectSpellReflect(amulet);
        break;

    case AmuletEffectId::hp_bon:
        ret = new AmuletEffectHpBon(amulet);
        break;

    case AmuletEffectId::hp_pen:
        ret = new AmuletEffectHpPen(amulet);
        break;

    case AmuletEffectId::spi_bon:
        ret = new AmuletEffectSpiBon(amulet);
        break;

    case AmuletEffectId::spi_pen:
        ret = new AmuletEffectSpiPen(amulet);
        break;

    case AmuletEffectId::random_tele:
        ret = new AmuletEffectRandomTele(amulet);
        break;

    case AmuletEffectId::fire:
        ret = new AmuletEffectFire(amulet);
        break;

    case AmuletEffectId::conflict:
        ret = new AmuletEffectConflict(amulet);
        break;

    case AmuletEffectId::burden:
        ret = new AmuletEffectBurden(amulet);
        break;

    case AmuletEffectId::hp_regen_bon:
        ret = new AmuletEffectHpRegenBon(amulet);
        break;

    case AmuletEffectId::hp_regen_pen:
        ret = new AmuletEffectHpRegenPen(amulet);
        break;

    case AmuletEffectId::haste:
        ret = new AmuletEffectHaste(amulet);
        break;

    case AmuletEffectId::shriek:
        ret = new AmuletEffectShriek(amulet);
        break;

    case AmuletEffectId::START_OF_SECONDARY_EFFECTS:
    case AmuletEffectId::END:
        break;
    }

    ASSERT(ret);
    return ret;
}

} //namespace

//--------------------------------------------------------- AMULET PROPERTY EFFECT
void AmuletPropertyEffect::on_equip(const Verbosity verbosity)
{
    Prop* const prop = mk_prop();

    ASSERT(prop);

    amulet_->add_carrier_prop(prop, verbosity);

    amulet_->effect_noticed(id());
}

UnequipAllowed AmuletPropertyEffect::on_unequip()
{
    amulet_->clear_carrier_props();

    return UnequipAllowed::yes;
}

//--------------------------------------------------------- FIRE RESISTANCE
Prop* AmuletEffectRFire::mk_prop() const
{
    return new PropRFire(PropTurns::indefinite);
}

//--------------------------------------------------------- ELEC RESISTANCE
Prop* AmuletEffectRElec::mk_prop() const
{
    return new PropRElec(PropTurns::indefinite);
}

//--------------------------------------------------------- POISON RESISTANCE
Prop* AmuletEffectRPoison::mk_prop() const
{
    return new PropRPoison(PropTurns::indefinite);
}

//--------------------------------------------------------- DISEASE RESISTANCE
Prop* AmuletEffectRDisease::mk_prop() const
{
    return new PropRDisease(PropTurns::indefinite);
}

//--------------------------------------------------------- TELEPORT CONTROL
Prop* AmuletEffectTeleControl::mk_prop() const
{
    return new PropTeleControl(PropTurns::indefinite);
}

//--------------------------------------------------------- LIGHT
Prop* AmuletEffectLight::mk_prop() const
{
    return new PropRadiant(PropTurns::indefinite);
}

//--------------------------------------------------------- SPELL REFLECTION
Prop* AmuletEffectSpellReflect::mk_prop() const
{
    return new PropSpellReflect(PropTurns::indefinite);
}

//--------------------------------------------------------- HASTE
Prop* AmuletEffectHaste::mk_prop() const
{
    return new PropHasted(PropTurns::indefinite);
}

//--------------------------------------------------------- hp BONUS
void AmuletEffectHpBon::on_equip(const Verbosity verbosity)
{
    (void)verbosity;

    amulet_->actor_carrying()->change_max_hp(4);
    amulet_->effect_noticed(id());
}

UnequipAllowed AmuletEffectHpBon::on_unequip()
{
    amulet_->actor_carrying()->change_max_hp(-4);

    return UnequipAllowed::yes;
}

//--------------------------------------------------------- hp PENALTY
void AmuletEffectHpPen::on_equip(const Verbosity verbosity)
{
    (void)verbosity;

    amulet_->actor_carrying()->change_max_hp(-4);
    amulet_->effect_noticed(id());
}

UnequipAllowed AmuletEffectHpPen::on_unequip()
{
    amulet_->actor_carrying()->change_max_hp(4);

    return UnequipAllowed::yes;
}

//--------------------------------------------------------- spi BONUS
void AmuletEffectSpiBon::on_equip(const Verbosity verbosity)
{
    (void)verbosity;

    amulet_->actor_carrying()->change_max_spi(4);
    amulet_->effect_noticed(id());
}

UnequipAllowed AmuletEffectSpiBon::on_unequip()
{
    amulet_->actor_carrying()->change_max_spi(-4);

    return UnequipAllowed::yes;
}

//--------------------------------------------------------- spi PENALTY
void AmuletEffectSpiPen::on_equip(const Verbosity verbosity)
{
    (void)verbosity;

    amulet_->actor_carrying()->change_max_spi(-4);
    amulet_->effect_noticed(id());
}

UnequipAllowed AmuletEffectSpiPen::on_unequip()
{
    amulet_->actor_carrying()->change_max_spi(4);

    return UnequipAllowed::yes;
}

//--------------------------------------------------------- RANDOM TELEPORTATION
void AmuletEffectRandomTele::on_std_turn_equipped()
{
    auto& prop_handler = map::player->prop_handler();

    const int tele_on_in_n = 200;

    if (rnd::one_in(tele_on_in_n) && prop_handler.allow_act())
    {
        msg_log::add("I am being teleported...",
                     clr_white,
                     true,
                     MorePromptOnMsg::yes);

        map::player->teleport();

        amulet_->effect_noticed(id());
    }
}

//--------------------------------------------------------- SUMMON MON
void AmuletEffectSummonMon::on_std_turn_equipped()
{
    const int sound_one_in_n  = 250;

    if (effects_known_[size_t(id())] && rnd::one_in(sound_one_in_n))
    {
        msg_log::add("I hear a faint whistling sound coming nearer...",
                     clr_white,
                     false,
                     MorePromptOnMsg::yes);

        map::player->incr_shock(ShockLvl::mild, ShockSrc::misc);
    }

    const int summon_one_in_n = 1200;

    if (rnd::one_in(summon_one_in_n))
    {
        msg_log::add("There is a loud whistling sound.",
                     clr_white,
                     false,
                     MorePromptOnMsg::yes);

        const P origin = map::player->pos;

        std::vector<Mon*> summoned_mon;

        actor_factory::summon(origin,
                              std::vector<ActorId>(1, ActorId::greater_polyp),
                              MakeMonAware::no,
                              nullptr,
                              &summoned_mon);

        const Mon* const mon = summoned_mon[0];

        if (map::player->can_see_actor(*mon))
        {
            msg_log::add(mon->name_a() + " appears!",
                         clr_white,
                         true,
                         MorePromptOnMsg::yes);
        }

        amulet_->effect_noticed(id());
    }
}

//--------------------------------------------------------- FIRE
void AmuletEffectFire::on_std_turn_equipped()
{
    const int fire_one_in_n = 300;

    if (rnd::one_in(fire_one_in_n))
    {
        const P& origin   = map::player->pos;
        const int d         = fov_std_radi_int - 2;
        const int X0        = std::max(1,            origin.x - d);
        const int Y0        = std::max(1,            origin.y - d);
        const int X1        = std::min(map_w - 2,    origin.x + d);
        const int Y1        = std::min(map_h - 2,    origin.y + d);

        const int fire_cell_one_in_n = 4;

        msg_log::add("The surrounding area suddenly burst into flames!", clr_white,
                     false, MorePromptOnMsg::yes);

        for (int x = X0; x <= X1; ++x)
        {
            for (int y = Y0; y <= Y1; ++y)
            {
                if (rnd::one_in(fire_cell_one_in_n) && P(x, y) != origin)
                {
                    map::cells[x][y].rigid->hit(DmgType::fire, DmgMethod::elemental);
                }
            }
        }

        amulet_->effect_noticed(id());
    }
}

//--------------------------------------------------------- CONFLICT
void AmuletEffectConflict::on_std_turn_equipped()
{
    const int conflict_one_in_n = 50;

    if (rnd::one_in(conflict_one_in_n))
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
                    actor->prop_handler().try_add(
                        new PropConflict(PropTurns::std));

                    amulet_->effect_noticed(id());

                    break;
                }
            }
        }
    }
}

//--------------------------------------------------------- SHRIEK
AmuletEffectShriek::AmuletEffectShriek(Amulet* const amulet) :
    AmuletEffect(amulet)
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
        "BLOOD",
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

void AmuletEffectShriek::on_std_turn_equipped()
{
    const int noise_one_in_n = 300;

    if (rnd::one_in(noise_one_in_n))
    {
        const std::string name = amulet_->name(ItemRefType::plain, ItemRefInf::none);

        msg_log::add("The " + name + " shrieks...",
                     clr_text,
                     false,
                     MorePromptOnMsg::yes);

        const int nr_words = 3;

        std::string phrase = "";

        for (int i = 0; i < nr_words; ++i)
        {
            const int   idx     = rnd::range(0, words_.size() - 1);
            const auto& word    = words_[size_t(idx)];

            phrase += word;

            if (i < nr_words - 1)
            {
                phrase += " ... ";
            }
        }

        phrase += "!!!";

        Snd snd(phrase,
                SfxId::END,
                IgnoreMsgIfOriginSeen::no,
                map::player->pos,
                map::player,
                SndVol::high,
                AlertsMon::yes);

        snd_emit::run(snd);

        map::player->incr_shock(ShockLvl::mild, ShockSrc::misc);

        msg_log::more_prompt();

        amulet_->effect_noticed(id());
    }
}

//--------------------------------------------------------- BURDEN
AmuletEffectBurden::AmuletEffectBurden(Amulet* const amulet) :
    AmuletEffect(amulet)
{
    effects_known_[size_t(id())] = true;
}

void AmuletEffectBurden::on_equip(const Verbosity verbosity)
{
    (void)verbosity;
}

void AmuletEffectBurden::change_item_weight(int& weight_ref)
{
    weight_ref = int(ItemWeight::medium);
}

//--------------------------------------------------------- hp REGEN BONUS
void AmuletEffectHpRegenBon::on_equip(const Verbosity verbosity)
{
    if (verbosity == Verbosity::verbose)
    {
        msg_log::add("I heal faster.");
    }

    amulet_->effect_noticed(id());
}

UnequipAllowed AmuletEffectHpRegenBon::on_unequip()
{
    msg_log::add("I heal slower.");
    return UnequipAllowed::yes;
}

//--------------------------------------------------------- hp REGEN PENALTY
void AmuletEffectHpRegenPen::on_equip(const Verbosity verbosity)
{
    if (verbosity == Verbosity::verbose)
    {
        msg_log::add("I heal slower.");
    }

    amulet_->effect_noticed(id());
}

UnequipAllowed AmuletEffectHpRegenPen::on_unequip()
{
    msg_log::add("I heal faster.");
    return UnequipAllowed::yes;
}

//--------------------------------------------------------- AMULET
Amulet::Amulet(ItemDataT* const item_data) :
    Item(item_data)
{
    for (size_t i = 0; i < size_t(AmuletEffectId::END); ++i)
    {
        if (effect_list_[i] == item_data->id)
        {
            auto* const effect = mk_effect(AmuletEffectId(i), this);
            effects_.push_back(effect);
        }
    }

    //Unique item, do not allow spawning more
    data_->allow_spawn = false;
}

Amulet::~Amulet()
{
    for (auto* const effect : effects_)
    {
        delete effect;
    }
}

std::vector<std::string> Amulet::descr() const
{
    std::vector<std::string> ret;

    for (auto* effect : effects_)
    {
        const size_t effect_idx = size_t(effect->id());

        if (effects_known_[effect_idx])
        {
            ret.push_back(effect->descr());
        }
    }

    if (data_->is_identified)
    {
        const std::string amulet_name = name(ItemRefType::plain, ItemRefInf::none);

        ret.push_back("All properties of the " + amulet_name + " are known to me.");
    }

    return ret;
}

std::string Amulet::name_inf() const
{
    return data_->is_identified ? "{Known}" : "";
}

void Amulet::on_equip_hook(const Verbosity verbosity)
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

UnequipAllowed Amulet::on_unequip_hook()
{
    auto unequip_allowed = UnequipAllowed::yes;

    for (auto* const effect : effects_)
    {
        if (effect->on_unequip() == UnequipAllowed::no)
        {
            unequip_allowed = UnequipAllowed::no;
        }
    }

    msg_log::more_prompt();

    return unequip_allowed;
}

void Amulet::on_std_turn_in_inv(const InvType inv_type)
{
    if (inv_type == InvType::slots)
    {
        for (auto* const effect : effects_)
        {
            effect->on_std_turn_equipped();
        }
    }
}

void Amulet::on_actor_turn_in_inv(const InvType inv_type)
{
    if (inv_type == InvType::slots)
    {
        for (auto* const effect : effects_)
        {
            effect->on_actor_turn_equipped();
        }
    }
}

void Amulet::identify(const Verbosity verbosity)
{
    if (!data_->is_identified)
    {
        data_->is_identified = true;

        for (auto* effect : effects_)
        {
            const size_t effect_idx = size_t(effect->id());

            effects_known_[effect_idx] = true;
        }

        if (verbosity == Verbosity::verbose)
        {
            const std::string name_plain = name(ItemRefType::plain, ItemRefInf::none);

            const std::string msg = "I feel like all properties of the " + name_plain +
                                    " are known to me.";

            msg_log::add(msg,
                         clr_white,
                         false,
                         MorePromptOnMsg::yes);

            const std::string name_a = name(ItemRefType::a, ItemRefInf::none);

            dungeon_master::add_history_event("Learned all the properties of " + name_a + ".");
        }
    }
}

int Amulet::weight() const
{
    int weight = Item::weight();

    for (auto* effect : effects_)
    {
        effect->change_item_weight(weight);
    }

    return weight;
}

int Amulet::hp_regen_change(const InvType inv_type) const
{
    int change = 0;

    if (inv_type == InvType::slots)
    {
        for (auto* effect : effects_)
        {
            change += effect->hp_regen_change();
        }
    }

    return change;
}

void Amulet::effect_noticed(const AmuletEffectId effect_id)
{
    const size_t effect_idx = size_t(effect_id);

    if (!effects_known_[effect_idx])
    {
        effects_known_[effect_idx] = true;

        const int   max_nr_effects_on_item      = 2;
        int         nr_effects_known_this_item  = 0;

        for (auto* effect : effects_)
        {
            const size_t check_effect_idx = size_t(effect->id());

            if (effects_known_[check_effect_idx])
            {
                ++nr_effects_known_this_item;
            }
        }

        ASSERT(nr_effects_known_this_item <= max_nr_effects_on_item);

        const std::string name_plain = name(ItemRefType::plain, ItemRefInf::none);

        msg_log::add("I gained new knowledge about the " + name_plain + ".",
                     clr_white,
                     false,
                     MorePromptOnMsg::yes);

        if (nr_effects_known_this_item == max_nr_effects_on_item)
        {
            //Amulet is completely identified - print id message and set id status in item data
            identify(Verbosity::verbose);
        }
        else //Not all properties are known
        {
            //NOTE: The identify method adds a history event about learning *all* properties,
            //so we only add an event about learning a property if it is not yet fully identified
            //(i.e. when we don't call the identify method) - otherwise the history would contain
            //something like "Learned a property" immediately followed by "Learned all properties",
            //which would be redundant.
            const std::string name_a = name(ItemRefType::a, ItemRefInf::none);

            dungeon_master::add_history_event("Learned a new property of " + name_a + ".");
        }
    }
}

//--------------------------------------------------------- AMULET HANDLING
namespace
{

bool allow_combine_effects(const AmuletEffectId id1,
                           const AmuletEffectId id2)
{
    typedef AmuletEffectId Id;

    ASSERT(id1 != Id::END && id2 != Id::END);

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

    case AmuletEffectId::START_OF_SECONDARY_EFFECTS:
    case Id::END:
        ASSERT(false);
        return false;
    }

    return false;
}

int rnd_item_bucket_idx_for_effect(const AmuletEffectId effect_to_assign,
                                   const std::vector<ItemId>& item_bucket)
{
    std::vector<int> item_idx_bucket;

    for (size_t item_bucket_idx = 0; item_bucket_idx < item_bucket.size(); ++item_bucket_idx)
    {
        bool can_be_placed_on_item = true;

        //Verify that the effect can be placed on this item by checking if it can
        //be combined with every effect currently assigned to this item.
        for (size_t i = 0; i < size_t(AmuletEffectId::END); ++i)
        {
            const AmuletEffectId cur_effect = AmuletEffectId(i);

            if (effect_list_[i] == item_bucket[item_bucket_idx])
            {
                const bool allow_combine = allow_combine_effects(effect_to_assign, cur_effect);

                if (!allow_combine)
                {
                    //Combination with effect already assigned on item not allowed
                    can_be_placed_on_item = false;
                    break;
                }
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

    const int idx_bucket_idx = rnd::range(0, item_idx_bucket.size() - 1);
    return item_idx_bucket[idx_bucket_idx];
}

} //namespace

namespace amulet_handling
{

void init()
{
    //Reset the effect list and knowledge status list
    for (size_t i = 0; i < size_t(AmuletEffectId::END); ++i)
    {
        effect_list_   [i] = ItemId::END;
        effects_known_ [i] = false;
    }

    std::vector<ItemId> item_bucket;

    for (auto& data : item_data::data)
    {
        if (data.type == ItemType::amulet)
        {
            item_bucket.push_back(data.id);
        }
    }

    std::random_shuffle(begin(item_bucket), end(item_bucket));

    std::vector<AmuletEffectId> primary_effect_bucket;
    std::vector<AmuletEffectId> secondary_effect_bucket;

    for (int i = 0; i < int(AmuletEffectId::END); ++i)
    {
        const AmuletEffectId id = AmuletEffectId(i);

        if (id <  AmuletEffectId::START_OF_SECONDARY_EFFECTS)
        {
            primary_effect_bucket.push_back(id);
        }
        else if (id >  AmuletEffectId::START_OF_SECONDARY_EFFECTS)
        {
            secondary_effect_bucket.push_back(id);
        }
    }

    //Assuming there are more amulet items than primary or secondary effects (if this changes,
    //just add more amulets to the item data)
    ASSERT(item_bucket.size() > primary_effect_bucket.size());
    ASSERT(item_bucket.size() > secondary_effect_bucket.size());

    random_shuffle(begin(primary_effect_bucket),   end(primary_effect_bucket));
    random_shuffle(begin(secondary_effect_bucket), end(secondary_effect_bucket));

    //Assign primary effects
    for (size_t i = 0; i < item_bucket.size(); ++i)
    {
        const auto item_id = item_bucket[i];

        if (i < primary_effect_bucket.size())
        {
            const auto effect_id = primary_effect_bucket[i];

            effect_list_[size_t(effect_id)] = item_id;
        }
        else //All primary effects have been assigned
        {
            //Prevent the remaining amulet data entries from spawning
            item_data::data[size_t(item_id)].allow_spawn = false;
        }
    }

    //Remove all items without a primary effect from the item bucket (this is why we assert that
    //there are more amulet items than primary effects above)
    item_bucket.resize(primary_effect_bucket.size());

    //Assign secondary effects
    for (const auto secondary_effect_id : secondary_effect_bucket)
    {
        ASSERT(secondary_effect_id != AmuletEffectId::START_OF_SECONDARY_EFFECTS);
        ASSERT(secondary_effect_id != AmuletEffectId::END);

        const int item_idx = rnd_item_bucket_idx_for_effect(secondary_effect_id, item_bucket);

        if (item_idx >= 0)
        {
            const ItemId id = item_bucket[size_t(item_idx)];

            ASSERT(item_data::data[size_t(id)].type == ItemType::amulet);

            effect_list_[size_t(secondary_effect_id)] = id;
            item_bucket.erase(begin(item_bucket) + size_t(item_idx));
        }
    }
}

void save()
{
    for (size_t i = 0; i < size_t(AmuletEffectId::END); ++i)
    {
        save_handling::put_int(int(effect_list_[i]));
        save_handling::put_bool(effects_known_[i]);
    }
}

void load()
{
    for (size_t i = 0; i < size_t(AmuletEffectId::END); ++i)
    {
        effect_list_[i]   = ItemId(save_handling::get_int());
        effects_known_[i] = save_handling::get_bool();
    }
}

} //amulet_handling
