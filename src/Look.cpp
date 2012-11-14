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

Entity::Entity(FeatureMob* feature_) :
  feature(dynamic_cast<Feature*>(feature_)), entityType(entityType_featureMob) {
}

Entity::Entity(FeatureStatic* feature_) :
  feature(dynamic_cast<Feature*>(feature_)), entityType(entityType_featureStatic) {
}

void Look::markerAtCoord(const coord pos) {
  eng->log->clearLog();
  if(eng->map->playerVision[pos.x][pos.y] == true) {
    eng->log->addMessage("I see here:");

    entityDescribed = getEntityToDescribe(pos);

    switch(entityDescribed.entityType) {
    case entityType_actor:
      describeBriefActor(entityDescribed.actor);
      break;
    case entityType_featureStatic:
      describeBriefFeatureStatic(entityDescribed.feature);
      break;
    case entityType_featureMob:
      describeBriefFeatureMob(entityDescribed.feature);
      break;
    case entityType_item:
      describeBriefItem(entityDescribed.item);
      break;
    }
  }
}

void Look::describeBriefActor(const Actor* const actor) const {
  eng->log->addMessage(actor->getNameA() + ".");
  eng->log->addMessage("Press 'l' for description.");
}

void Look::describeBriefFeatureMob(const Feature* const feature) const {
  eng->log->addMessage(feature->getDescription(false) + ".");
}

void Look::describeBriefItem(Item* const item) const {
  eng->log->addMessage(eng->itemData->itemInterfaceName(item, true) + ".");
}

void Look::describeBriefFeatureStatic(const Feature* const feature) const {
  eng->log->addMessage(feature->getDescription(false) + ".");
}

void Look::printExtraActorDescription(const coord pos) const {
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

      eng->renderer->clearAreaWithTextDimensions(renderArea_screen, START_X, START_Y, 1, NR_OF_LINES);

      for(unsigned int i = 0; i < NR_OF_LINES; i++) {
        eng->renderer->drawText(formattedText.at(i), renderArea_screen, START_X, START_Y + i, clrWhiteHigh);
      }

      eng->renderer->flip();

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
      return Entity(actor);
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

