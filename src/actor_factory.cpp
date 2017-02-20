#include "actor_factory.hpp"

#include <algorithm>

#include "actor_mon.hpp"
#include "actor_player.hpp"
#include "map.hpp"
#include "io.hpp"
#include "map_parsing.hpp"
#include "actor.hpp"
#include "feature_rigid.hpp"
#include "init.hpp"
#include "io.hpp"

namespace actor_factory
{

namespace
{

Actor* mk_actor_from_id(const ActorId id)
{
    switch (id)
    {
    case ActorId::player:
        return new Player();

    case ActorId::zombie:
        return new ZombieClaw();

    case ActorId::zombie_axe:
        return new ZombieAxe();

    case ActorId::bloated_zombie:
        return new BloatedZombie();

    case ActorId::major_clapham_lee:
        return new MajorClaphamLee();

    case ActorId::dean_halsey:
        return new DeanHalsey();

    case ActorId::crawling_intestines:
        return new CrawlingIntestines();

    case ActorId::crawling_hand:
        return new CrawlingHand();

    case ActorId::thing:
        return new Thing();

    case ActorId::floating_skull:
        return new FloatingSkull();

    case ActorId::rat:
        return new Rat();

    case ActorId::rat_thing:
        return new RatThing();

    case ActorId::brown_jenkin:
        return new BrownJenkin();

    case ActorId::green_spider:
        return new GreenSpider();

    case ActorId::red_spider:
        return new RedSpider();

    case ActorId::white_spider:
        return new WhiteSpider();

    case ActorId::shadow_spider:
        return new ShadowSpider();

    case ActorId::leng_spider:
        return new LengSpider();

    case ActorId::pit_viper:
        return new PitViper();

    case ActorId::spitting_cobra:
        return new SpittingCobra();

    case ActorId::black_mamba:
        return new BlackMamba();

    case ActorId::fire_hound:
        return new FireHound();

    case ActorId::zuul:
        return new Zuul();

    case ActorId::ghost:
        return new Ghost();

    case ActorId::wraith:
        return new Wraith();

    case ActorId::phantasm:
        return new Phantasm();

    case ActorId::raven:
        return new Raven();

    case ActorId::giant_bat:
        return new GiantBat();

    case ActorId::vampire_bat:
        return new VampireBat();

    case ActorId::baeleroch:
        return new Baeleroch();

    case ActorId::cultist:
        return new Cultist();

    case ActorId::bog_tcher:
        return new BogTcher();

    case ActorId::cultist_priest:
        return new CultistPriest();

    case ActorId::cultist_wizard:
        return new CultistWizard();

    case ActorId::cultist_grand_wizard:
        return new CultistGrandWizard();

    case ActorId::keziah_mason:
        return new KeziahMason();

    case ActorId::leng_elder:
        return new LengElder();

    case ActorId::wolf:
        return new Wolf();

    case ActorId::flying_polyp:
        return new FlyingPolyp();

    case ActorId::greater_polyp:
        return new GreaterPolyp();

    case ActorId::mind_eater:
        return new MindEater();

    case ActorId::mi_go:
        return new MiGo();

    case ActorId::mi_go_commander:
        return new MiGoCommander();

    case ActorId::ghoul:
        return new Ghoul();

    case ActorId::void_traveler:
        return new VoidTraveler();

    case ActorId::shadow:
        return new Shadow();

    case ActorId::invis_stalker:
        return new InvisStalker();

    case ActorId::byakhee:
        return new Byakhee();

    case ActorId::giant_mantis:
        return new GiantMantis();

    case ActorId::locust:
        return new GiantLocust();

    case ActorId::mummy:
        return new Mummy();

    case ActorId::croc_head_mummy:
        return new MummyCrocHead();

    case ActorId::khephren:
        return new Khephren();

    case ActorId::nitokris:
        return new MummyUnique();

    case ActorId::deep_one:
        return new DeepOne();

    case ActorId::ape:
        return new Ape();

    case ActorId::worm_mass:
        return new WormMass();

    case ActorId::mind_worms:
        return new MindWorms();

    case ActorId::dust_vortex:
        return new DustVortex();

    case ActorId::fire_vortex:
        return new FireVortex();

    case ActorId::ooze_black:
        return new OozeBlack();

    case ActorId::strange_color:
        return new StrangeColor();

    case ActorId::ooze_clear:
        return new OozeClear();

    case ActorId::ooze_putrid:
        return new OozePutrid();

    case ActorId::ooze_poison:
        return new OozePoison();

    case ActorId::chthonian:
        return new Chthonian();

    case ActorId::death_fiend:
        return new DeathFiend();

    case ActorId::hunting_horror:
        return new HuntingHorror();

    case ActorId::sentry_drone:
        return new SentryDrone();

    case ActorId::animated_wpn:
        return new AnimatedWpn();

    case ActorId::mold:
        return new Mold();

    case ActorId::gas_spore:
        return new GasSpore();

    case ActorId::the_high_priest:
        return new TheHighPriest();

    case ActorId::END:
        break;
    }

    return nullptr;
}

} // namespace

Actor* mk(const ActorId id, const P& pos)
{
    ASSERT(!map::cells[pos.x][pos.y].rigid ||
           (map::cells[pos.x][pos.y].rigid->id() != FeatureId::stairs));

    Actor* const actor = mk_actor_from_id(id);

    actor->place(pos, actor_data::data[size_t(id)]);

    auto& data = actor->data();

    if (data.nr_left_allowed_to_spawn > 0)
    {
        --data.nr_left_allowed_to_spawn;
    }

    game_time::add_actor(actor);

    return actor;
}

void delete_all_mon()
{
    std::vector<Actor*>& actors = game_time::actors;

    for (auto it = begin(actors); it != end(actors); /* No increment */)
    {
        Actor* const actor = *it;

        if (actor == map::player)
        {
            ++it;
        }
        else // Is monster
        {
            delete actor;

            it = actors.erase(it);
        }
    }
}

std::vector<Mon*> spawn(const P& origin,
                        const std::vector<ActorId>& monster_ids,
                        const MakeMonAware make_aware,
                        Actor* const actor_to_set_as_leader)
{
    TRACE_FUNC_BEGIN;

    std::vector<Mon*> monsters_summoned;

    bool blocked[map_w][map_h];

    map_parsers::BlocksMoveCmn(ParseActors::yes)
        .run(blocked);

    std::vector<P> free_cells;

    to_vec(blocked, false, free_cells);

    std::sort(begin(free_cells), end(free_cells), IsCloserToPos(origin));

    const size_t nr_free_cells = free_cells.size();

    const size_t nr_monster_ids = monster_ids.size();

    const int nr_to_spawn = std::min(nr_free_cells, nr_monster_ids);

    for (int i = 0; i < nr_to_spawn; ++i)
    {
        const P& pos = free_cells[i];

        const ActorId id = monster_ids[i];

        Actor* const actor = mk(id, pos);

        Mon* const mon = static_cast<Mon*>(actor);

        ASSERT(map::is_pos_inside_map(pos, false));

        // Should the player be aware of the monster?
        if (map::cells[mon->pos.x][mon->pos.y].is_seen_by_player &&
            !mon->is_sneaking())
        {
            mon->set_player_aware_of_me();
        }

        monsters_summoned.push_back(mon);

        if (actor_to_set_as_leader)
        {
            mon->leader_ = actor_to_set_as_leader;
        }

        if (make_aware == MakeMonAware::yes)
        {
            mon->aware_of_player_counter_ = mon->data().nr_turns_aware;
        }
    }

    TRACE_FUNC_END;

    return monsters_summoned;
}

} // actor_factory
