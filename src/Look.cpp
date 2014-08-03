#include "Look.h"

#include <string>

#include "ActorMonster.h"
#include "Feature.h"
#include "Item.h"
#include "Log.h"
#include "Map.h"
#include "ActorPlayer.h"
#include "TextFormatting.h"
#include "Query.h"
#include "Marker.h"
#include "Inventory.h"
#include "Attack.h"
#include "GameTime.h"
#include "Renderer.h"
#include "Utils.h"
#include "FeatureMob.h"
#include "FeatureRigid.h"

using namespace std;

Entity::Entity(Mob* feature_) :
  feature(static_cast<Feature*>(feature_)), entityType(EntityType::mob) {}

Entity::Entity(Rigid* feature_) :
  feature(static_cast<Feature*>(feature_)), entityType(EntityType::rigid) {}

namespace AutoDescrActor {

namespace {

string getNormalGroupSizeStr(const ActorDataT& def) {
  const MonsterGroupSize s = def.groupSize;

  return
    s == monsterGroupSize_alone ? "alone" :
    s == monsterGroupSizeFew    ? "in small groups" :
    s == monsterGroupSizeGroup  ? "in groups" :
    s == monsterGroupSizeHorde  ? "in hordes" :
    "in swarms";
}

string getSpeedStr(const ActorDataT& def) {
  switch(def.speed) {
    case ActorSpeed::sluggish:   {return "at sluggishly";}
    case ActorSpeed::slow:       {return "slowly";}
    case ActorSpeed::normal:     {return "at normal speed";}
    case ActorSpeed::fast:       {return "fast";}
    case ActorSpeed::fastest:    {return "very fast";}
    case ActorSpeed::END: {} break;
  }
  return "";
}

string getDwellingLvlStr(const ActorDataT& def) {
  return toStr(max(1, def.spawnMinDLVL));
}

} //namespace

void addAutoDescriptionLines(Actor* const actor, string& line) {

  const ActorDataT& def = actor->getData();

  if(def.isUnique) {
    if(def.spawnMinDLVL < LAST_CAVERN_LVL) {
      line += " " + def.nameThe + " is normally found beneath level " +
              getDwellingLvlStr(def) + ". ";
    }
  } else {
    line += " They tend to dwell " + getNormalGroupSizeStr(def) + ",";
    line += " and usually stay at depths beneath level " +
            getDwellingLvlStr(def) + ".";
    line += " They appear to move " + getSpeedStr(def) + ". ";
  }
}

} //AutoDescrActor

namespace Look {

namespace {

Entity getEntityToDescribe(const Pos pos) {

  //TODO this method is a little wonky

  Actor* actor = Utils::getFirstActorAtPos(pos);

  //If there is a living actor there, describe the actor.
  if(actor && actor != Map::player) {
    if(actor->deadState == ActorDeadState::alive) {
      if(Map::player->isSeeingActor(*actor, nullptr)) {
        return Entity(actor);
      }
    }
  }

  //Describe mob feature
  for(auto* mob : GameTime::mobs_) {
    if(mob->getPos() == pos) {return Entity(mob);}
  }

  //If item there, describe that.
  Item* item = Map::cells[pos.x][pos.y].item;
  if(item)
    return Entity(item);

  //If rigid, describe that.
  return Entity(Map::cells[pos.x][pos.y].rigid);

  return Entity();
}

void descrBriefMob(const Feature& feature) {
  Log::addMsg(feature.getName(Article::a) + ".");
}

void descrBriefRigid(const Feature& feature) {
  Log::addMsg(feature.getName(Article::a) + ".");
}

void descrBriefItem(const Item& item) {
  Log::addMsg(ItemData::getItemInterfaceRef(item, true) + ".");
}

void descrBriefActor(const Actor& actor) {
  Log::addMsg(actor.getNameA());
}

} //namespace

void printLocationInfoMsgs(const Pos& pos) {

  Log::clearLog();

  if(Map::cells[pos.x][pos.y].isSeenByPlayer) {
    Log::addMsg("I see here:");

    Entity entityDescribed = getEntityToDescribe(pos);

    switch(entityDescribed.entityType) {
      case EntityType::actor: {descrBriefActor(*entityDescribed.actor);}    break;
      case EntityType::rigid: {descrBriefRigid(*entityDescribed.feature);}  break;
      case EntityType::mob:   {descrBriefMob(*entityDescribed.feature);}    break;
      case EntityType::item:  {descrBriefItem(*entityDescribed.item);}      break;
    }
  } else {
    Log::addMsg("I have no vision here.");
  }
}

void printDetailedActorDescr(const Pos& pos) {
  Actor* actor = Utils::getFirstActorAtPos(pos);
  if(actor && actor != Map::player) {
    //Add written description.
    string descr = actor->getData().descr;

    //Add auto-description.
    if(actor->getData().isAutoDescrAllowed) {
      AutoDescrActor::addAutoDescriptionLines(actor, descr);
    }

    vector<string> formattedText;
    TextFormatting::lineToLines(descr, MAP_W - 1, formattedText);

    const size_t NR_OF_LINES = formattedText.size();

//      Renderer::drawMapAndInterface(false);

    Renderer::coverArea(Panel::screen, Pos(0, 1), Pos(MAP_W, NR_OF_LINES));

    int y = 1;
    for(string& s : formattedText) {
      Renderer::drawText(s, Panel::screen, Pos(0, y), clrWhiteHigh);
      y++;
    }

    Renderer::updateScreen();

    Query::waitForKeyPress();
  }
}

} //Look
