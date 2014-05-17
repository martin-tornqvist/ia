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

using namespace std;

Entity::Entity(FeatureMob* feature_) :
  feature(dynamic_cast<Feature*>(feature_)),
  entityType(entityFeatureMob) {}

Entity::Entity(FeatureStatic* feature_) :
  feature(dynamic_cast<Feature*>(feature_)),
  entityType(entityFeatureStatic) {}

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
    case ActorSpeed::endOfActorSpeed: {} break;
  }
  return "";
}

string getDwellingLevelStr(const ActorDataT& def) {
  return toStr(max(1, def.spawnMinDLVL));
}

} //namespace

void addAutoDescriptionLines(Actor* const actor, string& line) {

  const ActorDataT& def = actor->getData();

  if(def.isUnique) {
    if(def.spawnMinDLVL < LAST_CAVERN_LEVEL) {
      line += " " + def.name_the + " is normally found beneath level " +
              getDwellingLevelStr(def) + ". ";
    }
  } else {
    line += " They tend to dwell " + getNormalGroupSizeStr(def) + ",";
    line += " and usually stay at depths beneath level " +
            getDwellingLevelStr(def) + ".";
    line += " They appear to move " + getSpeedStr(def) + ". ";
  }
}

} //AutoDescrActor

namespace Look {

namespace {

Entity getEntityToDescribe(const Pos pos) {

  //TODO this method is a little wonky

  Actor* actor = Utils::getActorAtPos(pos);

  //If there is a living actor there, describe the actor.
  if(actor != nullptr && actor != Map::player) {
    if(actor->deadState == ActorDeadState::alive) {
      if(Map::player->isSeeingActor(*actor, nullptr)) {
        return Entity(actor);
      }
    }
  }

  //Describe mob feature
  for(FeatureMob * mob : GameTime::featureMobs_) {
    if(mob->getPos() == pos) {return Entity(mob);}
  }

  //If item there, describe that.
  Item* item = Map::cells[pos.x][pos.y].item;
  if(item != nullptr)
    return Entity(item);

  //If static feature, describe that.
  return Entity(Map::cells[pos.x][pos.y].featureStatic);

  return Entity();
}

void descrBriefFeatureMob(const Feature& feature) {
  Log::addMsg(feature.getDescr(false) + ".");
}

void descrBriefFeatureStatic(const Feature& feature) {
  Log::addMsg(feature.getDescr(false) + ".");
}

void descrBriefItem(const Item& item) {
  Log::addMsg(ItemData::getItemInterfaceRef(item, true) + ".");
}

void descrBriefActor(const Actor& actor, const MarkerTask markerTask,
                     const Item* const itemThrown) {
  Log::addMsg(actor.getNameA() + ".");

  if(markerTask == MarkerTask::look) {
    Log::addMsg("| v for description");
  } else if(actor.pos != Map::player->pos) {
    if(markerTask == MarkerTask::aimRangedWeapon) {
      Item* const item =
        Map::player->getInv().getItemInSlot(SlotId::wielded);
      Weapon* const wpn = dynamic_cast<Weapon*>(item);
      RangedAttData data(*Map::player, *wpn, actor.pos, actor.pos);
      Log::addMsg("| " + toStr(data.hitChanceTot) + "% hit chance");
    } else if(markerTask == MarkerTask::aimThrownWeapon) {
      MissileAttData data(
        *Map::player, *itemThrown, actor.pos, actor.pos);
      Log::addMsg("| " + toStr(data.hitChanceTot) + "% hit chance");
    }
  }
}

} //namespace

void onMarkerAtPos(const Pos& pos, const MarkerTask markerTask,
                   const Item* const itemThrown) {
  const bool IS_VISION = Map::cells[pos.x][pos.y].isSeenByPlayer;

  Log::clearLog();

  if(IS_VISION) {
    Log::addMsg("I see here:");

    Entity entityDescribed = getEntityToDescribe(pos);

    switch(entityDescribed.entityType) {
      case entityActor:
        descrBriefActor(*entityDescribed.actor, markerTask, itemThrown);
        break;
      case entityFeatureStatic:
        descrBriefFeatureStatic(*entityDescribed.feature);
        break;
      case entityFeatureMob:
        descrBriefFeatureMob(*entityDescribed.feature);
        break;
      case entityItem:
        descrBriefItem(*entityDescribed.item);
        break;
    }
  }

  if(pos != Map::player->pos) {
    if(markerTask == MarkerTask::aimRangedWeapon) {
      if(IS_VISION) {
        Log::addMsg("| f to fire");
      } else {
        Log::addMsg("f to fire");
      }
    } else if(markerTask == MarkerTask::aimThrownWeapon) {
      if(IS_VISION) {
        Log::addMsg("| t to throw");
      } else {
        Log::addMsg("t to throw");
      }
    }
  }
}

void printExtraActorDescription(const Pos& pos) {
  Actor* actor = Utils::getActorAtPos(pos);
  if(actor != nullptr) {
    if(actor != Map::player) {
      //Add written description.
      string descr = actor->getData().descr;

      //Add auto-description.
      if(actor->getData().isAutoDescrAllowed) {
        AutoDescrActor::addAutoDescriptionLines(actor, descr);
      }

      vector<string> formattedText;
      TextFormatting::lineToLines(descr, MAP_W - 1, formattedText);

      const unsigned int NR_OF_LINES = formattedText.size();

      Renderer::drawMapAndInterface(false);
      Marker::draw(MarkerTask::look);
      Renderer::coverArea(Panel::screen, Pos(0, 1), Pos(MAP_W, NR_OF_LINES));

      int y = 1;
      for(string & s : formattedText) {
        Renderer::drawText(s, Panel::screen, Pos(0, y), clrWhiteHigh);
        y++;
      }

      Renderer::updateScreen();

      Query::waitForKeyPress();
    }
  }
}

} //Look
