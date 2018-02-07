#include "property_factory.hpp"

namespace property_factory
{

Prop* mk(const PropId id,
         PropTurns turns_init,
         const int nr_turns)
{
    ASSERT(id != PropId::END);

    // Prop turns init type should either be:
    // * "specific", and number of turns specified greater than zero, OR
    // * NOT "specific" (i.e. "indefinite" or "std"), and number of turns NOT
    //   specified (-1)
    ASSERT(((turns_init == PropTurns::specific) && (nr_turns > 0)) ||
           ((turns_init != PropTurns::specific) && (nr_turns == -1)));

    switch (id)
    {
    case PropId::nailed:
        return new PropNailed(turns_init, nr_turns);

    case PropId::wound:
        return new PropWound(turns_init, nr_turns);

    case PropId::blind:
        return new PropBlind(turns_init, nr_turns);

    case PropId::deaf:
        return new PropDeaf(turns_init, nr_turns);

    case PropId::burning:
        return new PropBurning(turns_init, nr_turns);

    case PropId::flared:
        return new PropFlared(turns_init, nr_turns);

    case PropId::paralyzed:
        return new PropParalyzed(turns_init, nr_turns);

    case PropId::terrified:
        return new PropTerrified(turns_init, nr_turns);

    case PropId::weakened:
        return new PropWeakened(turns_init, nr_turns);

    case PropId::confused:
        return new PropConfused(turns_init, nr_turns);

    case PropId::stunned:
        return new PropStunned(turns_init, nr_turns);

    case PropId::waiting:
        return new PropWaiting(turns_init, nr_turns);

    case PropId::slowed:
        return new PropSlowed(turns_init, nr_turns);

    case PropId::hasted:
        return new PropHasted(turns_init, nr_turns);

    case PropId::clockwork_hasted:
        return new PropClockworkHasted(turns_init, nr_turns);

    case PropId::summoned:
        return new PropSummoned(turns_init, nr_turns);

    case PropId::infected:
        return new PropInfected(turns_init, nr_turns);

    case PropId::diseased:
        return new PropDiseased(turns_init, nr_turns);

    case PropId::descend:
        return new PropDescend(turns_init, nr_turns);

    case PropId::poisoned:
        return new PropPoisoned(turns_init, nr_turns);

    case PropId::fainted:
        return new PropFainted(turns_init, nr_turns);

    case PropId::frenzied:
        return new PropFrenzied(turns_init, nr_turns);

    case PropId::aiming:
        return new PropAiming(turns_init, nr_turns);

    case PropId::disabled_attack:
        return new PropDisabledAttack(turns_init, nr_turns);

    case PropId::disabled_melee:
        return new PropDisabledMelee(turns_init, nr_turns);

    case PropId::disabled_ranged:
        return new PropDisabledRanged(turns_init, nr_turns);

    case PropId::blessed:
        return new PropBlessed(turns_init, nr_turns);

    case PropId::cursed:
        return new PropCursed(turns_init, nr_turns);

    case PropId::r_acid:
        return new PropRAcid(turns_init, nr_turns);

    case PropId::r_conf:
        return new PropRConf(turns_init, nr_turns);

    case PropId::r_breath:
        return new PropRBreath(turns_init, nr_turns);

    case PropId::r_elec:
        return new PropRElec(turns_init, nr_turns);

    case PropId::r_fear:
        return new PropRFear(turns_init, nr_turns);

    case PropId::r_slow:
        return new PropRSlow(turns_init, nr_turns);

    case PropId::r_phys:
        return new PropRPhys(turns_init, nr_turns);

    case PropId::r_fire:
        return new PropRFire(turns_init, nr_turns);

    case PropId::r_spell:
        return new PropRSpell(turns_init, nr_turns);

    case PropId::r_poison:
        return new PropRPoison(turns_init, nr_turns);

    case PropId::r_sleep:
        return new PropRSleep(turns_init, nr_turns);

    case PropId::light_sensitive:
        return new PropLgtSens(turns_init, nr_turns);

    case PropId::poss_by_zuul:
        return new PropPossByZuul(turns_init, nr_turns);

    case PropId::major_clapham_summon:
        return new PropMajorClaphamSummon(turns_init, nr_turns);

    case PropId::flying:
        return new PropFlying(turns_init, nr_turns);

    case PropId::ethereal:
        return new PropEthereal(turns_init, nr_turns);

    case PropId::ooze:
        return new PropOoze(turns_init, nr_turns);

    case PropId::burrowing:
        return new PropBurrowing(turns_init, nr_turns);

    case PropId::radiant:
        return new PropRadiant(turns_init, nr_turns);

    case PropId::darkvision:
        return new PropDarkvis(turns_init, nr_turns);

    case PropId::r_disease:
        return new PropRDisease(turns_init, nr_turns);

    case PropId::r_blind:
        return new PropRBlind(turns_init, nr_turns);

    case PropId::r_para:
        return new PropRPara(turns_init, nr_turns);

    case PropId::tele_ctrl:
        return new PropTeleControl(turns_init, nr_turns);

    case PropId::spell_reflect:
        return new PropSpellReflect(turns_init, nr_turns);

    case PropId::conflict:
        return new PropConflict(turns_init, nr_turns);

    case PropId::vortex:
        return new PropVortex(turns_init, nr_turns);

    case PropId::explodes_on_death:
        return new PropExplodesOnDeath(turns_init, nr_turns);

    case PropId::splits_on_death:
        return new PropSplitsOnDeath(turns_init, nr_turns);

    case PropId::corpse_eater:
        return new PropCorpseEater(turns_init, nr_turns);

    case PropId::teleports:
        return new PropTeleports(turns_init, nr_turns);

    case PropId::corrupts_env_color:
        return new PropCorruptsEnvColor(turns_init, nr_turns);

    case PropId::regenerates:
        return new PropRegenerates(turns_init, nr_turns);

    case PropId::corpse_rises:
        return new PropCorpseRises(turns_init, nr_turns);

    case PropId::breeds:
        return new PropBreeds(turns_init, nr_turns);

    case PropId::confuses_adjacent:
        return new PropConfusesAdjacent(turns_init, nr_turns);

    case PropId::speaks_curses:
        return new PropSpeaksCurses(turns_init, nr_turns);

    case PropId::invis:
        return new PropInvisible(turns_init, nr_turns);

    case PropId::cloaked:
        return new PropCloaked(turns_init, nr_turns);

    case PropId::recloaks:
        return new PropRecloaks(turns_init, nr_turns);

    case PropId::see_invis:
        return new PropSeeInvis(turns_init, nr_turns);

    case PropId::hp_sap:
        return new PropHpSap(turns_init, nr_turns);

    case PropId::spi_sap:
        return new PropSpiSap(turns_init, nr_turns);

    case PropId::mind_sap:
        return new PropMindSap(turns_init, nr_turns);

    case PropId::END:
        break;
    }

    return nullptr;
}

} // property_factory
