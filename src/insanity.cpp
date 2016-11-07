#include "insanity.hpp"

#include "popup.hpp"
#include "game.hpp"
#include "sound.hpp"
#include "map.hpp"
#include "actor_player.hpp"
#include "msg_log.hpp"
#include "actor_mon.hpp"
#include "properties.hpp"
#include "feature_rigid.hpp"
#include "actor_factory.hpp"
#include "saving.hpp"
#include "init.hpp"
#include "game_time.hpp"
#include "io.hpp"

//-----------------------------------------------------------------------------
// Insanity symptoms
//-----------------------------------------------------------------------------
void InsSympt::on_start()
{
    msg_log::more_prompt();

    const std::string heading   = start_heading();
    const std::string msg       = "Insanity draws nearer... " + start_msg();

    ASSERT(!heading.empty() && !msg.empty());

    if (!heading.empty() && !msg.empty())
    {
        popup::show_msg(msg,
                        heading,
                        SfxId::insanity_rise);
    }

    const std::string history_event_msg = history_msg();

    ASSERT(!history_event_msg.empty());

    if (!history_event_msg.empty())
    {
        game::add_history_event(history_event_msg);
    }

    on_start_hook();
}

void InsSympt::on_end()
{
    const std::string msg = end_msg();

    ASSERT(!msg.empty());

    if (!msg.empty())
    {
        msg_log::add(msg);
    }

    const std::string history_event_msg = history_msg_end();

    ASSERT(!history_event_msg.empty());

    if (!history_event_msg.empty())
    {
        game::add_history_event(history_event_msg);
    }
}

bool InsScream::is_allowed() const
{
    return !map::player->has_prop(PropId::rFear);
}

void InsScream::on_start_hook()
{
    Snd snd("",
            SfxId::END,
            IgnoreMsgIfOriginSeen::yes,
            map::player->pos,
            map::player,
            SndVol::high,
            AlertsMon::yes);

    snd_emit::run(snd);
}

std::string InsScream::start_msg() const
{
    if (rnd::coin_toss())
    {
        return "I let out a terrified shriek.";
    }
    else
    {
        return "I scream in terror.";
    }
}

void InsBabbling::on_start_hook()
{
    const std::string player_name = map::player->name_the();

    for (int i = rnd::range(3, 5); i > 0; --i)
    {
        msg_log::add(player_name + ": " + Cultist::cultist_phrase());
    }

    Snd snd("",
            SfxId::END,
            IgnoreMsgIfOriginSeen::yes,
            map::player->pos,
            map::player,
            SndVol::low,
            AlertsMon::yes);

    snd_emit::run(snd);
}

void InsBabbling::on_new_player_turn(const std::vector<Actor*>& seen_foes)
{
    (void)seen_foes;
}

bool InsFaint::is_allowed() const
{
    return true;
}

void InsFaint::on_start_hook()
{
    map::player->prop_handler().try_add(new PropFainted(PropTurns::std));
}

void InsLaugh::on_start_hook()
{
    Snd snd("",
            SfxId::END,
            IgnoreMsgIfOriginSeen::yes,
            map::player->pos,
            map::player,
            SndVol::low,
            AlertsMon::yes);

    snd_emit::run(snd);
}

bool InsPhobiaRat::is_allowed() const
{
    const bool has_phobia   = insanity::has_sympt_type(InsSymptType::phobia);
    const bool is_rfear     = map::player->has_prop(PropId::rFear);

    return !is_rfear && (!has_phobia || rnd::one_in(20));
}

void InsPhobiaRat::on_new_player_turn(const std::vector<Actor*>& seen_foes)
{
    if (rnd::one_in(10))
    {
        for (Actor* const actor : seen_foes)
        {
            if (actor->data().is_rat)
            {
                msg_log::add("I am plagued by my phobia of rats!");

                map::player->prop_handler().try_add(new PropTerrified(PropTurns::std));

                break;
            }
        }
    }
}

void InsPhobiaRat::on_permanent_rfear()
{
    insanity::end_sympt(id());
}

bool InsPhobiaSpider::is_allowed() const
{
    const bool has_phobia   = insanity::has_sympt_type(InsSymptType::phobia);
    const bool is_rfear     = map::player->has_prop(PropId::rFear);

    return !is_rfear && (!has_phobia || rnd::one_in(20));
}

void InsPhobiaSpider::on_new_player_turn(const std::vector<Actor*>& seen_foes)
{
    if (rnd::one_in(10))
    {
        for (Actor* const actor : seen_foes)
        {
            if (actor->data().is_spider)
            {
                msg_log::add("I am plagued by my phobia of spiders!");

                map::player->prop_handler().try_add(new PropTerrified(PropTurns::std));

                break;
            }
        }
    }
}

void InsPhobiaSpider::on_permanent_rfear()
{
    insanity::end_sympt(id());
}

bool InsPhobiaReptileAndAmph::is_allowed() const
{
    const bool has_phobia   = insanity::has_sympt_type(InsSymptType::phobia);
    const bool is_rfear     = map::player->has_prop(PropId::rFear);

    return !is_rfear && (!has_phobia || rnd::one_in(20));
}

void InsPhobiaReptileAndAmph::on_new_player_turn(const std::vector<Actor*>& seen_foes)
{
    bool is_triggered = false;

    if (rnd::one_in(10))
    {
        for (Actor* const actor : seen_foes)
        {
            if (actor->data().is_reptile)
            {
                msg_log::add("I am plagued by my phobia of reptiles!");
                is_triggered = true;
                break;
            }

            if (actor->data().is_amphibian)
            {
                msg_log::add("I am plagued by my phobia of amphibians!");
                is_triggered = true;
                break;
            }
        }
    }

    if (is_triggered)
    {
        map::player->prop_handler().try_add(new PropTerrified(PropTurns::std));
    }
}

void InsPhobiaReptileAndAmph::on_permanent_rfear()
{
    insanity::end_sympt(id());
}

bool InsPhobiaCanine::is_allowed() const
{
    const bool has_phobia   = insanity::has_sympt_type(InsSymptType::phobia);
    const bool is_rfear     = map::player->has_prop(PropId::rFear);

    return !is_rfear && (!has_phobia || rnd::one_in(20));
}

void InsPhobiaCanine::on_new_player_turn(const std::vector<Actor*>& seen_foes)
{
    if (rnd::one_in(10))
    {
        for (Actor* const actor : seen_foes)
        {
            if (actor->data().is_canine)
            {
                msg_log::add("I am plagued by my phobia of canines!");

                map::player->prop_handler().try_add(new PropTerrified(PropTurns::std));

                break;
            }
        }
    }
}

void InsPhobiaCanine::on_permanent_rfear()
{
    insanity::end_sympt(id());
}

bool InsPhobiaDead::is_allowed() const
{
    const bool has_phobia   = insanity::has_sympt_type(InsSymptType::phobia);
    const bool is_rfear     = map::player->has_prop(PropId::rFear);

    return !is_rfear && (!has_phobia || rnd::one_in(20));
}

void InsPhobiaDead::on_new_player_turn(const std::vector<Actor*>& seen_foes)
{
    if (rnd::one_in(10))
    {
        for (Actor* const actor : seen_foes)
        {
            if (actor->data().is_undead)
            {
                msg_log::add("I am plagued by my phobia of the dead!");

                map::player->prop_handler().try_add(new PropTerrified(PropTurns::std));

                break;
            }
        }
    }
}

void InsPhobiaDead::on_permanent_rfear()
{
    insanity::end_sympt(id());
}

bool InsPhobiaOpen::is_allowed() const
{
    const bool has_phobia   = insanity::has_sympt_type(InsSymptType::phobia);
    const bool is_rfear     = map::player->has_prop(PropId::rFear);

    return !is_rfear && (!has_phobia || rnd::one_in(20));
}

void InsPhobiaOpen::on_new_player_turn(const std::vector<Actor*>& seen_foes)
{
    (void)seen_foes;

    if (rnd::one_in(10) && map::player->is_standing_in_open_place())
    {
        msg_log::add("I am plagued by my phobia of open places!");

        map::player->prop_handler().try_add(new PropTerrified(PropTurns::std));
    }
}

void InsPhobiaOpen::on_permanent_rfear()
{
    insanity::end_sympt(id());
}

bool InsPhobiaConfined::is_allowed() const
{
    const bool has_phobia   = insanity::has_sympt_type(InsSymptType::phobia);
    const bool is_rfear     = map::player->has_prop(PropId::rFear);

    return !is_rfear && (!has_phobia || rnd::one_in(20));
}

void InsPhobiaConfined::on_new_player_turn(const std::vector<Actor*>& seen_foes)
{
    (void)seen_foes;

    if (rnd::one_in(10) && map::player->is_standing_in_cramped_place())
    {
        msg_log::add("I am plagued by my phobia of confined places!");

        map::player->prop_handler().try_add(new PropTerrified(PropTurns::std));
    }
}

void InsPhobiaConfined::on_permanent_rfear()
{
    insanity::end_sympt(id());
}

bool InsPhobiaDeep::is_allowed() const
{
    const bool has_phobia   = insanity::has_sympt_type(InsSymptType::phobia);
    const bool is_rfear     = map::player->has_prop(PropId::rFear);

    return !is_rfear && (!has_phobia || rnd::one_in(20));
}

void InsPhobiaDeep::on_new_player_turn(const std::vector<Actor*>& seen_foes)
{
    (void)seen_foes;

    if (rnd::one_in(10))
    {
        for (const P& d : dir_utils::dir_list)
        {
            const P p(map::player->pos + d);

            if (map::cells[p.x][p.y].rigid->is_bottomless())
            {
                msg_log::add("I am plagued by my phobia of deep places!");

                map::player->prop_handler().try_add(new PropTerrified(PropTurns::std));

                break;
            }
        }
    }
}

void InsPhobiaDeep::on_permanent_rfear()
{
    insanity::end_sympt(id());
}

bool InsPhobiaDark::is_allowed() const
{
    const bool has_phobia   = insanity::has_sympt_type(InsSymptType::phobia);
    const bool is_rfear     = map::player->has_prop(PropId::rFear);

    return !is_rfear && (!has_phobia || rnd::one_in(20));
}

void InsPhobiaDark::on_new_player_turn(const std::vector<Actor*>& seen_foes)
{
    (void)seen_foes;

    if (rnd::one_in(10))
    {
        const P             p(map::player->pos);
        const PropHandler& props = map::player->prop_handler();

        if (
            (props.allow_act() && !props.allow_see()) ||
            (map::cells[p.x][p.y].is_dark && !map::cells[p.x][p.y].is_lit))
        {
            msg_log::add("I am plagued by my phobia of the dark!");

            map::player->prop_handler().try_add(new PropTerrified(PropTurns::std));
        }
    }
}

void InsPhobiaDark::on_permanent_rfear()
{
    insanity::end_sympt(id());
}

bool InsMasoch::is_allowed() const
{
    const bool is_sadist = insanity::has_sympt(InsSymptId::sadism);

    return !is_sadist && rnd::one_in(10);
}

bool InsSadism::is_allowed() const
{
    const bool is_masoch = insanity::has_sympt(InsSymptId::masoch);

    return !is_masoch && rnd::one_in(4);
}

void InsShadows::on_start_hook()
{
    TRACE_FUNC_BEGIN;

    const int nr_shadows_lower = 2;

    const int nr_shadows_upper = constr_in_range(nr_shadows_lower,
                                                 map::dlvl - 2,
                                                 8);

    const int nr = rnd::range(nr_shadows_lower, nr_shadows_upper);

    std::vector<ActorId> shadow_ids(nr, ActorId::shadow);

    std::vector<Mon*> summoned;

    actor_factory::summon(map::player->pos,
                          shadow_ids,
                          MakeMonAware::yes,
                          nullptr,
                          &summoned,
                          Verbosity::silent);

    ASSERT(!summoned.empty());

    for (Mon* const mon : summoned)
    {
        mon->is_sneaking_                   = true;
        mon->player_aware_of_me_counter_    = 0;

        auto* const disabled_att =
            new PropDisabledAttack(PropTurns::specific, 1);

        mon->prop_handler().try_add(disabled_att);
    }

    map::update_vision();

    std::vector<Actor*> player_seen_foes;
    map::player->seen_foes(player_seen_foes);

    for (Actor* const actor : player_seen_foes)
    {
        static_cast<Mon*>(actor)->set_player_aware_of_me();
    }


    TRACE_FUNC_END;
}

void InsParanoia::on_start_hook()
{
    //Flip a coint to decide if we should spawn a stalker or not
    //(Maybe it's just paranoia, or maybe it's real)
    if (rnd::coin_toss())
    {
        std::vector<ActorId> stalker_id(1, ActorId::invis_stalker);

        std::vector<Mon*> summoned;

        actor_factory::summon(map::player->pos,
                              stalker_id,
                              MakeMonAware::yes,
                              nullptr,
                              &summoned,
                              Verbosity::silent);

        ASSERT(summoned.size() == 1);

        Mon* const mon = summoned[0];

        auto* const disabled_att = new PropDisabledAttack(PropTurns::specific, 2);

        mon->prop_handler().try_add(disabled_att);
    }
}

bool InsConfusion::is_allowed() const
{
    return !map::player->has_prop(PropId::rConf);
}

void InsConfusion::on_start_hook()
{
    map::player->prop_handler().try_add(new PropConfused(PropTurns::std));
}

bool InsFrenzy::is_allowed() const
{
    return true;
}

void InsFrenzy::on_start_hook()
{
    map::player->prop_handler().try_add(new PropFrenzied(PropTurns::std));
}


//-----------------------------------------------------------------------------
// Insanity handling
//-----------------------------------------------------------------------------
namespace insanity
{

namespace
{

InsSympt* sympts_[size_t(InsSymptId::END)];

InsSympt* mk_sympt(const InsSymptId id)
{
    switch (id)
    {
    case InsSymptId::scream:
        return new InsScream();

    case InsSymptId::babbling:
        return new InsBabbling();

    case InsSymptId::faint:
        return new InsFaint();

    case InsSymptId::laugh:
        return new InsLaugh();

    case InsSymptId::phobia_rat:
        return new InsPhobiaRat();

    case InsSymptId::phobia_spider:
        return new InsPhobiaSpider();

    case InsSymptId::phobia_reptile_and_amph:
        return new InsPhobiaReptileAndAmph();

    case InsSymptId::phobia_canine:
        return new InsPhobiaCanine();

    case InsSymptId::phobia_dead:
        return new InsPhobiaDead();

    case InsSymptId::phobia_open:
        return new InsPhobiaOpen();

    case InsSymptId::phobia_confined:
        return new InsPhobiaConfined();

    case InsSymptId::phobia_deep:
        return new InsPhobiaDeep();

    case InsSymptId::phobia_dark:
        return new InsPhobiaDark();

    case InsSymptId::masoch:
        return new InsMasoch();

    case InsSymptId::sadism:
        return new InsSadism();

    case InsSymptId::shadows:
        return new InsShadows();

    case InsSymptId::paranoia:
        return new InsParanoia();

    case InsSymptId::confusion:
        return new InsConfusion();

    case InsSymptId::frenzy:
        return new InsFrenzy();

    case InsSymptId::strange_sensation:
        return new InsStrangeSensation();

    case InsSymptId::END:
        break;
    }

    ASSERT(false);

    return nullptr;
}

} //namespace


void init()
{
    for (size_t i = 0; i < size_t(InsSymptId::END); ++i)
    {
        sympts_[i] = nullptr;
    }
}

void cleanup()
{
    for (size_t i = 0; i < size_t(InsSymptId::END); ++i)
    {
        delete sympts_[i];

        sympts_[i] = nullptr;
    }
}

void save()
{
    for (size_t i = 0; i < size_t(InsSymptId::END); ++i)
    {
        const InsSympt* const sympt = sympts_[i];

        saving::put_bool(sympt);

        if (sympt)
        {
            sympt->save();
        }
    }
}

void load()
{
    for (size_t i = 0; i < size_t(InsSymptId::END); ++i)
    {
        const bool has_symptom = saving::get_bool();

        if (has_symptom)
        {
            InsSympt* const sympt = mk_sympt(InsSymptId(i));

            sympts_[i] = sympt;

            sympt->load();
        }
    }
}

void run_sympt()
{
    std::vector<InsSympt*> sympt_bucket;

    for (size_t i = 0; i < size_t(InsSymptId::END); ++i)
    {
        const InsSympt* const active_sympt = sympts_[i];

        //Symptoms are only allowed if not already active
        if (!active_sympt)
        {
            InsSympt* const new_sympt = mk_sympt(InsSymptId(i));

            const bool is_allowed = new_sympt->is_allowed();

            if (is_allowed)
            {
                sympt_bucket.push_back(new_sympt);
            }
            else //Not allowed (e.g. player resistant to fear, and this is a phobia)
            {
                delete new_sympt;
            }
        }
    }

    if (sympt_bucket.empty())
    {
        //This should never be able to happen, since there are symptoms which can occur repeatedly,
        //unconditionally - but we do a check anyway for robustness.
        return;
    }

    const size_t bucket_idx = rnd::range(0, sympt_bucket.size() - 1);

    InsSympt* const sympt = sympt_bucket[bucket_idx];

    sympt_bucket.erase(begin(sympt_bucket) + bucket_idx);

    //Delete the remaining symptoms in the bucket
    for (auto* const sympt : sympt_bucket)
    {
        delete sympt;
    }

    //If the symptom is permanent (i.e. not a one-shot thing like screaming),
    //set it as active in the symptoms list
    if (sympt->is_permanent())
    {
        const size_t sympt_idx = size_t(sympt->id());

        ASSERT(!sympts_[sympt_idx]);

        sympts_[sympt_idx] = sympt;
    }

    sympt->on_start();
}

bool has_sympt(const InsSymptId id)
{
    ASSERT(id != InsSymptId::END);

    return sympts_[size_t(id)];
}

bool has_sympt_type(const InsSymptType type)
{
    for (size_t i = 0; i < size_t(InsSymptId::END); ++i)
    {
        const InsSympt* const s = sympts_[i];

        if (s && s->type() == type)
        {
            return true;
        }
    }

    return false;
}

std::vector<const InsSympt*> active_sympts()
{
    std::vector<const InsSympt*> out;

    for (size_t i = 0; i < size_t(InsSymptId::END); ++i)
    {
        const InsSympt* const sympt = sympts_[i];

        if (sympt)
        {
            out.push_back(sympt);
        }
    }

    return out;
}

void on_new_player_turn(const std::vector<Actor*>& seen_foes)
{
    for (size_t i = 0; i < size_t(InsSymptId::END); ++i)
    {
        InsSympt* const sympt = sympts_[i];

        if (sympt)
        {
            sympt->on_new_player_turn(seen_foes);
        }
    }
}

void on_permanent_rfear()
{
    for (size_t i = 0; i < size_t(InsSymptId::END); ++i)
    {
        InsSympt* const sympt = sympts_[i];

        if (sympt)
        {
            sympt->on_permanent_rfear();
        }
    }
}

void end_sympt(const InsSymptId id)
{
    ASSERT(id != InsSymptId::END);

    const size_t idx = size_t(id);

    InsSympt* const sympt = sympts_[idx];

    ASSERT(sympt);

    sympts_[idx] = nullptr;

    sympt->on_end();

    delete sympt;
}

} //insanity
