#include "Explosion.h"

#include "Render.h"
#include "Map.h"
#include "Log.h"
#include "MapParsing.h"
#include "SdlWrapper.h"
#include "LineCalc.h"
#include "ActorPlayer.h"
#include "Utils.h"
#include "SdlWrapper.h"
#include "PlayerBon.h"
#include "FeatureRigid.h"
#include "FeatureMob.h"

using namespace std;

namespace
{

void draw(const vector< vector<Pos> >& posLists, bool blocked[MAP_W][MAP_H],
          const Clr* const clrOverride)
{
    Render::drawMapAndInterface();

    const Clr& clrInner = clrOverride ? *clrOverride : clrYellow;
    const Clr& clrOuter = clrOverride ? *clrOverride : clrRedLgt;

    const bool IS_TILES     = Config::isTilesMode();
    const int NR_ANIM_STEPS = IS_TILES ? 2 : 1;

    bool isAnyCellSeenByPlayer = false;

    for (int iAnim = 0; iAnim < NR_ANIM_STEPS; iAnim++)
    {

        const TileId tile = iAnim == 0 ? TileId::blast1 : TileId::blast2;

        const int NR_OUTER = posLists.size();
        for (int iOuter = 0; iOuter < NR_OUTER; iOuter++)
        {
            const Clr& clr = iOuter == NR_OUTER - 1 ? clrOuter : clrInner;
            const vector<Pos>& inner = posLists[iOuter];
            for (const Pos& pos : inner)
            {
                if (Map::cells[pos.x][pos.y].isSeenByPlayer && !blocked[pos.x][pos.y])
                {
                    isAnyCellSeenByPlayer = true;
                    if (IS_TILES)
                    {
                        Render::drawTile(tile, Panel::map, pos, clr, clrBlack);
                    }
                    else
                    {
                        Render::drawGlyph('*', Panel::map, pos, clr, true, clrBlack);
                    }
                }
            }
        }
        if (isAnyCellSeenByPlayer)
        {
            Render::updateScreen();
            SdlWrapper::sleep(Config::getDelayExplosion() / NR_ANIM_STEPS);
        }
    }
}

void getArea(const Pos& c, const int RADI, Rect& rectRef)
{
    rectRef = Rect(Pos(max(c.x - RADI, 1),         max(c.y - RADI, 1)),
                   Pos(min(c.x + RADI, MAP_W - 2), min(c.y + RADI, MAP_H - 2)));
}

void getCellsReached(const Rect& area, const Pos& origin,
                     bool blocked[MAP_W][MAP_H],
                     vector< vector<Pos> >& posListRef)
{
    vector<Pos> line;
    for (int y = area.p0.y; y <= area.p1.y; ++y)
    {
        for (int x = area.p0.x; x <= area.p1.x; ++x)
        {
            const Pos pos(x, y);
            const int DIST = Utils::kingDist(pos, origin);
            bool isReached = true;
            if (DIST > 1)
            {
                LineCalc::calcNewLine(origin, pos, true, 999, false, line);
                for (Pos& posCheckBlock : line)
                {
                    if (blocked[posCheckBlock.x][posCheckBlock.y])
                    {
                        isReached = false;
                        break;
                    }
                }
            }
            if (isReached)
            {
                if (int(posListRef.size()) <= DIST) {posListRef.resize(DIST + 1);}
                posListRef[DIST].push_back(pos);
            }
        }
    }
}

} //namespace


namespace Explosion
{

void runExplosionAt(const Pos& origin, const ExplType explType,
                    const ExplSrc explSrc, const int RADI_CHANGE,
                    const SfxId sfx, Prop* const prop, const Clr* const clrOverride)
{
    Rect area;
    const int RADI = EXPLOSION_STD_RADI + RADI_CHANGE;
    getArea(origin, RADI, area);

    bool blocked[MAP_W][MAP_H];
    MapParse::run(CellCheck::BlocksProjectiles(), blocked);

    vector< vector<Pos> > posLists;
    getCellsReached(area, origin, blocked, posLists);

    SndVol vol = explType == ExplType::expl ? SndVol::high : SndVol::low;

    Snd snd("I hear an explosion!", sfx, IgnoreMsgIfOriginSeen::yes, origin,
            nullptr, vol, AlertsMon::yes);
    SndEmit::emitSnd(snd);

    draw(posLists, blocked, clrOverride);

    //Do damage, apply effect

    Actor* livingActors[MAP_W][MAP_H];
    vector<Actor*> corpses[MAP_W][MAP_H];

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            livingActors[x][y] = nullptr;
            corpses[x][y].clear();
        }
    }

    for (Actor* actor : GameTime::actors_)
    {
        const Pos& pos = actor->pos;
        if (actor->isAlive())
        {
            livingActors[pos.x][pos.y] = actor;
        }
        else if (actor->isCorpse())
        {
            corpses[pos.x][pos.y].push_back(actor);
        }
    }

    const bool IS_DEM_EXP = PlayerBon::traits[int(Trait::demExpert)];

    const int NR_OUTER = posLists.size();
    for (int curRadi = 0; curRadi < NR_OUTER; curRadi++)
    {
        const vector<Pos>& positionsAtCurRadi = posLists[curRadi];

        for (const Pos& pos : positionsAtCurRadi)
        {

            Actor* livingActor          = livingActors[pos.x][pos.y];
            vector<Actor*> corpsesHere  = corpses[pos.x][pos.y];

            if (explType == ExplType::expl)
            {
                //Damage environment
                Cell& cell = Map::cells[pos.x][pos.y];
                cell.rigid->hit(DmgType::physical, DmgMethod::explosion, nullptr);

                const int ROLLS = EXPL_DMG_ROLLS - curRadi;
                const int DMG   = Rnd::dice(ROLLS, EXPL_DMG_SIDES) + EXPL_DMG_PLUS;

                //Damage living actor
                if (livingActor)
                {
                    if (livingActor == Map::player)
                    {
                        Log::addMsg("I am hit by an explosion!", clrMsgBad);
                    }
                    livingActor->hit(DMG, DmgType::physical);
                }
                //Damage dead actors
                for (Actor* corpse : corpsesHere) {corpse->hit(DMG, DmgType::physical);}

                //Add smoke
                if (Rnd::fraction(6, 10)) {GameTime::addMob(new Smoke(pos, Rnd::range(2, 4)));}
            }

            //Apply property
            if (prop)
            {
                bool shouldApplyOnLivingActor = livingActor;

                //Do not apply burning if actor is player with the demolition expert trait, and
                //intentionally throwing a Molotov
                if (
                    livingActor    == Map::player &&
                    prop->getId()  == PropId::burning &&
                    IS_DEM_EXP                    &&
                    explSrc        == ExplSrc::playerUseMoltvIntended)
                {
                    shouldApplyOnLivingActor = false;
                }

                if (shouldApplyOnLivingActor)
                {
                    PropHandler& propHlr = livingActor->getPropHandler();
                    Prop* propCpy = propHlr.mkProp(prop->getId(), PropTurns::specific,
                                                   prop->turnsLeft_);
                    propHlr.tryApplyProp(propCpy);
                }

                //If property is burning, also apply it to corpses and environment
                if (prop->getId() == PropId::burning)
                {
                    Cell& cell = Map::cells[pos.x][pos.y];
                    cell.rigid->hit(DmgType::fire, DmgMethod::elemental, nullptr);

                    for (Actor* corpse : corpsesHere)
                    {
                        PropHandler& propHlr = corpse->getPropHandler();
                        Prop* propCpy = propHlr.mkProp(prop->getId(), PropTurns::specific,
                                                       prop->turnsLeft_);
                        propHlr.tryApplyProp(propCpy);
                    }
                }
            }
        }
    }

    Map::player->updateFov();
    Render::drawMapAndInterface();

    if (prop) {delete prop;}
}

void runSmokeExplosionAt(const Pos& origin/*, const int SMOKE_DURATION*/)
{
    Rect area;
    const int RADI = EXPLOSION_STD_RADI;
    getArea(origin, RADI, area);

    bool blocked[MAP_W][MAP_H];
    MapParse::run(CellCheck::BlocksProjectiles(), blocked);

    vector< vector<Pos> > posLists;
    getCellsReached(area, origin, blocked, posLists);

    //TODO: Sound message?
    Snd snd("", SfxId::END, IgnoreMsgIfOriginSeen::yes, origin, nullptr,
            SndVol::low, AlertsMon::yes);
    SndEmit::emitSnd(snd);

    for (const vector<Pos>& inner : posLists)
    {
        for (const Pos& pos : inner)
        {
            if (!blocked[pos.x][pos.y])
            {
                GameTime::addMob(new Smoke(pos, Rnd::range(25, 30)));
            }
        }
    }

    Map::player->updateFov();
    Render::drawMapAndInterface();
}

} //Explosion

