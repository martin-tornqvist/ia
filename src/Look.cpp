#include "Look.h"

#include "Engine.h"

#include "ActorMonster.h"
#include "Feature.h"
#include "Item.h"
#include "Log.h"
#include "Map.h"
#include "ActorPlayer.h"
#include "AutoDescribeActor.h"
#include "TextFormatting.h"
#include "Query.h"
#include "Marker.h"
#include "Inventory.h"
#include "Attack.h"
#include "GameTime.h"
#include "Renderer.h"
#include "Utils.h"

Entity::Entity(FeatureMob* feature_) :
  feature(dynamic_cast<Feature*>(feature_)),
  entityType(entityFeatureMob) {}

Entity::Entity(FeatureStatic* feature_) :
  feature(dynamic_cast<Feature*>(feature_)),
  entityType(entityFeatureStatic) {}

void Look::markerAtPos(const Pos& pos, const MarkerTask markerTask,
                       const Item* const itemThrown) {
  const bool IS_VISION = eng.map->cells[pos.x][pos.y].isSeenByPlayer;

  eng.log->clearLog();

  if(IS_VISION) {
    eng.log->addMsg("I see here:");

    entityDescribed = getEntityToDescribe(pos);

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

  if(pos != eng.player->pos) {
    if(markerTask == markerTask_aimRangedWeapon) {
      if(IS_VISION) {
        eng.log->addMsg("| f to fire");
      } else {
        eng.log->addMsg("f to fire");
      }
    } else if(markerTask == markerTask_aimThrownWeapon) {
      if(IS_VISION) {
        eng.log->addMsg("| t to throw");
      } else {
        eng.log->addMsg("t to throw");
      }
    }
  }
}

void Look::descrBriefActor(const Actor& actor, const MarkerTask markerTask,
                           const Item* const itemThrown) const {
  eng.log->addMsg(actor.getNameA() + ".");

  if(markerTask == markerTask_look) {
    eng.log->addMsg("| l for description");
  } else if(actor.pos != eng.player->pos) {
    if(markerTask == markerTask_aimRangedWeapon) {
      Item* const item =
        eng.player->getInv().getItemInSlot(slot_wielded);
      Weapon* const wpn = dynamic_cast<Weapon*>(item);
      RangedAttackData data(*eng.player, *wpn, actor.pos, actor.pos, eng);
      eng.log->addMsg("| " + toString(data.hitChanceTot) + "% hit chance");
    } else if(markerTask == markerTask_aimThrownWeapon) {
      MissileAttackData data(
        *eng.player, *itemThrown, actor.pos, actor.pos, eng);
      eng.log->addMsg("| " + toString(data.hitChanceTot) + "% hit chance");
    }
  }
}

void Look::descrBriefFeatureMob(const Feature& feature) const {
  eng.log->addMsg(feature.getDescr(false) + ".");
}

void Look::descrBriefItem(const Item& item) const {
  eng.log->addMsg(
    eng.itemDataHandler->getItemInterfaceRef(item, true) + ".");
}

void Look::descrBriefFeatureStatic(const Feature& feature) const {
  eng.log->addMsg(feature.getDescr(false) + ".");
}

void Look::printExtraActorDescription(const Pos& pos) const {
  Actor* actor = Utils::getActorAtPos(pos, eng);
  if(actor != NULL) {
    if(actor != eng.player) {
      //Add written description.
      string descr = actor->getData().description;

      //Add auto-description.
      if(actor->getData().isAutoDescriptionAllowed) {
        eng.autoDescribeActor->addAutoDescriptionLines(actor, descr);
      }

      vector<string> formattedText;
      TextFormatting::lineToLines(descr, MAP_W - 1, formattedText);

      const unsigned int NR_OF_LINES = formattedText.size();

      Renderer::drawMapAndInterface(false);
      eng.marker->draw(markerTask_look);
      Renderer::coverArea(panel_screen, Pos(0, 1), Pos(MAP_W, NR_OF_LINES));

      int y = 1;
      for(string & s : formattedText) {
        Renderer::drawText(s, panel_screen, Pos(0, y), clrWhiteHigh);
        y++;
      }

      Renderer::updateScreen();

      eng.query->waitForKeyPress();
    }
  }
}

Entity Look::getEntityToDescribe(const Pos pos) {

  //TODO this method is a little wonky

  Actor* actor = Utils::getActorAtPos(pos, eng);

  //If there is a living actor there, describe the actor.
  if(actor != NULL && actor != eng.player) {
    if(actor->deadState == ActorDeadState::alive) {
      if(eng.player->isSeeingActor(*actor, NULL)) {
        return Entity(actor);
      }
    }
  }

  //Describe mob feature
  for(FeatureMob * mob : eng.gameTime->featureMobs_) {
    if(mob->getPos() == pos) {return Entity(mob);}
  }

  //If item there, describe that.
  Item* item = eng.map->cells[pos.x][pos.y].item;
  if(item != NULL)
    return Entity(item);

  //If static feature, describe that.
  return Entity(eng.map->cells[pos.x][pos.y].featureStatic);

  return Entity();
}

