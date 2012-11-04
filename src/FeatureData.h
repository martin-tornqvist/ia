#ifndef FEATURE_DATA_H
#define FEATURE_DATA_H

#include <vector>
#include <string>
#include <iostream>
#include "SDL.h"

#include "Art.h"
#include "ConstTypes.h"

using namespace std;

class Engine;

enum Feature_t {
  feature_empty,
  feature_stoneFloor,
  feature_stoneWall,
  feature_stoneWallSlimy,
  feature_tree,
  feature_grass,
  feature_grassWithered,
  feature_bush,
  feature_bushWithered,
  feature_forestPath,
  feature_stairsDown,
  feature_lever,
  feature_brazierGolden,
  feature_caveWall,
  feature_caveFloor,
  feature_gravestone,
  feature_tomb,
  feature_churchBench,
  feature_altar,
  feature_churchCarpet,
  feature_rubbleHigh,
  feature_rubbleLow,
  feature_statue,
  feature_ghoulStatue,
  feature_cocoon,
  feature_chasm,
  feature_shallowWater,
  feature_deepWater,
  feature_poolBlood,
  feature_shallowMud,
  //	feature_poolAcid,

  feature_door,
  feature_litDynamite,
  feature_litFlare,
  feature_trap,
  feature_trashedSpiderWeb,
  feature_gore,
  feature_smoke,
  feature_proxEventWallCrumble,

  feature_pit,

  endOfFeatures
};

enum FeatureSpawnType_t {
  featureSpawnType_mob, featureSpawnType_static, featureSpawnType_other
};

enum MaterialType_t {
  materialType_empty, materialType_soft, materialType_hard, materialType_fluid
};

struct FeatureDef {
  Feature_t id;
  FeatureSpawnType_t spawnType;
  char glyph;
  SDL_Color color;
  Tile_t tile;
  bool isMovePassable[endOfMoveType];
  bool isShootPassable;
  bool isVisionPassable;
  bool isSmokePassable;
  bool canHaveBlood;
  bool canHaveGore;
  bool canHaveCorpse;
  bool canHaveStaticFeature;
  bool canHaveItem;
  bool isBottomless;
  MaterialType_t materialType;
  string name_a;
  string name_the;
  string messageOnPlayerBlocked;
  string messageOnPlayerBlockedBlind;
  int dodgeModifier;
  int shockWhenAdjacent;
  vector<Feature_t> featuresOnDestroyed;
};

class FeatureData {
public:
  FeatureData(Engine* engine) :
    eng(engine) {
    makeList();
  }
  ~FeatureData() {
  }
  const FeatureDef* getFeatureDef(const Feature_t id) const {
    return &(featureDefs[id]);
  }
  void makeList();
private:
  void addToListAndReset(FeatureDef& d);
  FeatureDef featureDefs[endOfFeatures];
  void resetDef(FeatureDef& d);
  Engine* eng;
};

#endif
