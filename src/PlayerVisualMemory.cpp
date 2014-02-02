#include "PlayerVisualMemory.h"

#include "Engine.h"
#include "Map.h"
#include "Renderer.h"

void PlayerVisualMemory::updateVisualMemory() {
  for(int x = 0; x < MAP_W; x++) {
    for(int y = 0; y < MAP_H; y++) {
      eng.map->cells[x][y].playerVisualMemory =
        eng.renderer->renderArrayNoActors[x][y];
    }
  }
}

