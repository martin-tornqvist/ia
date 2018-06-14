#include "property_factory.hpp"

#include "property.hpp"

namespace property_factory
{

Prop* make(const PropId id)
{
        ASSERT(id != PropId::END);

        switch (id)
        {
        case PropId::nailed:
                return new PropNailed();

        case PropId::wound:
                return new PropWound();

        case PropId::blind:
                return new PropBlind();

        case PropId::deaf:
                return new PropDeaf();

        case PropId::burning:
                return new PropBurning();

        case PropId::flared:
                return new PropFlared();

        case PropId::paralyzed:
                return new PropParalyzed();

        case PropId::terrified:
                return new PropTerrified();

        case PropId::weakened:
                return new PropWeakened();

        case PropId::confused:
                return new PropConfused();

        case PropId::stunned:
                return new PropStunned();

        case PropId::waiting:
                return new PropWaiting();

        case PropId::slowed:
                return new PropSlowed();

        case PropId::hasted:
                return new PropHasted();

        case PropId::clockwork_hasted:
                return new PropClockworkHasted();

        case PropId::summoned:
                return new PropSummoned();

        case PropId::infected:
                return new PropInfected();

        case PropId::diseased:
                return new PropDiseased();

        case PropId::descend:
                return new PropDescend();

        case PropId::poisoned:
                return new PropPoisoned();

        case PropId::fainted:
                return new PropFainted();

        case PropId::frenzied:
                return new PropFrenzied();

        case PropId::aiming:
                return new PropAiming();

        case PropId::disabled_attack:
                return new PropDisabledAttack();

        case PropId::disabled_melee:
                return new PropDisabledMelee();

        case PropId::disabled_ranged:
                return new PropDisabledRanged();

        case PropId::blessed:
                return new PropBlessed();

        case PropId::cursed:
                return new PropCursed();

        case PropId::entangled:
                return new PropEntangled();

        case PropId::r_acid:
                return new PropRAcid();

        case PropId::r_conf:
                return new PropRConf();

        case PropId::r_breath:
                return new PropRBreath();

        case PropId::r_elec:
                return new PropRElec();

        case PropId::r_fear:
                return new PropRFear();

        case PropId::r_slow:
                return new PropRSlow();

        case PropId::r_phys:
                return new PropRPhys();

        case PropId::r_fire:
                return new PropRFire();

        case PropId::r_spell:
                return new PropRSpell();

        case PropId::r_poison:
                return new PropRPoison();

        case PropId::r_sleep:
                return new PropRSleep();

        case PropId::light_sensitive:
                return new PropLgtSens();

        case PropId::zuul_possess_priest:
                return new PropZuulPossessPriest();

        case PropId::possessed_by_zuul:
                return new PropPossessedByZuul();

        case PropId::major_clapham_summon:
                return new PropMajorClaphamSummon();

        case PropId::flying:
                return new PropFlying();

        case PropId::ethereal:
                return new PropEthereal();

        case PropId::ooze:
                return new PropOoze();

        case PropId::burrowing:
                return new PropBurrowing();

        case PropId::radiant:
                return new PropRadiant();

        case PropId::darkvision:
                return new PropDarkvis();

        case PropId::r_disease:
                return new PropRDisease();

        case PropId::r_blind:
                return new PropRBlind();

        case PropId::r_para:
                return new PropRPara();

        case PropId::tele_ctrl:
                return new PropTeleControl();

        case PropId::spell_reflect:
                return new PropSpellReflect();

        case PropId::conflict:
                return new PropConflict();

        case PropId::vortex:
                return new PropVortex();

        case PropId::explodes_on_death:
                return new PropExplodesOnDeath();

        case PropId::splits_on_death:
                return new PropSplitsOnDeath();

        case PropId::corpse_eater:
                return new PropCorpseEater();

        case PropId::teleports:
                return new PropTeleports();

        case PropId::alters_env:
                return new PropAltersEnv();

        case PropId::corrupts_env_color:
                return new PropCorruptsEnvColor();

        case PropId::regenerates:
                return new PropRegenerates();

        case PropId::corpse_rises:
                return new PropCorpseRises();

        case PropId::breeds:
                return new PropBreeds();

        case PropId::confuses_adjacent:
                return new PropConfusesAdjacent();

        case PropId::speaks_curses:
                return new PropSpeaksCurses();

        case PropId::spawns_zombie_parts_on_destroyed:
                return new PropSpawnsZombiePartsOnDestroyed();

        case PropId::invis:
                return new PropInvisible();

        case PropId::cloaked:
                return new PropCloaked();

        case PropId::recloaks:
                return new PropRecloaks();

        case PropId::see_invis:
                return new PropSeeInvis();

        case PropId::hp_sap:
                return new PropHpSap();

        case PropId::spi_sap:
                return new PropSpiSap();

        case PropId::mind_sap:
                return new PropMindSap();

        case PropId::END:
                break;
        }

        return nullptr;
}

} // property_factory
