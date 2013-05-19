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

Entity::Entity(FeatureMob* feature_) :
  feature(dynamic_cast<Feature*>(feature_)), entityType(entityType_featureMob) {
}

Entity::Entity(FeatureStatic* feature_) :
  feature(dynamic_cast<Feature*>(feature_)), entityType(entityType_featureStatic) {
}

void Look::markerAtCoord(const coord& pos, const MarkerTask_t markerTask,
                         const Item* const itemThrown) {
  eng->log->clearLog();
  if(eng->map->playerVision[pos.x][pos.y] == true) {
    eng->log->addMessage("I see here:");

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
      eng->log->addMessage("| f to fire");
    } else   if(markerTask == markerTask_aimThrownWeapon) {
      eng->log->addMessage("| t to throw");
    }
  }
}

void Look::describeBriefActor(const Actor& actor,
                              const MarkerTask_t markerTask,
                              const Item* const itemThrown) const {
  eng->log->addMessage(actor.getNameA() + ".");

  if(markerTask == markerTask_look) {
    eng->log->addMessage("| l for description");
  } else if(actor.pos != eng->player->pos) {
    if(markerTask == markerTask_aimRangedWeapon) {
      Item* const item = eng->player->getInventory()->getItemInSlot(slot_wielded);
      Weapon* const wpn = dynamic_cast<Weapon*>(item);
      RangedAttackData data(*eng->player, *wpn, actor.pos, actor.pos, eng);
      eng->log->addMessage("| " + intToString(data.hitChanceTot) + "% hit chance");
    } else if(markerTask == markerTask_aimThrownWeapon) {
      MissileAttackData data(*eng->player, *itemThrown, actor.pos, actor.pos, eng);
      eng->log->addMessage("| " + intToString(data.hitChanceTot) + "% hit chance");
    }
  }
}

void Look::describeBriefFeatureMob(const Feature& feature) const {
  eng->log->addMessage(feature.getDescription(false) + ".");
}

void Look::describeBriefItem(const Item& item) const {
  eng->log->addMessage(eng->itemData->getItemInterfaceRef(item, true) + ".");
}

void Look::describeBriefFeatureStatic(const Feature& feature) const {
  eng->log->addMessage(feature.getDescription(false) + ".");
}

void Look::printExtraActorDescription(const coord& pos) const {
  Actor* actor = eng->mapTests->getActorAtPos(pos);
  if(actor != NULL) {
    if(actor != eng->player) {
      //Add written description.
      string description = actor->getDef()->description;

      //Add auto-description.
      if(actor->getDef()->isAutoDescriptionAllowed) {
        eng->autoDescribeActor->addAutoDescriptionLines(actor, description);
      }

      //Format the text
      vector<string> formattedText = eng->textFormatting->lineToLines(description, MAP_X_CELLS - 1);

      const unsigned int NR_OF_LINES = formattedText.size();

      const int START_X = 1;
      const int START_Y = 2;

      eng->renderer->drawMapAndInterface(false);
      eng->marker->draw(markerTask_look);
      eng->renderer->coverArea(renderArea_screen, START_X, START_Y, 1, NR_OF_LINES);

      for(unsigned int i = 0; i < NR_OF_LINES; i++) {
        eng->renderer->drawText(formattedText.at(i), renderArea_screen,
                                START_X, START_Y + i, clrWhiteHigh);
      }

      eng->renderer->updateScreen();

      eng->query->waitForKeyPress();
    }
  }
}

Entity Look::getEntityToDescribe(const coord pos) {

  //TODO this method is a little wonky

  Actor* actor = eng->mapTests->getActorAtPos(pos);

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

