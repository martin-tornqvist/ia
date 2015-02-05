#include "Actor.h"

#include "Init.h"

#include "Render.h"
#include "GameTime.h"
#include "ActorPlayer.h"
#include "ActorMon.h"
#include "Map.h"
#include "Fov.h"
#include "Log.h"
#include "FeatureTrap.h"
#include "Drop.h"
#include "Explosion.h"
#include "DungeonMaster.h"
#include "Inventory.h"
#include "MapParsing.h"
#include "Item.h"
#include "Utils.h"
#include "Input.h"
#include "Marker.h"
#include "Look.h"

using namespace std;

Actor::Actor() :
    pos           (),
    state_        (ActorState::alive),
    clr_          (clrBlack),
    glyph_        (' '),
    tile_         (TileId::empty),
    hp_           (-1),
    hpMax_        (-1),
    spi_          (-1),
    spiMax_       (-1),
    lairCell_     (),
    propHandler_  (nullptr),
    data_         (nullptr),
    inv_          (nullptr) {}

Actor::~Actor()
{
    delete propHandler_;
    delete inv_;
}

bool Actor::isSpottingHiddenActor(Actor& other)
{
    const Pos& otherPos = other.pos;

    const int PLAYER_SEARCH_MOD =
        isPlayer() ?
        (data_->abilityVals.getVal(AbilityId::searching, true, *this) / 3) : 0;

    const auto& abilitiesOther  = other.getData().abilityVals;

    const int   SNEAK_SKILL     = abilitiesOther.getVal(AbilityId::stealth, true, other);

    const int   DIST            = Utils::kingDist(pos, otherPos);
    const int   SNEAK_DIST_MOD  = getConstrInRange(0, (DIST - 1) * 10, 60);
    const Cell& cell            = Map::cells[otherPos.x][otherPos.y];
    const int   SNEAK_LGT_MOD   = cell.isLit                    ? -40 : 0;
    const int   SNEAK_DRK_MOD   = (cell.isDark && ! cell.isLit) ?  40 : 0;
    const int   SNEAK_TOT       = getConstrInRange(
                                      0,
                                      SNEAK_SKILL     +
                                      SNEAK_DIST_MOD  +
                                      SNEAK_LGT_MOD   +
                                      SNEAK_DRK_MOD   -
                                      PLAYER_SEARCH_MOD,
                                      99);

    return AbilityRoll::roll(SNEAK_TOT) <= failSmall;
}

int Actor::getHpMax(const bool WITH_MODIFIERS) const
{
    return WITH_MODIFIERS ? propHandler_->getChangedMaxHp(hpMax_) : hpMax_;
}

ActorSpeed Actor::getSpeed() const
{
    const auto baseSpeed = data_->speed;

    bool props[size_t(PropId::END)];

    propHandler_->getPropIds(props);

    int speedInt = int(baseSpeed);

    //"Slowed" gives speed penalty
    if (props[int(PropId::slowed)] && speedInt > 0)
    {
        --speedInt;
    }

    //"Hasted" or "frenzied" gives speed bonus.
    if (
        (props[int(PropId::hasted)] || props[int(PropId::frenzied)]) &&
        speedInt < int(ActorSpeed::END) - 1)
    {
        ++speedInt;
    }

    assert(speedInt >= 0 && speedInt < int(ActorSpeed::END));

    return ActorSpeed(speedInt);
}

bool Actor::canSeeActor(const Actor& other, const bool blockedLos[MAP_W][MAP_H]) const
{
    if (this == &other)
    {
        return true;
    }

    if (!other.isAlive())
    {
        return false;
    }

    if (isPlayer())
    {
        return Map::cells[other.pos.x][other.pos.y].isSeenByPlayer &&
               !static_cast<const Mon*>(&other)->isStealth_;
    }

    //This point reached means its a monster checking

    if (
        pos.x - other.pos.x > FOV_STD_RADI_INT ||
        other.pos.x - pos.x > FOV_STD_RADI_INT ||
        other.pos.y - pos.y > FOV_STD_RADI_INT ||
        pos.y - other.pos.y > FOV_STD_RADI_INT)
    {
        return false;
    }

    //Monster allied to player looking at other monster?
    if (
        isActorMyLeader(Map::player)  &&
        !other.isPlayer()             &&
        static_cast<const Mon*>(&other)->isStealth_)
    {
        return false;
    }

    if (!propHandler_->allowSee())
    {
        return false;
    }

    if (blockedLos)
    {
        return Fov::checkCell(blockedLos, other.pos, pos, !data_->canSeeInDarkness);
    }

    return false;
}

void Actor::getSeenFoes(vector<Actor*>& out)
{
    out.clear();

    bool blockedLos[MAP_W][MAP_H];

    if (!isPlayer())
    {
        Rect losRect(max(0,         pos.x - FOV_STD_RADI_INT),
                     max(0,         pos.y - FOV_STD_RADI_INT),
                     min(MAP_W - 1, pos.x + FOV_STD_RADI_INT),
                     min(MAP_H - 1, pos.y + FOV_STD_RADI_INT));

        MapParse::run(CellCheck::BlocksLos(), blockedLos, MapParseMode::overwrite,
                      losRect);
    }

    for (Actor* actor : GameTime::actors_)
    {
        if (actor != this && actor->isAlive())
        {
            if (isPlayer())
            {
                if (canSeeActor(*actor, nullptr) && !isLeaderOf(actor))
                {
                    out.push_back(actor);
                }
            }
            else //Not player
            {
                const bool IS_HOSTILE_TO_PLAYER = !isActorMyLeader(Map::player);
                const bool IS_OTHER_HOSTILE_TO_PLAYER =
                    actor->isPlayer() ? false : !actor->isActorMyLeader(Map::player);

                //"IS_OTHER_HOSTILE_TO_PLAYER" is false if other IS the player, there is
                //no need to check if "IS_HOSTILE_TO_PLAYER && IS_OTHER_PLAYER"
                if (
                    (IS_HOSTILE_TO_PLAYER  && !IS_OTHER_HOSTILE_TO_PLAYER) ||
                    (!IS_HOSTILE_TO_PLAYER &&  IS_OTHER_HOSTILE_TO_PLAYER))
                {
                    if (canSeeActor(*actor, blockedLos))
                    {
                        out.push_back(actor);
                    }
                }
            }
        }
    }
}

void Actor::place(const Pos& pos_, ActorDataT& data)
{
    pos             = pos_;
    data_           = &data;
    inv_            = new Inventory();
    propHandler_    = new PropHandler(this);
    state_          = ActorState::alive;
    clr_            = data_->color;
    glyph_          = data_->glyph;
    tile_           = data_->tile;
    hp_             = hpMax_  = data_->hp;
    spi_            = spiMax_ = data_->spi;
    lairCell_       = pos;

    if (data_->id != ActorId::player) {mkStartItems();}

    place_();

    updateClr();
}

void Actor::teleport()
{
    bool blocked[MAP_W][MAP_H];
    MapParse::run(CellCheck::BlocksActor(*this, true), blocked);

    vector<Pos> posBucket;
    Utils::mkVectorFromBoolMap(false, blocked, posBucket);

    if (posBucket.empty())
    {
        return;
    }

    if (!isPlayer() && Map::player->canSeeActor(*this, nullptr))
    {
        Log::addMsg(getNameThe() + " suddenly disappears!");
    }

    Pos   tgtPos                = posBucket[Rnd::range(0, posBucket.size() - 1)];
    bool  playerHasTeleControl  = false;

    if (isPlayer())
    {
        Map::player->updateFov();
        Render::drawMapAndInterface();
        Map::updateVisualMemory();

        //Teleport control?
        bool props[size_t(PropId::END)];

        propHandler_->getPropIds(props);

        if (props[int(PropId::teleCtrl)] && !props[int(PropId::confused)])
        {
            playerHasTeleControl = true;

            auto getChanceOfTeleSuccess = [](const Pos & tgt)
            {
                const int DIST = Utils::kingDist(Map::player->pos, tgt);
                return getConstrInRange(25, 100 - DIST, 95);
            };

            auto onMarkerAtPos = [getChanceOfTeleSuccess](const Pos & p)
            {
                Log::clearLog();
                Look::printLocationInfoMsgs(p);

                const int CHANCE_PCT = getChanceOfTeleSuccess(p);

                Log::addMsg(toStr(CHANCE_PCT) + "% chance of success.");

                Log::addMsg("[enter] to teleport here");
                Log::addMsg(cancelInfoStrNoSpace);
            };

            auto onKeyPress = [](const Pos & p, const KeyData & keyData)
            {
                (void)p;

                if (keyData.sdlKey == SDLK_RETURN)
                {
                    Log::clearLog();
                    return MarkerDone::yes;
                }
                return MarkerDone::no;
            };

            Log::addMsg("I have the power to control teleportation.", clrWhite, false,
                        true);

            const Pos markerTgtPos =
                Marker::run(MarkerDrawTail::yes, MarkerUsePlayerTgt::no, onMarkerAtPos,
                            onKeyPress);

            if (blocked[markerTgtPos.x][markerTgtPos.y])
            {
                //Blocked
                Log::addMsg("Something is blocking me...", clrWhite, false, true);
            }
            else if (Rnd::percent(getChanceOfTeleSuccess(markerTgtPos)))
            {
                //Success
                tgtPos = markerTgtPos;
            }
            else //Distance roll failed
            {
                Log::addMsg("I failed to go there...", clrWhite, false, true);
            }
        }
    }
    else
    {
        static_cast<Mon*>(this)->playerAwareOfMeCounter_ = 0;
    }

    pos = tgtPos;

    if (isPlayer())
    {
        Map::player->updateFov();
        Render::drawMapAndInterface();
        Map::updateVisualMemory();
        if (!playerHasTeleControl)
        {
            Log::addMsg("I suddenly find myself in a different location!");
            propHandler_->tryApplyProp(new PropConfused(PropTurns::specific, 8));
        }
    }
}

void Actor::updateClr()
{
    if (state_ != ActorState::alive)
    {
        clr_ = data_->color;
        return;
    }

    if (propHandler_->changeActorClr(clr_))
    {
        return;
    }

    if (isPlayer() && Map::player->activeExplosive)
    {
        clr_ = clrYellow;
        return;
    }

    clr_ = data_->color;
}

bool Actor::restoreHp(const int HP_RESTORED, const bool ALLOW_MSG,
                      const bool IS_ALLOWED_ABOVE_MAX)
{
    bool      isHpGained    = IS_ALLOWED_ABOVE_MAX;
    const int DIF_FROM_MAX  = getHpMax(true) - HP_RESTORED;

    //If hp is below limit, but restored hp will push it over the limit, HP is set to max.
    if (!IS_ALLOWED_ABOVE_MAX && getHp() > DIF_FROM_MAX && getHp() < getHpMax(true))
    {
        hp_         = getHpMax(true);
        isHpGained  = true;
    }

    //If HP is below limit, and restored hp will NOT push it over the limit -
    //restored hp is added to current.
    if (IS_ALLOWED_ABOVE_MAX || getHp() <= DIF_FROM_MAX)
    {
        hp_ += HP_RESTORED;
        isHpGained = true;
    }

    updateClr();

    if (ALLOW_MSG && isHpGained)
    {
        if (isPlayer())
        {
            Log::addMsg("I feel healthier!", clrMsgGood);
        }
        else //Is a monster
        {
            if (Map::player->canSeeActor(*this, nullptr))
            {
                Log::addMsg(data_->nameThe + " looks healthier.");
            }
        }
        Render::drawMapAndInterface();
    }

    return isHpGained;
}

bool Actor::restoreSpi(const int SPI_RESTORED, const bool ALLOW_MSG,
                       const bool IS_ALLOWED_ABOVE_MAX)
{
    bool isSpiGained = IS_ALLOWED_ABOVE_MAX;

    const int DIF_FROM_MAX = getSpiMax() - SPI_RESTORED;

    //If spi is below limit, but will be pushed over the limit, spi is set to max.
    if (!IS_ALLOWED_ABOVE_MAX && getSpi() > DIF_FROM_MAX && getSpi() < getSpiMax())
    {
        spi_        = getSpiMax();
        isSpiGained = true;
    }

    //If spi is below limit, and will not NOT be pushed over the limit - restored spi is
    //added to current.
    if (IS_ALLOWED_ABOVE_MAX || getSpi() <= DIF_FROM_MAX)
    {
        spi_ += SPI_RESTORED;
        isSpiGained = true;
    }

    if (ALLOW_MSG && isSpiGained)
    {
        if (isPlayer())
        {
            Log::addMsg("I feel more spirited!", clrMsgGood);
        }
        else
        {
            if (Map::player->canSeeActor(*this, nullptr))
            {
                Log::addMsg(data_->nameThe + " looks more spirited.");
            }
        }
        Render::drawMapAndInterface();
    }

    return isSpiGained;
}

void Actor::changeMaxHp(const int CHANGE, const bool ALLOW_MSG)
{
    hpMax_  = max(1, hpMax_ + CHANGE);
    hp_     = max(1, hp_ + CHANGE);

    if (ALLOW_MSG)
    {
        if (isPlayer())
        {
            if (CHANGE > 0)
            {
                Log::addMsg("I feel more vigorous!", clrMsgGood);
            }
            else if (CHANGE < 0)
            {
                Log::addMsg("I feel frailer!", clrMsgBad);
            }
        }
        else //Is monster
        {
            if (Map::player->canSeeActor(*this, nullptr))
            {
                if (CHANGE > 0)
                {
                    Log::addMsg(getNameThe() + " looks more vigorous.");
                }
                else if (CHANGE < 0)
                {
                    Log::addMsg(getNameThe() + " looks frailer.");
                }
            }
        }
    }
}

void Actor::changeMaxSpi(const int CHANGE, const bool ALLOW_MSG)
{
    spiMax_ = max(1, spiMax_ + CHANGE);
    spi_    = max(1, spi_ + CHANGE);

    if (ALLOW_MSG)
    {
        if (isPlayer())
        {
            if (CHANGE > 0)
            {
                Log::addMsg("My spirit is stronger!", clrMsgGood);
            }
            else if (CHANGE < 0)
            {
                Log::addMsg("My spirit is weaker!", clrMsgBad);
            }
        }
        else //Is monster
        {
            if (Map::player->canSeeActor(*this, nullptr))
            {
                if (CHANGE > 0)
                {
                    Log::addMsg(getNameThe() + " appears to grow in spirit.");
                }
                else if (CHANGE < 0)
                {
                    Log::addMsg(getNameThe() + " appears to shrink in spirit.");
                }
            }
        }
    }
}

ActorDied Actor::hit(int dmg, const DmgType dmgType, DmgMethod method)
{
    TRACE_FUNC_BEGIN_VERBOSE;

    if (state_ == ActorState::destroyed)
    {
        TRACE_FUNC_END_VERBOSE;
        return ActorDied::no;
    }

    TRACE_VERBOSE << "Damage from parameter: " << dmg << endl;

    bool props[size_t(PropId::END)];
    propHandler_->getPropIds(props);

    if (dmgType == DmgType::light && !props[int(PropId::lgtSens)])
    {
        return ActorDied::no;
    }

    if (isPlayer()) {Map::player->interruptActions();}

    //Damage to corpses
    //Note: Corpse is automatically destroyed if damage is high enough, otherwise it is
    //destroyed with a random chance
    if (isCorpse() && !isPlayer())
    {
        if (Rnd::fraction(5, 8) || dmg >= ((getHpMax(true) * 2) / 3))
        {
            if (method == DmgMethod::kick)
            {
                SndEmit::emitSnd({"*Crack!*", SfxId::hitCorpseBreak, IgnoreMsgIfOriginSeen::yes,
                                  pos, nullptr, SndVol::low, AlertsMon::yes
                                 });
            }

            state_ = ActorState::destroyed;
            glyph_ = ' ';

            if (isHumanoid()) {Map::mkGore(pos);}

            if (Map::cells[pos.x][pos.y].isSeenByPlayer)
            {
                Log::addMsg(getCorpseNameThe() + " is destroyed.");
            }
        }
        else //Not destroyed
        {
            if (method == DmgMethod::kick)
            {
                SndEmit::emitSnd({"*Thud*", SfxId::hitMedium, IgnoreMsgIfOriginSeen::yes, pos,
                                  nullptr, SndVol::low, AlertsMon::yes
                                 });
            }
        }
        TRACE_FUNC_END_VERBOSE;
        return ActorDied::no;
    }

    if (dmgType == DmgType::spirit) {return hitSpi(dmg, true);}

    //Property resists?
    const bool ALLOW_DMG_RES_MSG = isAlive();
    if (propHandler_->tryResistDmg(dmgType, ALLOW_DMG_RES_MSG))
    {
        TRACE_FUNC_END_VERBOSE;
        return ActorDied::no;
    }

    hit_(dmg);
    TRACE_VERBOSE << "Damage after hit_(): " << dmg << endl;

    dmg = max(1, dmg);

    //Filter damage through worn armor
    if (isHumanoid())
    {
        Armor* armor = static_cast<Armor*>(inv_->getItemInSlot(SlotId::body));
        if (armor)
        {
            TRACE_VERBOSE << "Has armor, running hit on armor" << endl;

            if (dmgType == DmgType::physical)
            {
                dmg = armor->takeDurHitAndGetReducedDmg(dmg);

                if (armor->isDestroyed())
                {
                    TRACE << "Armor was destroyed" << endl;
                    if (isPlayer())
                    {
                        const string armorName =
                            armor->getName(ItemRefType::plain, ItemRefInf::none);
                        Log::addMsg("My " + armorName + " is torn apart!", clrMsgNote);
                    }
                    delete armor;
                    armor = nullptr;
                    inv_->slots_[int(SlotId::body)].item = nullptr;
                }
            }
        }
    }

    propHandler_->onHit();

    if (!isPlayer() || !Config::isBotPlaying()) {hp_ -= dmg;}

    if (getHp() <= 0)
    {
        const bool IS_ON_BOTTOMLESS = Map::cells[pos.x][pos.y].rigid->isBottomless();
        const bool IS_DMG_ENOUGH_TO_DESTROY = dmg > ((getHpMax(true) * 5) / 4);
        const bool IS_DESTROYED = !data_->canLeaveCorpse  ||
                                  IS_ON_BOTTOMLESS        ||
                                  IS_DMG_ENOUGH_TO_DESTROY;

        die(IS_DESTROYED, !IS_ON_BOTTOMLESS, !IS_ON_BOTTOMLESS);
        TRACE_FUNC_END_VERBOSE;
        return ActorDied::yes;
    }
    else //HP is greater than 0
    {
        TRACE_FUNC_END_VERBOSE;
        return ActorDied::no;
    }
}

ActorDied Actor::hitSpi(const int DMG, const bool ALLOW_MSG)
{
    if (ALLOW_MSG)
    {
        if (isPlayer())
        {
            Log::addMsg("My spirit is drained!", clrMsgBad);
        }
    }

    propHandler_->onHit();

    if (!isPlayer() || !Config::isBotPlaying())
    {
        spi_ = max(0, spi_ - DMG);
    }
    if (getSpi() <= 0)
    {
        if (isPlayer())
        {
            Log::addMsg("All my spirit is depleted, I am devoid of life!", clrMsgBad);
        }
        else
        {
            if (Map::player->canSeeActor(*this, nullptr))
            {
                Log::addMsg(getNameThe() + " has no spirit left!");
            }
        }

        const bool IS_ON_BOTTOMLESS = Map::cells[pos.x][pos.y].rigid->isBottomless();
        const bool IS_DESTROYED     = !data_->canLeaveCorpse || IS_ON_BOTTOMLESS;

        die(IS_DESTROYED, false, true);
        return ActorDied::yes;
    }
    return ActorDied::no;
}

void Actor::die(const bool IS_DESTROYED, const bool ALLOW_GORE,
                const bool ALLOW_DROP_ITEMS)
{
    assert(data_->canLeaveCorpse || IS_DESTROYED);

    //Check all monsters and unset this actor as leader
    for (Actor* actor : GameTime::actors_)
    {
        if (actor != this && !actor->isPlayer() && isLeaderOf(actor))
        {
            static_cast<Mon*>(actor)->leader_ = nullptr;
        }
    }

    bool isOnVisibleTrap = false;

    //If died on a visible trap, destroy the corpse
    const auto* const f = Map::cells[pos.x][pos.y].rigid;
    if (f->getId() == FeatureId::trap)
    {
        if (!static_cast<const Trap*>(f)->isHidden()) {isOnVisibleTrap = true;}
    }

    bool isPlayerSeeDyingActor = true;

    if (!isPlayer())
    {
        //If this monster is player's target, unset the target
        if (Map::player->tgt_ == this)
        {
            Map::player->tgt_ = nullptr;
        }

        //Print death messages
        if (Map::player->canSeeActor(*this, nullptr))
        {
            isPlayerSeeDyingActor = true;

            const string& deathMsgOverride = data_->deathMsgOverride;

            if (!deathMsgOverride.empty())
            {
                Log::addMsg(deathMsgOverride);
            }
            else
            {
                Log::addMsg(getNameThe() + " dies.");
            }
        }
    }

    if (IS_DESTROYED || (isOnVisibleTrap && !isPlayer()))
    {
        state_ = ActorState::destroyed;
    }
    else
    {
        state_ = ActorState::corpse;
    }

    if (!isPlayer())
    {
        if (isHumanoid())
        {
            SndEmit::emitSnd({"I hear agonized screaming.", SfxId::END,
                              IgnoreMsgIfOriginSeen::yes, pos, this, SndVol::low,
                              AlertsMon::no
                             });
        }
    }

    if (ALLOW_DROP_ITEMS)
    {
        ItemDrop::dropAllCharactersItems(*this);
    }

    if (IS_DESTROYED)
    {
        glyph_ = ' ';
        tile_ = TileId::empty;
        if (isHumanoid() && ALLOW_GORE)
        {
            Map::mkGore(pos);
        }
    }
    else //Not destroyed
    {
        if (!isPlayer())
        {
            Pos newPos;
            auto* featureHere = Map::cells[pos.x][pos.y].rigid;
            //TODO: this should be decided with a floodfill instead
            if (!featureHere->canHaveCorpse())
            {
                for (int dx = -1; dx <= 1; ++dx)
                {
                    for (int dy = -1; dy <= 1; ++dy)
                    {
                        newPos      = pos + Pos(dx, dy);
                        featureHere = Map::cells[pos.x + dx][pos.y + dy].rigid;
                        if (featureHere->canHaveCorpse())
                        {
                            pos.set(newPos);
                            dx = 9999;
                            dy = 9999;
                        }
                    }
                }
            }
        }
        glyph_ = '&';
        tile_ = TileId::corpse2;
    }

    clr_ = clrRedLgt;

    die_();

    propHandler_->onDeath(isPlayerSeeDyingActor);

    if (!isPlayer())
    {
        DungeonMaster::onMonKilled(*this);
        static_cast<Mon*>(this)->leader_ = nullptr;
    }

    Render::drawMapAndInterface();
}

void Actor::addLight(bool lightMap[MAP_W][MAP_H]) const
{
    bool props[size_t(PropId::END)];
    propHandler_->getPropIds(props);

    if (state_ == ActorState::alive && props[int(PropId::radiant)])
    {
        //TODO: Much of the code below is duplicated from ActorPlayer::addLight_(), some
        //refactoring is needed.

        bool myLight[MAP_W][MAP_H];
        Utils::resetArray(myLight, false);
        const int RADI = FOV_STD_RADI_INT;
        Pos p0(max(0, pos.x - RADI), max(0, pos.y - RADI));
        Pos p1(min(MAP_W - 1, pos.x + RADI), min(MAP_H - 1, pos.y + RADI));

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
    else if (props[int(PropId::burning)])
    {
        for (int dx = -1; dx <= 1; ++dx)
        {
            for (int dy = -1; dy <= 1; ++dy)
            {
                lightMap[pos.x + dx][pos.y + dy] = true;
            }
        }
    }
    addLight_(lightMap);
}

bool Actor::isPlayer() const
{
    return this == Map::player;
}
