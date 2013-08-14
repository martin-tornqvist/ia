#ifndef DRAW_MAINSCREEN_H
#define DRAW_MAINSCREEN_H

#include <vector>
#include <iostream>

#include "SDL/SDL_video.h"

#include "Panel.h"
#include "CommonData.h"
#include "GameTime.h"
#include "Config.h"
#include "Art.h"

class Engine;
struct Projectile;

class Renderer {
public:
  Renderer(Engine* engine);

  ~Renderer();

  void drawMapAndInterface(const bool UPDATE_SCREEN = true);

  inline void updateScreen() {SDL_Flip(screenSurface_);}

  inline void clearScreen() {
    SDL_FillRect(screenSurface_, NULL,
                 SDL_MapRGB(screenSurface_->format, 0, 0, 0));
  }

  void initAndClearPrev();

  void drawTile(const Tile_t tile, const Panel_t panel, const Pos& pos,
                const SDL_Color& clr,
                const SDL_Color& bgClr = clrBlack);

  void drawGlyph(const char GLYPH, const Panel_t panel, const Pos& pos,
                 const SDL_Color& clr,
                 const bool DRAW_BG_CLR = true,
                 const SDL_Color& bgClr = clrBlack);

  void drawText(const string& str, const Panel_t panel,
                const Pos& pos, const SDL_Color& clr,
                const SDL_Color& bgClr = clrBlack);

  int drawTextCentered(const string& str, const Panel_t panel,
                       const Pos& pos, const SDL_Color& clr,
                       const SDL_Color& bgClr = clrBlack,
                       const bool IS_PIXEL_POS_ADJ_ALLOWED = true);

  void coverCellInMap(const Pos& pos);

  void coverPanel(const Panel_t panel);

  void coverArea(const Panel_t panel, const Pos& pos, const Pos& dims);

  void coverAreaPixel(const Pos& pixelPos, const Pos& pixelDims);

  void drawRectangleSolid(const Pos& pixelPos, const Pos& pixelDims,
                          const SDL_Color& clr);

  void drawLineHorizontal(const Pos& pixelPos, const int W,
                          const SDL_Color& clr);

  void drawLineVertical(const Pos& pixelPos, const int H,
                        const SDL_Color& clr);

  void drawMarker(const vector<Pos>& trace, const int EFFECTIVE_RANGE = -1);

  void drawBlastAnimationAtField(const Pos& center, const int RADIUS,
                                 bool forbiddenCells[MAP_X_CELLS][MAP_Y_CELLS],
                                 const SDL_Color& colorInner,
                                 const SDL_Color& colorOuter, const int DURATION);

  void drawBlastAnimationAtPositions(const vector<Pos>& positions,
                                     const SDL_Color& color, const int DURATION);

  void drawBlastAnimationAtPositionsWithPlayerVision(const vector<Pos>& positions,
      const SDL_Color& clr);

  void drawMainMenuLogo(const int Y_POS);

  CellRenderDataAscii renderArrayActorsOmittedAscii[MAP_X_CELLS][MAP_Y_CELLS];
  CellRenderDataTiles renderArrayActorsOmittedTiles[MAP_X_CELLS][MAP_Y_CELLS];

  CellRenderDataAscii renderArrayAscii[MAP_X_CELLS][MAP_Y_CELLS];
  CellRenderDataTiles renderArrayTiles[MAP_X_CELLS][MAP_Y_CELLS];

  void drawProjectiles(vector<Projectile*>& projectiles);

  void drawPopupBox(const Rect& area, const Panel_t panel,
                    const SDL_Color& clr = clrGray);

private:
  friend class Postmortem;
  friend class Input;
  friend class Config;
  void applySurface(const Pos& pixelPos, SDL_Surface* const src,
                    SDL_Rect* clip = NULL);

  void drawGlyphAtPixel(const char GLYPH, const Pos& pixelPos,
                        const SDL_Color& clr, const bool DRAW_BG_CLR = true,
                        const SDL_Color& bgClr = clrBlack);

  void coverGlyphAtPixel(const Pos& pixelPos);
  void coverTileAtPixel(const Pos& pixelPos);

  Pos getPixelPosForCellInPanel(const Panel_t panel, const Pos& pos);

  int getLifebarLength(const Actor& actor) const;
  void drawLifeBar(const Pos& pos, const int LENGTH);

  void putPixelsOnScreenForTile(const Tile_t tile, const Pos& pixelPos,
                                const SDL_Color& clr);

  void putPixelsOnScreenForGlyph(const char GLYPH, const Pos& pixelPos,
                                 const SDL_Color& clr);

  Uint32 getpixel(SDL_Surface* const surface, const Pos& pos);
  void putpixel(SDL_Surface* const surface, const Pos& pos, Uint32 pixel);

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

