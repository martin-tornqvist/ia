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
  entityType(entityFeatureMob) {}

Entity::Entity(FeatureStatic* feature_) :
  feature(dynamic_cast<Feature*>(feature_)),
  entityType(entityFeatureStatic) {}

bool Look::markerAtPos(const Pos& pos, const MarkerTask_t markerTask,
                       const Item* const itemThrown) {

  bool isMsgPrinted = false;

  const bool IS_VISION = eng.map->cells[pos.x][pos.y].isSeenByPlayer;

  eng.log->clearLog();

  if(IS_VISION) {
    eng.log->addMsg("I see here:");

    entityDescribed = getEntityToDescribe(pos);

    switch(entityDescribed.entityType) {
      case entityActor:
        describeBriefActor(*entityDescribed.actor, markerTask, itemThrown);
        break;
      case entityFeatureStatic:
        describeBriefFeatureStatic(*entityDescribed.feature);
        break;
      case entityFeatureMob:
        describeBriefFeatureMob(*entityDescribed.feature);
        break;
      case entityItem:
        describeBriefItem(*entityDescribed.item);
        break;
    }
    isMsgPrinted = true;
  }

  if(pos != eng.player->pos) {
    if(markerTask == markerTask_aimRangedWeapon) {
      if(IS_VISION) {
        eng.log->addMsg("| f to fire");
      } else {
        eng.log->addMsg("f to fire");
      }
      isMsgPrinted = true;
    } else if(markerTask == markerTask_aimThrownWeapon) {
      if(IS_VISION) {
        eng.log->addMsg("| t to throw");
      } else {
        eng.log->addMsg("t to throw");
      }
      isMsgPrinted = true;
    }
  }
  return isMsgPrinted;
}

void Look::describeBriefActor(const Actor& actor,
                              const MarkerTask_t markerTask,
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

void Look::describeBriefFeatureMob(const Feature& feature) const {
  eng.log->addMsg(feature.getDescr(false) + ".");
}

void Look::describeBriefItem(const Item& item) const {
  eng.log->addMsg(
    eng.itemDataHandler->getItemInterfaceRef(item, true) + ".");
}

void Look::describeBriefFeatureStatic(const Feature& feature) const {
  eng.log->addMsg(feature.getDescr(false) + ".");
}

void Look::printExtraActorDescription(const Pos& pos) const {
  Actor* actor = eng.basicUtils->getActorAtPos(pos);
  if(actor != NULL) {
    if(actor != eng.player) {
      //Add written description.
      string description = actor->getData().description;

      //Add auto-description.
      if(actor->getData().isAutoDescriptionAllowed) {
        eng.autoDescribeActor->addAutoDescriptionLines(actor, description);
      }

      vector<string> formattedText;
      TextFormatting::lineToLines(
        description, MAP_W - 2, formattedText);

      const unsigned int NR_OF_LINES = formattedText.size();

      const int START_X = 1;
      const int START_Y = 2;

      eng.renderer->drawMapAndInterface(false);
      eng.marker->draw(markerTask_look);
      eng.renderer->coverArea(panel_screen,
                               Pos(START_X, START_Y),
                               Pos(1, NR_OF_LINES));

      for(unsigned int i = 0; i < NR_OF_LINES; i++) {
        eng.renderer->drawText(formattedText.at(i), panel_screen,
                                Pos(START_X, START_Y + i), clrWhiteHigh);
      }

      eng.renderer->updateScreen();

      eng.query->waitForKeyPress();
    }
  }
}

Entity Look::getEntityToDescribe(const Pos pos) {

  //TODO this method is a little wonky

  Actor* actor = eng.basicUtils->getActorAtPos(pos);

  //If there is a living actor there, describe the actor.
  if(actor != NULL && actor != eng.player) {
    if(actor->deadState == actorDeadState_alive) {
      if(eng.player->checkIfSeeActor(*actor, NULL)) {
        return Entity(actor);
      }
    }
  }

  //Describe mob feature
  const int NR_MOBS = eng.gameTime->getNrFeatureMobs();
  for(int i = 0; i < NR_MOBS; i++) {
    FeatureMob& f = eng.gameTime->getFeatureMobAtElement(i);
    if(f.getPos() == pos) {
      return Entity(&f);
    }
  }

  //If item there, describe that.
  Item* item = eng.map->cells[pos.x][pos.y].item;
  if(item != NULL)
    return Entity(item);

  //If static feature, describe that.
  return Entity(eng.map->cells[pos.x][pos.y].featureStatic);

  return Entity();
}

