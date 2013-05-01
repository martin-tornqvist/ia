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

  inline void updateScreen() {SDL_Flip(screenSurface_);}

  inline void clearScreen() {
    SDL_FillRect(screenSurface_, NULL, SDL_MapRGB(screenSurface_->format, 0, 0, 0));
  }

  void initAndClearPrev();

  void drawTileInMap(const Tile_t tile, const int X, const int Y, const SDL_Color& clr,
                      const SDL_Color& bgClr = clrBlack);
  inline void drawTileInMap(const Tile_t tile, const coord& pos, const SDL_Color& clr,
                            const SDL_Color& bgClr = clrBlack) {
    drawTileInMap(tile, pos.x, pos.y, clr, bgClr);
  }

  void drawTileInScreen(const Tile_t tile, const int X, const int Y,
                        const SDL_Color& clr, const SDL_Color& bgClr = clrBlack);
  inline void drawTileInScreen(const Tile_t tile, const coord& pos,
                               const SDL_Color& clr,
                               const SDL_Color& bgClr = clrBlack) {
    drawTileInMap(tile, pos.x, pos.y , clr, bgClr);
  }

  void drawGlyphInMap(const char GLYPH, const int X, const int Y, const SDL_Color& clr,
                      const bool DRAW_BG_CLR = true, const SDL_Color& bgClr = clrBlack);
  inline void drawGlyphInMap(const char GLYPH, const coord& pos, const SDL_Color& clr,
                             const bool DRAW_BG_CLR = true,
                             const SDL_Color& bgClr = clrBlack) {
    drawGlyphInMap(GLYPH, pos, clr, DRAW_BG_CLR, bgClr);
  }

  void drawText(const string& str, const RenderArea_t renderArea,
                const int X, const int Y, const SDL_Color& clr,
                const SDL_Color& bgClr = clrBlack);
  inline void drawText(const string& str, const RenderArea_t renderArea,
                       const coord& pos, const SDL_Color& clr,
                       const SDL_Color& bgClr = clrBlack) {
    drawText(str, renderArea, pos.x, pos.y, clr, bgClr);
  }

  int drawTextCentered(const string& str, const RenderArea_t renderArea,
                       const int X, const int Y, const SDL_Color& clr,
                       const SDL_Color& bgClr = clrBlack,
                       const bool IS_PIXEL_POS_ADJ_ALLOWED = true);
  inline int drawTextCentered(const string& str, const RenderArea_t renderArea,
                              const coord& pos, const SDL_Color& clr,
                              const SDL_Color& bgClr,
                              const bool IS_PIXEL_POS_ADJ_ALLOWED = true) {
    return drawTextCentered(str, renderArea, pos.x, pos.y, clr, bgClr,
                            IS_PIXEL_POS_ADJ_ALLOWED);
  }

  void drawCharacter(const char CHARACTER, const RenderArea_t renderArea,
                     const int X, const int Y, const SDL_Color& clr,
                     const bool DRAW_BG_CLR = true,
                     const SDL_Color& bgClr = clrBlack);
  inline void drawCharacter(const char CHARACTER, const RenderArea_t renderArea,
                            const coord& pos, const SDL_Color& clr,
                            const bool DRAW_BG_CLR = true ,
                            const SDL_Color& bgClr = clrBlack) {
    drawCharacter(CHARACTER, renderArea, pos.x, pos.y, clr, DRAW_BG_CLR, bgClr);
  }

  void coverCellInMap(const int X, const int Y);
  inline void coverCellInMap(const coord& pos) {
    coverCellInMap(pos.x, pos.y);
  }

  void coverRenderArea(const RenderArea_t renderArea);

  void coverArea(const RenderArea_t renderArea, const int X, const int Y,
                 const int W, const int H);
  inline void coverArea(const RenderArea_t renderArea, const coord& pos,
                        const coord& dims) {
    coverArea(renderArea, pos.x, pos.y, dims.x, dims.y);
  }

  void coverAreaPixel(const int X, const int Y, const int W, const int H);

  void drawRectangleSolid(const int X, const int Y, const int W, const int H,
                          const SDL_Color& clr);

  void drawLineHorizontal(const int X, const int Y, const int W, const SDL_Color& clr);

  void drawLineVertical(const int X, const int Y, const int H, const SDL_Color& clr);

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

  void drawProjectilesAndUpdateWindow(vector<Projectile*>& projectiles);

private:
  friend class Postmortem;
  friend class Input;
  void applySurface(const int PIXEL_X, const int PIXEL_Y, SDL_Surface* const source,
                    SDL_Rect* clip = NULL);

  void drawCharacterAtPixel(const char CHARACTER, const int X, const int Y,
                            const SDL_Color& clr, const bool DRAW_BG_CLR = true,
                            const SDL_Color& bgClr = clrBlack);

  void coverCharAtPixel(const int X, const int Y);
  void coverTileAtPixel(const int X, const int Y);

  coord getPixelPosForCharacter(const RenderArea_t renderArea, const int X, const int Y);

  int getLifebarLength(const Actor& actor) const;
  void drawLifeBar(const int X_CELL, const int Y_CELL, const int LENGTH);

  void putPixelsOnScreenForTile(const Tile_t tile,
                                const int PIXEL_X,
                                const int PIXEL_Y,
                                const SDL_Color& clr);

  void putPixelsOnScreenForGlyph(const char GLYPH,
                                 const int PIXEL_X,
                                 const int PIXEL_Y,
                                 const SDL_Color& clr);

  Uint32 getpixel(SDL_Surface* const surface, const int X, const int Y);
  void putpixel(SDL_Surface* const surface, const int X, const int Y, Uint32 pixel);

  bool tilePixelData_[400][400];
  bool fontPixelData_[400][400];

  void loadFont();
  void loadTiles();
  void loadMainMenuLogo();

  void freeAssets();

  void drawAscii();
  void drawTiles();

  Engine* eng;

  SDL_Surface* screenSurface_;
  SDL_Surface* mainMenuLogoSurface_;
};

#endif

