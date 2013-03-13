#include "PopulateTraps.h"

#include "Engine.h"
#include "Map.h"
#include "MapBuildBSP.h"
#include "FeatureTrap.h"
#include "FeatureFactory.h"

void PopulateTraps::populateRoomAndCorridorLevel(RoomTheme_t themeMap[MAP_X_CELLS][MAP_Y_CELLS], const vector<Room*>& rooms) const {
  const int CHANCE_FOR_ALLOW_TRAPPED_PLAIN_AREAS = 25 + (eng->map->getDungeonLevel() * 5);

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeMoveBlockerArrayForMoveTypeFeaturesOnly(moveType_walk, blockers);

  //Put traps in non-plain rooms
  for(unsigned int i = 0; i < rooms.size(); i++) {
    Room* const room = rooms.at(i);
    const RoomTheme_t theme = room->roomTheme;

    if(theme != roomTheme_plain) {

      int chanceForTrappedRoom = 0;

      switch(theme) {
      case roomTheme_human:
        chanceForTrappedRoom = 25;
        break;
      case roomTheme_ritual:
        chanceForTrappedRoom = 25;
        break;
      case roomTheme_spider:
        chanceForTrappedRoom = 100;
        break;
      case roomTheme_crypt:
        chanceForTrappedRoom = 75;
        break;
      case roomTheme_monster:
        chanceForTrappedRoom = 25;
        break;
      case roomTheme_plain:
        break;
      case roomTheme_dungeon:
        break;
      case endOfRoomThemes:
        break;
      }

      if(eng->dice.getInRange(1, 100) < chanceForTrappedRoom) {

        vector<coord> trapPositionCandidates;

        const coord& x0y0 = room->getX0Y0();
        const coord& x1y1 = room->getX1Y1();
        for(int y = x0y0.y; y <= x1y1.y; y++) {
          for(int x = x0y0.x; x <= x1y1.x; x++) {
            if(blockers[x][y] == false) {
              if(eng->map->featuresStatic[x][y]->canHaveStaticFeature()) {
                trapPositionCandidates.push_back(coord(x, y));
              }
            }
          }
        }

        const int NR_TRAP_POSITION_CANDIDATES = static_cast<int>(trapPositionCandidates.size());
        if(NR_TRAP_POSITION_CANDIDATES > 0) {
          const int MIN_NR_TRAPS = min(NR_TRAP_POSITION_CANDIDATES / 4, theme == roomTheme_spider ? 4 : 1);
          const int MAX_NR_TRAPS = min(NR_TRAP_POSITION_CANDIDATES / 2, theme == roomTheme_spider ? 8 : 4);
          const int NR_TRAPS = eng->dice.getInRange(MIN_NR_TRAPS, MAX_NR_TRAPS);
          for(int i_trap = 0; i_trap < NR_TRAPS; i_trap++) {
            const unsigned int CANDIDATE_ELEMENT = eng->dice.getInRange(0, trapPositionCandidates.size() - 1);
            const coord& pos = trapPositionCandidates.at(CANDIDATE_ELEMENT);
            const Trap_t trapType = theme == roomTheme_spider ? trap_spiderWeb : trap_any;
            FeatureStatic* const f = eng->map->featuresStatic[pos.x][pos.y];
            const FeatureDef* const defAtTrap = eng->featureData->getFeatureDef(f->getId());
            eng->featureFactory->spawnFeatureAt(feature_trap, pos, new TrapSpawnData(defAtTrap, trapType));
            trapPositionCandidates.erase(trapPositionCandidates.begin() + CANDIDATE_ELEMENT);
          }
        }
      }
    }
  }
}
