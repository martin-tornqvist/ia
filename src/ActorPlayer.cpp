#include "ActorPlayer.h"

#include "Init.h"
#include "Render.h"
#include "Audio.h"
#include "FeatureTrap.h"
#include "CreateCharacter.h"
#include "Log.h"
#include "CharacterLines.h"
#include "Popup.h"
#include "Postmortem.h"
#include "DungeonMaster.h"
#include "Map.h"
#include "Explosion.h"
#include "ActorMon.h"
#include "FeatureDoor.h"
#include "FeatureMob.h"
#include "Query.h"
#include "Attack.h"
#include "Fov.h"
#include "ItemFactory.h"
#include "ActorFactory.h"
#include "PlayerBon.h"
#include "Inventory.h"
#include "InventoryHandling.h"
#include "PlayerSpellsHandling.h"
#include "Bot.h"
#include "Input.h"
#include "MapParsing.h"
#include "Properties.h"
#include "ItemDevice.h"
#include "ItemScroll.h"
#include "ItemPotion.h"
#include "TextFormatting.h"
#include "Utils.h"

using namespace std;

const int MIN_SHOCK_WHEN_OBSESSION = 35;

Player::Player() :
    Actor                   (),
    activeMedicalBag        (nullptr),
    waitTurnsLeft           (-1),
    activeExplosive         (nullptr),
    tgt_                    (nullptr),
    ins_                    (0),
    shock_                  (0.0),
    shockTmp_               (0.0),
    permShockTakenCurTurn_  (0.0),
    nrMovesUntilFreeAction_ (-1),
    nrTurnsUntilIns_        (-1),
    nrQuickMoveStepsLeft_   (-1),
    quickMoveDir_           (Dir::END),
    CARRY_WEIGHT_BASE_      (450)
{
    for (int i = 0; i < int(Phobia::END); ++i)     {phobias[i]     = false;}
    for (int i = 0; i < int(Obsession::END); ++i)  {obsessions[i]  = false;}
}

Player::~Player()
{
    if (activeExplosive) {delete activeExplosive;}
}

void Player::mkStartItems()
{
    data_->abilityVals.reset();

    for (int i = 0; i < int(Phobia::END); ++i)     {phobias[i]     = false;}
    for (int i = 0; i < int(Obsession::END); ++i)  {obsessions[i]  = false;}

    int nrCartridges  = 2;
    int nrDynamite    = 2;
    int nrMolotov     = 2;
    int nrThrKnives   = 6;

    //------------------------------------------------------- BACKGROUND SPECIFIC SETUP
    const auto bg = PlayerBon::getBg();

    if (bg == Bg::occultist)
    {
        //Occultist starts with zero explosives and throwing knives.
        //(They are not so thematically fitting for this background.)
        nrDynamite    = 0;
        nrMolotov     = 0;
        nrThrKnives   = 0;

        //Occultist starts with a scroll of Darkbolt, and one other random scroll.
        //(Both are identified.)
        Item* scroll = ItemFactory::mk(ItemId::scrollDarkbolt);
        static_cast<Scroll*>(scroll)->identify(true);
        inv_->putInGeneral(scroll);
        while (true)
        {
            scroll = ItemFactory::mkRandomScrollOrPotion(true, false);

            SpellId       id          = scroll->getData().spellCastFromScroll;
            Spell* const  spell       = SpellHandling::mkSpellFromId(id);
            const bool    IS_AVAIL    = spell->isAvailForPlayer();
            const bool    SPI_COST_OK = spell->getSpiCost(true).upper <=
                                        PlayerBon::getSpiOccultistCanCastAtLvl(4);
            delete spell;

            if (IS_AVAIL && SPI_COST_OK && id != SpellId::darkbolt)
            {
                static_cast<Scroll*>(scroll)->identify(true);
                inv_->putInGeneral(scroll);
                break;
            }
        }

        //Occultist starts with a few potions (identified).
        const int NR_POTIONS = 2;
        for (int i = 0; i < NR_POTIONS; ++i)
        {
            Item* const potion = ItemFactory::mkRandomScrollOrPotion(false, true);
            static_cast<Potion*>(potion)->identify(true);
            inv_->putInGeneral(potion);
        }
    }

    if (bg == Bg::rogue)
    {
        //Rogue starts with extra throwing knives
        nrThrKnives += 6;

        //Rogue starts with a +1 dagger
        auto* const dagger = ItemFactory::mk(ItemId::dagger);
        static_cast<Wpn*>(dagger)->meleeDmgPlus_ = 1;
        inv_->slots_[int(SlotId::wielded)].item = dagger;

        //Rogue starts with some iron spikes (useful tool)
        inv_->putInGeneral(ItemFactory::mk(ItemId::ironSpike, 8));
    }

    if (bg == Bg::warVet)
    {
        //War Veteran starts with some smoke grenades and a gas mask
        inv_->putInGeneral(ItemFactory::mk(ItemId::smokeGrenade, 4));
        inv_->putInGeneral(ItemFactory::mk(ItemId::gasMask));
    }

    //------------------------------------------------------- GENERAL SETUP
    //Randomize a melee weapon if not already wielding one.
    if (!inv_->slots_[int(SlotId::wielded)].item)
    {
        const int WEAPON_CHOICE = Rnd::range(1, 5);
        auto      weaponId      = ItemId::dagger;

        switch (WEAPON_CHOICE)
        {
        case 1:   weaponId = ItemId::dagger;   break;
        case 2:   weaponId = ItemId::hatchet;  break;
        case 3:   weaponId = ItemId::hammer;   break;
        case 4:   weaponId = ItemId::machete;  break;
        case 5:   weaponId = ItemId::axe;      break;
        }
        inv_->putInSlot(SlotId::wielded, ItemFactory::mk(weaponId));
    }

    inv_->putInSlot(SlotId::wieldedAlt, ItemFactory::mk(ItemId::pistol));

    for (int i = 0; i < nrCartridges; ++i)
    {
        inv_->putInGeneral(ItemFactory::mk(ItemId::pistolClip));
    }

    if (nrDynamite > 0)
    {
        inv_->putInGeneral(ItemFactory::mk(ItemId::dynamite,  nrDynamite));
    }
    if (nrMolotov > 0)
    {
        inv_->putInGeneral(ItemFactory::mk(ItemId::molotov,   nrMolotov));
    }

    if (nrThrKnives > 0)
    {
        inv_->putInSlot(SlotId::thrown, ItemFactory::mk(ItemId::thrKnife, nrThrKnives));
    }

    inv_->putInSlot(SlotId::body, ItemFactory::mk(ItemId::armorLeatherJacket));

    inv_->putInGeneral(ItemFactory::mk(ItemId::electricLantern));
    inv_->putInGeneral(ItemFactory::mk(ItemId::medicalBag));
}

void Player::storeToSaveLines(vector<string>& lines) const
{
    lines.push_back(toStr(propHandler_->appliedProps_.size()));
    for (Prop* prop : propHandler_->appliedProps_)
    {
        lines.push_back(toStr(int(prop->getId())));
        lines.push_back(toStr(prop->turnsLeft_));
        prop->storeToSaveLines(lines);
    }

    lines.push_back(toStr(ins_));
    lines.push_back(toStr(int(shock_)));
    lines.push_back(toStr(hp_));
    lines.push_back(toStr(hpMax_));
    lines.push_back(toStr(spi_));
    lines.push_back(toStr(spiMax_));
    lines.push_back(toStr(pos.x));
    lines.push_back(toStr(pos.y));

    for (int i = 0; i < int(AbilityId::END); ++i)
    {
        lines.push_back(toStr(data_->abilityVals.getRawVal(AbilityId(i))));
    }

    for (int i = 0; i < int(Phobia::END); ++i)
    {
        lines.push_back(phobias[i] == 0 ? "0" : "1");
    }
    for (int i = 0; i < int(Obsession::END); ++i)
    {
        lines.push_back(obsessions[i] == 0 ? "0" : "1");
    }
}

void Player::setupFromSaveLines(vector<string>& lines)
{
    const int NR_PROPS = toInt(lines.front());
    lines.erase(begin(lines));
    for (int i = 0; i < NR_PROPS; ++i)
    {
        const auto id = PropId(toInt(lines.front()));
        lines.erase(begin(lines));
        const int NR_TURNS = toInt(lines.front());
        lines.erase(begin(lines));
        auto* const prop = propHandler_->mkProp(id, PropTurns::specific, NR_TURNS);
        propHandler_->tryApplyProp(prop, true, true, true, true);
        prop->setupFromSaveLines(lines);
    }

    ins_ = toInt(lines.front());
    lines.erase(begin(lines));
    shock_ = double(toInt(lines.front()));
    lines.erase(begin(lines));
    hp_ = toInt(lines.front());
    lines.erase(begin(lines));
    hpMax_ = toInt(lines.front());
    lines.erase(begin(lines));
    spi_ = toInt(lines.front());
    lines.erase(begin(lines));
    spiMax_ = toInt(lines.front());
    lines.erase(begin(lines));
    pos.x = toInt(lines.front());
    lines.erase(begin(lines));
    pos.y = toInt(lines.front());
    lines.erase(begin(lines));

    for (int i = 0; i < int(AbilityId::END); ++i)
    {
        data_->abilityVals.setVal(AbilityId(i), toInt(lines.front()));
        lines.erase(begin(lines));
    }

    for (int i = 0; i < int(Phobia::END); ++i)
    {
        phobias[i] = lines.front() == "0" ? false : true;
        lines.erase(begin(lines));
    }
    for (int i = 0; i < int(Obsession::END); ++i)
    {
        obsessions[i] = lines.front() == "0" ? false : true;
        lines.erase(begin(lines));
    }
}

void Player::hit_(int& dmg)
{
    (void)dmg;

    if (!obsessions[int(Obsession::masochism)]) {incrShock(1, ShockSrc::misc);}

    Render::drawMapAndInterface();
}

int Player::getEncPercent() const
{
    const int TOTAL_W = inv_->getTotalItemWeight();
    const int MAX_W   = getCarryWeightLmt();
    return int((double(TOTAL_W) / double(MAX_W)) * 100.0);
}

int Player::getCarryWeightLmt() const
{
    const bool IS_TOUGH         = PlayerBon::traitsPicked[int(Trait::tough)];
    const bool IS_RUGGED        = PlayerBon::traitsPicked[int(Trait::rugged)];
    const bool IS_UNBREAKABLE   = PlayerBon::traitsPicked[int(Trait::unbreakable)];
    const bool IS_STRONG_BACKED = PlayerBon::traitsPicked[int(Trait::strongBacked)];

    bool props[size_t(PropId::END)];
    propHandler_->getPropIds(props);
    const bool IS_WEAKENED = props[int(PropId::weakened)];

    const int CARRY_WEIGHT_MOD = (IS_TOUGH         * 10) +
                                 (IS_RUGGED        * 10) +
                                 (IS_UNBREAKABLE   * 10) +
                                 (IS_STRONG_BACKED * 30) -
                                 (IS_WEAKENED      * 15);

    return (CARRY_WEIGHT_BASE_ * (CARRY_WEIGHT_MOD + 100)) / 100;
}

int Player::getShockResistance(const ShockSrc shockSrc) const
{
    int res = 0;
    if (PlayerBon::traitsPicked[int(Trait::fearless)])    {res += 5;}
    if (PlayerBon::traitsPicked[int(Trait::coolHeaded)])  {res += 20;}
    if (PlayerBon::traitsPicked[int(Trait::courageous)])  {res += 20;}

    switch (shockSrc)
    {
    case ShockSrc::useStrangeItem:
        if (PlayerBon::getBg() == Bg::occultist) {res += 50;}
        break;

    case ShockSrc::castIntrSpell:
    case ShockSrc::seeMon:
    case ShockSrc::time:
    case ShockSrc::misc:
    case ShockSrc::END: {}
        break;
    }

    return getConstrInRange(0, res, 100);
}

double Player::getShockTakenAfterMods(const int BASE_SHOCK,
                                      const ShockSrc shockSrc) const
{
    const double SHOCK_RES_DB   = double(getShockResistance(shockSrc));
    const double BASE_SHOCK_DB  = double(BASE_SHOCK);
    return (BASE_SHOCK_DB * (100.0 - SHOCK_RES_DB)) / 100.0;
}

void Player::incrShock(const int SHOCK, ShockSrc shockSrc)
{
    const double SHOCK_AFTER_MODS = getShockTakenAfterMods(SHOCK, shockSrc);

    shock_                  += SHOCK_AFTER_MODS;
    permShockTakenCurTurn_  += SHOCK_AFTER_MODS;

    constrInRange(0.0f, shock_, 100.0f);
}

void Player::incrShock(const ShockLvl shockValue, ShockSrc shockSrc)
{
    incrShock(int(shockValue), shockSrc);
}

void Player::restoreShock(const int amountRestored, const bool IS_TEMP_SHOCK_RESTORED)
{
    // If an obsession is active, only restore to a certain min level
    bool isObsessionActive = 0;
    for (int i = 0; i < int(Obsession::END); ++i)
    {
        if (obsessions[i])
        {
            isObsessionActive = true;
            break;
        }
    }
    const double MIN_SHOCK_WHEN_OBSESSION_DB = double(MIN_SHOCK_WHEN_OBSESSION);
    shock_ = max((isObsessionActive ? MIN_SHOCK_WHEN_OBSESSION_DB : 0.0),
                 shock_ - amountRestored);
    if (IS_TEMP_SHOCK_RESTORED) {shockTmp_ = 0;}
}

void Player::incrInsanity()
{
    TRACE << "Increasing insanity" << endl;

    if (!Config::isBotPlaying())
    {
        const int INS_INCR = 6;
        ins_ += INS_INCR;
    }

    restoreShock(70, false);

    updateClr();
    Render::drawMapAndInterface();

    if (getInsanity() >= 100)
    {
        const string msg = "My mind can no longer withstand what it has grasped. "
                           "I am hopelessly lost.";
        Popup::showMsg(msg, true, "Completely insane!", SfxId::insanityRise);
        die(true, false, false);
        return;
    }

    //This point reached means sanity is below 100%
    string msg = "Insanity draws nearer... ";

    //When long term insanity increases, something happens (mostly bad)
    //(Reroll until something actually happens)
    while (true)
    {
        const int RND_INS_EVENT = Rnd::range(1, 9);

        switch (RND_INS_EVENT)
        {
        case 1:
        {
            if (Rnd::coinToss())
            {
                msg += "I let out a terrified shriek.";
            }
            else
            {
                msg += "I scream in terror.";
            }
            Popup::showMsg(msg, true, "Screaming!", SfxId::insanityRise);

            Snd snd("", SfxId::END, IgnoreMsgIfOriginSeen::yes, pos, this,
                    SndVol::high, AlertsMon::yes);

            SndEmit::emitSnd(snd);
            return;
        } break;

        case 2:
        {
            msg += "I find myself babbling incoherently.";
            Popup::showMsg(msg, true, "Babbling!", SfxId::insanityRise);
            const string playerName = getNameThe();
            for (int i = Rnd::range(3, 5); i > 0; --i)
            {
                Log::addMsg(playerName + ": " + Cultist::getCultistPhrase());
            }
            Snd snd("", SfxId::END, IgnoreMsgIfOriginSeen::yes, pos, this,
                    SndVol::low, AlertsMon::yes);
            SndEmit::emitSnd(snd);
            return;
        } break;

        case 3:
        {
            msg += "I struggle to not fall into a stupor.";
            Popup::showMsg(msg, true, "Fainting!", SfxId::insanityRise);
            propHandler_->tryApplyProp(new PropFainted(PropTurns::std));
            return;
        } break;

        case 4:
        {
            msg += "I laugh nervously.";
            Popup::showMsg(msg, true, "HAHAHA!", SfxId::insanityRise);
            Snd snd("", SfxId::END, IgnoreMsgIfOriginSeen::yes, pos, this,
                    SndVol::low, AlertsMon::yes);
            SndEmit::emitSnd(snd);
            return;
        } break;

        case 5:
        {
            bool props[size_t(PropId::END)];
            propHandler_->getPropIds(props);

            if (ins_ >= 10 && !props[int(PropId::rFear)])
            {
                if (Rnd::coinToss())
                {
                    //Monster phobias

                    const int RND_MON = Rnd::range(1, 4);

                    switch (RND_MON)
                    {
                    case 1:
                        if (!phobias[int(Phobia::rat)])
                        {
                            msg +=
                                "I am afflicted by Murophobia. Rats suddenly seem "
                                "terrifying.";
                            Popup::showMsg(msg, true, "Murophobia!", SfxId::insanityRise);
                            phobias[int(Phobia::rat)] = true;
                            return;
                        }
                        break;

                    case 2:
                        if (!phobias[int(Phobia::spider)])
                        {
                            msg +=
                                "I am afflicted by Arachnophobia. Spiders suddenly seem "
                                "terrifying.";
                            Popup::showMsg(msg, true, "Arachnophobia!",
                                           SfxId::insanityRise);
                            phobias[int(Phobia::spider)] = true;
                            return;
                        }
                        break;

                    case 3:
                        if (!phobias[int(Phobia::dog)])
                        {
                            msg +=
                                "I am afflicted by Cynophobia. Canines suddenly seem "
                                "terrifying.";
                            Popup::showMsg(msg, true, "Cynophobia!", SfxId::insanityRise);
                            phobias[int(Phobia::dog)] = true;
                            return;
                        }
                        break;

                    case 4:
                        if (!phobias[int(Phobia::undead)])
                        {
                            msg +=
                                "I am afflicted by Necrophobia. The undead suddenly "
                                "seem far more terrifying.";
                            Popup::showMsg(msg, true, "Necrophobia!", SfxId::insanityRise);
                            phobias[int(Phobia::undead)] = true;
                            return;
                        }
                        break;
                    }
                }
                else //Cointoss (not a monster phobia)
                {
                    //Environment phobias

                    const int RND_ENV_PHOBIA = Rnd::range(1, 3);

                    switch (RND_ENV_PHOBIA)
                    {
                    case 1:
                        //Fear of cramped or open places
                        if (isStandingInOpenSpace() && !phobias[int(Phobia::openPlace)])
                        {
                            msg +=
                                "I am afflicted by Agoraphobia. Open places suddenly "
                                "seem terrifying.";
                            Popup::showMsg(msg, true, "Agoraphobia!", SfxId::insanityRise);
                            phobias[int(Phobia::openPlace)] = true;
                            return;
                        }
                        else if (isStandingInCrampedSpace())
                        {
                            if (!phobias[int(Phobia::crampedPlace)])
                            {
                                msg +=
                                    "I am afflicted by Claustrophobia. Confined places "
                                    "suddenly seem terrifying.";
                                Popup::showMsg(msg, true, "Claustrophobia!",
                                               SfxId::insanityRise);
                                phobias[int(Phobia::crampedPlace)] = true;
                                return;
                            }
                        }
                        break;

                    case 2:
                        //Fear of deep places
                        if (!phobias[int(Phobia::deepPlaces)])
                        {
                            msg +=
                                "I am afflicted by Bathophobia. It suddenly seems "
                                "terrifying to delve deeper.";
                            Popup::showMsg(msg, true, "Bathophobia!");
                            phobias[int(Phobia::deepPlaces)] = true;
                            return;
                        }
                        break;

                    case 3:
                        //Fear of the dark
                        if (!phobias[int(Phobia::dark)])
                        {
                            msg +=
                                "I am afflicted by Nyctophobia. Darkness suddenly seem "
                                "far more terrifying.";
                            Popup::showMsg(msg, true, "Nyctophobia!");
                            phobias[int(Phobia::dark)] = true;
                            return;
                        }
                        break;
                    }
                }
            }
        } break;

        case 6:
        {
            if (ins_ > 20)
            {
                int obsessionsActive = 0;
                for (int i = 0; i < int(Obsession::END); ++i)
                {
                    if (obsessions[i]) {obsessionsActive++;}
                }
                if (obsessionsActive == 0)
                {
                    const Obsession obsession =
                        (Obsession)(Rnd::range(0, int(Obsession::END) - 1));
                    switch (obsession)
                    {
                    case Obsession::masochism:
                    {
                        msg +=
                            "To my alarm, I find myself encouraged by the sensation of "
                            "pain. Physical suffering does not frighten me at all. "
                            "However, my depraved mind can never find complete peace "
                            "(no shock from taking damage, but shock cannot go below "
                            + toStr(MIN_SHOCK_WHEN_OBSESSION) + " % ).";
                        Popup::showMsg(msg, true, "Masochistic obsession!",
                                       SfxId::insanityRise);
                        obsessions[int(Obsession::masochism)] = true;
                        return;
                    } break;

                    case Obsession::sadism:
                    {
                        msg +=
                            "To my alarm, I find myself encouraged by the pain I cause "
                            "in others. For every life I take, I find a little relief. "
                            "However, my depraved mind can no longer find complete "
                            "peace (shock cannot go below "
                            + toStr(MIN_SHOCK_WHEN_OBSESSION) + " % ).";
                        Popup::showMsg(msg, true, "Sadistic obsession!",
                                       SfxId::insanityRise);
                        obsessions[int(Obsession::sadism)] = true;
                        return;
                    } break;
                    default: {} break;
                    }
                }
            }
        } break;

        case 7:
        {
            if (ins_ > 8)
            {
                msg += "The shadows are closing in on me!";
                Popup::showMsg(msg, true, "Haunted by shadows!", SfxId::insanityRise);
                const int NR_SHADOWS_LOWER  = 1;
                const int NR_SHADOWS_UPPER  =
                    getConstrInRange(2, (Map::dlvl + 1) / 2, 6);
                const int NR                =
                    Rnd::range(NR_SHADOWS_LOWER, NR_SHADOWS_UPPER);
                ActorFactory::summonMon(pos, vector<ActorId>(NR, ActorId::shadow), true);
                return;
            }
        } break;

        case 8:
        {
            msg +=
                "I find myself in a peculiar trance. I struggle to recall where I am, "
                " and what is happening.";
            Popup::showMsg(msg, true, "Confusion!", SfxId::insanityRise);
            propHandler_->tryApplyProp(new PropConfused(PropTurns::std));
            return;
        } break;

        case 9:
        {
            msg +=
                "There is a strange itch, as if something is crawling on the back of "
                "my neck.";
            Popup::showMsg(msg, true, "Strange sensation", SfxId::insanityRise);
            return;
        } break;
        }
    }
}

bool Player::isStandingInOpenSpace() const
{
    bool blocked[MAP_W][MAP_H];
    MapParse::run(CellCheck::BlocksMoveCmn(false), blocked);
    for (int x = pos.x - 1; x <= pos.x + 1; ++x)
    {
        for (int y = pos.y - 1; y <= pos.y + 1; ++y)
        {
            if (blocked[x][y]) {return false;}
        }
    }
    return true;
}

bool Player::isStandingInCrampedSpace() const
{
    bool blocked[MAP_W][MAP_H];
    MapParse::run(CellCheck::BlocksMoveCmn(false), blocked);
    int blockCount = 0;
    for (int x = pos.x - 1; x <= pos.x + 1; ++x)
    {
        for (int y = pos.y - 1; y <= pos.y + 1; ++y)
        {
            if (blocked[x][y])
            {
                blockCount++;
                if (blockCount >= 6) {return true;}
            }
        }
    }

    return false;
}

void Player::testPhobias()
{
    if (!Map::player->getPropHandler().allowAct())
    {
        return;
    }

    if (Rnd::oneIn(10))
    {
        //Monster phobia?
        vector<Actor*> seenFoes;
        getSeenFoes(seenFoes);

        for (Actor* const actor : seenFoes)
        {
            const ActorDataT& monData = actor->getData();
            if (monData.isCanine && phobias[int(Phobia::dog)])
            {
                Log::addMsg("I am plagued by my canine phobia!");
                propHandler_->tryApplyProp(new PropTerrified(PropTurns::std));
                return;
            }
            if (monData.isRat && phobias[int(Phobia::rat)])
            {
                Log::addMsg("I am plagued by my rat phobia!");
                propHandler_->tryApplyProp(new PropTerrified(PropTurns::std));
                return;
            }
            if (monData.isUndead && phobias[int(Phobia::undead)])
            {
                Log::addMsg("I am plagued by my phobia of the dead!");
                propHandler_->tryApplyProp(new PropTerrified(PropTurns::std));
                return;
            }
            if (monData.isSpider && phobias[int(Phobia::spider)])
            {
                Log::addMsg("I am plagued by my spider phobia!");
                propHandler_->tryApplyProp(new PropTerrified(PropTurns::std));
                return;
            }
        }

        //Environment phobia?
        if (phobias[int(Phobia::openPlace)] && isStandingInOpenSpace())
        {
            Log::addMsg("I am plagued by my phobia of open places!");
            propHandler_->tryApplyProp(new PropTerrified(PropTurns::std));
            return;
        }

        if (phobias[int(Phobia::crampedPlace)] && isStandingInCrampedSpace())
        {
            Log::addMsg("I am plagued by my phobia of closed places!");
            propHandler_->tryApplyProp(new PropTerrified(PropTurns::std));
            return;
        }

        for (const Pos& d : DirUtils::dirList)
        {
            const Pos p(pos + d);

            if (
                phobias[int(Phobia::deepPlaces)] &&
                Map::cells[p.x][p.y].rigid->isBottomless())
            {
                Log::addMsg("I am plagued by my phobia of deep places!");
                propHandler_->tryApplyProp(new PropTerrified(PropTurns::std));
                break;
            }

            if (
                phobias[int(Phobia::dark)]  &&
                Map::cells[p.x][p.y].isDark &&
                !Map::cells[p.x][p.y].isLit)
            {
                Log::addMsg("I am plagued by my fear of the dark!");
                propHandler_->tryApplyProp(new PropTerrified(PropTurns::std));
                break;
            }
        }
    }
}

void Player::updateClr()
{
    if (!isAlive())
    {
        clr_ = clrRed;
        return;
    }

    if (propHandler_->changeActorClr(clr_))
    {
        return;
    }

    if (activeExplosive)
    {
        clr_ = clrYellow;
        return;
    }

    const int CUR_SHOCK = shock_ + shockTmp_;
    if (CUR_SHOCK >= 75)
    {
        clr_ = clrMagenta;
        return;
    }

    clr_ = data_->color;
}

void Player::setQuickMove(const Dir dir)
{
    nrQuickMoveStepsLeft_ = 10;
    quickMoveDir_         = dir;
}

void Player::onActorTurn()
{
    Render::drawMapAndInterface();

    resetPermShockTakenCurTurn();

    if (!isAlive())
    {
        return;
    }

    if (tgt_ && tgt_->getState() != ActorState::alive)
    {
        tgt_ = nullptr;
    }

    //If player dropped item, check if should go back to inventory screen
    vector<Actor*> seenFoes;
    getSeenFoes(seenFoes);

    if (seenFoes.empty())
    {
        const auto invScreen = InvHandling::screenToOpenAfterDrop;
        if (invScreen != InvScrId::END)
        {
            switch (invScreen)
            {
            case InvScrId::inv:
                InvHandling::runInvScreen();
                break;

            case InvScrId::equip:
                InvHandling::runEquipScreen(*InvHandling::equipSlotToOpenAfterDrop);
                break;

            case InvScrId::END: {} break;
            }
            return;
        }
    }
    else //There are seen monsters
    {
        InvHandling::screenToOpenAfterDrop    = InvScrId::END;
        InvHandling::browserIdxToSetAfterDrop = 0;
    }

    //Quick move
    if (nrQuickMoveStepsLeft_ > 0)
    {
        //Note: There is no need to check for items here, since the message from stepping
        //on an item will interrupt player actions.

        const Pos destPos(pos + DirUtils::getOffset(quickMoveDir_));

        const Cell&         tgtCell   = Map::cells[destPos.x][destPos.y];
        const Rigid* const  tgtRigid  = tgtCell.rigid;

        const bool IS_TGT_KNOWN_TRAP  = tgtRigid->getId() == FeatureId::trap &&
                                        !static_cast<const Trap*>(tgtRigid)->isHidden();

        const bool SHOULD_ABORT = !tgtRigid->canMoveCmn()                         ||
                                  IS_TGT_KNOWN_TRAP                               ||
                                  tgtRigid->getBurnState() == BurnState::burning  ||
                                  (tgtCell.isDark && !tgtCell.isLit);

        if (SHOULD_ABORT)
        {
            nrQuickMoveStepsLeft_ = -1;
            quickMoveDir_         = Dir::END;
        }
        else
        {
            --nrQuickMoveStepsLeft_;
            moveDir(quickMoveDir_);
            return;
        }
    }

    //If this point is reached - read input from player
    if (Config::isBotPlaying())
    {
        Bot::act();
    }
    else
    {
        Input::clearEvents();
        Input::mapModeInput();
    }
}

void Player::onStdTurn()
{
    shockTmp_ = 0.0;

    //Temporary shock from features
    Cell& cell = Map::cells[pos.x][pos.y];

    if (cell.isDark && !cell.isLit)
    {
        shockTmp_ += 20;
    }

    for (const Pos& d : DirUtils::dirList)
    {
        const Pos p(pos + d);
        shockTmp_ += Map::cells[p.x][p.y].rigid->getShockWhenAdj();
    }

    shockTmp_ = min(99.0, shockTmp_);

    //Temporary shock from items
    for (auto& slot : inv_->slots_)
    {
        if (slot.item)
        {
            shockTmp_ += slot.item->getData().shockWhileEquipped;
        }
    }

    for (const Item* const item : inv_->general_)
    {
        shockTmp_ += item->getData().shockWhileInBackpack;
    }

    if (activeExplosive)   {activeExplosive->onStdTurnPlayerHoldIgnited();}

    if (!activeMedicalBag) {testPhobias();}

    //If obsessions are active, raise shock to a minimum level
    for (int i = 0; i < int(Obsession::END); ++i)
    {
        if (obsessions[i])
        {
            shock_ = max(double(MIN_SHOCK_WHEN_OBSESSION), shock_);
            break;
        }
    }

    vector<Actor*> seenFoes;
    getSeenFoes(seenFoes);

    double shockFromMonCurPlayerTurn = 0.0;

    for (Actor* actor : seenFoes)
    {
        DungeonMaster::onMonSpotted(*actor);

        Mon* mon = static_cast<Mon*>(actor);

        mon->playerBecomeAwareOfMe();

        const ActorDataT& data = mon->getData();
        if (data.monShockLvl != MonShockLvl::none)
        {
            switch (data.monShockLvl)
            {
            case MonShockLvl::unsettling:
                mon->shockCausedCur_ = min(mon->shockCausedCur_ + 0.05,  1.0);
                break;

            case MonShockLvl::scary:
                mon->shockCausedCur_ = min(mon->shockCausedCur_ + 0.1,   1.0);
                break;

            case MonShockLvl::terrifying:
                mon->shockCausedCur_ = min(mon->shockCausedCur_ + 0.5,   2.0);
                break;

            case MonShockLvl::mindShattering:
                mon->shockCausedCur_ = min(mon->shockCausedCur_ + 0.75,  3.0);
                break;

            default: {} break;
            }
            if (shockFromMonCurPlayerTurn < 2.5)
            {
                incrShock(int(floor(mon->shockCausedCur_)), ShockSrc::seeMon);
                shockFromMonCurPlayerTurn += mon->shockCausedCur_;
            }
        }
    }

    //Some shock is taken every Xth turn
    if (propHandler_->allowAct())
    {
        int turnNrIncrShock = 12;

        if (PlayerBon::getBg() == Bg::rogue)
        {
            turnNrIncrShock *= 2;
        }

        const int TURN = GameTime::getTurn();

        if (TURN % turnNrIncrShock == 0 && TURN > 1)
        {
            if (Rnd::oneIn(850))
            {
                if (Rnd::coinToss())
                {
                    Popup::showMsg("I have a bad feeling about this...", true);
                }
                else
                {
                    Popup::showMsg("A chill runs down my spine...", true);
                }
                incrShock(ShockLvl::heavy, ShockSrc::misc);
                Render::drawMapAndInterface();
            }
            else
            {
                if (Map::dlvl != 0)
                {
                    incrShock(1, ShockSrc::time);
                }
            }
        }
    }

    //Take sanity hit from high shock?
    if (getShockTotal() >= 100)
    {
        nrTurnsUntilIns_ = nrTurnsUntilIns_ < 0 ? 3 : nrTurnsUntilIns_ - 1;
        if (nrTurnsUntilIns_ > 0)
        {
            Render::drawMapAndInterface(true);
            Log::addMsg("I feel my sanity slipping...", clrMsgNote, true, true);
        }
        else
        {
            nrTurnsUntilIns_ = -1;
            incrInsanity();
            if (isAlive())
            {
                GameTime::tick();
            }
            return;
        }
    }
    else //Total shock is less than 100%
    {
        nrTurnsUntilIns_ = -1;
    }

    //Check for monsters coming into view, and try to spot hidden monsters.
    for (Actor* actor : GameTime::actors_)
    {
        if (!actor->isPlayer() && !Map::player->isLeaderOf(actor) && actor->isAlive())
        {
            Mon& mon                = *static_cast<Mon*>(actor);
            const bool IS_MON_SEEN  = isSeeingActor(*actor, nullptr);
            if (IS_MON_SEEN)
            {
                if (!mon.isMsgMonInViewPrinted_)
                {
                    if (
                        activeMedicalBag    ||
                        waitTurnsLeft > 0   ||
                        nrQuickMoveStepsLeft_ > 0)
                    {
                        Log::addMsg(actor->getNameA() + " comes into my view.", clrWhite,
                                    true);
                    }
                    mon.isMsgMonInViewPrinted_ = true;
                }
            }
            else //Monster is not seen
            {
                mon.isMsgMonInViewPrinted_ = false;

                //Is the monster sneaking? Try to spot it
                if (
                    Map::cells[mon.pos.x][mon.pos.y].isSeenByPlayer &&
                    mon.isStealth_                              &&
                    isSpottingHiddenActor(mon))
                {
                    mon.isStealth_ = false;
                    updateFov();
                    Render::drawMapAndInterface();
                    const string monName = mon.getNameA();
                    Log::addMsg("I spot " + monName + "!", clrMsgNote, true, true);
                }
            }
        }
    }

    const int DECR_ABOVE_MAX_N_TURNS = 7;
    if (getHp() > getHpMax(true))
    {
        if (GameTime::getTurn() % DECR_ABOVE_MAX_N_TURNS == 0) {hp_--;}
    }
    if (getSpi() > getSpiMax())
    {
        if (GameTime::getTurn() % DECR_ABOVE_MAX_N_TURNS == 0) {spi_--;}
    }

    if (!activeMedicalBag)
    {
        bool props[size_t(PropId::END)];
        propHandler_->getPropIds(props);

        if (!props[int(PropId::poisoned)])
        {
            const bool IS_RAPID_REC   =
                PlayerBon::traitsPicked[int(Trait::rapidRecoverer)];

            const bool IS_SURVIVALIST =
                PlayerBon::traitsPicked[int(Trait::survivalist)];

            const int REGEN_N_TURNS   = IS_SURVIVALIST  ? 20 :
                                        IS_RAPID_REC    ? 30 : 40;

            const int TURN = GameTime::getTurn();

            if (
                getHp() < getHpMax(true)    &&
                (TURN % REGEN_N_TURNS == 0) &&
                TURN > 1)
            {
                ++hp_;
            }
        }

        if (!props[int(PropId::confused)] && propHandler_->allowSee())
        {
            const int R = PlayerBon::traitsPicked[int(Trait::perceptive)] ? 3 :
                          (PlayerBon::traitsPicked[int(Trait::observant)] ? 2 : 1);

            int x0 = max(0, pos.x - R);
            int y0 = max(0, pos.y - R);
            int x1 = min(MAP_W - 1, pos.x + R);
            int y1 = min(MAP_H - 1, pos.y + R);

            for (int y = y0; y <= y1; ++y)
            {
                for (int x = x0; x <= x1; ++x)
                {
                    if (Map::cells[x][y].isSeenByPlayer)
                    {
                        auto* f = Map::cells[x][y].rigid;

                        if (f->getId() == FeatureId::trap)
                        {
                            static_cast<Trap*>(f)->playerTrySpotHidden();
                        }
                        if (f->getId() == FeatureId::door)
                        {
                            static_cast<Door*>(f)->playerTrySpotHidden();
                        }
                    }
                }
            }
        }
    }

    if (activeMedicalBag)
    {
        activeMedicalBag->continueAction();
    }

    if (waitTurnsLeft > 0)
    {
        waitTurnsLeft--;
        GameTime::tick();
    }
}

void Player::onLogMsgPrinted()
{
    //Note: There cannot be any calls to Log::addMsg() in this function, as that would
    //cause endless recursion.

    //Abort waiting
    waitTurnsLeft         = -1;

    //Abort quick move
    nrQuickMoveStepsLeft_ = -1;
    quickMoveDir_         = Dir::END;
}

void Player::interruptActions()
{
    Render::drawMapAndInterface();

    //Abort browsing inventory
    InvHandling::screenToOpenAfterDrop    = InvScrId::END;
    InvHandling::browserIdxToSetAfterDrop = 0;

    //Abort waiting
    if (waitTurnsLeft > 0)
    {
        Log::addMsg("I stop waiting.", clrWhite);
        Render::drawMapAndInterface();
    }
    waitTurnsLeft = -1;

    //Abort healing
    if (activeMedicalBag)
    {
        activeMedicalBag->interrupted();
        activeMedicalBag = nullptr;
    }

    //Abort quick move
    nrQuickMoveStepsLeft_ = -1;
    quickMoveDir_         = Dir::END;
}

void Player::hearSound(const Snd& snd, const bool IS_ORIGIN_SEEN_BY_PLAYER,
                       const Dir dirToOrigin,
                       const int PERCENT_AUDIBLE_DISTANCE)
{
    const SfxId sfx = snd.getSfx();

    const string& msg = snd.getMsg();
    const bool HAS_SND_MSG = !msg.empty() && msg != " ";

    if (HAS_SND_MSG) {Log::addMsg(msg, clrWhite);}

    //Play audio after message to ensure sync between audio and animation
    //If origin is hidden, we only play the sound if there is a message
    if (HAS_SND_MSG || IS_ORIGIN_SEEN_BY_PLAYER)
    {
        Audio::play(sfx, dirToOrigin, PERCENT_AUDIBLE_DISTANCE);
    }

    if (HAS_SND_MSG)
    {
        Actor* const actorWhoMadeSnd = snd.getActorWhoMadeSound();
        if (actorWhoMadeSnd && actorWhoMadeSnd != this)
        {
            static_cast<Mon*>(actorWhoMadeSnd)->playerBecomeAwareOfMe();
        }
    }
}

void Player::moveDir(Dir dir)
{
    if (isAlive())
    {

        propHandler_->changeMoveDir(pos, dir);

        //Trap affects leaving?
        if (dir != Dir::center)
        {
            Feature* f = Map::cells[pos.x][pos.y].rigid;
            if (f->getId() == FeatureId::trap)
            {
                TRACE << "Standing on trap, check if affects move" << endl;
                dir = static_cast<Trap*>(f)->actorTryLeave(*this, dir);
            }
        }

        bool isFreeTurn = false;;

        const Pos dest(pos + DirUtils::getOffset(dir));

        if (dir != Dir::center)
        {
            Mon* const monAtDest = static_cast<Mon*>(Utils::getActorAtPos(dest));

            //Attack?
            if (monAtDest && !isLeaderOf(monAtDest))
            {
                if (propHandler_->allowAttackMelee(true))
                {
                    bool hasMeleeWpn = false;
                    Item* const item = inv_->getItemInSlot(SlotId::wielded);
                    if (item)
                    {
                        Wpn* const wpn = static_cast<Wpn*>(item);

                        if (wpn->getData().melee.isMeleeWpn)
                        {
                            if (
                                Config::isRangedWpnMeleeePrompt()   &&
                                isSeeingActor(*monAtDest, nullptr)  &&
                                wpn->getData().ranged.isRangedWpn)
                            {
                                const string wpnName = wpn->getName(ItemRefType::a);

                                Log::addMsg("Attack " + monAtDest->getNameThe() +
                                            " with " + wpnName + " ? [y / n]",
                                            clrWhiteHigh);

                                Render::drawMapAndInterface();

                                if (Query::yesOrNo() == YesNoAnswer::no)
                                {
                                    Log::clearLog();
                                    Render::drawMapAndInterface();
                                    return;
                                }
                            }
                            Attack::melee(*this, *wpn, *monAtDest);
                            tgt_ = monAtDest;
                            return;
                        }
                    }
                    if (!hasMeleeWpn)
                    {
                        punchMon(*monAtDest);
                    }
                }
                return;
            }

            //This point reached means no actor in the destination cell.

            //Blocking mobile or rigid?
            bool props[size_t(PropId::END)];
            getPropHandler().getPropIds(props);
            Cell& cell = Map::cells[dest.x][dest.y];
            bool isFeaturesAllowMove = cell.rigid->canMove(props);

            vector<Mob*> mobs;
            GameTime::getMobsAtPos(dest, mobs);

            if (isFeaturesAllowMove)
            {
                for (auto* m : mobs)
                {
                    if (!m->canMove(props))
                    {
                        isFeaturesAllowMove = false;
                        break;
                    }
                }
            }

            if (isFeaturesAllowMove)
            {
                //Encumbrance
                const int ENC = getEncPercent();
                if (ENC >= ENC_IMMOBILE_LVL)
                {
                    Log::addMsg("I am too encumbered to move!");
                    Render::drawMapAndInterface();
                    return;
                }
                else if (ENC >= 100)
                {
                    Log::addMsg("I stagger.", clrMsgNote);
                    propHandler_->tryApplyProp(new PropWaiting(PropTurns::std));
                }

                //Displace allied monster
                if (monAtDest && isLeaderOf(monAtDest))
                {
                    Log::addMsg("I displace " + monAtDest->getNameA() + ".");
                    monAtDest->pos = pos;
                }

                pos = dest;

                const int FREE_MOVE_EVERY_N_TURN =
                    PlayerBon::traitsPicked[int(Trait::mobile)]     ? 2 :
                    PlayerBon::traitsPicked[int(Trait::lithe)]      ? 4 :
                    PlayerBon::traitsPicked[int(Trait::dexterous)]  ? 5 : 0;

                if (FREE_MOVE_EVERY_N_TURN > 0)
                {
                    if (nrMovesUntilFreeAction_ == -1)
                    {
                        nrMovesUntilFreeAction_ = FREE_MOVE_EVERY_N_TURN - 2;
                    }
                    else if (nrMovesUntilFreeAction_ == 0)
                    {
                        nrMovesUntilFreeAction_ = FREE_MOVE_EVERY_N_TURN - 1;
                        isFreeTurn = true;
                    }
                    else
                    {
                        nrMovesUntilFreeAction_--;
                    }
                }

                //Print message if walking on item
                Item* const item = Map::cells[pos.x][pos.y].item;
                if (item)
                {
                    const bool CAN_SEE = propHandler_->allowSee();
                    Log::addMsg(CAN_SEE ? "I see here: " :
                                "I try to feel what is lying here...",
                                clrWhite, true);

                    const string itemName =
                        item->getName(ItemRefType::plural, ItemRefInf::yes,
                                      ItemRefAttInf::wpnContext);

                    Log::addMsg(TextFormatting::firstToUpper(itemName) + ".");
                }
            }

            //Note: bump() prints block messages.
            for (auto* m : mobs) {m->bump(*this);}

            Map::cells[dest.x][dest.y].rigid->bump(*this);
        }

        if (pos == dest)
        {
            GameTime::tick(isFreeTurn);
            return;
        }
    }
}

void Player::autoMelee()
{
    if (
        tgt_                                    &&
        tgt_->getState() == ActorState::alive   &&
        Utils::isPosAdj(pos, tgt_->pos, false)  &&
        isSeeingActor(*tgt_, nullptr))
    {
        moveDir(DirUtils::getDir(tgt_->pos - pos));
        return;
    }

    //If this line reached, there is no adjacent cur target.
    for (const Pos& d : DirUtils::dirList)
    {
        Actor* const actor = Utils::getActorAtPos(pos + d);
        if (actor && !isLeaderOf(actor) && isSeeingActor(*actor, nullptr))
        {
            tgt_ = actor;
            moveDir(DirUtils::getDir(d));
            return;
        }
    }
}

void Player::kickMon(Actor& actorToKick)
{
    Wpn* kickWpn = nullptr;

    const ActorDataT& d = actorToKick.getData();

    if (d.actorSize == ActorSize::floor && (d.isSpider || d.isRat))
    {
        kickWpn = static_cast<Wpn*>(ItemFactory::mk(ItemId::playerStomp));
    }
    else
    {
        kickWpn = static_cast<Wpn*>(ItemFactory::mk(ItemId::playerKick));
    }
    Attack::melee(*this, *kickWpn, actorToKick);
    delete kickWpn;
}

void Player::punchMon(Actor& actorToPunch)
{
    //Spawn a temporary punch weapon to attack with
    Wpn* punchWpn = static_cast<Wpn*>(ItemFactory::mk(ItemId::playerPunch));
    Attack::melee(*this, *punchWpn, actorToPunch);
    delete punchWpn;
}

void Player::addLight_(bool lightMap[MAP_W][MAP_H]) const
{
    LgtSize lgtSize = LgtSize::none;

    if (activeExplosive)
    {
        if (activeExplosive->getData().id == ItemId::flare)
        {
            lgtSize = LgtSize::fov;
        }
    }

    if (lgtSize != LgtSize::fov)
    {
        for (Item* const item : inv_->general_)
        {
            LgtSize itemLgtSize = item->getLgtSize();

            if (int(lgtSize) < int(itemLgtSize))
            {
                lgtSize = itemLgtSize;
            }
        }
    }

    switch (lgtSize)
    {
    case LgtSize::fov:
    {
        bool myLight[MAP_W][MAP_H];
        Utils::resetArray(myLight, false);

        const int R = FOV_STD_RADI_INT;

        Pos p0(max(0,         pos.x - R), max(0,         pos.y - R));
        Pos p1(min(MAP_W - 1, pos.x + R), min(MAP_H - 1, pos.y + R));

        bool blockedLos[MAP_W][MAP_H];

        for (int y = p0.y; y <= p1.y; ++y)
        {
            for (int x = p0.x; x <= p1.x; ++x)
            {
                const auto* const f = Map::cells[x][y].rigid;
                blockedLos[x][y]    = !f->isLosPassable();
            }
        }

        Fov::runFovOnArray(blockedLos, pos, myLight, false);
        for (int y = p0.y; y <= p1.y; ++y)
        {
            for (int x = p0.x; x <= p1.x; ++x)
            {
                if (myLight[x][y])
                {
                    lightMap[x][y] = true;
                }
            }
        }

    }
    break;

    case LgtSize::small:
        for (int y = pos.y - 1; y <= pos.y + 1; ++y)
        {
            for (int x = pos.x - 1; x <= pos.x + 1; ++x)
            {
                lightMap[x][y] = true;
            }
        }
        break;

    case LgtSize::none: {}
        break;
    }
}

void Player::updateFov()
{
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            Map::cells[x][y].isSeenByPlayer = false;
        }
    }

    if (propHandler_->allowSee())
    {
        bool blocked[MAP_W][MAP_H];
        MapParse::run(CellCheck::BlocksLos(), blocked);
        Fov::runPlayerFov(blocked, pos);
        Map::cells[pos.x][pos.y].isSeenByPlayer = true;
    }

    if (propHandler_->allowSee()) {FOVhack();}

    if (Init::isCheatVisionEnabled)
    {
        for (int x = 0; x < MAP_W; ++x)
        {
            for (int y = 0; y < MAP_H; ++y)
            {
                Map::cells[x][y].isSeenByPlayer = true;
            }
        }
    }

    //Explore
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            Cell& cell = Map::cells[x][y];
            const bool IS_BLOCKING = CellCheck::BlocksMoveCmn(false).check(cell);
            //Do not explore dark floor cells
            if (cell.isSeenByPlayer && (!cell.isDark || IS_BLOCKING))
            {
                cell.isExplored = true;
            }
        }
    }
}

void Player::FOVhack()
{
    bool blockedLos[MAP_W][MAP_H];
    MapParse::run(CellCheck::BlocksLos(), blockedLos);

    bool blocked[MAP_W][MAP_H];
    MapParse::run(CellCheck::BlocksMoveCmn(false), blocked);

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            if (blockedLos[x][y] && blocked[x][y])
            {
                const Pos p(x, y);

                for (const Pos& d : DirUtils::dirList)
                {
                    const Pos pAdj(p + d);

                    if (Utils::isPosInsideMap(pAdj))
                    {
                        const Cell& adjCell = Map::cells[pAdj.x][pAdj.y];
                        if (
                            adjCell.isSeenByPlayer              &&
                            (!adjCell.isDark || adjCell.isLit)  &&
                            !blocked[pAdj.x][pAdj.y])
                        {
                            Map::cells[x][y].isSeenByPlayer = true;
                            break;
                        }
                    }
                }
            }
        }
    }
}

bool Player::isLeaderOf(const Actor* const actor) const
{
    if (!actor || actor == this)
    {
        return false;
    }

    //Actor is monster
    return static_cast<const Mon*>(actor)->leader_ == this;
}

bool Player::isActorMyLeader(const Actor* const actor) const
{
    //Should never happen
    (void)actor;
    return false;
}
