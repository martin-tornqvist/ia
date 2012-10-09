#ifndef DRAW_MAINSCREEN_H
#define DRAW_MAINSCREEN_H

#include <vector>
#include <iostream>

#include "SDL/SDL.h"

#include "ConstTypes.h"
#include "GameTime.h"
#include "Config.h"
#include "Art.h"

class Engine;

enum RenderArea_t {
  renderArea_screen, renderArea_mainScreen, renderArea_log, renderArea_characterLines
};

class Renderer {
public:
  Renderer(Engine* engine);

  ~Renderer() {
    SDL_FreeSurface(m_glyphSheet);
  }

  void drawMapAndInterface(const bool FLIP_SCREEN = true);

  void drawMarker(vector<coord> &trace, const int EFFECTIVE_RANGE = -1);

  void drawCharacter(const char CHARACTER, const RenderArea_t renderArea, const int X, const int Y, const SDL_Color clr);
  void drawText(const string str, const RenderArea_t renderArea, const int X, const int Y, const SDL_Color clr);
  void drawTextCentered(const string str, const RenderArea_t renderArea, const int X, const int Y, const SDL_Color clr);
  void drawTileInMap(const Tile_t tile, const int X, const int Y, const SDL_Color clr);

  void clearCellInMap(const int X, const int Y);
  void clearRenderArea(const RenderArea_t renderArea);
  void clearAreaWithTextDimensions(const RenderArea_t renderArea, const int X, const int Y, const int W, const int H);
  void clearAreaPixel(const int X, const int Y, const int W, const int H);

  void drawRectangle(const int x, const int y, const int w, const int h, SDL_Color clr);
  void drawLineHorizontal(const int x0, const int y0, const int w, SDL_Color clr);
  void drawLineVertical(const int x0, const int y0, const int h, SDL_Color clr);

  void drawBlastAnimationAtField(const coord center, const int RADIUS, bool forbiddenCells[MAP_X_CELLS][MAP_Y_CELLS], const SDL_Color colorInner, const SDL_Color colorOuter, const int DURATION);
  void drawBlastAnimationAt(const coord pos, const SDL_Color color, const int DURATION);

  void flip() {
    SDL_Flip(m_screen);
  }

  GlyphAndColor renderArrayActorsOmitted[MAP_X_CELLS][MAP_Y_CELLS];
  TileAndColor renderArrayActorsOmittedTiles[MAP_X_CELLS][MAP_Y_CELLS];

  GlyphAndColor renderArray[MAP_X_CELLS][MAP_Y_CELLS];
  TileAndColor renderArrayTiles[MAP_X_CELLS][MAP_Y_CELLS];

private:
  friend class Postmortem;

  void clearTileAtPixel(const int X, const int Y);

  void drawCharacterAtPixel(const char CHARACTER, const int X, const int Y, const SDL_Color clr);
  void clearCharacterAtPixel(const int X, const int Y);
  coord getPixelCoordsForCharacter(const RenderArea_t renderArea, const int X, const int Y);

  void loadFontSheet();
  void loadTiles();

  void applySurface(int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip = NULL);
  void drawASCII();
  void drawTiles();

  bool SDL_init();

  SDL_Surface* scaleSurface(SDL_Surface* surface, const double SCALE_FACTOR);
  Uint32 getPixel(SDL_Surface* surface, int x, int y);
  void putPixel(SDL_Surface *surface, int x, int y, Uint32 pixel);

  SDL_Rect dsrect;
  SDL_Rect cell_rect;
//  SDL_Rect cell_rectTiles;
  SDL_Rect clipRect;
//  SDL_Rect clipRectTiles;

  bool drwLandscape;

  SDL_Surface* m_screen;
  SDL_Surface* m_glyphSheet;
  SDL_Surface* m_tileSheet;
  SDL_Surface* m_drw;

  Engine* eng;
};

#endif

