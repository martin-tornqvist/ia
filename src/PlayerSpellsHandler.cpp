#include "PlayerSpellsHandler.h"

#include "Engine.h"
#include "ItemScroll.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "MenuInputHandler.h"
#include "Renderer.h"
#include "Inventory.h"
#include "ItemFactory.h"
#include "PlayerBon.h"
#include "Query.h"
#include "Utils.h"

PlayerSpellsHandler::~PlayerSpellsHandler() {
  for(Spell * spell : knownSpells_) {delete spell;}
}

void PlayerSpellsHandler::playerSelectSpellToCast() {
  if(knownSpells_.empty()) {
    eng.log->addMsg("I do not know any spells to invoke.");
  } else {
    sort(knownSpells_.begin(), knownSpells_.end(),
    [](Spell * s1, Spell * s2) {
      return s1->getName() < s2->getName();
    });

    MenuBrowser browser(knownSpells_.size(), 0);

    Renderer::drawMapAndInterface();

    draw(browser);

    while(true) {
      const MenuAction action = eng.menuInputHandler->getAction(browser);
      switch(action) {
        case MenuAction::browsed: {
          draw(browser);
        } break;

        case MenuAction::esc:
        case MenuAction::space: {
          eng.log->clearLog();
          Renderer::drawMapAndInterface();
          return;
        } break;

        case MenuAction::selected: {
          tryCast(knownSpells_.at(browser.getPos().y));
          return;
        } break;

        default: {} break;
      }
    }
  }
}

void PlayerSpellsHandler::tryCastPrevSpell() {
  if(prevSpellCast_ == NULL) {
    playerSelectSpellToCast();
  } else {
    tryCast(prevSpellCast_);
  }
}

void PlayerSpellsHandler::tryCast(const Spell* const spell) {
  if(eng.player->getPropHandler().allowRead(true)) {
    eng.log->clearLog();
    Renderer::drawMapAndInterface();

    const Range spiCost = spell->getSpiCost(false, eng.player, eng);
    if(spiCost.upper >= eng.player->getSpi()) {
      eng.log->addMsg("Cast spell and risk depleting your spirit (y/n)?",
                      clrWhiteHigh);
      Renderer::drawMapAndInterface();
      if(eng.query->yesOrNo() == YesNoAnswer::no) {
        eng.log->clearLog();
        Renderer::drawMapAndInterface();
        return;
      }
      eng.log->clearLog();
    }

    bool isBloodSorc  = false;
    bool isWarlock    = false;
    for(Trait id : PlayerBon::traitsPicked_) {
      if(id == Trait::bloodSorcerer)  isBloodSorc = true;
      if(id == Trait::warlock)        isWarlock   = true;
    }

    const int BLOOD_SORC_HP_DRAINED = 2;
    if(isBloodSorc) {
      if(eng.player->getHp() <= BLOOD_SORC_HP_DRAINED) {
        eng.log->addMsg("I do not have enough life force to cast this spell.");
        Renderer::drawMapAndInterface();
        return;
      }
    }

    eng.log->addMsg("I cast " + spell->getName() + "!");

    if(isBloodSorc) {
//      eng.log->addMsg("My life force fuels the spell.", clrMsgBad);
      eng.player->hit(BLOOD_SORC_HP_DRAINED, DmgType::pure, false);
    }
    if(eng.player->deadState == ActorDeadState::alive) {
      spell->cast(eng.player, true, eng);
      prevSpellCast_ = spell;
      if(isWarlock && Rnd::oneIn(2)) {
        eng.player->getPropHandler().tryApplyProp(
          new PropWarlockCharged(eng, propTurnsStd));
      }
    }
  }
}

void PlayerSpellsHandler::draw(MenuBrowser& browser) {
  const int NR_SPELLS = knownSpells_.size();
  string endLetter = "a";
  endLetter[0] += char(NR_SPELLS - 1);

  Renderer::coverArea(Panel::screen, Pos(0, 0), Pos(MAP_W, NR_SPELLS + 1));

  const string label = "Evoke which power? | space/esc to cancel";
  Renderer::drawText(label, Panel::screen, Pos(0, 0), clrWhiteHigh);

  int y = 1;

  for(int i = 0; i < NR_SPELLS; i++) {
    const int CURRENT_ELEMENT = i;
    Scroll scroll(NULL, eng);
    SDL_Color scrollClr = scroll.getInterfaceClr();
    const SDL_Color clr =
      browser.isPosAtElement(CURRENT_ELEMENT) ? clrWhite : scrollClr;
    Spell* const spell = knownSpells_.at(i);
    string str = spell->getName();

    Renderer::drawText(str, Panel::screen, Pos(0, y), clr);

    string fillStr = "";
    const int FILL_SIZE = 28 - str.size();
    for(int ii = 0; ii < FILL_SIZE; ii++) {fillStr.push_back('.');}
    SDL_Color fillClr = clrGray;
    fillClr.r /= 3; fillClr.g /= 3; fillClr.b /= 3;
    Renderer::drawText(fillStr, Panel::screen, Pos(str.size(), y), fillClr);

    int x = 28;
    str = "SPI:";
    const Range spiCost = spell->getSpiCost(false, eng.player, eng);
    const string lowerStr = toStr(spiCost.lower);
    const string upperStr = toStr(spiCost.upper);
    str += spiCost.upper == 1 ? "1" : (lowerStr +  "-" + upperStr);
    Renderer::drawText(str, Panel::screen, Pos(x, y), clrWhite);

    x += 10;
    str = "SHOCK: ";
    const IntrSpellShock shockType = spell->getShockTypeIntrCast();
    switch(shockType) {
      case IntrSpellShock::mild:        str += "Mild";       break;
      case IntrSpellShock::disturbing:  str += "Disturbing"; break;
      case IntrSpellShock::severe:      str += "Severe";     break;
    }
    Renderer::drawText(str, Panel::screen, Pos(x, y), clrWhite);

    y++;
  }

  Renderer::updateScreen();
}

void PlayerSpellsHandler::addSaveLines(vector<string>& lines) const {
  lines.push_back(toStr(knownSpells_.size()));
  for(Spell * s : knownSpells_) {lines.push_back(toStr(int(s->getId())));}
}

void PlayerSpellsHandler::setParamsFromSaveLines(vector<string>& lines) {
  const int NR_SPELLS = toInt(lines.front());
  lines.erase(lines.begin());

  for(int i = 0; i < NR_SPELLS; i++) {
    const int ID = toInt(lines.front());
    lines.erase(lines.begin());
    knownSpells_.push_back(eng.spellHandler->getSpellFromId(SpellId(ID)));
  }
}

bool PlayerSpellsHandler::isSpellLearned(const SpellId id) {
  for(Spell * s : knownSpells_) {if(s->getId() == id) {return true;}}
  return false;
}

void PlayerSpellsHandler::learnSpellIfNotKnown(const SpellId id) {
  learnSpellIfNotKnown(eng.spellHandler->getSpellFromId(id));
}

void PlayerSpellsHandler::learnSpellIfNotKnown(Spell* const spell) {
  bool isAlreadyLearned = false;
  for(Spell * spellCmpr : knownSpells_) {
    if(spellCmpr->getId() == spell->getId()) {
      isAlreadyLearned = true;
      break;
    }
  }
  if(isAlreadyLearned) {
    delete spell;
  } else {
    knownSpells_.push_back(spell);
  }
}
