#include "DungeonMaster.h"

#include "Engine.h"
#include "Highscore.h"
#include "TextFormatting.h"
#include "Render.h"
#include "Timer.h"
#include "Query.h"
#include "ActorPlayer.h"
#include "Interface.h"
#include "Log.h"
#include "PlayerAllocBonus.h"

//This number represents how much extra xp is needed per level
const int XP_STEP = 60;
//Example
//-------
//LVL 1:   0 + XP_STEP      = 100 xp
//LVL 2: 100 + XP_STEP * 2  = 300
//LVL 3: 300 + XP_STEP * 3  = 600
//LVL 4: 600 + XP_STEP * 4  = 1000


//What's a good formula for exp from monsters then?
//Should nr of monsters per player lvl be roughly constant?
//Example
//-------
//Assume:
// - Player only fights monsters of same level
// - Player should fight 10 monsters at every lvl to reach next lvl
//LVL  1 Monster:   100 / 10 =  10 exp per monster
//LVL  2 Monster:   300 / 10 =  30 exp per monster
//LVL 10 Monster: 5 500 / 10 = 550 exp per monster


//Perhaps there should be some incline in kills needed
//Example
//-------
//Assume:
// - Kills needed starts at 5 and increases by 1 for each lvl
//LVL  1 Monster:    100 /  5 =  20 exp per monster
//LVL  2 Monster:    300 /  6 =  50 exp per monster
//LVL 10 Monster:  5 500 / 14 = 392 exp per monster
//LVL 20 Monster: 21 000 / 24 = 875 exp per monster


//This gives the following formula for monster exp:
// monsterExp = expTable[monsterLvl] / (BASE_MONSTERS_TO_LVL + monsterLvl)
// perhaps divide the right hand "monsterLvl" by 2, to not have such a steep
// xp deterioration?
const int BASE_MONSTERS_TO_LVL = 40;

void DungeonMaster::init() {
	playerExp = 0;
	playerLvl = 1;

	initExpTable();
}

void DungeonMaster::addSaveLines(vector<string>& lines) const {
	lines.push_back(intToString(playerExp));
	lines.push_back(intToString(playerLvl));
}

void DungeonMaster::setParametersFromSaveLines(vector<string>& lines) {
	playerExp = stringToInt(lines.front());
	lines.erase(lines.begin());
	playerLvl = stringToInt(lines.front());
	lines.erase(lines.begin());
}

void DungeonMaster::winGame() {
	eng->highScore->gameOver(true);

	string winMessage = "As you touch the crystal, there is a jolt of electricity. A surreal glow suddenly illuminates the area. ";
	winMessage += "You feel as if you have stirred something. You notice a dark figure observing you from the edge of the light. ";
	winMessage += "It is the shape of a human. The figure approaches you, but still no light falls on it as it enters. ";
	winMessage += "There is no doubt in your mind concerning the nature of this entity; It is the Faceless God who dwells in the depths of ";
	winMessage += "the earth, it is the Crawling Chaos, the 'Black Man' of the witch-cult, the Devil - NYARLATHOTEP! ";
	winMessage += "You panic. Why is it you find yourself here, stumbling around in darkness? ";
	winMessage += "The being beckons you to gaze into the stone. In the divine radiance you see visions beyond eternity, ";
	winMessage += "visions of unreal reality, visions of the brightest light of day and the darkest night of madness. ";
	winMessage += "The torrent of revelations does not seem unwelcome - You feel as if under a spell. There is only onward now. ";
	winMessage += "You demand to attain more - everything! So you make a pact with the Fiend......... ";
	winMessage += "You now harness the shadows that stride from world to world to sow death and madness. ";
	winMessage += "The destinies of all things on earth, living and dead, are yours. ";

	const vector<string> winMessageLines = eng->textFormatting->lineToLines(winMessage, 68);

	eng->renderer->clearRenderArea(renderArea_screen);
	eng->renderer->flip();

	const int Y0 = 2;
	const unsigned int NR_OF_WIN_MESSAGE_LINES = winMessageLines.size();
	const int DELAY_BETWEEN_LINES = 75;
	Timer t;
	t.start();
	while(t.get_ticks() < DELAY_BETWEEN_LINES) {
	}
	for(unsigned int i = 0; i < NR_OF_WIN_MESSAGE_LINES; i++) {
		eng->renderer->drawTextCentered(winMessageLines.at(i), renderArea_screen, MAP_X_CELLS_HALF, Y0 + i, clrMessageBad);
		eng->renderer->flip();
		t.stop();
		t.start();
		while(t.get_ticks() < DELAY_BETWEEN_LINES) {
		}
	}
	const string CMD_LABEL = "[Space/Esc] Record high-score and return to main menu";
	eng->renderer->drawTextCentered(CMD_LABEL, renderArea_screen, MAP_X_CELLS_HALF, Y0 + NR_OF_WIN_MESSAGE_LINES + 2, clrWhite);
	eng->renderer->flip();
	eng->query->waitForEscOrSpace();
	eng->renderer->clearRenderArea(renderArea_screen);
	eng->renderer->flip();
}

void DungeonMaster::initExpTable() {
	expTable[1] = XP_STEP;

	for(int i = 2; i <= PLAYER_MAX_LEVEL + 1; i++) {
		expTable[i] = i * XP_STEP + expTable[i - 1];
	}
}

void DungeonMaster::monsterKilled(Actor* monster) {
	const int MONSTER_LVL = monster->getInstanceDefinition()->monsterLvl;

	const int MONSTER_XP = getXpToNextLvlAtLvl(MONSTER_LVL) / (BASE_MONSTERS_TO_LVL + MONSTER_LVL / 2);

	eng->renderer->drawMapAndInterface();

	if(playerLvl < PLAYER_MAX_LEVEL) {
		playerGainsExp(MONSTER_XP);
	}

	monster->getArchetypeDefinition()->nrOfKills += 1;
	monster->getInstanceDefinition() ->nrOfKills += 1;

	if(MONSTER_LVL > 1) {
		if(eng->player->insanityCompulsions[insanityCompulsion_sadism] == true) {
			eng->player->insanityShort = max(0, eng->player->insanityShort - 4);
		}
	}
}

void DungeonMaster::playerGainsExp(int exp) {
	if(eng->player->deadState == actorDeadState_alive) {
		if(playerLvl <= PLAYER_MAX_LEVEL) {
			//Exp increase must be looped in case player gains several lvls in one kill
			for(int i = 0; i < exp; i++) {
				playerExp++;

				if(playerLvl < PLAYER_MAX_LEVEL) {

					if(playerExp >= getXpToNextLvl()) {
					   eng->renderer->drawMapAndInterface();
						playerLvl++;
						eng->log->addMessage("You have reached level " + intToString(playerLvl) + "!", clrGreen);
						eng->log->addMessage("You gain new abilities, press any key to select...", clrGreen);
						eng->renderer->flip();
						eng->query->waitForKeyPress();

						int& hp_max = eng->player->getInstanceDefinition()->HP_max;

						hp_max += 3; //eng->dice(1, 4);

						eng->player->restoreHP(hp_max / 2, false);

						eng->playerAllocBonus->run();
						eng->log->clearLog();
						eng->renderer->flip();
					}
				}
			}
		}
	}
}

