#include "DungeonMaster.h"

#include "Engine.h"
#include "Highscore.h"
#include "TextFormatting.h"
#include "Renderer.h"
#include "Query.h"
#include "ActorPlayer.h"
#include "CharacterLines.h"
#include "Log.h"

void DungeonMaster::initXpArray() {
  xpForLvl[0] = 0;
  xpForLvl[1] = 0;
  for(int lvl = 2; lvl <= PLAYER_MAX_CLVL; lvl++) {
    xpForLvl[lvl] = xpForLvl[lvl - 1] + (100 * lvl);
  }
}

int DungeonMaster::getMonsterXpWorth(const ActorData& d) const {
  const double K          = 1.0;  //K regulates player XP rate
  //Higher -> more XP per monster
  const double HP         = d.hp;
  const double SPEED      = d.speed;
  const double SHOCK      = d.monsterShockLevel;
  const double UNIQUE_MOD = d.isUnique ? 2.0 : 1.0;
  return K * HP * (1.0 + (SPEED / 3.0)) * (1.0 + (SHOCK / 6.0)) * UNIQUE_MOD;
}

void DungeonMaster::playerGainLvl() {
  clvl++;

  eng.log->addMsg(
    "--- Welcome to level " + toString(clvl) + "! ---", clrGreen);
  if(clvl % 2 != 0) {
    eng.playerCreateCharacter->pickNewTrait(false);
  }

  eng.player->restoreHp(999, false);
  eng.player->changeMaxHp(2, true);

  const int BON_EVERY_N_LVL = 2;
  if(clvl % BON_EVERY_N_LVL == 0) {
    eng.player->changeMaxSpi(1, true);
    eng.player->incrMth(2, true);
  }
}

void DungeonMaster::playerGainXp(const int XP_GAINED) {
  for(int i = 0; i < XP_GAINED; i++) {
    xp++;
    if(clvl < PLAYER_MAX_CLVL) {
      if(xp >= xpForLvl[clvl + 1]) {
        playerGainLvl();
      }
    }
  }
}

int DungeonMaster::getXpToNextLvl() const {
  if(clvl == PLAYER_MAX_CLVL) {
    return -1;
  }
  return xpForLvl[clvl + 1] - xp;
}

void DungeonMaster::playerLoseXpPercent(const int PERCENT) {
  xp = (xp * (100 - PERCENT)) / 100;
}

void DungeonMaster::addSaveLines(vector<string>& lines) const {
  lines.push_back(toString(clvl));
  lines.push_back(toString(xp));
  lines.push_back(toString(timeStarted.year_));
  lines.push_back(toString(timeStarted.month_));
  lines.push_back(toString(timeStarted.day_));
  lines.push_back(toString(timeStarted.hour_));
  lines.push_back(toString(timeStarted.minute_));
  lines.push_back(toString(timeStarted.second_));
}

void DungeonMaster::setParamsFromSaveLines(vector<string>& lines) {
  clvl = toInt(lines.front());
  lines.erase(lines.begin());
  xp = toInt(lines.front());
  lines.erase(lines.begin());
  timeStarted.year_ = toInt(lines.front());
  lines.erase(lines.begin());
  timeStarted.month_ = toInt(lines.front());
  lines.erase(lines.begin());
  timeStarted.day_ = toInt(lines.front());
  lines.erase(lines.begin());
  timeStarted.hour_ = toInt(lines.front());
  lines.erase(lines.begin());
  timeStarted.minute_ = toInt(lines.front());
  lines.erase(lines.begin());
  timeStarted.second_ = toInt(lines.front());
  lines.erase(lines.begin());
}

void DungeonMaster::winGame() {
  eng.highScore->gameOver(true);

  string winMessage = "As I touch the crystal, there is a jolt of electricity. A surreal glow suddenly illuminates the area. ";
  winMessage += "I feel as if I have stirred something. I notice a dark figure observing me from the edge of the light. ";
  winMessage += "It is the shape of a human. The figure approaches me, but still no light falls on it as it enters. ";
  winMessage += "There is no doubt in my mind concerning the nature of this entity; It is the Faceless God who dwells in the depths of ";
  winMessage += "the earth, it is the Crawling Chaos - NYARLATHOTEP! ";
  winMessage += "I panic. Why is it I find myself here, stumbling around in darkness? ";
  winMessage += "The being beckons me to gaze into the stone. In the divine radiance I see visions beyond eternity, ";
  winMessage += "visions of unreal reality, visions of the brightest light of day and the darkest night of madness. ";
  winMessage += "The torrent of revelations does not seem unwelcome - I feel as if under a spell. There is only onward now. ";
  winMessage += "I demand to attain more - everything! So I make a pact with the Fiend......... ";
  winMessage += "I now harness the shadows that stride from world to world to sow death and madness. ";
  winMessage += "The destinies of all things on earth, living and dead, are mine. ";

  vector<string> winMessageLines;
  TextFormatting::lineToLines(winMessage, 68, winMessageLines);

  eng.renderer->coverPanel(panel_screen);
  eng.renderer->updateScreen();

  const int Y0 = 2;
  const unsigned int NR_OF_WIN_MESSAGE_LINES = winMessageLines.size();
  const int DELAY_BETWEEN_LINES = 40;
  eng.sleep(DELAY_BETWEEN_LINES);
  for(unsigned int i = 0; i < NR_OF_WIN_MESSAGE_LINES; i++) {
    for(unsigned int ii = 0; ii <= i; ii++) {
      eng.renderer->drawTextCentered(winMessageLines.at(ii), panel_screen,
                                     Pos(MAP_W_HALF, Y0 + ii),
                                     clrMsgBad, clrBlack, true);
      if(i == ii && ii == NR_OF_WIN_MESSAGE_LINES - 1) {
        const string CMD_LABEL =
          "Space/Esc to record high-score and return to main menu";
        eng.renderer->drawTextCentered(
          CMD_LABEL, panel_screen,
          Pos(MAP_W_HALF, Y0 + NR_OF_WIN_MESSAGE_LINES + 2),
          clrWhite, clrBlack, true);
      }
    }
    eng.renderer->updateScreen();
    eng.sleep(DELAY_BETWEEN_LINES);
  }

  eng.query->waitForEscOrSpace();
}

void DungeonMaster::monsterKilled(Actor* monster) {
  ActorData& d = monster->getData();

  d.nrOfKills += 1;

  if(d.hp >= 3) {
    if(eng.player->insanityObsessions[insanityObsession_sadism]) {
      eng.player->shock_ = max(0.0, eng.player->shock_ - 3.0);
    }
  }

  playerGainXp(getMonsterXpWorth(d));
}

void DungeonMaster::setTimeStartedToNow() {
  timeStarted = eng.basicUtils->getCurrentTime();
}

