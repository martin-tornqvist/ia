#ifndef CONST_DUNGEON_SETTINGS_H
#define CONST_DUNGEON_SETTINGS_H

const unsigned int PLAYER_NAME_MAX_LENGTH = 12;

const int MAP_X_CELLS = 80;
const int MAP_Y_CELLS = 26;//22;
const int MAP_X_CELLS_HALF = MAP_X_CELLS / 2;
const int MAP_Y_CELLS_HALF = MAP_Y_CELLS / 2;

//Maximum player experience level
const int PLAYER_MAX_LEVEL = 30;//10;

const int FIRST_CAVERN_LEVEL = 15;
const int LAST_CAVERN_LEVEL = 25;

const int FOV_RADI_INT = 6;
const int FOV_WIDTH_INT = 2 * FOV_RADI_INT + 1;

const double FOV_RADI = static_cast<const double>(FOV_RADI_INT);
const double FOV_WIDTH = static_cast<const double>(FOV_WIDTH_INT);

//Base delay for actor actions (this is divided by speed attribute)
//const float BASE_DELAY = 100.0;

//How many skills to allocate per key press
const int PLAYER_ALLOC_SKILLS_BATCH_SIZE = 10;

//How many turns to explosion from lighting a stick of dynamite
const int DYNAMITE_FUSE_TURNS = 5;


const int NUMBER_OF_TRIES_TO_BUILD_FROM_JUNCTIONS = 10;

const int NUMBER_OF_MACHINEGUN_PROJECTILES_PER_BURST = 5;
const int NUMBER_OF_CELLJUMPS_BETWEEN_MACHINEGUN_PROJECTILES = 2;


const int CHANCE_TO_DESTROY_COMMON_ITEMS_ON_DROP = 50;

#endif
