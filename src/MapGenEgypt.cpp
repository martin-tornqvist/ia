#include "MapGen.h"

#include <vector>

#include "ActorPlayer.h"
#include "ActorFactory.h"
#include "ActorMonster.h"
#include "Map.h"
#include "Utils.h"
#include "ActorFactory.h"
#include "FeatureRigid.h"

using namespace std;

namespace MapGen {

namespace Egypt {

bool run() {
  Map::resetMap();

  const MapTempl& templ     = MapTemplHandling::getTempl(MapTemplId::egypt);
  const Pos       templDims = templ.getDims();
  const int       STAIR_VAL = Rnd::range(1, 2);

  for(int y = 0; y < templDims.y; ++y) {
    for(int x = 0; x < templDims.x; ++x) {
      const auto& templCell = templ.getCell(x, y);
      const Pos p(x, y);
      if(templCell.featureId != FeatureId::empty) {
        if(templCell.val == STAIR_VAL) {
          Map::put(new Stairs(p));
        } else {
          const auto& d = FeatureData::getData(templCell.featureId);
          Map::put(static_cast<Rigid*>(d.mkObj(p)));
        }
      }
      if(templCell.actorId != ActorId::empty) {
        ActorFactory::mk(templCell.actorId, p);
      }
      if(templCell.val == 3) {
        Map::player->pos = p;
      }
    }
  }

  for(int y = 0; y < MAP_H; ++y) {
    for(int x = 0; x < MAP_W; ++x) {
      Rigid* const f = Map::cells[x][y].rigid;
      if(f->getId() == FeatureId::wall) {
        static_cast<Wall*>(f)->type_ = WallType::egypt;
      }
    }
  }

  return true;
}

} //Egypt

} //MapGen
