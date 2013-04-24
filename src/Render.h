#ifndef DRAW_MAINSCREEN_H
#define DRAW_MAINSCREEN_H

#include <vector>
#include <iostream>

#include "SDL/SDL_video.h"

#include "ConstTypes.h"
#include "GameTime.h"
#include "Config.h"
#include "Art.h"

class Engine;
struct Projectile;

enum RenderArea_t {
  renderArea_screen,
  renderArea_mainScreen,
  renderArea_log,
  renderArea_characterLines
};

class Renderer {
public:
  Renderer(Engine* engine);

  ~Renderer();

  void drawMapAndInterface(const bool UPDATE_SCREEN = true);

  void updateScreen();

  void clearScreen();

  void          drawText(const string& str, const RenderArea_t renderArea,
                         const coord& pos, const SDL_Color& clr);

  inline void   drawText(const string& str, const RenderArea_t renderArea,
                         const int X, const int Y, const SDL_Color& clr) {
    drawText(str, renderArea, coord(X, Y), clr);
  }

  int           drawTextCentered(const string& str, const RenderArea_t renderArea,
                                 const coord& pos, const SDL_Color& clr,
                                 const bool IS_PIXEL_POS_ADJ_ALLOWED = true);

  inline int    drawTextCentered(const string& str, const RenderArea_t renderArea,
                                 const int X, const int Y, const SDL_Color& clr,
                                 const bool IS_PIXEL_POS_ADJ_ALLOWED = true) {
    return drawTextCentered(str, renderArea, coord(X, Y), clr,
                            IS_PIXEL_POS_ADJ_ALLOWED);
  }

  void          drawCharacter(const char CHARACTER, const RenderArea_t renderArea,
                              const coord& pos, const SDL_Color& clr,
                              const bool drawBgClr = false,
                              const SDL_Color& bgClr = clrBlue);

  inline void   drawCharacter(const char CHARACTER, const RenderArea_t renderArea,
                              const int X, const int Y, const SDL_Color& clr,
                              const bool drawBgClr = false,
                              const SDL_Color& bgClr = clrBlue) {
    drawCharacter(CHARACTER, renderArea, coord(X, Y), clr, drawBgClr, bgClr);
  }

  void drawTileInMap(const Tile_t tile, const coord& pos, const SDL_Color& clr,
                     const bool drawBgClr = false, const SDL_Color& bgClr = clrBlue);

  void drawTileInScreen(const Tile_t tile, const coord& pos, const SDL_Color& clr,
                        const bool drawBgClr = false, const SDL_Color& bgClr = clrBlue);

  void drawGlyphInMap(const char GLYPH, const coord& pos, const SDL_Color& clr,
                      const bool drawBgClr = false, const SDL_Color& bgClr = clrBlue);

  void coverCellInMap(const coord& pos);

  void coverRenderArea(const RenderArea_t renderArea);

  void coverArea(const RenderArea_t renderArea, const coord& pos,
                 const int W, const int H);

  void coverAreaPixel(const coord& pos, const coord& dim);

  void drawRectangleSolid(const coord& pixelPos, const coord& pixelDim,
                          const SDL_Color& clr);

  void drawLineHorizontal(const coord& pos, const int W, const SDL_Color& clr);

  void drawLineVertical(const coord& pos, const int H, const SDL_Color& clr);

  void drawMarker(const vector<coord>& trace, const int EFFECTIVE_RANGE = -1);

  void drawBlastAnimationAtField(const coord& center, const int RADIUS,
                                 bool forbiddenCells[MAP_X_CELLS][MAP_Y_CELLS],
                                 const SDL_Color& colorInner,
                                 const SDL_Color& colorOuter, const int DURATION);

  void drawBlastAnimationAtPositions(const vector<coord>& positions,
                                     const SDL_Color& color, const int DURATION);

  void drawBlastAnimationAtPositionsWithPlayerVision(const vector<coord>& positions,
      const SDL_Color& clr, const int EXPLOSION_DELAY_FACTOR, Engine* const engine);

  void drawMainMenuLogo(const int Y_POS);

  CellRenderDataAscii renderArrayActorsOmittedAscii[MAP_X_CELLS][MAP_Y_CELLS];
  CellRenderDataTiles renderArrayActorsOmittedTiles[MAP_X_CELLS][MAP_Y_CELLS];

  CellRenderDataAscii renderArrayAscii[MAP_X_CELLS][MAP_Y_CELLS];
  CellRenderDataTiles renderArrayTiles[MAP_X_CELLS][MAP_Y_CELLS];

  void initAndClearPrev();

  void drawProjectilesAndUpdateWindow(vector<Projectile*>& projectiles);

private:
  friend class Postmortem;
  friend class Input;
  void drawCharacterAtPixel(const char CHARACTER, const coord& pixelPos, const SDL_Color& clr,
                            const bool drawBgClr = false, const SDL_Color& bgClr = clrBlue);

  void coverCharAtPixel(const coord& pixelPos);
  void coverTileAtPixel(const coord& pixelPos);

  coord getPixelPosForCharacter(const RenderArea_t renderArea, const coord& pos);

  int getLifebarLength(const Actor& actor) const;
  void drawLifeBar(const int X_CELL, const int Y_CELL, const int LENGTH);

  void loadFont();
  void loadTiles();
  void loadMainMenuLogo();

  void freeAssets();

  void drawAscii();
  void drawTiles();

  Engine* eng;

  friend class Query;
  SDL_Surface* surfaceScreen;

  SDL_Surface* surfacesFont_[FONT_SHEET_X_CELLS][FONT_SHEET_Y_CELLS];
  SDL_Surface* surfacesTiles_[TILE_SHEET_X_CELLS][TILE_SHEET_X_CELLS];
  SDL_Surface* surfaceMainMenuLogo_;
};

#endif

