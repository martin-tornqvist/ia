#include "Marker.h"

#include "Engine.h"

#include "InventoryHandler.h"
#include "ItemWeapon.h"
#include "Input.h"
#include "ActorPlayer.h"
#include "Attack.h"
#include "Log.h"
#include "Look.h"
#include "Thrower.h"
#include "Render.h"

void Marker::readKeys(const MarkerTask_t markerTask) {
	Uint16 key = 0;

	while(SDL_PollEvent(&m_event)) {
		const bool SHIFT = SDL_GetModState() & KMOD_SHIFT;
		const bool CTRL = SDL_GetModState() & KMOD_CTRL;

		switch(m_event.type) {
		case SDL_KEYDOWN: {

			key = static_cast<Uint16>(m_event.key.keysym.sym);

			if(key == SDLK_RIGHT || key == SDLK_6 || key == SDLK_KP6) {
				eng->input->clearKeyEvents();
				if(SHIFT) {
					move(1, -1, markerTask);
				} else {
					if(CTRL) {
						move(1, 1, markerTask);
					} else {
						move(1, 0, markerTask);
					}
				}
			}

			if(key == SDLK_UP || key == SDLK_8 || key == SDLK_KP8) {
				eng->input->clearKeyEvents();
				move(0, -1, markerTask);
			}

			if(key == SDLK_LEFT || key == SDLK_4 || key == SDLK_KP4) {
				eng->input->clearKeyEvents();
				if(SHIFT) {
					move(-1, -1, markerTask);
				} else {
					if(CTRL) {
						move(-1, 1, markerTask);
					} else {
						move(-1, 0, markerTask);
					}
				}
			}

			if(key == SDLK_DOWN || key == SDLK_2 || key == SDLK_KP2) {
				eng->input->clearKeyEvents();
				move(0, 1, markerTask);
			}

			if(key == SDLK_KP9 || key == SDLK_9 || key == SDLK_PAGEUP) {
				move(1, -1, markerTask);
			}

			if(key == SDLK_KP7 || key == SDLK_7 || key == SDLK_HOME) {
				eng->input->clearKeyEvents();
				move(-1, -1, markerTask);
			}

			if(key == SDLK_KP1 || key == SDLK_1 || key == SDLK_END) {
				eng->input->clearKeyEvents();
				move(-1, 1, markerTask);
			}

			if(key == SDLK_KP3 || key == SDLK_3 || key == SDLK_PAGEDOWN) {
				eng->input->clearKeyEvents();
				move(1, 1, markerTask);
			}

			//Fire
			if(key == SDLK_f || key == SDLK_RETURN || key == SDLK_KP_ENTER) {
				eng->input->clearKeyEvents();
				if(markerTask == markerTask_aim) {
					if(m_xPos != eng->player->pos.x || m_yPos != eng->player->pos.y) {

						const Actor* const actor = eng->mapTests->getActorAtPos(coord(m_xPos, m_yPos));
						if(actor != NULL) {
							eng->player->target = actor;
						}

						Weapon* const weapon = dynamic_cast<Weapon*>(eng->player->getInventory()->getItemInSlot(slot_wielded));
						if(eng->attack->ranged(m_xPos, m_yPos, weapon) == false) {
							eng->log->addMessage("No ammunition loaded.");
						}
					} else {
						eng->log->addMessage("I think I can persevere a little longer.");
					}
					done();
				}
			}

			//Extra actor descriptions
			if(key == SDLK_l || key == SDLK_RETURN || key == SDLK_KP_ENTER) {
				eng->input->clearKeyEvents();
				if(markerTask == markerTask_look) {
					eng->look->printExtraActorDescription(coord(m_xPos, m_yPos));
				}
			}

			//Throw missile
			if(key == SDLK_t || key == SDLK_RETURN || key == SDLK_KP_ENTER) {
				eng->input->clearKeyEvents();
				if(markerTask == markerTask_throw) {
					if(m_xPos == eng->player->pos.x && m_yPos == eng->player->pos.y) {
						eng->log->addMessage("Throw it somewhere else.");
					} else {
						const Actor* const actor = eng->mapTests->getActorAtPos(coord(m_xPos, m_yPos));
						if(actor != NULL) {
							eng->player->target = actor;
						}
						eng->thrower->throwMissile(eng->player, coord(m_xPos, m_yPos));
					}

					done();
				}
			}

			//Throw dynamite
			if(key == SDLK_e || key == SDLK_RETURN || key == SDLK_KP_ENTER) {
				if(markerTask == markerTask_throwLitExplosive) {
					eng->input->clearKeyEvents();
					eng->thrower->playerThrowLitExplosive(coord(m_xPos, m_yPos));
					done();
				}
			}

			//Cancel
			if(key == SDLK_ESCAPE || key == SDLK_SPACE) {
				eng->input->clearKeyEvents();
				cancel();
			}

			eng->input->clearKeyEvents();
		}
		default: {
		}
		break;
		}
	}
}

void Marker::drawMarker(const MarkerTask_t markerTask) const {
	vector<coord> trace;
	trace.resize(0);

	int originX = eng->player->pos.x;
	int originY = eng->player->pos.y;

	trace = eng->mapTests->getLine(originX, originY, m_xPos, m_yPos, true, 99999);

	int effectiveRange = -1;

	if(markerTask == markerTask_aim) {
		Weapon* const weapon = dynamic_cast<Weapon*>(eng->player->getInventory()->getItemInSlot(slot_wielded));
		effectiveRange = weapon->effectiveRangeLimit;
	}

	eng->renderer->drawMarker(trace, effectiveRange);
}

void Marker::place(const MarkerTask_t markerTask) {
	m_xPos = eng->player->pos.x;
	m_yPos = eng->player->pos.y;

	if(markerTask == markerTask_aim || markerTask == markerTask_look || markerTask == markerTask_throw) {
		//Attempt to place marker at target.
		if(setCoordToTargetIfVisible() == false) {
			//Else NULL the target, and attempt to place marker at closest visible enemy.
			//This sets a new target if successful.
			eng->player->target = NULL;
			setCoordToClosestEnemyIfVisible();
		}
	}

	if(markerTask == markerTask_look) {
		eng->look->markerAtCoord(coord(m_xPos, m_yPos));
	}

	drawMarker(markerTask);

	m_done = false;
	while(m_done == false) {
		readKeys(markerTask);

//		if(m_done == false) {
//			drawMarker(markerTask);
//		}

		SDL_Delay(1);
	}
}

void Marker::setCoordToClosestEnemyIfVisible() {
	eng->player->getSpotedEnemiesPositions();

	//If player sees enemies, suggest one for targeting
	const vector<coord>& positions = eng->player->spotedEnemiesPositions;
	if(positions.size() != 0) {
		coord pos = eng->mapTests->getClosestPos(eng->player->pos, positions);

		m_xPos = pos.x;
		m_yPos = pos.y;

		const Actor* const actor = eng->mapTests->getActorAtPos(pos);
		eng->player->target = actor;
	}
}

bool Marker::setCoordToTargetIfVisible() {
	const Actor* const target = eng->player->target;

	if(target != NULL) {
		eng->player->getSpotedEnemies();

		const vector<Actor*>& spotedEnemies = eng->player->spotedEnemies;
		if(spotedEnemies.size() != 0) {

			for(unsigned int i = 0; i < spotedEnemies.size(); i++) {
				if(target == spotedEnemies.at(i)) {
					m_xPos = spotedEnemies.at(i)->pos.x;
					m_yPos = spotedEnemies.at(i)->pos.y;
					return true;
				}
			}
		}
	}
	return false;
}

void Marker::move(const int xDif, const int yDif, const MarkerTask_t markerTask) {
	if(eng->mapTests->isCellInsideMainScreen(m_xPos + xDif, m_yPos + yDif)) {
		m_xPos += xDif;
		m_yPos += yDif;

		if(markerTask == markerTask_look) {
			eng->look->markerAtCoord(coord(m_xPos, m_yPos));
		}
	}
	drawMarker(markerTask);
}

void Marker::done() {
	eng->renderer->drawMapAndInterface();
	m_done = true;
}

void Marker::cancel() {
	eng->renderer->clearRenderArea(renderArea_screen);
	eng->renderer->drawMapAndInterface();
	m_done = true;
}

