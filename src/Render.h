#ifndef DRAW_MAINSCREEN_H
#define DRAW_MAINSCREEN_H

#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/Sprite.hpp"

#include <vector>
#include <iostream>

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

  void drawMapAndInterface(const bool UPDATE_WINDOW = true);

  void updateWindow();

  inline void clearWindow() {
    renderWindow_->clear();
  }

  void drawMarker(vector<coord> &trace, const int EFFECTIVE_RANGE = -1);

  void drawCharacter(const char CHARACTER, const RenderArea_t renderArea, const int X, const int Y, const sf::Color& clr,
                     const bool drawBgClr = false, const sf::Color& bgClr = clrBlue);

  inline void drawCharacter(const char CHARACTER, const RenderArea_t renderArea, const coord& pos, const sf::Color& clr,
                            const bool drawBgClr = false, const sf::Color& bgClr = clrBlue) {
    drawCharacter(CHARACTER, renderArea, pos.x, pos.y, clr, drawBgClr, bgClr);
  }

  void drawText(const string& str, const RenderArea_t renderArea, const int X, const int Y, const sf::Color& clr);

  int drawTextCentered(const string& str, const RenderArea_t renderArea, const int X, const int Y,
                       const sf::Color& clr, const bool IS_PIXEL_POS_ADJ_ALLOWED = true);

  inline void drawTileInMap(const Tile_t tile, const coord& pos, const sf::Color& clr,
                     const bool drawBgClr = false, const sf::Color& bgClr = clrBlue) {
    drawTileInMap(tile, pos.x, pos.y, clr, drawBgClr, bgClr);
  }

  void drawTileInMap(const Tile_t tile, const int X, const int Y, const sf::Color& clr,
                     const bool drawBgClr = false, const sf::Color& bgClr = clrBlue);

  inline void drawTileInScreen(const Tile_t tile, const coord& pos, const sf::Color& clr,
                        const bool drawBgClr = false, const sf::Color& bgClr = clrBlue) {
    drawTileInScreen(tile, pos.x, pos.y, clr, drawBgClr, bgClr);
  }

  void drawTileInScreen(const Tile_t tile, const int X, const int Y, const sf::Color& clr,
                        const bool drawBgClr = false, const sf::Color& bgClr = clrBlue);

  inline void drawGlyphInMap(const Tile_t GLYPH, const coord& pos, const sf::Color& clr,
                      const bool drawBgClr = false, const sf::Color& bgClr = clrBlue) {
    drawGlyphInMap(GLYPH, pos.x, pos.y, clr, drawBgClr, bgClr);
  }

  void drawGlyphInMap(const char GLYPH, const int X, const int Y, const sf::Color& clr,
                      const bool drawBgClr = false, const sf::Color& bgClr = clrBlue);

  inline void coverCellInMap(const coord& pos) {
    coverCellInMap(pos.x, pos.y);
  }

  void coverCellInMap(const int X, const int Y);

  void coverRenderArea(const RenderArea_t renderArea);

  inline void coverArea(const RenderArea_t renderArea, const coord& pos, const int W, const int H) {
    coverArea(renderArea, pos.x, pos.y, W, H);
  }

  void coverArea(const RenderArea_t renderArea, const int X, const int Y, const int W, const int H);

  inline void coverAreaPixel(const coord& posPixel, const int W, const int H) {
    coverAreaPixel(posPixel.x, posPixel.y, W, H);
  }

  void coverAreaPixel(const int X, const int Y, const int W, const int H);

  inline void drawRectangleSolid(const coord& posPixel, const int w, const int h, const sf::Color& clr) {
    drawRectangleSolid(posPixel.x, posPixel.y, w, h, clr);
  }

  void drawRectangleSolid(const int X, const int Y, const int W, const int H, const sf::Color& clr);

  void drawLineHorizontal(const int x0, const int y0, const int w, const sf::Color& clr);

  void drawLineVertical(const int x0, const int y0, const int h, const sf::Color& clr);

  void drawBlastAnimationAtField(const coord& center, const int RADIUS, bool forbiddenCells[MAP_X_CELLS][MAP_Y_CELLS],
                                 const sf::Color& colorInner, const sf::Color& colorOuter, const int DURATION);

  void drawBlastAnimationAtPositions(const vector<coord>& positions, const sf::Color& color, const int DURATION);

  void drawBlastAnimationAtPositionsWithPlayerVision(const vector<coord>& positions,
      const sf::Color& clr, const int EXPLOSION_DELAY_FACTOR, Engine* const engine);

  void drawMainMenuLogo(const int Y_POS);

  CellRenderDataAscii renderArrayActorsOmitted[MAP_X_CELLS][MAP_Y_CELLS];
  CellRenderDataTile renderArrayActorsOmittedTiles[MAP_X_CELLS][MAP_Y_CELLS];

  CellRenderDataAscii renderArray[MAP_X_CELLS][MAP_Y_CELLS];
  CellRenderDataTile renderArrayTiles[MAP_X_CELLS][MAP_Y_CELLS];

  void setupWindowAndImagesClearPrev();

  void drawProjectilesAndUpdateWindow(vector<Projectile*>& projectiles);

  sf::Texture getScreenTextureCopy() const {
    return sf::Texture(screenTexture);
  }

  void drawScreenSizedTexture(const sf::Texture& texture);

private:
  friend class Postmortem;
  friend class Input;
  void drawCharacterAtPixel(const char CHARACTER, const int X, const int Y, const sf::Color& clr,
                            const bool drawBgClr = false, const sf::Color& bgClr = clrBlue);

  // This is used to keep a copy of the screen, for things
  // like player number entering to render over
  sf::Texture screenTexture;

  void drawSprite(const int X, const int Y, sf::Sprite& sprite);

  void coverCharacterAtPixel(const int X, const int Y);
  void coverTileAtPixel(const int X, const int Y);

  coord getPixelCoordsForCharacter(const RenderArea_t renderArea, const coord& pos) {
    return getPixelCoordsForCharacter(renderArea, pos.x, pos.y);
  }

  coord getPixelCoordsForCharacter(const RenderArea_t renderArea, const int X, const int Y);

  int getLifebarLength(const Actor& actor) const;
  void drawLifeBar(const int X_CELL, const int Y_CELL, const int LENGTH);

  void loadFont();
  void loadTiles();
  void loadMainMenuLogo();

  void freeWindowAndImages();

  void drawASCII();
  void drawTiles();

  Engine* eng;

  friend class Query;
  sf::RenderWindow* renderWindow_;

  sf::Texture* textureFontSheet_;
  sf::Texture* textureTileSheet_;
  sf::Texture* textureMainMenuLogo_;

  sf::Sprite* spritesFont_[FONT_SHEET_X_CELLS][FONT_SHEET_Y_CELLS];
  sf::Sprite* spritesTiles_[TILE_SHEET_X_CELLS][TILE_SHEET_X_CELLS];
  sf::Sprite* spriteMainMenuLogo_;

  // 2012-12-08:
  // Due to a bug in AMD Catalyst 12.10 drivers (confirmed by many users), drawing shapes
  // sometimes mess up the screen. Calling this function directly after the shapes provides a
  // workaround to the issue. The 12.11 beta drivers contains a fix. But keep this workaround
  // until the official 12.11 drivers has been out for a year or so.
  void workaroundAMDBug();
};

#endif

