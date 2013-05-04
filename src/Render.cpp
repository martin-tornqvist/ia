#include "Render.h"

#include <vector>
#include <iostream>

#include "SDL/SDL_image.h"

#include "Engine.h"
#include "Item.h"
#include "Interface.h"
#include "Marker.h"
#include "Map.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "Log.h"
#include "Attack.h"
#include "FeatureWall.h"
#include "FeatureDoor.h"
#include "Viewport.h"

using namespace std;

Renderer::Renderer(Engine* engine) : eng(engine), screenSurface_(NULL),
/*fontSurface_(NULL), tileSurface_(NULL),*/ mainMenuLogoSurface_(NULL) {

  initAndClearPrev();
}

Renderer::~Renderer() {
  freeAssets();
}

void Renderer::freeAssets() {
  tracer << "Renderer::freeAssets()..." << endl;

  if(screenSurface_ != NULL) {
    SDL_FreeSurface(screenSurface_);
    screenSurface_ = NULL;
  }

//  if(fontSurface_ != NULL) {
//    SDL_FreeSurface(fontSurface_);
//    fontSurface_ = NULL;
//  }

//  if(tileSurface_ != NULL) {
//    SDL_FreeSurface(tileSurface_);
//    tileSurface_ = NULL;
//  }

  if(mainMenuLogoSurface_ != NULL) {
    delete mainMenuLogoSurface_;
    mainMenuLogoSurface_ = NULL;
  }

  tracer << "Renderer::freeAssets() [DONE]" << endl;
}

void Renderer::initAndClearPrev() {
  tracer << "Renderer::initAndClearPrev()..." << endl;
  freeAssets();

  tracer << "Renderer: Setting up rendering window" << endl;
  const string title = "IA " + eng->config->GAME_VERSION;

  if(false /*ng->config->FULLSCREEN*/) {
    screenSurface_ = SDL_SetVideoMode(
                       eng->config->SCREEN_WIDTH,
                       eng->config->SCREEN_HEIGHT,
                       eng->config->SCREEN_BPP,
                       SDL_SWSURFACE | SDL_FULLSCREEN);
  } else {
    screenSurface_ = SDL_SetVideoMode(
                       eng->config->SCREEN_WIDTH,
                       eng->config->SCREEN_HEIGHT,
                       eng->config->SCREEN_BPP,
                       SDL_SWSURFACE);
  }

  loadFont();

  if(eng->config->USE_TILE_SET) {
    loadTiles();
    loadMainMenuLogo();
  }

  tracer << "Renderer::initAndClearPrev() [DONE]" << endl;
}

void Renderer::loadFont() {
  tracer << "Renderer::loadFont()..." << endl;

  SDL_Surface* fontSurfaceTmp =
    IMG_Load(eng->config->fontImageName.data());
//    IMG_LoadPNG_RW(SDL_RWFromFile(, "r"));

//  fontSurface_ = SDL_DisplayFormatAlpha(fontSurfaceTmp);

  Uint32 imgClr = SDL_MapRGB(fontSurfaceTmp->format, 255, 255, 255);

  for(int y = 0; y < fontSurfaceTmp->h; y++) {
    for(int x = 0; x < fontSurfaceTmp->w; x++) {
      fontPixelData_[x][y] = (getpixel(fontSurfaceTmp, x, y) == imgClr);
    }
  }

  SDL_FreeSurface(fontSurfaceTmp);

  tracer << "Renderer::loadFont() [DONE]" << endl;
}

void Renderer::loadTiles() {
  tracer << "Renderer::loadTiles()..." << endl;

  SDL_Surface* tileSurfaceTmp =
    IMG_Load(eng->config->TILES_IMAGE_NAME.data());

  Uint32 imgClr = SDL_MapRGB(tileSurfaceTmp->format, 255, 255, 255);
  for(int y = 0; y < tileSurfaceTmp->h; y++) {
    for(int x = 0; x < tileSurfaceTmp->w; x++) {
      tilePixelData_[x][y] = (getpixel(tileSurfaceTmp, x, y) == imgClr);
    }
  }

  SDL_FreeSurface(tileSurfaceTmp);

  tracer << "Renderer::loadTiles() [DONE]" << endl;
}

void Renderer::loadMainMenuLogo() {
  tracer << "Renderer::loadMainMenuLogo()..." << endl;

  SDL_Surface* mainMenuLogoSurfaceTmp =
    IMG_Load(eng->config->MAIN_MENU_LOGO_IMAGE_NAME.data());

  mainMenuLogoSurface_ = SDL_DisplayFormatAlpha(mainMenuLogoSurfaceTmp);

  SDL_FreeSurface(mainMenuLogoSurfaceTmp);

  tracer << "Renderer::loadMainMenuLogo() [DONE]" << endl;
}


void Renderer::putPixelsOnScreenForTile(const Tile_t tile, const int PIXEL_X,
                                        const int PIXEL_Y, const SDL_Color& clr) {
  const int CLR_TO = SDL_MapRGB(screenSurface_->format, clr.r, clr.g, clr.b);

  SDL_LockSurface(screenSurface_);

  const int CELL_W = eng->config->CELL_W;
  const int CELL_H = eng->config->CELL_H;

  const coord sheetCoords = eng->art->getTileCoords(tile);
  const int SHEET_X0 = sheetCoords.x * CELL_W;
  const int SHEET_Y0 = sheetCoords.y * CELL_H;

  for(int dy = 0; dy < CELL_H; dy++) {
    for(int dx = 0; dx < CELL_W; dx++) {
      if(tilePixelData_[SHEET_X0 + dx][SHEET_Y0 + dy]) {
        putpixel(screenSurface_, PIXEL_X + dx, PIXEL_Y + dy, CLR_TO);
      }
    }
  }

  SDL_UnlockSurface(screenSurface_);
}

void Renderer::putPixelsOnScreenForGlyph(const char GLYPH, const int PIXEL_X,
    const int PIXEL_Y, const SDL_Color& clr) {
  const int CLR_TO = SDL_MapRGB(screenSurface_->format, clr.r, clr.g, clr.b);

  SDL_LockSurface(screenSurface_);

  const int CELL_W = eng->config->CELL_W;
  const int CELL_H = eng->config->CELL_H;

  const int SCALE = eng->config->FONT_SCALE;

  const int CELL_W_SHEET = CELL_W / SCALE;
  const int CELL_H_SHEET = CELL_H / SCALE;

  const coord sheetCoords = eng->art->getGlyphCoords(GLYPH);
  const int SHEET_X0 = sheetCoords.x * CELL_W_SHEET;
  const int SHEET_Y0 = sheetCoords.y * CELL_H_SHEET;

  for(int dy = 0; dy < CELL_H_SHEET; dy++) {
    for(int dx = 0; dx < CELL_W_SHEET; dx++) {
      const int DX_SCALED = dx * SCALE;
      const int DY_SCALED = dy * SCALE;
      if(fontPixelData_[SHEET_X0 + dx][SHEET_Y0 + dy]) {
        for(int m = 0; m < SCALE; m++) {
          for(int n = 0; n < SCALE; n++) {
            putpixel(screenSurface_,
                     PIXEL_X + DX_SCALED + m,
                     PIXEL_Y + DY_SCALED + n,
                     CLR_TO);
          }
        }
      }
    }
  }

  SDL_UnlockSurface(screenSurface_);
}

Uint32 Renderer::getpixel(SDL_Surface* const surface, const int X, const int Y) {
  int bpp = surface->format->BytesPerPixel;
  /* Here p is the address to the pixel we want to retrieve */
  Uint8* p = (Uint8 *)surface->pixels + Y * surface->pitch + X * bpp;

  switch(bpp) {
    case 1:   return *p;            break;
    case 2:   return *(Uint16 *)p;  break;
    case 3: {
      if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
        return p[0] << 16 | p[1] << 8 | p[2];
      }   else {
        return p[0] | p[1] << 8 | p[2] << 16;
      }
    } break;
    case 4:   return *(Uint32 *)p;  break;
    default:  return -1;            break;
  }
  return -1;
}

void Renderer::putpixel(SDL_Surface* const surface, const int X, const int Y, Uint32 pixel) {
  int bpp = surface->format->BytesPerPixel;
  /* Here p is the address to the pixel we want to set */
  Uint8* p = (Uint8 *)surface->pixels + Y * surface->pitch + X * bpp;

  switch(bpp) {
    case 1:   *p = pixel;             break;
    case 2:   *(Uint16 *)p = pixel;   break;
    case 3: {
      if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
        p[0] = (pixel >> 16) & 0xff;
        p[1] = (pixel >> 8) & 0xff;
        p[2] = pixel & 0xff;
      } else {
        p[0] = pixel & 0xff;
        p[1] = (pixel >> 8) & 0xff;
        p[2] = (pixel >> 16) & 0xff;
      }
    } break;
    case 4:   *(Uint32 *)p = pixel;   break;
    default:  {}                      break;
  }
}

void Renderer::applySurface(const int PIXEL_X, const int PIXEL_Y,
                            SDL_Surface* const source, SDL_Rect* clip) {
  SDL_Rect offset;
  offset.x = PIXEL_X;
  offset.y = PIXEL_Y;

  SDL_BlitSurface(source, clip, screenSurface_, &offset);
}

void Renderer::drawMainMenuLogo(const int Y_POS) {
  const int IMG_W = mainMenuLogoSurface_->w;
  const int X = (eng->config->SCREEN_WIDTH - IMG_W) / 2;
  const int Y = eng->config->CELL_H * Y_POS;
  applySurface(X, Y, mainMenuLogoSurface_);
}

void Renderer::drawMarker(const vector<coord>& trace, const int EFFECTIVE_RANGE) {
  if(trace.size() > 2) {
    for(unsigned int i = 1; i < trace.size() - 1; i++) {
      coverCellInMap(trace.at(i));

      SDL_Color clr = clrGreenLight;

      if(EFFECTIVE_RANGE != -1) {
        const int CHEB_DIST = eng->basicUtils->chebyshevDistance(trace.at(0), trace.at(i));
        if(CHEB_DIST > EFFECTIVE_RANGE) {
          clr = clrYellow;
        }
      }
      if(eng->config->USE_TILE_SET) {
        drawTileInMap(tile_aimMarkerTrail, trace.at(i), clr);
      } else {
        drawCharacter('*', renderArea_mainScreen, trace.at(i), clr);
      }
    }
  }

  const coord& headPos = eng->marker->getPos();

  SDL_Color clr = clrGreenLight;

  if(trace.size() > 2) {
    if(EFFECTIVE_RANGE != -1) {
      const int CHEB_DIST = eng->basicUtils->chebyshevDistance(trace.at(0), headPos);
      if(CHEB_DIST > EFFECTIVE_RANGE) {
        clr = clrYellow;
      }
    }
  }

  coverCellInMap(headPos);

  if(eng->config->USE_TILE_SET) {
    drawTileInMap(tile_aimMarkerHead, headPos, clr);
  } else {
    drawCharacter('X', renderArea_mainScreen, headPos, clr);
  }
}

void Renderer::drawBlastAnimationAtField(const coord& center, const int RADIUS,
    bool forbiddenCells[MAP_X_CELLS][MAP_Y_CELLS], const SDL_Color& colorInner,
    const SDL_Color& colorOuter, const int DURATION) {
  drawMapAndInterface();
  clearScreen();

  bool isAnyBlastRendered = false;

  for(int y = max(1, center.y - RADIUS); y <= min(MAP_Y_CELLS - 2, center.y + RADIUS); y++) {
    for(int x = max(1, center.x - RADIUS); x <= min(MAP_X_CELLS - 2, center.x + RADIUS); x++) {
      if(forbiddenCells[x][y] == false) {
        const bool
        IS_OUTER = x == center.x - RADIUS ||
                   x == center.x + RADIUS ||
                   y == center.y - RADIUS ||
                   y == center.y + RADIUS;
        const SDL_Color color = IS_OUTER ? colorOuter : colorInner;
        coverCellInMap(x, y);
        drawTileInMap(tile_blastAnimation1, x, y, color);
        isAnyBlastRendered = true;
      }
    }
  }
  updateScreen();
  if(isAnyBlastRendered) {
    eng->sleep(DURATION / 2);
  }
  clearScreen();

  for(int y = max(1, center.y - RADIUS); y <= min(MAP_Y_CELLS - 2, center.y + RADIUS); y++) {
    for(int x = max(1, center.x - RADIUS); x <= min(MAP_X_CELLS - 2, center.x + RADIUS); x++) {
      if(forbiddenCells[x][y] == false) {
        const bool IS_OUTER = x == center.x - RADIUS ||
                              x == center.x + RADIUS ||
                              y == center.y - RADIUS ||
                              y == center.y + RADIUS;
        const SDL_Color color = IS_OUTER ? colorOuter : colorInner;
        coverCellInMap(x, y);
        drawTileInMap(tile_blastAnimation2, x, y, color);
      }
    }
  }
  updateScreen();
  if(isAnyBlastRendered) {
    eng->sleep(DURATION / 2);
  }
  drawMapAndInterface();
}

void Renderer::drawBlastAnimationAtPositions(const vector<coord>& positions,
    const SDL_Color& color,
    const int DURATION) {
  drawMapAndInterface();
  clearScreen();

  for(unsigned int i = 0; i < positions.size(); i++) {
    const coord& pos = positions.at(i);
    drawTileInMap(tile_blastAnimation1, pos.x, pos.y, color, clrBlack);
  }
  updateScreen();
  eng->sleep(DURATION / 2);
  clearScreen();

  for(unsigned int i = 0; i < positions.size(); i++) {
    const coord& pos = positions.at(i);
    drawTileInMap(tile_blastAnimation2, pos.x, pos.y, color, clrBlack);
  }
  updateScreen();
  eng->sleep(DURATION / 2);
  drawMapAndInterface();
}

void Renderer::drawBlastAnimationAtPositionsWithPlayerVision(
  const vector<coord>& positions, const SDL_Color& clr,
  const int EXPLOSION_DELAY_FACTOR) {

  const int DELAY = eng->config->DELAY_EXPLOSION * EXPLOSION_DELAY_FACTOR;

  vector<coord> positionsWithVision;
  for(unsigned int i = 0; i < positions.size(); i++) {
    const coord& pos = positions.at(i);
    if(eng->map->playerVision[pos.x][pos.y]) {
      positionsWithVision.push_back(pos);
    }
  }

  if(eng->config->USE_TILE_SET) {
    eng->renderer->drawBlastAnimationAtPositions(positionsWithVision, clr, DELAY);
  } else {
    //TODO
  }
}

void Renderer::drawTileInScreen(const Tile_t tile, const int X, const int Y,
                                const SDL_Color& clr, const SDL_Color& bgClr) {
  const int& CELL_W = eng->config->CELL_W;
  const int& CELL_H = eng->config->CELL_H;

  const int PIXEL_X = X * CELL_W;
  const int PIXEL_Y = Y * CELL_H;

  drawRectangleSolid(PIXEL_X, PIXEL_Y, CELL_W, CELL_H, bgClr);

  putPixelsOnScreenForTile(tile, PIXEL_X, PIXEL_Y, clr);
}

void Renderer::drawTileInMap(const Tile_t tile, const int X, const int Y,
                             const SDL_Color& clr, const SDL_Color& bgClr) {
  drawTileInScreen(tile, X, Y + eng->config->MAINSCREEN_Y_CELLS_OFFSET, clr, bgClr);
}

void Renderer::drawGlyphInMap(const char GLYPH, const int X, const int Y,
                              const SDL_Color& clr, const bool DRAW_BG_CLR,
                              const SDL_Color& bgClr) {
  const int& CELL_W = eng->config->CELL_W;
  const int& CELL_H = eng->config->CELL_H;

  const int PIXEL_X = X * CELL_W;
  const int PIXEL_Y = Y * CELL_H + eng->config->MAINSCREEN_Y_OFFSET;

  if(DRAW_BG_CLR) {
    drawRectangleSolid(PIXEL_X, PIXEL_Y, CELL_W, CELL_H, bgClr);
  }

  putPixelsOnScreenForGlyph(GLYPH, PIXEL_X, PIXEL_Y, clr);
}

void Renderer::drawCharacterAtPixel(const char CHARACTER, const int X, const int Y,
                                    const SDL_Color& clr, const bool DRAW_BG_CLR,
                                    const SDL_Color& bgClr) {
  if(DRAW_BG_CLR) {
    const int& CELL_W = eng->config->CELL_W;
    const int& CELL_H = eng->config->CELL_H;
    drawRectangleSolid(X, Y, CELL_W, CELL_H, bgClr);
  }

  putPixelsOnScreenForGlyph(CHARACTER, X, Y, clr);
}

coord Renderer::getPixelPosForCharacter(const RenderArea_t renderArea, const int X, const int Y) {
  const int CELL_W = eng->config->CELL_W;
  const int CELL_H = eng->config->CELL_H;

  switch(renderArea) {
    case renderArea_screen: {
      return coord(X * CELL_W, Y * CELL_H);
    }
    break;
    case renderArea_mainScreen: {
      return coord(X * CELL_W, eng->config->MAINSCREEN_Y_OFFSET + Y * CELL_H);
    }
    break;
    case renderArea_log: {
      return coord(eng->config->LOG_X_OFFSET + X * CELL_W, eng->config->LOG_Y_OFFSET + Y * CELL_H);
    }
    break;
    case renderArea_characterLines: {
      return coord(X * CELL_W, eng->config->CHARACTER_LINES_Y_OFFSET + Y * CELL_H);
    }
    break;
  }
  return coord();
}

void Renderer::drawCharacter(const char CHARACTER, const RenderArea_t renderArea,
                             const int X, const int Y, const SDL_Color& clr,
                             const bool DRAW_BG_CLR,
                             const SDL_Color& bgClr) {
  const coord pixelCoord = getPixelPosForCharacter(renderArea, X, Y);
  drawCharacterAtPixel(CHARACTER, pixelCoord.x, pixelCoord.y, clr, DRAW_BG_CLR, bgClr);
}

void Renderer::drawText(const string& str, const RenderArea_t renderArea,
                        const int X, const int Y, const SDL_Color& clr,
                        const SDL_Color& bgClr) {
  coord pixelCoord = getPixelPosForCharacter(renderArea, X, Y);

  if(pixelCoord.y < 0 || pixelCoord.y >= eng->config->SCREEN_HEIGHT) {
    return;
  }

  const int& CELL_W = eng->config->CELL_W;
  const int& CELL_H = eng->config->CELL_H;
  const int TOT_W = CELL_W * str.size();

  drawRectangleSolid(pixelCoord.x, pixelCoord.y, TOT_W, CELL_H, bgClr);

  const unsigned int W_TOT = str.size();

  for(unsigned int i = 0; i < W_TOT; i++) {
    if(pixelCoord.x < 0 || pixelCoord.x >= eng->config->SCREEN_WIDTH) {
      return;
    }
    drawCharacterAtPixel(str.at(i), pixelCoord.x, pixelCoord.y, clr, false);
    pixelCoord.x += CELL_W;
  }
}

int Renderer::drawTextCentered(const string& str, const RenderArea_t renderArea,
                               const int X, const int Y, const SDL_Color& clr,
                               const SDL_Color& bgClr,
                               const bool IS_PIXEL_POS_ADJ_ALLOWED) {
  const unsigned int STR_LEN_HALF = str.size() / 2;
  const int X_POS_LEFT = X - STR_LEN_HALF;

  const int CELL_H = eng->config->CELL_H;
  const int CELL_W = eng->config->CELL_W;

  coord pixelCoord = getPixelPosForCharacter(renderArea, X_POS_LEFT, Y);

  if(IS_PIXEL_POS_ADJ_ALLOWED) {
    const int PIXEL_X_ADJ = STR_LEN_HALF * 2 == str.size() ? CELL_W / 2 : 0;
    pixelCoord += coord(PIXEL_X_ADJ, 0);
  }

  const unsigned int W_TOT  = str.size();
  const int W_TOT_PIXEL     = W_TOT * CELL_W;

  SDL_Rect sdlRect = {
    (Sint16)pixelCoord.x, (Sint16)pixelCoord.y,
    (Uint16)W_TOT_PIXEL, (Uint16)CELL_H
  };
  SDL_FillRect(screenSurface_, &sdlRect, SDL_MapRGB(screenSurface_->format,
               bgClr.r, bgClr.g, bgClr.b));

  for(unsigned int i = 0; i < W_TOT; i++) {
    if(pixelCoord.x < 0 || pixelCoord.x >= eng->config->SCREEN_WIDTH) {
      return X_POS_LEFT;
    }
    drawCharacterAtPixel(str.at(i), pixelCoord.x, pixelCoord.y, clr, false);
    pixelCoord.x += eng->config->CELL_W;
  }
  return X_POS_LEFT;
}

void Renderer::coverRenderArea(const RenderArea_t renderArea) {
  const coord x0y0Pixel = getPixelPosForCharacter(renderArea, 0, 0);
  switch(renderArea) {
    case renderArea_characterLines: {
      coverAreaPixel(x0y0Pixel.x, x0y0Pixel.y, eng->config->SCREEN_WIDTH,
                     eng->config->CHARACTER_LINES_HEIGHT);
    }
    break;
    case renderArea_log: {
      coverAreaPixel(0, 0, eng->config->SCREEN_WIDTH,
                     eng->config->LOG_HEIGHT + eng->config->LOG_Y_OFFSET);
    }
    break;
    case renderArea_mainScreen: {
      coverAreaPixel(x0y0Pixel.x, x0y0Pixel.y, eng->config->SCREEN_WIDTH,
                     eng->config->MAINSCREEN_HEIGHT);
    }
    break;
    case renderArea_screen: {
      clearScreen();
    }
    break;

  }
}

void Renderer::coverArea(const RenderArea_t renderArea, const int X, const int Y, const int W, const int H) {
  const coord x0y0 = getPixelPosForCharacter(renderArea, X, Y);
  coverAreaPixel(x0y0.x, x0y0.y, W * eng->config->CELL_W, H * eng->config->CELL_H);
}

void Renderer::coverAreaPixel(const int X, const int Y, const int W, const int H) {
  drawRectangleSolid(X, Y, W, H, clrBlack);
}

void Renderer::coverCellInMap(const int X, const int Y) {
  const int CELL_W = eng->config->CELL_W;
  const int CELL_H = eng->config->CELL_H;
  coverAreaPixel(X * CELL_W, eng->config->MAINSCREEN_Y_OFFSET + (Y * CELL_H),
                 CELL_W, CELL_H);
}

void Renderer::drawLineVertical(const int X, const int Y, const int H, const SDL_Color& clr) {
  drawRectangleSolid(X, Y, 1, H, clr);
}

void Renderer::drawLineHorizontal(const int X, const int Y, const int W, const SDL_Color& clr) {
  drawRectangleSolid(X, Y, W, 2, clr);
}

void Renderer::drawRectangleSolid(const int X, const int Y, const int W, const int H,
                                  const SDL_Color& clr) {
  SDL_Rect sdlRect = {(Sint16)X, (Sint16)Y, (Uint16)W, (Uint16)H};
  SDL_FillRect(screenSurface_, &sdlRect, SDL_MapRGB(screenSurface_->format, clr.r, clr.g, clr.b));
}

void Renderer::coverCharAtPixel(const int X, const int Y) {
  coverAreaPixel(X, Y, eng->config->CELL_W, eng->config->CELL_H);
}

void Renderer::coverTileAtPixel(const int X, const int Y) {
  coverAreaPixel(X, Y, eng->config->CELL_W, eng->config->CELL_H);
}

void Renderer::drawProjectilesAndUpdateWindow(vector<Projectile*>& projectiles) {
  drawMapAndInterface(false);

  for(unsigned int i = 0; i < projectiles.size(); i++) {
    Projectile* const p = projectiles.at(i);
    if(p->isDoneRendering == false && p->isVisibleToPlayer) {
      coverCellInMap(p->pos);
      if(eng->config->USE_TILE_SET) {
        if(p->tile != tile_empty) {
          drawTileInMap(p->tile, p->pos, p->clr);
        }
      } else {
        if(p->glyph != -1) {
          drawCharacter(p->glyph, renderArea_mainScreen, p->pos, p->clr);
        }
      }
    }
  }

  updateScreen();
}

void Renderer::drawMapAndInterface(const bool UPDATE_SCREEN) {
  clearScreen();

  if(eng->config->USE_TILE_SET) {
    drawTiles();
  } else {
    drawAscii();
  }

  eng->interfaceRenderer->drawInfoLines();
  eng->interfaceRenderer->drawLocationInfo();
  eng->log->drawLog();

  if(UPDATE_SCREEN) {
    updateScreen();
  }
}

int Renderer::getLifebarLength(const Actor& actor) const {
  const int ACTOR_HP = max(0, actor.getHp());
  const int ACTOR_HP_MAX = actor.getHpMax(true);
  if(ACTOR_HP < ACTOR_HP_MAX) {
    int HP_PERCENT = (ACTOR_HP * 100) / ACTOR_HP_MAX;
    return ((eng->config->CELL_W - 2) * HP_PERCENT) / 100;
  }
  return -1;
}

void Renderer::drawLifeBar(const int X_CELL, const int Y_CELL, const int LENGTH) {
  if(LENGTH >= 0) {
    const int W_GREEN = LENGTH;
    const int& W_CELL = eng->config->CELL_W;
    const int W_BAR_TOT = W_CELL - 2;
    const int W_RED = W_BAR_TOT - W_GREEN;
    const int Y0 = eng->config->MAINSCREEN_Y_OFFSET + ((Y_CELL + 1) * eng->config->CELL_H) - 2;
    const int X0_GREEN = X_CELL * W_CELL + 1;
    const int X0_RED = X0_GREEN + W_GREEN;

    if(W_GREEN > 0) {
      drawLineHorizontal(X0_GREEN, Y0, W_GREEN, clrGreenLight);
    }
    if(W_RED > 0) {
      drawLineHorizontal(X0_RED, Y0, W_RED, clrRedLight);
    }
  }
}

void Renderer::drawAscii() {
  CellRenderDataAscii* currentDrw = NULL;
  CellRenderDataAscii tempDrw;

  //-------------------------------------------- INSERT STATIC FEATURES AND BLOOD INTO ARRAY
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {

      currentDrw = &renderArrayAscii[x][y];
      currentDrw->clear();

      if(eng->map->playerVision[x][y]) {
        //Static features
        char goreGlyph = ' ';
        if(eng->map->featuresStatic[x][y]->canHaveGore()) {
          goreGlyph = eng->map->featuresStatic[x][y]->getGoreGlyph();
        }
        if(goreGlyph == ' ') {
          currentDrw->glyph = eng->map->featuresStatic[x][y]->getGlyph();
          const SDL_Color& featureClr = eng->map->featuresStatic[x][y]->getColor();
          const SDL_Color& featureClrBg = eng->map->featuresStatic[x][y]->getColorBg();
          currentDrw->color = eng->map->featuresStatic[x][y]->hasBlood() ? clrRedLight : featureClr;
          if(eng->basicUtils->isClrEq(featureClrBg, clrBlack) == false) {
            currentDrw->colorBg = featureClrBg;
          }
        } else {
          currentDrw->glyph = goreGlyph;
          currentDrw->color = clrRed;
        }
      }
    }
  }

  int xPos, yPos;
  const unsigned int LOOP_SIZE = eng->gameTime->actors_.size();
  //-------------------------------------------- INSERT DEAD ACTORS INTO ARRAY
  Actor* actor = NULL;
  for(unsigned int i = 0; i < LOOP_SIZE; i++) {
    actor = eng->gameTime->getActorAt(i);
    xPos = actor->pos.x;
    yPos = actor->pos.y;
    if(actor->deadState == actorDeadState_corpse && actor->getDef()->glyph != ' ' && eng->map->playerVision[xPos][yPos]) {
      currentDrw = &renderArrayAscii[xPos][yPos];
      currentDrw->color = clrRed;
      currentDrw->glyph = actor->getGlyph();
    }
  }

  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      currentDrw = &renderArrayAscii[x][y];
      if(eng->map->playerVision[x][y] == true) {
        //-------------------------------------------- INSERT ITEMS INTO ARRAY
        if(eng->map->items[x][y] != NULL) {
          currentDrw->color = eng->map->items[x][y]->getColor();
          currentDrw->glyph = eng->map->items[x][y]->getGlyph();
        }

        //COPY ARRAY TO PLAYER MEMORY (BEFORE LIVING ACTORS AND TIME ENTITIES)
        renderArrayActorsOmittedAscii[x][y] = renderArrayAscii[x][y];
      }
    }
  }

  //-------------------------------------------- INSERT MOBILE FEATURES INTO ARRAY
  const unsigned int SIZE_OF_FEAT_MOB = eng->gameTime->getFeatureMobsSize();
  for(unsigned int i = 0; i < SIZE_OF_FEAT_MOB; i++) {
    FeatureMob* feature = eng->gameTime->getFeatureMobAt(i);
    xPos = feature->getX();
    yPos = feature->getY();
    if(feature->getGlyph() != ' ' && eng->map->playerVision[xPos][yPos] == true) {
      currentDrw = &renderArrayAscii[xPos][yPos];
      currentDrw->color = feature->getColor();
      currentDrw->glyph = feature->getGlyph();
    }
  }

  //-------------------------------------------- INSERT LIVING ACTORS INTO ARRAY
  for(unsigned int i = 0; i < LOOP_SIZE; i++) {
    actor = eng->gameTime->getActorAt(i);
    if(actor != eng->player) {
      xPos = actor->pos.x;
      yPos = actor->pos.y;
      if(
        actor->deadState == actorDeadState_alive &&
        actor->getGlyph() != ' ' &&
        eng->player->checkIfSeeActor(*actor, NULL)) {
        currentDrw = &renderArrayAscii[xPos][yPos];
        currentDrw->color = actor->getColor();
        currentDrw->glyph = actor->getGlyph();

        currentDrw->lifebarLength = getLifebarLength(*actor);

        const Monster* const monster = dynamic_cast<const Monster*>(actor);
        if(monster->leader == eng->player) {
          // TODO reimplement allied indicator
        } else {
          if(monster->playerAwarenessCounter <= 0) {
            currentDrw->colorBg = clrBlue;
          }
        }
      }
    }
  }

  //-------------------------------------------- DRAW THE GRID
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {

      tempDrw.clear();

      if(eng->map->playerVision[x][y]) {
        tempDrw = renderArrayAscii[x][y];
        if(tempDrw.isFadeEffectAllowed) {
          const int DIST_FROM_PLAYER = eng->basicUtils->chebyshevDistance(eng->player->pos, coord(x, y));
          if(DIST_FROM_PLAYER > 1) {
            const double DIST_FADE_DIV = min(2.0, 1.0 + ((DIST_FROM_PLAYER - 1) * 0.33));
            tempDrw.color.r /= DIST_FADE_DIV;
            tempDrw.color.g /= DIST_FADE_DIV;
            tempDrw.color.b /= DIST_FADE_DIV;
          }
        }
      } else if(eng->map->explored[x][y]) {
        renderArrayAscii[x][y] = eng->map->playerVisualMemoryAscii[x][y];
        tempDrw = renderArrayAscii[x][y];

        tempDrw.color.r /= 5;
        tempDrw.color.g /= 5;
        tempDrw.color.b /= 5;
      }

      if(tempDrw.glyph != ' ') {
        drawCharacter(tempDrw.glyph, renderArea_mainScreen, x, y, tempDrw.color,
                      true, tempDrw.colorBg);

        if(tempDrw.lifebarLength != -1) {
          drawLifeBar(x, y, tempDrw.lifebarLength);
        }
      }
    }
  }

  //-------------------------------------------- DRAW PLAYER CHARACTER
  drawGlyphInMap(eng->player->getGlyph(), eng->player->pos.x, eng->player->pos.y,
                 eng->player->getColor(), true, clrBlack);
  const int LIFE_BAR_LENGTH = getLifebarLength(*eng->player);
  if(LIFE_BAR_LENGTH != -1) {
    drawLifeBar(eng->player->pos.x, eng->player->pos.y, LIFE_BAR_LENGTH);
  }
}

void Renderer::drawTiles() {
  CellRenderDataTiles* currentDrw = NULL;
  CellRenderDataTiles tempDrw;

  //-------------------------------------------- INSERT STATIC FEATURES AND BLOOD INTO TILE ARRAY
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      if(eng->map->playerVision[x][y]) {
        currentDrw = &renderArrayTiles[x][y];
        currentDrw->clear();

        if(eng->map->playerVision[x][y]) {

          //Static features
          Tile_t goreTile = tile_empty;
          if(eng->map->featuresStatic[x][y]->canHaveGore()) {
            goreTile = eng->map->featuresStatic[x][y]->getGoreTile();
          }
          if(goreTile == tile_empty) {
            Feature* const f = eng->map->featuresStatic[x][y];
            const Feature_t featureId = f->getId();
            if(featureId == feature_stoneWall) {
              currentDrw->tile = dynamic_cast<Wall*>(f)->getTopWallTile();
            } else {
              currentDrw->tile = f->getTile();
            }
            const SDL_Color featureClr = f->getColor();
            const SDL_Color featureClrBg = f->getColorBg();
            currentDrw->color = f->hasBlood() ? clrRedLight : featureClr;
            if(eng->basicUtils->isClrEq(featureClrBg, clrBlack) == false) {
              currentDrw->colorBg = featureClrBg;
            }
          } else {
            currentDrw->tile = goreTile;
            currentDrw->color = clrRed;
          }
        }
      }
    }
  }

  int xPos, yPos;
  const unsigned int LOOP_SIZE = eng->gameTime->actors_.size();
  //-------------------------------------------- INSERT DEAD ACTORS INTO TILE ARRAY
  Actor* actor = NULL;
  for(unsigned int i = 0; i < LOOP_SIZE; i++) {
    actor = eng->gameTime->getActorAt(i);
    xPos = actor->pos.x;
    yPos = actor->pos.y;
    if(actor->deadState == actorDeadState_corpse && actor->getTile() != ' ' && eng->map->playerVision[xPos][yPos]) {
      currentDrw = &renderArrayTiles[xPos][yPos];
      currentDrw->color = clrRed;
      currentDrw->tile = actor->getTile();
    }
  }

  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      currentDrw = &renderArrayTiles[x][y];
      if(eng->map->playerVision[x][y]) {
        //-------------------------------------------- INSERT ITEMS INTO TILE ARRAY
        if(eng->map->items[x][y] != NULL) {
          currentDrw->color = eng->map->items[x][y]->getColor();
          currentDrw->tile = eng->map->items[x][y]->getTile();
        }
        //COPY ARRAY TO PLAYER MEMORY (BEFORE LIVING ACTORS AND MOBILE FEATURES)
        renderArrayActorsOmittedTiles[x][y] = renderArrayTiles[x][y];
      }
    }
  }

  //-------------------------------------------- INSERT MOBILE FEATURES INTO TILE ARRAY
  const unsigned int SIZE_OF_FEAT_MOB = eng->gameTime->getFeatureMobsSize();
  for(unsigned int i = 0; i < SIZE_OF_FEAT_MOB; i++) {
    FeatureMob* feature = eng->gameTime->getFeatureMobAt(i);
    xPos = feature->getX();
    yPos = feature->getY();
    if(feature->getGlyph() != ' ' && eng->map->playerVision[xPos][yPos]) {
      currentDrw = &renderArrayTiles[xPos][yPos];
      currentDrw->color = feature->getColor();
      currentDrw->tile = feature->getTile();
    }
  }

  //-------------------------------------------- INSERT LIVING ACTORS INTO TILE ARRAY
  for(unsigned int i = 0; i < LOOP_SIZE; i++) {
    actor = eng->gameTime->getActorAt(i);
    if(actor != eng->player) {
      if(
        actor->deadState == actorDeadState_alive &&
        actor->getTile() != tile_empty &&
        eng->player->checkIfSeeActor(*actor, NULL)) {
        currentDrw = &(renderArrayTiles[actor->pos.x][actor->pos.y]);
        currentDrw->isLivingActorSeenHere = true;
        currentDrw->color = actor->getColor();
        currentDrw->tile = actor->getTile();

        currentDrw->lifebarLength = getLifebarLength(*actor);

        currentDrw->isFadeEffectAllowed = false;

        const Monster* const monster = dynamic_cast<const Monster*>(actor);
        if(monster->leader == eng->player) {
          // TODO implement allied indicator
        } else {
          if(monster->playerAwarenessCounter <= 0) {
            currentDrw->colorBg = clrBlue;
          }
        }
      }
    }
  }

  //-------------------------------------------- DRAW THE TILE GRID
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {

      tempDrw.clear();

      if(eng->map->playerVision[x][y]) {
        tempDrw = renderArrayTiles[x][y];
        if(tempDrw.isFadeEffectAllowed) {
          const int DIST_FROM_PLAYER = eng->basicUtils->chebyshevDistance(eng->player->pos, coord(x, y));
          if(DIST_FROM_PLAYER > 1) {
            const double DIST_FADE_DIV = min(2.0, 1.0 + ((DIST_FROM_PLAYER - 1) * 0.33));
            tempDrw.color.r /= DIST_FADE_DIV;
            tempDrw.color.g /= DIST_FADE_DIV;
            tempDrw.color.b /= DIST_FADE_DIV;
          }
        }
      } else if(eng->map->explored[x][y]) {
        renderArrayTiles[x][y] = eng->map->playerVisualMemoryTiles[x][y];
        tempDrw = renderArrayTiles[x][y];

        tempDrw.color.r /= 5;
        tempDrw.color.g /= 5;
        tempDrw.color.b /= 5;
        tempDrw.colorBg.r /= 5;
        tempDrw.colorBg.g /= 5;
        tempDrw.colorBg.b /= 5;
      }

      //Walls are given perspective.
      //If the tile to be set is a (top) wall tile, check the tile beneath it. If the tile beneath is
      //not a front or top wall tile, and that cell is explored, change the current tile to wall front
      if(tempDrw.isLivingActorSeenHere == false) {
        const Tile_t tileSeen = renderArrayActorsOmittedTiles[x][y].tile;
        const Tile_t tileMem = eng->map->playerVisualMemoryTiles[x][y].tile;
        const bool IS_TILE_WALL =
          eng->map->playerVision[x][y] ? Wall::isTileAnyWallTop(tileSeen) :
          Wall::isTileAnyWallTop(tileMem);
        if(IS_TILE_WALL) {
          const Feature* const f = eng->map->featuresStatic[x][y];
          const Feature_t featureId = f->getId();
          bool isHiddenDoor = false;
          if(featureId == feature_door) {
            isHiddenDoor = dynamic_cast<const Door*>(f)->isSecret();
          }
          if(y < MAP_Y_CELLS - 1 && (featureId == feature_stoneWall || isHiddenDoor)) {
            if(eng->map->explored[x][y + 1]) {
              const bool IS_CELL_BELOW_SEEN = eng->map->playerVision[x][y + 1];

              const Tile_t tileBelowSeen = renderArrayActorsOmittedTiles[x][y + 1].tile;
              const Tile_t tileBelowMem = eng->map->playerVisualMemoryTiles[x][y + 1].tile;

              const bool TILE_BELOW_IS_WALL_FRONT =
                IS_CELL_BELOW_SEEN ? Wall::isTileAnyWallFront(tileBelowSeen) :
                Wall::isTileAnyWallFront(tileBelowMem);

              const bool TILE_BELOW_IS_WALL_TOP =
                IS_CELL_BELOW_SEEN ? Wall::isTileAnyWallTop(tileBelowSeen) :
                Wall::isTileAnyWallTop(tileBelowMem);

              if(TILE_BELOW_IS_WALL_FRONT == false && TILE_BELOW_IS_WALL_TOP == false) {
                if(isHiddenDoor) {
                  tempDrw.tile = tile_wallFront;
                } else if(featureId == feature_stoneWall) {
                  const Wall* const wall = dynamic_cast<const Wall*>(f);
                  tempDrw.tile = wall->getFrontWallTile();
                }
              }
            }
          }
        }
      }

      if(tempDrw.tile != tile_empty) {
        drawTileInMap(tempDrw.tile, x, y, tempDrw.color, tempDrw.colorBg);

        if(tempDrw.lifebarLength != -1) {
          drawLifeBar(x, y, tempDrw.lifebarLength);
        }
      }
    }
  }

  //-------------------------------------------- DRAW PLAYER CHARACTER
  bool isWieldingRangedWeapon = false;
  Item* item = eng->player->getInventory()->getItemInSlot(slot_wielded);
  if(item != NULL) {
    isWieldingRangedWeapon = item->getDef().isRangedWeapon;
  }
  drawTileInMap(isWieldingRangedWeapon ? tile_playerFirearm : tile_playerMelee,
                eng->player->pos, eng->player->getColor(), clrBlack);
  const int LIFE_BAR_LENGTH = getLifebarLength(*eng->player);
  if(LIFE_BAR_LENGTH != -1) {
    drawLifeBar(eng->player->pos.x, eng->player->pos.y, LIFE_BAR_LENGTH);
  }
}

