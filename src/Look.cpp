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

Entity::Entity(FeatureMob* feature_) :
  feature(dynamic_cast<Feature*>(feature_)),
  entityType(entityType_featureMob) {}

Entity::Entity(FeatureStatic* feature_) :
  feature(dynamic_cast<Feature*>(feature_)),
  entityType(entityType_featureStatic) {}

void Look::markerAtPos(const Pos& pos, const MarkerTask_t markerTask,
                       const Item* const itemThrown) {

  const bool IS_VISION = eng->map->playerVision[pos.x][pos.y];

  eng->log->clearLog();
  if(IS_VISION) {
    eng->log->addMsg("I see here:");

    entityDescribed = getEntityToDescribe(pos);

    switch(entityDescribed.entityType) {
      case entityType_actor:
        describeBriefActor(*entityDescribed.actor, markerTask, itemThrown);
        break;
      case entityType_featureStatic:
        describeBriefFeatureStatic(*entityDescribed.feature);
        break;
      case entityType_featureMob:
        describeBriefFeatureMob(*entityDescribed.feature);
        break;
      case entityType_item:
        describeBriefItem(*entityDescribed.item);
        break;
    }
  }

  if(pos != eng->player->pos) {
    if(markerTask == markerTask_aimRangedWeapon) {
      if(IS_VISION) {
        eng->log->addMsg("| f to fire");
      } else {
        eng->log->addMsg("f to fire");
      }
    } else   if(markerTask == markerTask_aimThrownWeapon) {
      if(IS_VISION) {
        eng->log->addMsg("| t to throw");
      } else {
        eng->log->addMsg("t to throw");
      }
    }
  }
}

void Look::describeBriefActor(const Actor& actor,
                              const MarkerTask_t markerTask,
                              const Item* const itemThrown) const {
  eng->log->addMsg(actor.getNameA() + ".");

  if(markerTask == markerTask_look) {
    eng->log->addMsg("| l for description");
  } else if(actor.pos != eng->player->pos) {
    if(markerTask == markerTask_aimRangedWeapon) {
      Item* const item =
        eng->player->getInventory()->getItemInSlot(slot_wielded);
      Weapon* const wpn = dynamic_cast<Weapon*>(item);
      RangedAttackData data(*eng->player, *wpn, actor.pos, actor.pos, eng);
      eng->log->addMsg("| " + toString(data.hitChanceTot) + "% hit chance");
    } else if(markerTask == markerTask_aimThrownWeapon) {
      MissileAttackData data(
        *eng->player, *itemThrown, actor.pos, actor.pos, eng);
      eng->log->addMsg("| " + toString(data.hitChanceTot) + "% hit chance");
    }
  }
}

void Look::describeBriefFeatureMob(const Feature& feature) const {
  eng->log->addMsg(feature.getDescription(false) + ".");
}

void Look::describeBriefItem(const Item& item) const {
  eng->log->addMsg(
    eng->itemDataHandler->getItemInterfaceRef(item, true) + ".");
}

void Look::describeBriefFeatureStatic(const Feature& feature) const {
  eng->log->addMsg(feature.getDescription(false) + ".");
}

void Look::printExtraActorDescription(const Pos& pos) const {
  Actor* actor = eng->basicUtils->getActorAtPos(pos);
  if(actor != NULL) {
    if(actor != eng->player) {
      //Add written description.
      string description = actor->getData()->description;

      //Add auto-description.
      if(actor->getData()->isAutoDescriptionAllowed) {
        eng->autoDescribeActor->addAutoDescriptionLines(actor, description);
      }

      vector<string> formattedText;
      eng->textFormatting->lineToLines(
        description, MAP_X_CELLS - 2, formattedText);

      const unsigned int NR_OF_LINES = formattedText.size();

      const int START_X = 1;
      const int START_Y = 2;

      eng->renderer->drawMapAndInterface(false);
      eng->marker->draw(markerTask_look);
      eng->renderer->coverArea(panel_screen,
                               Pos(START_X, START_Y),
                               Pos(1, NR_OF_LINES));

      for(unsigned int i = 0; i < NR_OF_LINES; i++) {
        eng->renderer->drawText(formattedText.at(i), panel_screen,
                                Pos(START_X, START_Y + i), clrWhiteHigh);
      }

      eng->renderer->updateScreen();

      eng->query->waitForKeyPress();
    }
  }
}

Entity Look::getEntityToDescribe(const Pos pos) {

  //TODO this method is a little wonky

  Actor* actor = eng->basicUtils->getActorAtPos(pos);

  //If there is a living actor there, describe the actor.
  if(actor != NULL && actor != eng->player) {
    if(actor->deadState == actorDeadState_alive) {
      if(eng->player->checkIfSeeActor(*actor, NULL)) {
        return Entity(actor);
      }
    }
  }

  //Describe mob feature
  for(unsigned int i = 0; i < eng->gameTime->getFeatureMobsSize(); i++) {
    FeatureMob* f = eng->gameTime->getFeatureMobAt(i);
    if(f->getPos() == pos) {
      return Entity(f);
    }
  }

  //If item there, describe that.
  Item* item = eng->map->items[pos.x][pos.y];
  if(item != NULL)
    return Entity(item);

  //If static feature, describe that.
  return Entity(eng->map->featuresStatic[pos.x][pos.y]);

  return Entity();
}

