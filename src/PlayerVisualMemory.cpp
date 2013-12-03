#include "PlayerVisualMemory.h"

#include "Engine.h"
#include "Map.h"
#include "Renderer.h"

void PlayerVisualMemory::updateVisualMemory() {
  if(eng->config->isTilesMode) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      for(int y = 0; y < MAP_Y_CELLS; y++) {
        eng->map->cells[x][y].playerVisualMemoryTiles =
          eng->renderer->renderArrayActorsOmittedTiles[x][y];
      }
    }
  } else {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      for(int y = 0; y < MAP_Y_CELLS; y++) {
        eng->map->cells[x][y].playerVisualMemoryAscii =
          eng->renderer->renderArrayActorsOmittedAscii[x][y];
      }
    }
  }

}

