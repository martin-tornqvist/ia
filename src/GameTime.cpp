#include "GameTime.h"

#include <vector>
#include <assert.h>

#include "CmnTypes.h"
#include "FeatureRigid.h"
#include "FeatureMob.h"
#include "ActorPlayer.h"
#include "ActorMon.h"
#include "Map.h"
#include "PopulateMonsters.h"
#include "Input.h"
#include "Inventory.h"
#include "InventoryHandling.h"
#include "PlayerBon.h"
#include "Audio.h"
#include "MapParsing.h"
#include "Render.h"
#include "Utils.h"
#include "MapTravel.h"
#include "Item.h"

using namespace std;

namespace GameTime
{

vector<Actor*>      actors_;
vector<Mob*> mobs_;

namespace
{

vector<ActorSpeed>  turnTypeVector_;
int                 curTurnTypePos_   = 0;
size_t              curActorIndex_    = 0;
int                 turnNr_             = 0;

bool isSpiRegenThisTurn(const int REGEN_N_TURNS)
{
    assert(REGEN_N_TURNS != 0);
    return turnNr_ == (turnNr_ / REGEN_N_TURNS) * REGEN_N_TURNS;
}

void runStdTurnEvents()
{
    ++turnNr_;

    int regenSpiNTurns = 12;

    for (size_t i = 0; i < actors_.size(); ++i)
    {
        Actor* const actor = actors_[i];

        //Delete destroyed actors
        if (actor->getState() == ActorState::destroyed)
        {
            //Do not delete player if player died, just return
            if (actor == Map::player)
            {
                return;
            }

            if (Map::player->tgt_ == actor)
            {
                Map::player->tgt_ = nullptr;
            }

            delete actor;

            actors_.erase(actors_.begin() + i);
            i--;
            if (curActorIndex_ >= actors_.size()) {curActorIndex_ = 0;}
        }
        else  //Actor is alive or is a corpse
        {
            actor->getPropHandler().tick(PropTurnMode::std);

            if (!actor->isPlayer())
            {
                Mon* const mon = static_cast<Mon*>(actor);
                if (mon->playerAwareOfMeCounter_ > 0)
                {
                    mon->playerAwareOfMeCounter_--;
                }
            }

            //Do light damage if actor in lit cell
            const Pos& pos = actor->pos;
            if (Map::cells[pos.x][pos.y].isLit)
            {
                actor->hit(1, DmgType::light);
            }

            if (actor->isAlive())
            {
                //Regen Spi
                if (actor == Map::player)
                {
                    if (PlayerBon::traits[int(Trait::stoutSpirit)])
                        regenSpiNTurns -= 2;

                    if (PlayerBon::traits[int(Trait::strongSpirit)])
                        regenSpiNTurns -= 2;

                    if (PlayerBon::traits[int(Trait::mightySpirit)])
                        regenSpiNTurns -= 2;
                }

                regenSpiNTurns = max(2, regenSpiNTurns);

                if (isSpiRegenThisTurn(regenSpiNTurns))
                {
                    actor->restoreSpi(1, false);
                }

                actor->onStdTurn();
            }
        }
    }

    //New turn for rigids
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            Map::cells[x][y].rigid->onNewTurn();
        }
    }

    //New turn for mobs (using a copied vector, since mobs may get destroyed)
    const vector<Mob*> mobsCpy = mobs_;
    for (auto* f : mobsCpy) {f->onNewTurn();}

    //Spawn more monsters?
    //(If an unexplored cell is selected, the spawn is canceled)
    if (Map::dlvl >= 1 && Map::dlvl <= DLVL_LAST)
    {
        const int SPAWN_N_TURNS = 130;

        if (turnNr_ % SPAWN_N_TURNS == 0)
        {
            PopulateMon::trySpawnDueToTimePassed();
        }
    }

    //Run new turn events on all player items
    auto& playerInv = Map::player->getInv();

    for (Item* const item : playerInv.general_)
    {
        item->onStdTurnInInv(InvType::general);
    }

    for (InvSlot& slot : playerInv.slots_)
    {
        if (slot.item)
        {
            slot.item->onStdTurnInInv(InvType::slots);
        }
    }

    SndEmit::resetNrSndMsgPrintedCurTurn();

    if (Map::dlvl > 0)
    {
        Audio::tryPlayAmb(75);
    }
}

void runAtomicTurnEvents()
{
    updateLightMap();
}

} //namespace

void init()
{
    curTurnTypePos_ = curActorIndex_ = turnNr_ = 0;
    actors_.clear();
    mobs_  .clear();
}

void cleanup()
{
    for (Actor* a : actors_) {delete a;}
    actors_.clear();

    for (auto* f : mobs_) {delete f;}
    mobs_.clear();
}

void storeToSaveLines(vector<string>& lines)
{
    lines.push_back(toStr(turnNr_));
}

void setupFromSaveLines(vector<string>& lines)
{
    turnNr_ = toInt(lines.front());
    lines.erase(begin(lines));
}

int getTurn()
{
    return turnNr_;
}

void getMobsAtPos(const Pos& p, vector<Mob*>& vectorRef)
{
    vectorRef.clear();
    for (auto* m : mobs_) {if (m->getPos() == p) {vectorRef.push_back(m);}}
}

void addMob(Mob* const f)
{
    mobs_.push_back(f);
}

void eraseMob(Mob* const f, const bool DESTROY_OBJECT)
{
    for (auto it = mobs_.begin(); it != mobs_.end(); ++it)
    {
        if (*it == f)
        {
            if (DESTROY_OBJECT) {delete f;}
            mobs_.erase(it);
            return;
        }
    }
}

void eraseAllMobs()
{
    for (auto* m : mobs_) {delete m;}
    mobs_.clear();
}

void eraseActorInElement(const size_t i)
{
    if (!actors_.empty())
    {
        delete actors_[i];
        actors_.erase(actors_.begin() + i);
    }
}

void addActor(Actor* actor)
{
    //Sanity check actor inserted
    assert(Utils::isPosInsideMap(actor->pos));
    actors_.push_back(actor);
}

void resetTurnTypeAndActorCounters()
{
    curTurnTypePos_ = curActorIndex_ = 0;
}

//For every turn type step, run through all actors and let those who can act during this
//type of turn act. When all actors who can act on this phase have acted, and if this is
//a normal speed phase - consider it a standard turn (update properties, update features,
//spawn more monsters etc.)
void tick(const bool IS_FREE_TURN)
{
    runAtomicTurnEvents();

    auto* curActor = getCurActor();

    if (curActor == Map::player)
    {
        Map::player->updateFov();
        Render::drawMapAndInterface();
        Map::updateVisualMemory();

        //Run new turn events on all player items
        auto& inv = Map::player->getInv();

        for (Item* const item : inv.general_)
        {
            item->onActorTurnInInv(InvType::general);
        }

        for (InvSlot& slot : inv.slots_)
        {
            if (slot.item)
            {
                slot.item->onActorTurnInInv(InvType::slots);
            }
        }
    }
    else //Actor is monster
    {
        auto* mon = static_cast<Mon*>(curActor);
        if (mon->awareCounter_ > 0)
        {
            mon->awareCounter_ -= 1;
        }
    }

    //Tick properties running on actor turns
    curActor->getPropHandler().tick(PropTurnMode::actor);

    if (!IS_FREE_TURN)
    {
        bool canAct = false;

        while (!canAct)
        {
            auto curTurnType = TurnType(curTurnTypePos_);

            ++curActorIndex_;

            if (curActorIndex_ >= actors_.size())
            {
                curActorIndex_ = 0;

                ++curTurnTypePos_;

                if (curTurnTypePos_ == int(TurnType::END))
                {
                    curTurnTypePos_ = 0;
                }

                if (curTurnType != TurnType::fast && curTurnType != TurnType::fastest)
                {
                    runStdTurnEvents();
                }
            }

            const auto speed = getCurActor()->getSpeed();

            switch (speed)
            {
            case ActorSpeed::sluggish:
                canAct = (curTurnType == TurnType::slow ||
                          curTurnType == TurnType::normal2)
                         && Rnd::fraction(2, 3);
                break;

            case ActorSpeed::slow:
                canAct = curTurnType == TurnType::slow ||
                         curTurnType == TurnType::normal2;
                break;

            case ActorSpeed::normal:
                canAct = curTurnType != TurnType::fast &&
                         curTurnType != TurnType::fastest;
                break;

            case ActorSpeed::fast:
                canAct = curTurnType != TurnType::fastest;
                break;

            case ActorSpeed::fastest:
                canAct = true;
                break;

            case ActorSpeed::END:
            {
                assert(false);
            } break;
            }
        }
    }
}

void updateLightMap()
{
    bool lightTmp[MAP_W][MAP_H];

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            Map::cells[x][y].isLit = lightTmp[x][y] = false;
        }
    }

    //Do not add light on Leng
    if (MapTravel::getMapType() == MapType::leng) {return;}

    for (const auto* const a : actors_)  {a->addLight(lightTmp);}

    for (const auto* const m : mobs_)    {m->addLight(lightTmp);}

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            auto* rigid = Map::cells[x][y].rigid;

            if (rigid->getBurnState() == BurnState::burning)
            {
                lightTmp[x][y] = true;
            }

            rigid->addLight(lightTmp);

            //Copy the temp values to the real light map.
            Map::cells[x][y].isLit = lightTmp[x][y];
        }
    }
}

Actor* getCurActor()
{
    Actor* const actor = actors_[curActorIndex_];

    //Sanity check actor retrieved
    assert(Utils::isPosInsideMap(actor->pos));
    return actor;
}

} //GameTime
