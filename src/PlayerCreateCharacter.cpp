#include "PlayerCreateCharacter.h"

#include "Engine.h"
#include "Render.h"
#include "ActorPlayer.h"

void PlayerCreateCharacter::run() {
	int currentRenderYpos = 3; //1;
//	playerPickBackground(currentRenderYpos);

	PlayerEnterName playerEnterName(eng);
	playerEnterName.run(currentRenderYpos);
}

void PlayerCreateCharacter::playerPickBackground(int& currentRenderYpos) {
	int currentSelectedPos = 0;

	int lastRenderYpos = drawAndReturnLastYpos(currentSelectedPos, currentRenderYpos);

	bool done = false;
	while(done == false) {
		while(SDL_PollEvent(&m_event)) {
			switch(m_event.type) {
			case SDL_KEYDOWN: {
				Uint16 key = static_cast<Uint16>(m_event.key.keysym.sym);

				if(key == SDLK_RETURN || key == SDLK_KP_ENTER) {
					playerBackground_ = static_cast<PlayerBackgrounds_t>(currentSelectedPos);
					currentRenderYpos = lastRenderYpos;
					return;
				}

				if(key == SDLK_UP || key == SDLK_KP8 || key == SDLK_8) {
					currentSelectedPos = currentSelectedPos == 0 ? endOfPlayerBackgrounds - 1 : currentSelectedPos - 1;
					lastRenderYpos = drawAndReturnLastYpos(currentSelectedPos, currentRenderYpos);
				}

				if(key == SDLK_DOWN || key == SDLK_KP2 || key == SDLK_2) {
					currentSelectedPos = currentSelectedPos == endOfPlayerBackgrounds - 1 ? 0 : currentSelectedPos + 1;
					lastRenderYpos = drawAndReturnLastYpos(currentSelectedPos, currentRenderYpos);
				}

			}
			default: {
			}
			break;
			}
		}
		SDL_Delay(1);
	}
}

int PlayerCreateCharacter::drawAndReturnLastYpos(const int CURRENT_SELECTED_POS, const int RENDER_Y_POS_START) const {
	eng->renderer->clearRenderArea(renderArea_screen);

   const int Y_POS_START = 1;
	int xPos = Y_POS_START;
	int yPos = RENDER_Y_POS_START;

	eng->renderer->drawText("--- Choose character background ---", renderArea_screen, xPos, yPos, clrWhite);
	yPos++;

	for(int i = 0; i < endOfPlayerBackgrounds; i++) {
		string backgroundTitle;

		const PlayerBackgrounds_t currentBackground = static_cast<PlayerBackgrounds_t>(i);

		switch(currentBackground) {
		case playerBackground_soldier: backgroundTitle = "Soldier"; break;
		case playerBackground_occultScholar: backgroundTitle = "Occult scholar"; break;
		case playerBackground_tombRaider: backgroundTitle = "Tomb raider"; break;
		default: {} break;
		}
		const SDL_Color clr = i == CURRENT_SELECTED_POS ? clrRedLight : clrRed;

		eng->renderer->drawText(backgroundTitle , renderArea_screen, xPos, yPos, clr);

		yPos++;
	}

   yPos++;

   eng->renderer->drawText("Bla bla" , renderArea_screen, xPos, yPos, clrRedLight);

   yPos += 2;

	eng->renderer->flip();

	return yPos;
}

//-----------------------------------------------------PLAYER NAME ENTERING
void PlayerEnterName::run(const int RENDER_Y_POS) {
	string name = "";
	bool done = false;
	while(done == false) {
		if(eng->config->BOT_PLAYING == false) {
			readKeys(name, done);
		} else {
			name = "AZATHOTH";
			done = true;
		}
		draw(name, RENDER_Y_POS);
		SDL_Delay(1);
	}

	ActorDefinition& iDef = *(eng->player->getInstanceDefinition());
	ActorDefinition& aDef = *(eng->player->getArchetypeDefinition());
	iDef.name_a = iDef.name_the = aDef.name_a = aDef.name_the = name;
}


void PlayerEnterName::draw(const string& currentString, const int RENDER_Y_POS) {
	eng->renderer->clearAreaWithTextDimensions(renderArea_screen, 0, RENDER_Y_POS, MAP_X_CELLS, RENDER_Y_POS + 1);
	int x0 = 1;
	const string LABEL = "Enter character name: ";
	eng->renderer->drawText(LABEL, renderArea_screen, x0, RENDER_Y_POS, clrWhite);
	x0 += LABEL.size();
	const string NAME_STR = currentString.size() < PLAYER_NAME_MAX_LENGTH ? currentString + "_" : currentString;
	eng->renderer->drawText(NAME_STR, renderArea_screen, x0, RENDER_Y_POS, clrRedLight);
	eng->renderer->flip();
}


void PlayerEnterName::readKeys(string& currentString, bool& done) {
	SDL_Event event;

	while(SDL_PollEvent(&event)) {
		switch(event.type) {
		case SDL_KEYDOWN:
		{
			int key = event.key.keysym.unicode;

			if(key == SDLK_RETURN) {
				done = true;
				return;
			}

			if(currentString.size() < PLAYER_NAME_MAX_LENGTH) {
				if(
				   key == int(' ') ||
				   (key >= int('a') && key <= int('z')) ||
				   (key >= int('A') && key <= int('Z')) ||
				   (key >= int('0') && key <= int('9')))
				{
					currentString.push_back(char(key));
					return;
				}
			}

			if(currentString.size() > 0) {
				if(key == SDLK_BACKSPACE) {
					currentString.erase(currentString.end()-1);
				}
			}
		}
		break;
		}
	}
}
