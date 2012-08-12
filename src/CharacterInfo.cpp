#include "CharacterInfo.h"

#include "Engine.h"
#include "PlayerBonuses.h"
#include "ActorPlayer.h"
#include "Render.h"

void CharacterInfo::makeLines() {
	lines.resize(0);

	const string offsetSpaces = "  ";
	const SDL_Color colorHeader = clrCyanLight;

	lines.push_back(StringAndColor(" ", clrRedLight));

	lines.push_back(StringAndColor("Skills", colorHeader));
	const int NR_BONUSES = eng->playerBonusHandler->getNrOfBonuses();
	for(int i = 0; i < NR_BONUSES; i++) {
		const int CURRENT_RANKS = eng->playerBonusHandler->getBonusRankAt(i);
		if(CURRENT_RANKS > 0) {
			const string currentTitle = eng->playerBonusHandler->getBonusTitleAt(i);
			lines.push_back(StringAndColor(offsetSpaces + currentTitle + " (" + intToString(CURRENT_RANKS) + ")", clrRedLight));
		}
	}

	lines.push_back(StringAndColor(" ", clrRedLight));

	lines.push_back(StringAndColor("Mental conditions", colorHeader));
	const int NR_LINES_BEFORE_MENTAL = lines.size();
	if(eng->player->insanityPhobias[insanityPhobia_closedPlace])
		lines.push_back(StringAndColor(offsetSpaces + "I have a phobia of enclosed spaces", clrRedLight));
	if(eng->player->insanityPhobias[insanityPhobia_dog])
		lines.push_back(StringAndColor(offsetSpaces + "I have a phobia of dogs", clrRedLight));
	if(eng->player->insanityPhobias[insanityPhobia_rat])
		lines.push_back(StringAndColor(offsetSpaces + "I have a phobia of rats", clrRedLight));
	if(eng->player->insanityPhobias[insanityPhobia_undead])
		lines.push_back(StringAndColor(offsetSpaces + "I have a phobia of the dead", clrRedLight));
	if(eng->player->insanityPhobias[insanityPhobia_openPlace])
		lines.push_back(StringAndColor(offsetSpaces + "I have a phobia of open places", clrRedLight));
	if(eng->player->insanityPhobias[insanityPhobia_spider])
		lines.push_back(StringAndColor(offsetSpaces + "I have a phobia of spiders", clrRedLight));
	if(eng->player->insanityPhobias[insanityPhobia_deepPlaces])
		lines.push_back(StringAndColor(offsetSpaces + "I have a phobia of deep places", clrRedLight));

	if(eng->player->insanityCompulsions[insanityCompulsion_masochism])
		lines.push_back(StringAndColor(offsetSpaces + "I have a masochistic compulsion", clrRedLight));
	if(eng->player->insanityCompulsions[insanityCompulsion_sadism])
		lines.push_back(StringAndColor(offsetSpaces + "I have a sadistic compulsion", clrRedLight));
	const int NR_LINES_AFTER_MENTAL = lines.size();

	if(NR_LINES_BEFORE_MENTAL == NR_LINES_AFTER_MENTAL) {
		lines.push_back(StringAndColor(offsetSpaces + "I have no mental conditions", clrRedLight));
	}

	lines.push_back(StringAndColor(" ", clrRedLight));

	//TODO With the current implementation below, it's possible to know what each scroll, potion, etc
	//does, just by the order they are listed. If object knowledge should be included, the order they
	//are listed needs to be randomized
//    lines.push_back(StringAndColor("Object knowledge", colorHeader));
//    for(unsigned int i = 1; i < endOfItemDevNames; i++) {
//        const ItemDefinition* const d = eng->itemData->itemDefinitions[i];
//        if(d->abilityToIdentify != ability_empty) {
//            lines.push_back(StringAndColor(offsetSpaces + d->name.name, d->color));
//        }
//    }
//
//    lines.push_back(StringAndColor(" ", clrRedLight));
}

void CharacterInfo::drawInterface() {
	const string decorationLine(MAP_X_CELLS - 2, '-');

	eng->renderer->clearAreaWithTextDimensions(renderArea_screen, 0, 1, MAP_X_CELLS, 2);
	eng->renderer->drawText(decorationLine, renderArea_screen, 1, 1, clrWhite);

	eng->renderer->drawText("Displaying character info", renderArea_screen, 3, 1, clrWhite);

	eng->renderer->drawText(decorationLine, renderArea_characterLines, 1, 1, clrWhite);

	eng->renderer->drawText(" [2/8, Down/Up] to navigate  [Space/Esc] to exit. ", renderArea_characterLines, 3, 1, clrWhite);
}

void CharacterInfo::run() {
	makeLines();

	eng->renderer->clearRenderArea(renderArea_screen);

	StringAndColor currentLine;

	int topElement = 0;
	int btmElement = min(topElement + MAP_Y_CELLS - 1, static_cast<int>(lines.size()) - 1);

	drawInterface();

	int yCell = 2;
	for(int i = topElement; i <= btmElement; i++) {
		eng->renderer->drawText(lines.at(i).str , renderArea_screen, 1, yCell, lines.at(i).color);
		yCell++;
	}

	eng->renderer->flip();

	//Read keys
	SDL_Event event;
	bool done = false;
	while(done == false) {
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
			case SDL_KEYDOWN: {
				int key = event.key.keysym.sym;

				switch(key) {
				case SDLK_2:
				case SDLK_KP2:
				case SDLK_DOWN: {
					topElement = max(0, min(topElement + static_cast<int>(MAP_Y_CELLS / 5), static_cast<int>(lines.size())
					                        - static_cast<int>(MAP_Y_CELLS)));

					btmElement = min(topElement + MAP_Y_CELLS - 1, static_cast<int>(lines.size()) - 1);

					eng->renderer->clearAreaWithTextDimensions(renderArea_screen, 0, 2, MAP_X_CELLS, MAP_Y_CELLS);

					drawInterface();

					yCell = 2;

					for(int i = topElement; i <= btmElement; i++) {
						eng->renderer->drawText(lines.at(i).str , renderArea_screen, 1, yCell, lines.at(i).color);
						yCell++;
					}
					eng->renderer->flip();
				}
				break;
				case SDLK_8:
				case SDLK_KP8:
				case SDLK_UP: {
					topElement = max(0, min(topElement - static_cast<int>(MAP_Y_CELLS / 5), static_cast<int>(lines.size())
					                        - static_cast<int>(MAP_Y_CELLS)));

					btmElement = min(topElement + MAP_Y_CELLS - 1, static_cast<int>(lines.size()) - 1);

					eng->renderer->clearAreaWithTextDimensions(renderArea_screen, 0, 2, MAP_X_CELLS, MAP_Y_CELLS);

					drawInterface();

					yCell = 2;

					for(int i = topElement; i <= btmElement; i++) {
						eng->renderer->drawText(lines.at(i).str , renderArea_screen, 1, yCell, lines.at(i).color);
						yCell++;
					}
					eng->renderer->flip();
				}
				break;
				case SDLK_SPACE:
				case SDLK_ESCAPE: {
					done = true;
				}
				break;

				}
			}
			break;
			default: {
			}
			break;
			}
		}
		SDL_Delay(1);
	}

	eng->renderer->clearRenderArea(renderArea_screen);
	eng->renderer->drawMapAndInterface();
}
