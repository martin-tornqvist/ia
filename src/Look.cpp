#include "Look.h"

#include <string>

#include "ActorMon.h"
#include "Feature.h"
#include "Item.h"
#include "Log.h"
#include "Map.h"
#include "ActorPlayer.h"
#include "TextFormat.h"
#include "Query.h"
#include "Marker.h"
#include "Inventory.h"
#include "Attack.h"
#include "GameTime.h"
#include "Render.h"
#include "Utils.h"
#include "FeatureMob.h"
#include "FeatureRigid.h"

using namespace std;

namespace AutoDescrActor
{

namespace
{

string getNormalGroupSizeStr(const ActorDataT& def)
{
    const MonGroupSize s = def.groupSize;

    return
        s == MonGroupSize::alone ? "alone"           :
        s == MonGroupSize::few    ? "in small groups" :
        s == MonGroupSize::group  ? "in groups"       :
        s == MonGroupSize::horde  ? "in hordes"       : "in swarms";
}

string getSpeedStr(const ActorDataT& def)
{
    switch (def.speed)
    {
    case ActorSpeed::sluggish:   {return "sluggishly";}
    case ActorSpeed::slow:       {return "slowly";}
    case ActorSpeed::normal:     {return "at normal speed";}
    case ActorSpeed::fast:       {return "fast";}
    case ActorSpeed::fastest:    {return "very fast";}
    case ActorSpeed::END: {} break;
    }
    return "";
}

string getDwellingLvlStr(const ActorDataT& def)
{
    return toStr(max(1, def.spawnMinDLVL - 1));
}

} //namespace

void addAutoDescriptionLines(Actor* const actor, string& line)
{
    const ActorDataT& def = actor->getData();

    if (def.isUnique)
    {
        if (def.spawnMinDLVL < DLVL_LAST)
        {
            line += " " + def.nameThe + " is normally found beneath level " +
                    getDwellingLvlStr(def) + ". ";
        }
    }
    else
    {
        line += " They tend to dwell " + getNormalGroupSizeStr(def) + ",";
        line += " and usually stay at depths beneath level " +
                getDwellingLvlStr(def) + ".";
        line += " They appear to move " + getSpeedStr(def) + ". ";
    }
}

} //AutoDescrActor

namespace Look
{

void printLocationInfoMsgs(const Pos& pos)
{
    const Cell& cell = Map::cells[pos.x][pos.y];

    if (Map::cells[pos.x][pos.y].isSeenByPlayer)
    {
        Log::addMsg("I see here:");

        //Describe rigid.
        string str = cell.rigid->getName(Article::a);

        TextFormat::firstToUpper(str);

        Log::addMsg(str + ".");

        //Describe mobile features.
        for (auto* mob : GameTime::mobs_)
        {
            if (mob->getPos() == pos)
            {
                str = mob->getName(Article::a);

                TextFormat::firstToUpper(str);

                Log::addMsg(str  + ".");
            }
        }

        //Describe item.
        Item* item = cell.item;
        if (item)
        {
            str = item->getName(ItemRefType::plural, ItemRefInf::yes,
                                ItemRefAttInf::wpnContext);

            TextFormat::firstToUpper(str);

            Log::addMsg(str + ".");
        }

        //Describe dead actors.
        for (Actor* actor : GameTime::actors_)
        {
            if (actor->isCorpse() && actor->pos == pos)
            {
                str = actor->getCorpseNameA();

                TextFormat::firstToUpper(str);

                Log::addMsg(str + ".");
            }
        }

        //Describe living actor.
        Actor* actor = Utils::getActorAtPos(pos);
        if (actor && actor != Map::player)
        {
            if (actor->isAlive())
            {
                if (Map::player->isSeeingActor(*actor, nullptr))
                {
                    str = actor->getNameA();

                    TextFormat::firstToUpper(str);

                    Log::addMsg(str + ".");
                }
            }
        }

    }
    else //Cell not seen
    {
        Log::addMsg("I have no vision here.");
    }
}

void printDetailedActorDescr(const Pos& pos)
{
    Actor* actor = Utils::getActorAtPos(pos);
    if (actor && actor != Map::player)
    {
        //Add written description.
        string descr = actor->getData().descr;

        //Add auto-description.
        if (actor->getData().isAutoDescrAllowed)
        {
            AutoDescrActor::addAutoDescriptionLines(actor, descr);
        }

        vector<string> formattedText;
        TextFormat::lineToLines(descr, MAP_W - 1, formattedText);

        const size_t NR_OF_LINES = formattedText.size();

//      Render::drawMapAndInterface(false);

        Render::coverArea(Panel::screen, Pos(0, 1), Pos(MAP_W, NR_OF_LINES));

        int y = 1;
        for (string& s : formattedText)
        {
            Render::drawText(s, Panel::screen, Pos(0, y), clrWhiteHigh);
            y++;
        }

        Render::updateScreen();

        Query::waitForKeyPress();
    }
}

} //Look
