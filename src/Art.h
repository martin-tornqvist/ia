#ifndef ART_H
#define ART_H

const unsigned FONT_ARRAY_X_CELLS = 16;
const unsigned FONT_ARRAY_Y_CELLS = 16;

enum Tile_t {
	tile_empty,
	tile_playerFirearm,
	tile_playerMelee,
	tile_zombieUnarmed,
	tile_zombieArmed,
	tile_zombieBloated,
	tile_cultistFirearm,
	tile_cultistDagger,
	tile_witchOrWarlock,
	tile_ghoul,
	tile_mummy,
	tile_deepOne,
	tile_armor,
	tile_potion,
	tile_ammo,
	tile_scroll,
	tile_rat,
	tile_spider,
	tile_wolf,
	tile_phantasm,
	tile_ratThing,
	tile_hound,
	tile_bat,
	tile_byakhee,
	tile_massOfWorms,
	tile_ooze,
	tile_fireVampire,
	tile_ghost,
	tile_wraith,
	tile_pistol,
	tile_tommyGun,
	tile_shotgun,
	tile_dynamite,
	tile_dynamiteLit,
	tile_molotov,
	tile_incinerator,
	tile_teslaCannon,
	tile_flare,
	tile_flareGun,
	tile_flareLit,
	tile_dagger,
	tile_axe,
	tile_club,
	tile_hammer,
	tile_machete,
	tile_pitchfork,
	tile_sledgeHammer,
	tile_rock,
	tile_ironSpike,
	tile_huntingHorror,
	tile_spiderLeng,
	tile_migo,
	tile_floor,
	tile_aimMarkerHead,
	tile_aimMarkerTrail,
	tile_wall,
	tile_wallFront,
	tile_squareCheckered,
	tile_rubbleHigh,
	tile_rubbleLow,
	tile_stairsDown,
  tile_lever_left,
  tile_lever_right,
	tile_brazier,
	tile_altar,
	tile_spiderWeb,
	tile_doorClosed,
	tile_doorOpen,
	tile_doorBroken,
	tile_tree,
	tile_bush,
	tile_churchBench,
	tile_graveStone,
	tile_tomb,
	tile_water1,
	tile_water2,
	tile_trapGeneral,
	tile_cocoon,
	tile_blastAnimation1,
	tile_blastAnimation2,
	tile_corpse,
	tile_projectileStandardFrontSlash,
	tile_projectileStandardBackSlash,
	tile_projectileStandardDash,
	tile_projectileStandardVerticalBar,
	tile_gore1,
	tile_gore2,
	tile_gore3,
	tile_gore4,
	tile_gore5,
	tile_gore6,
	tile_gore7,
	tile_gore8,
	tile_smoke,
	tile_trapezohedron,
	tile_pit
};

class Engine;
struct coord;

class Art {
public:
	coord getGlyphCoords(const char glyph, Engine* engine);
	coord getTileCoords(const Tile_t tile, Engine* engine);

private:
	coord getColumnAndRow(const char glyph);
	coord getColumnAndRowTile(const Tile_t tile);
};

#endif
