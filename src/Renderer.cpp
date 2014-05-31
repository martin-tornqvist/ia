#include "Renderer.h"

#include <vector>
#include <iostream>

#include <SDL_image.h>

#include "Init.h"
#include "Item.h"
#include "CharacterLines.h"
#include "Marker.h"
#include "Map.h"
#include "Actor.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "Log.h"
#include "Attack.h"
#include "FeatureWall.h"
#include "FeatureDoor.h"
#include "Inventory.h"
#include "Utils.h"
#include "CmnData.h"
#include "SdlWrapper.h"

using namespace std;

namespace Renderer {

CellRenderData  renderArray[MAP_W][MAP_H];
CellRenderData  renderArrayNoActors[MAP_W][MAP_H];
SDL_Surface*    screenSurface       = nullptr;
SDL_Surface*    mainMenuLogoSurface = nullptr;

namespace {

bool tilePixelData_[400][400];
bool fontPixelData_[400][400];

bool isInited() {
  return screenSurface != nullptr;
}

void loadMainMenuLogo() {
  TRACE << "Renderer::loadMainMenuLogo()..." << endl;

  SDL_Surface* mainMenuLogoSurfaceTmp = IMG_Load(mainMenuLogoImgName.data());

  mainMenuLogoSurface = SDL_DisplayFormatAlpha(mainMenuLogoSurfaceTmp);

  SDL_FreeSurface(mainMenuLogoSurfaceTmp);

  TRACE << "Renderer::loadMainMenuLogo() [DONE]" << endl;
}

Uint32 getPixel(SDL_Surface* const surface,
                const int PIXEL_X, const int PIXEL_Y) {
  int bpp = surface->format->BytesPerPixel;
  /* Here p is the address to the pixel we want to retrieve */
  Uint8* p = (Uint8*)surface->pixels + PIXEL_Y * surface->pitch + PIXEL_X * bpp;

  switch(bpp) {
    case 1:   return *p;          break;
    case 2:   return *(Uint16*)p; break;
    case 3: {
      if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
        return p[0] << 16 | p[1] << 8 | p[2];
      }   else {
        return p[0] | p[1] << 8 | p[2] << 16;
      }
    } break;
    case 4:   return *(Uint32*)p; break;
    default:  return -1;          break;
  }
  return -1;
}

void putPixel(SDL_Surface* const surface,
              const int PIXEL_X, const int PIXEL_Y, Uint32 pixel) {
  int bpp = surface->format->BytesPerPixel;
  /* Here p is the address to the pixel we want to set */
  Uint8* p = (Uint8*)surface->pixels + PIXEL_Y * surface->pitch + PIXEL_X * bpp;

  switch(bpp) {
    case 1:   *p = pixel;             break;
    case 2:   *(Uint16*)p = pixel;    break;
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
    case 4:   *(Uint32*)p = pixel;    break;
    default:  {}                      break;
  }
}

void loadFont() {
  TRACE << "Renderer::loadFont()..." << endl;

  SDL_Surface* fontSurfaceTmp = IMG_Load(Config::getFontName().data());

  Uint32 bgClr = SDL_MapRGB(fontSurfaceTmp->format, 0, 0, 0);

  for(int y = 0; y < fontSurfaceTmp->h; y++) {
    for(int x = 0; x < fontSurfaceTmp->w; x++) {
      fontPixelData_[x][y] = getPixel(fontSurfaceTmp, x, y) != bgClr;
    }
  }

  SDL_FreeSurface(fontSurfaceTmp);

  TRACE << "Renderer::loadFont() [DONE]" << endl;
}

void loadTiles() {
  TRACE << "Renderer::loadTiles()..." << endl;

  SDL_Surface* tileSurfaceTmp = IMG_Load(tilesImgName.data());

  Uint32 imgClr = SDL_MapRGB(tileSurfaceTmp->format, 255, 255, 255);
  for(int y = 0; y < tileSurfaceTmp->h; y++) {
    for(int x = 0; x < tileSurfaceTmp->w; x++) {
      tilePixelData_[x][y] = getPixel(tileSurfaceTmp, x, y) == imgClr;
    }
  }

  SDL_FreeSurface(tileSurfaceTmp);

  TRACE << "Renderer::loadTiles() [DONE]" << endl;
}

void putPixelsOnScreenForTile(const TileId tile, const Pos& pixelPos,
                              const SDL_Color& clr) {
  if(isInited()) {
    const int CLR_TO = SDL_MapRGB(screenSurface->format, clr.r, clr.g, clr.b);

    SDL_LockSurface(screenSurface);

    const int CELL_W = Config::getCellW();
    const int CELL_H = Config::getCellH();

    const Pos sheetPos  = Art::getTilePos(tile);
    const int SHEET_X0  = sheetPos.x * CELL_W;
    const int SHEET_Y0  = sheetPos.y * CELL_H;
    const int SHEET_X1  = SHEET_X0 + CELL_W - 1;
    const int SHEET_Y1  = SHEET_Y0 + CELL_H - 1;
    const int SCREEN_X0 = pixelPos.x;
    const int SCREEN_Y0 = pixelPos.y;

    int screenX = SCREEN_X0;
    int screenY = SCREEN_Y0;

    for(int sheetY = SHEET_Y0; sheetY <= SHEET_Y1; sheetY++) {
      screenX = SCREEN_X0;
      for(int sheetX = SHEET_X0; sheetX <= SHEET_X1; sheetX++) {
        if(tilePixelData_[sheetX][sheetY]) {
          putPixel(screenSurface, screenX, screenY, CLR_TO);
        }
        screenX++;
      }
      screenY++;
    }

    SDL_UnlockSurface(screenSurface);
  }
}

void putPixelsOnScreenForGlyph(const char GLYPH, const Pos& pixelPos,
                               const SDL_Color& clr) {
  const int CLR_TO = SDL_MapRGB(screenSurface->format, clr.r, clr.g, clr.b);

  SDL_LockSurface(screenSurface);

  const int CELL_W = Config::getCellW();
  const int CELL_H = Config::getCellH();

  const Pos sheetPos  = Art::getGlyphPos(GLYPH);
  const int SHEET_X0  = sheetPos.x * CELL_W;
  const int SHEET_Y0  = sheetPos.y * CELL_H;
  const int SHEET_X1  = SHEET_X0 + CELL_W - 1;
  const int SHEET_Y1  = SHEET_Y0 + CELL_H - 1;
  const int SCREEN_X0 = pixelPos.x;
  const int SCREEN_Y0 = pixelPos.y;

  int screenX = SCREEN_X0;
  int screenY = SCREEN_Y0;

  for(int sheetY = SHEET_Y0; sheetY <= SHEET_Y1; sheetY++) {
    screenX = SCREEN_X0;
    for(int sheetX = SHEET_X0; sheetX <= SHEET_X1; sheetX++) {
      if(fontPixelData_[sheetX][sheetY]) {
        putPixel(screenSurface, screenX, screenY, CLR_TO);
      }
      screenX++;
    }
    screenY++;
  }

  SDL_UnlockSurface(screenSurface);
}

void drawGlyphAtPixel(const char GLYPH, const Pos& pixelPos,
                      const SDL_Color& clr, const bool DRAW_BG_CLR,
                      const SDL_Color& bgClr = clrBlack) {
  if(DRAW_BG_CLR) {
    const Pos cellDims(Config::getCellW(), Config::getCellH());
    drawRectangleSolid(pixelPos, cellDims, bgClr);
  }

  putPixelsOnScreenForGlyph(GLYPH, pixelPos, clr);
}

Pos getPixelPosForCellInPanel(const Panel panel, const Pos& pos) {
  const Pos cellDims(Config::getCellW(), Config::getCellH());

  switch(panel) {
    case Panel::screen: {
      return Pos(pos.x * cellDims.x, pos.y * cellDims.y);
    } break;

    case Panel::map: {
      return (pos * cellDims) + Pos(0, Config::getMapPixelOffsetH());
    } break;

    case Panel::log: {
      return pos * cellDims;
    } break;

    case Panel::charLines: {
      return (pos * cellDims) + Pos(0, Config::getCharLinesPixelOffsetH());
    } break;
  }
  return Pos();
}

int getLifebarLength(const Actor& actor) {
  const int ACTOR_HP = max(0, actor.getHp());
  const int ACTOR_HP_MAX = actor.getHpMax(true);
  if(ACTOR_HP < ACTOR_HP_MAX) {
    int HP_PERCENT = (ACTOR_HP * 100) / ACTOR_HP_MAX;
    return ((Config::getCellW() - 2) * HP_PERCENT) / 100;
  }
  return -1;
}

void drawLifeBar(const Pos& pos, const int LENGTH) {
  if(LENGTH >= 0) {
    const Pos cellDims(Config::getCellW(),  Config::getCellH());
    const int W_GREEN   = LENGTH;
    const int W_BAR_TOT = cellDims.x - 2;
    const int W_RED     = W_BAR_TOT - W_GREEN;
    const Pos pixelPos =
      getPixelPosForCellInPanel(Panel::map, pos + Pos(0, 1)) - Pos(0, 2);
    const int X0_GREEN  = pixelPos.x + 1;
    const int X0_RED    = X0_GREEN + W_GREEN;

    if(W_GREEN > 0) {
      drawLineHor(Pos(X0_GREEN, pixelPos.y), W_GREEN, clrGreenLgt);
    }
    if(W_RED > 0) {
      drawLineHor(Pos(X0_RED, pixelPos.y), W_RED, clrRedLgt);
    }
  }
}

void drawExclMarkAt(const Pos& pixelPos) {
  drawRectangleSolid(pixelPos,  Pos(3, 12),     clrBlack);
  drawLineVer(pixelPos +        Pos(1,  1), 6,  clrMagentaLgt);
  drawLineVer(pixelPos +        Pos(1,  9), 2,  clrMagentaLgt);
}

void drawPlayerShockExclMarks() {
  const double SHOCK  = Map::player->getPermShockTakenCurTurn();
  const int NR_EXCL   = SHOCK > 8 ? 3 : SHOCK > 3 ? 2 : SHOCK > 1 ? 1 : 0;

  if(NR_EXCL > 0) {
    const Pos& playerPos = Map::player->pos;
    const Pos pixelPosRight = getPixelPosForCellInPanel(Panel::map, playerPos);

    for(int i = 0; i < NR_EXCL; i++) {
      drawExclMarkAt(pixelPosRight + Pos(i * 3, 0));
    }
  }
}

} //Namespace

void init() {
  TRACE << "Renderer::init()..." << endl;
  cleanup();

  TRACE << "Renderer: Setting up rendering window" << endl;
  const string title = "IA " + gameVersionStr;
  SDL_WM_SetCaption(title.data(), nullptr);

  const int W = Config::getScreenPixelW();
  const int H = Config::getScreenPixelH();
  if(Config::isFullscreen()) {
    screenSurface = SDL_SetVideoMode(W, H, SCREEN_BPP,
                                     SDL_SWSURFACE | SDL_FULLSCREEN);
  }
  if(Config::isFullscreen() == false || screenSurface == nullptr) {
    screenSurface = SDL_SetVideoMode(W, H, SCREEN_BPP, SDL_SWSURFACE);
  }

  if(screenSurface == nullptr) {
    TRACE << "[WARNING] Failed to create screen surface, ";
    TRACE << "in Renderer::init()" << endl;
  }

  loadFont();

  if(Config::isTilesMode()) {
    loadTiles();
    loadMainMenuLogo();
  }

  TRACE << "Renderer::init() [DONE]" << endl;
}

void cleanup() {
  TRACE << "Renderer::cleanup()..." << endl;

  if(screenSurface != nullptr) {
    SDL_FreeSurface(screenSurface);
    screenSurface = nullptr;
  }

  if(mainMenuLogoSurface != nullptr) {
    SDL_FreeSurface(mainMenuLogoSurface);
    mainMenuLogoSurface = nullptr;
  }

  TRACE << "Renderer::cleanup() [DONE]" << endl;
}

void updateScreen() {
  if(isInited()) {SDL_Flip(screenSurface);}
}

void clearScreen() {
  if(isInited()) {
    SDL_FillRect(screenSurface, nullptr,
                 SDL_MapRGB(screenSurface->format, 0, 0, 0));
  }
}

void applySurface(const Pos& pixelPos, SDL_Surface* const src,
                  SDL_Rect* clip) {
  if(isInited()) {
    SDL_Rect offset;
    offset.x = pixelPos.x;
    offset.y = pixelPos.y;
    SDL_BlitSurface(src, clip, screenSurface, &offset);
  }
}

void drawMainMenuLogo(const int Y_POS) {
  const Pos pos((Config::getScreenPixelW() - mainMenuLogoSurface->w) / 2,
                Config::getCellH() * Y_POS);
  applySurface(pos, mainMenuLogoSurface);
}

void drawMarker(const vector<Pos>& trail, const int EFFECTIVE_RANGE) {
  if(trail.size() > 2) {
    for(size_t i = 1; i < trail.size(); i++) {
      const Pos& pos = trail.at(i);
      coverCellInMap(pos);

      SDL_Color clr = clrGreenLgt;

      if(EFFECTIVE_RANGE != -1) {
        const int CHEB_DIST = Utils::kingDist(trail.at(0), pos);
        if(CHEB_DIST > EFFECTIVE_RANGE) {clr = clrYellow;}
      }
      if(Config::isTilesMode()) {
        drawTile(TileId::aimMarkerTrail, Panel::map, pos, clr, clrBlack);
      } else {
        drawGlyph('*', Panel::map, pos, clr, true, clrBlack);
      }
    }
  }

  const Pos& headPos = Marker::getPos();

  SDL_Color clr = clrGreenLgt;

  if(trail.size() > 2) {
    if(EFFECTIVE_RANGE != -1) {
      const int CHEB_DIST = Utils::kingDist(trail.at(0), headPos);
      if(CHEB_DIST > EFFECTIVE_RANGE) {
        clr = clrYellow;
      }
    }
  }

  if(Config::isTilesMode()) {
    drawTile(TileId::aimMarkerHead, Panel::map, headPos, clr, clrBlack);
  } else {
    drawGlyph('X', Panel::map, headPos, clr, true, clrBlack);
  }
}

void drawBlastAnimAtField(const Pos& centerPos, const int RADIUS,
                          bool forbiddenCells[MAP_W][MAP_H],
                          const SDL_Color& colorInner,
                          const SDL_Color& colorOuter) {
  TRACE << "Renderer::drawBlastAnimAtField()..." << endl;
  if(isInited()) {
    drawMapAndInterface();

    bool isAnyBlastRendered = false;

    Pos pos;

    for(
      pos.y = max(1, centerPos.y - RADIUS);
      pos.y <= min(MAP_H - 2, centerPos.y + RADIUS);
      pos.y++) {
      for(
        pos.x = max(1, centerPos.x - RADIUS);
        pos.x <= min(MAP_W - 2, centerPos.x + RADIUS);
        pos.x++) {
        if(forbiddenCells[pos.x][pos.y] == false) {
          const bool IS_OUTER = pos.x == centerPos.x - RADIUS ||
                                pos.x == centerPos.x + RADIUS ||
                                pos.y == centerPos.y - RADIUS ||
                                pos.y == centerPos.y + RADIUS;
          const SDL_Color color = IS_OUTER ? colorOuter : colorInner;
          if(Config::isTilesMode()) {
            drawTile(TileId::blast1, Panel::map, pos, color, clrBlack);
          } else {
            drawGlyph('*', Panel::map, pos, color, true, clrBlack);
          }
          isAnyBlastRendered = true;
        }
      }
    }
    updateScreen();
    if(isAnyBlastRendered) {SdlWrapper::sleep(Config::getDelayExplosion() / 2);}

    for(
      pos.y = max(1, centerPos.y - RADIUS);
      pos.y <= min(MAP_H - 2, centerPos.y + RADIUS);
      pos.y++) {
      for(
        pos.x = max(1, centerPos.x - RADIUS);
        pos.x <= min(MAP_W - 2, centerPos.x + RADIUS);
        pos.x++) {
        if(forbiddenCells[pos.x][pos.y] == false) {
          const bool IS_OUTER = pos.x == centerPos.x - RADIUS ||
                                pos.x == centerPos.x + RADIUS ||
                                pos.y == centerPos.y - RADIUS ||
                                pos.y == centerPos.y + RADIUS;
          const SDL_Color color = IS_OUTER ? colorOuter : colorInner;
          if(Config::isTilesMode()) {
            drawTile(TileId::blast2, Panel::map, pos, color, clrBlack);
          } else {
            drawGlyph('*', Panel::map, pos, color, true, clrBlack);
          }
        }
      }
    }
    updateScreen();
    if(isAnyBlastRendered) {SdlWrapper::sleep(Config::getDelayExplosion() / 2);}
    drawMapAndInterface();
  }
  TRACE << "Renderer::drawBlastAnimAtField() [DONE]" << endl;
}

void drawBlastAnimAtPositions(const vector<Pos>& positions,
                              const SDL_Color& color) {
  TRACE << "Renderer::drawBlastAnimAtPositions()..." << endl;
  if(isInited()) {
    drawMapAndInterface();

    for(const Pos& pos : positions) {
      if(Config::isTilesMode()) {
        drawTile(TileId::blast1, Panel::map, pos, color, clrBlack);
      } else {
        drawGlyph('*', Panel::map, pos, color, true, clrBlack);
      }
    }
    updateScreen();
    SdlWrapper::sleep(Config::getDelayExplosion() / 2);

    for(const Pos& pos : positions) {
      if(Config::isTilesMode()) {
        drawTile(TileId::blast2, Panel::map, pos, color, clrBlack);
      } else {
        drawGlyph('*', Panel::map, pos, color, true, clrBlack);
      }
    }
    updateScreen();
    SdlWrapper::sleep(Config::getDelayExplosion() / 2);
    drawMapAndInterface();
  }
  TRACE << "Renderer::drawBlastAnimAtPositions() [DONE]" << endl;
}

void drawBlastAnimAtPositionsWithPlayerVision(
  const vector<Pos>& positions, const SDL_Color& clr) {

  vector<Pos> positionsWithVision;
  for(unsigned int i = 0; i < positions.size(); i++) {
    const Pos& pos = positions.at(i);
    if(Map::cells[pos.x][pos.y].isSeenByPlayer) {
      positionsWithVision.push_back(pos);
    }
  }

  Renderer::drawBlastAnimAtPositions(positionsWithVision, clr);
}

void drawTile(const TileId tile, const Panel panel, const Pos& pos,
              const SDL_Color& clr, const SDL_Color& bgClr) {
  if(isInited()) {
    const Pos pixelPos = getPixelPosForCellInPanel(panel, pos);
    const Pos cellDims(Config::getCellW(), Config::getCellH());
    drawRectangleSolid(pixelPos, cellDims, bgClr);
    putPixelsOnScreenForTile(tile, pixelPos, clr);
  }
}

void drawGlyph(const char GLYPH, const Panel panel, const Pos& pos,
               const SDL_Color& clr, const bool DRAW_BG_CLR,
               const SDL_Color& bgClr) {
  if(isInited()) {
    const Pos pixelPos = getPixelPosForCellInPanel(panel, pos);
    drawGlyphAtPixel(GLYPH, pixelPos, clr, DRAW_BG_CLR, bgClr);
  }
}

void drawText(const string& str, const Panel panel, const Pos& pos,
              const SDL_Color& clr, const SDL_Color& bgClr) {
  if(isInited()) {
    Pos pixelPos = getPixelPosForCellInPanel(panel, pos);

    if(pixelPos.y < 0 || pixelPos.y >= Config::getScreenPixelH()) {
      return;
    }

    const Pos cellDims(Config::getCellW(), Config::getCellH());
    const int LEN = str.size();
    drawRectangleSolid(pixelPos, Pos(cellDims.x * LEN, cellDims.y), bgClr);

    for(int i = 0; i < LEN; i++) {
      if(pixelPos.x < 0 || pixelPos.x >= Config::getScreenPixelW()) {
        return;
      }
      drawGlyphAtPixel(str.at(i), pixelPos, clr, false);
      pixelPos.x += cellDims.x;
    }
  }
}

int drawTextCentered(const string& str, const Panel panel,
                     const Pos& pos, const SDL_Color& clr,
                     const SDL_Color& bgClr,
                     const bool IS_PIXEL_POS_ADJ_ALLOWED) {
  const int LEN         = str.size();
  const int LEN_HALF    = LEN / 2;
  const int X_POS_LEFT  = pos.x - LEN_HALF;

  const Pos cellDims(Config::getCellW(), Config::getCellH());

  Pos pixelPos = getPixelPosForCellInPanel(panel, Pos(X_POS_LEFT, pos.y));

  if(IS_PIXEL_POS_ADJ_ALLOWED) {
    const int PIXEL_X_ADJ = LEN_HALF * 2 == LEN ? cellDims.x / 2 : 0;
    pixelPos += Pos(PIXEL_X_ADJ, 0);
  }

  const int W_TOT_PIXEL = LEN * cellDims.x;

  SDL_Rect sdlRect = {
    (Sint16)pixelPos.x, (Sint16)pixelPos.y,
    (Uint16)W_TOT_PIXEL, (Uint16)cellDims.y
  };
  SDL_FillRect(screenSurface, &sdlRect, SDL_MapRGB(screenSurface->format,
               bgClr.r, bgClr.g, bgClr.b));

  for(int i = 0; i < LEN; i++) {
    if(pixelPos.x < 0 || pixelPos.x >= Config::getScreenPixelW()) {
      return X_POS_LEFT;
    }
    drawGlyphAtPixel(str.at(i), pixelPos, clr, false, bgClr);
    pixelPos.x += cellDims.x;
  }
  return X_POS_LEFT;
}

void coverPanel(const Panel panel) {
  const int SCREEN_PIXEL_W = Config::getScreenPixelW();

  switch(panel) {
    case Panel::charLines: {
      const Pos pixelPos = getPixelPosForCellInPanel(panel, Pos(0, 0));
      coverAreaPixel(pixelPos,
                     Pos(SCREEN_PIXEL_W, Config::getCharLinesPixelH()));
    } break;

    case Panel::log: {
      coverAreaPixel(Pos(0, 0), Pos(SCREEN_PIXEL_W, Config::getLogPixelH()));
    } break;

    case Panel::map: {
      const Pos pixelPos = getPixelPosForCellInPanel(panel, Pos(0, 0));
      coverAreaPixel(pixelPos, Pos(SCREEN_PIXEL_W, Config::getMapPixelH()));
    } break;

    case Panel::screen: {clearScreen();} break;
  }
}

void coverArea(const Panel panel, const Pos& pos, const Pos& dims) {
  const Pos pixelPos = getPixelPosForCellInPanel(panel, pos);
  const Pos cellDims(Config::getCellW(), Config::getCellH());
  coverAreaPixel(pixelPos, dims * cellDims);
}

void coverAreaPixel(const Pos& pixelPos, const Pos& pixelDims) {
  drawRectangleSolid(pixelPos, pixelDims, clrBlack);
}

void coverCellInMap(const Pos& pos) {
  const Pos cellDims(Config::getCellW(), Config::getCellH());
  Pos pixelPos = getPixelPosForCellInPanel(Panel::map, pos);
  coverAreaPixel(pixelPos, cellDims);
}

void drawLineHor(const Pos& pixelPos, const int W,
                 const SDL_Color& clr) {
  drawRectangleSolid(pixelPos, Pos(W, 2), clr);
}

void drawLineVer(const Pos& pixelPos, const int H,
                 const SDL_Color& clr) {
  drawRectangleSolid(pixelPos, Pos(1, H), clr);
}

void drawRectangleSolid(const Pos& pixelPos, const Pos& pixelDims,
                        const SDL_Color& clr) {
  if(isInited()) {
    SDL_Rect sdlRect = {(Sint16)pixelPos.x, (Sint16)pixelPos.y,
                        (Uint16)pixelDims.x, (Uint16)pixelDims.y
                       };
    SDL_FillRect(screenSurface, &sdlRect,
                 SDL_MapRGB(screenSurface->format, clr.r, clr.g, clr.b));
  }
}

void drawProjectiles(vector<Projectile*>& projectiles,
                     const bool SHOULD_DRAW_MAP_BEFORE) {

  if(SHOULD_DRAW_MAP_BEFORE) {drawMapAndInterface(false);}

  for(Projectile* p : projectiles) {
    if(p->isDoneRendering == false && p->isVisibleToPlayer) {
      coverCellInMap(p->pos);
      if(Config::isTilesMode()) {
        if(p->tile != TileId::empty) {
          drawTile(p->tile, Panel::map, p->pos, p->clr);
        }
      } else {
        if(p->glyph != -1) {
          drawGlyph(p->glyph, Panel::map, p->pos, p->clr);
        }
      }
    }
  }

  updateScreen();
}

void drawPopupBox(const Rect& border, const Panel panel,
                  const SDL_Color& clr) {
  const bool IS_TILES = Config::isTilesMode();

  //Vertical bars
  const int Y0_VERT = border.p0.y + 1;
  const int Y1_VERT = border.p1.y - 1;
  for(int y = Y0_VERT; y <= Y1_VERT; y++) {
    if(IS_TILES) {
      drawTile(TileId::popupVerticalBar,
               panel, Pos(border.p0.x, y), clr, clrBlack);
      drawTile(TileId::popupVerticalBar,
               panel, Pos(border.p1.x, y), clr, clrBlack);
    } else {
      drawGlyph('|',
                panel, Pos(border.p0.x, y), clr, true, clrBlack);
      drawGlyph('|',
                panel, Pos(border.p1.x, y), clr, true, clrBlack);
    }
  }

  //Horizontal bars
  const int X0_VERT = border.p0.x + 1;
  const int X1_VERT = border.p1.x - 1;
  for(int x = X0_VERT; x <= X1_VERT; x++) {
    if(IS_TILES) {
      drawTile(TileId::popupHorizontalBar,
               panel, Pos(x, border.p0.y), clr, clrBlack);
      drawTile(TileId::popupHorizontalBar,
               panel, Pos(x, border.p1.y), clr, clrBlack);
    } else {
      drawGlyph('-', panel, Pos(x, border.p0.y), clr, true, clrBlack);
      drawGlyph('-', panel, Pos(x, border.p1.y), clr, true, clrBlack);
    }
  }

  //Corners
  if(IS_TILES) {
    drawTile(TileId::popupCornerTopLeft,
             panel, Pos(border.p0.x, border.p0.y), clr, clrBlack);
    drawTile(TileId::popupCornerTopRight,
             panel, Pos(border.p1.x, border.p0.y), clr, clrBlack);
    drawTile(TileId::popupCornerBottomLeft,
             panel, Pos(border.p0.x, border.p1.y), clr, clrBlack);
    drawTile(TileId::popupCornerBottomRight,
             panel, Pos(border.p1.x, border.p1.y), clr, clrBlack);
  } else {
    drawGlyph(
      '+', panel, Pos(border.p0.x, border.p0.y), clr, true, clrBlack);
    drawGlyph(
      '+', panel, Pos(border.p1.x, border.p0.y), clr, true, clrBlack);
    drawGlyph(
      '+', panel, Pos(border.p0.x, border.p1.y), clr, true, clrBlack);
    drawGlyph(
      '+', panel, Pos(border.p1.x, border.p1.y), clr, true, clrBlack);
  }
}

void drawMapAndInterface(const bool SHOULD_UPDATE_SCREEN) {
  if(isInited()) {
    clearScreen();

    drawMap();

    CharacterLines::drawInfoLines();
    CharacterLines::drawLocationInfo();
    Log::drawLog(false);

    if(SHOULD_UPDATE_SCREEN) {updateScreen();}
  }
}

void drawMap() {
  if(isInited() == false) {return;}

  CellRenderData* curDrw = nullptr;
  CellRenderData tmpDrw;

  const bool IS_TILES = Config::isTilesMode();

  //---------------- INSERT STATIC FEATURES AND BLOOD INTO ARRAY
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {

      if(Map::cells[x][y].isSeenByPlayer) {

        curDrw = &renderArray[x][y];
        curDrw->clear();

        const auto* const f = Map::cells[x][y].featureStatic;

        TileId  goreTile  = TileId::empty;
        char    goreGlyph = ' ';
        if(f->canHaveGore()) {
          goreTile  = f->getGoreTile();
          goreGlyph = f->getGoreGlyph();
        }
        if(goreTile == TileId::empty) {
          curDrw->tile  = f->getTile();
          curDrw->glyph = f->getGlyph();
          const SDL_Color& featureClr   = f->getClr();
          const SDL_Color& featureClrBg = f->getClrBg();
          curDrw->clr = f->hasBlood() ? clrRedLgt : featureClr;
          if(Utils::isClrEq(featureClrBg, clrBlack) == false) {
            curDrw->clrBg = featureClrBg;
          }
        } else {
          curDrw->tile  = goreTile;
          curDrw->glyph = goreGlyph;
          curDrw->clr   = clrRed;
        }
        if(Map::cells[x][y].isLight && f->canMoveCmn()) {
          curDrw->isMarkedAsLit = true;
        }
      }
    }
  }

  int xPos, yPos;
  //---------------- INSERT DEAD ACTORS INTO ARRAY
  for(Actor* actor : GameTime::actors_) {
    xPos = actor->pos.x;
    yPos = actor->pos.y;
    if(
      actor->deadState == ActorDeadState::corpse &&
      actor->getData().glyph != ' ' &&
      actor->getData().tile != TileId::empty &&
      Map::cells[xPos][yPos].isSeenByPlayer) {
      curDrw = &renderArray[xPos][yPos];
      curDrw->clr   = clrRed;
      curDrw->tile  = actor->getTile();
      curDrw->glyph = actor->getGlyph();
    }
  }

  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      curDrw = &renderArray[x][y];
      if(Map::cells[x][y].isSeenByPlayer) {
        //---------------- INSERT ITEMS INTO ARRAY
        const Item* const item = Map::cells[x][y].item;
        if(item != nullptr) {
          curDrw->clr   = item->getClr();
          curDrw->tile  = item->getTile();
          curDrw->glyph = item->getGlyph();
        }

        //COPY ARRAY TO PLAYER MEMORY (BEFORE LIVING ACTORS AND MOBILE FEATURES)
        renderArrayNoActors[x][y] = renderArray[x][y];

        //COLOR CELLS MARKED AS LIT YELLOW
        if(curDrw->isMarkedAsLit) {
          curDrw->clr = clrYellow;
        }
      }
    }
  }

  //---------------- INSERT MOBILE FEATURES INTO ARRAY
  for(auto* mob : GameTime::featureMobs_) {
    xPos = mob->getX();
    yPos = mob->getY();
    const TileId  mobTile   = mob->getTile();
    const char    mobGlyph  = mob->getGlyph();
    if(
      mobTile != TileId::empty && mobGlyph != ' ' &&
      Map::cells[xPos][yPos].isSeenByPlayer) {
      curDrw = &renderArray[xPos][yPos];
      curDrw->clr = mob->getClr();
      curDrw->tile  = mobTile;
      curDrw->glyph = mobGlyph;
    }
  }

  //---------------- INSERT LIVING ACTORS INTO ARRAY
  for(Actor* actor : GameTime::actors_) {
    if(actor != Map::player) {
      xPos = actor->pos.x;
      yPos = actor->pos.y;

      if(actor->deadState == ActorDeadState::alive) {

        curDrw = &renderArray[xPos][yPos];

        const auto* const monster = dynamic_cast<const Monster*>(actor);

        if(Map::player->isSeeingActor(*actor, nullptr)) {

          if(
            actor->getTile()  != TileId::empty &&
            actor->getGlyph() != ' ') {

            curDrw->clr   = actor->getClr();
            curDrw->tile  = actor->getTile();
            curDrw->glyph = actor->getGlyph();

            curDrw->lifebarLength = getLifebarLength(*actor);
            curDrw->isLivingActorSeenHere = true;
            curDrw->isFadeEffectAllowed = false;

            if(monster->leader == Map::player) {
              // TODO reimplement allied indicator
            } else {
              if(monster->awareOfPlayerCounter_ <= 0) {
                curDrw->clrBg = clrBlue;
              }
            }
          }
        } else {
          if(monster->playerAwareOfMeCounter_ > 0) {
            curDrw->isAwareOfMonsterHere  = true;
          }
        }
      }
    }
  }

  //---------------- DRAW THE GRID
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {

      tmpDrw = renderArray[x][y];

      if(Map::cells[x][y].isSeenByPlayer) {
        if(tmpDrw.isFadeEffectAllowed) {
          const int DIST_FROM_PLAYER =
            Utils::kingDist(Map::player->pos, Pos(x, y));
          if(DIST_FROM_PLAYER > 1) {
            const double DIST_FADE_DIV =
              min(2.0, 1.0 + (double(DIST_FROM_PLAYER - 1) * 0.33));
            tmpDrw.clr.r /= DIST_FADE_DIV;
            tmpDrw.clr.g /= DIST_FADE_DIV;
            tmpDrw.clr.b /= DIST_FADE_DIV;
          }
        }
      } else if(Map::cells[x][y].isExplored) {
        bool isAwareOfMonsterHere = tmpDrw.isAwareOfMonsterHere;
        renderArray[x][y] = Map::cells[x][y].playerVisualMemory;
        tmpDrw = renderArray[x][y];
        tmpDrw.isAwareOfMonsterHere = isAwareOfMonsterHere;

        tmpDrw.clr.r   /= 4; tmpDrw.clr.g   /= 4; tmpDrw.clr.b   /= 4;
        tmpDrw.clrBg.r /= 4; tmpDrw.clrBg.g /= 4; tmpDrw.clrBg.b /= 4;
      }

      if(IS_TILES) {
        //Walls are given perspective.
        //If the tile to be set is a (top) wall tile, check the tile beneath it.
        //If the tile beneath is not a front or top wall tile, and that cell is
        //explored, change the current tile to wall front
        if(
          tmpDrw.isLivingActorSeenHere == false &&
          tmpDrw.isAwareOfMonsterHere  == false) {
          const auto tileSeen = renderArrayNoActors[x][y].tile;
          const auto tileMem  = Map::cells[x][y].playerVisualMemory.tile;
          const bool IS_TILE_WALL =
            Map::cells[x][y].isSeenByPlayer ?
            Wall::isTileAnyWallTop(tileSeen) :
            Wall::isTileAnyWallTop(tileMem);
          if(IS_TILE_WALL) {
            const auto* const f = Map::cells[x][y].featureStatic;
            const auto featureId = f->getId();
            bool isHiddenDoor = false;
            if(featureId == FeatureId::door) {
              isHiddenDoor = dynamic_cast<const Door*>(f)->isSecret();
            }
            if(
              y < MAP_H - 1 &&
              (featureId == FeatureId::wall || isHiddenDoor)) {
              if(Map::cells[x][y + 1].isExplored) {
                const bool IS_SEEN_BELOW = Map::cells[x][y + 1].isSeenByPlayer;

                const auto tileBelowSeen = renderArrayNoActors[x][y + 1].tile;

                const auto tileBelowMem =
                  Map::cells[x][y + 1].playerVisualMemory.tile;

                const bool TILE_BELOW_IS_WALL_FRONT =
                  IS_SEEN_BELOW ? Wall::isTileAnyWallFront(tileBelowSeen) :
                  Wall::isTileAnyWallFront(tileBelowMem);

                const bool TILE_BELOW_IS_WALL_TOP =
                  IS_SEEN_BELOW ? Wall::isTileAnyWallTop(tileBelowSeen) :
                  Wall::isTileAnyWallTop(tileBelowMem);

                bool tileBelowIsRevealedDoor =
                  IS_SEEN_BELOW ? Door::isTileAnyDoor(tileBelowSeen) :
                  Door::isTileAnyDoor(tileBelowMem);

                if(
                  TILE_BELOW_IS_WALL_FRONT  ||
                  TILE_BELOW_IS_WALL_TOP    ||
                  tileBelowIsRevealedDoor) {
                  if(featureId == FeatureId::wall) {
                    const auto* const wall = dynamic_cast<const Wall*>(f);
                    tmpDrw.tile = wall->getTopWallTile();
                  }
                } else if(featureId == FeatureId::wall) {
                  const auto* const wall = dynamic_cast<const Wall*>(f);
                  tmpDrw.tile = wall->getFrontWallTile();
                } else if(isHiddenDoor) {
                  tmpDrw.tile = Config::isTilesWallFullSquare() ?
                                TileId::wallTop :
                                TileId::wallFront;
                }
              }
            }
          }
        }
      }

      Pos pos(x, y);

      if(tmpDrw.isAwareOfMonsterHere) {
        drawGlyph('!', Panel::map, pos, clrBlack, true, clrNosfTealDrk);
      } else if(tmpDrw.tile != TileId::empty && tmpDrw.glyph != ' ') {
        if(IS_TILES) {
          drawTile(tmpDrw.tile, Panel::map, pos, tmpDrw.clr, tmpDrw.clrBg);
        } else {
          drawGlyph(tmpDrw.glyph, Panel::map, pos, tmpDrw.clr, true,
                    tmpDrw.clrBg);
        }

        if(tmpDrw.lifebarLength != -1) {
          drawLifeBar(pos, tmpDrw.lifebarLength);
        }
      }

      if(Map::cells[x][y].isExplored == false) {
        renderArray[x][y].clear();
      }
    }
  }

  //---------------- DRAW PLAYER CHARACTER
  bool isRangedWpn = false;
  const Pos& pos = Map::player->pos;
  Item* item = Map::player->getInv().getItemInSlot(SlotId::wielded);
  if(item != nullptr) {
    isRangedWpn = item->getData().isRangedWeapon;
  }
  if(IS_TILES) {
    const TileId tile = isRangedWpn ?
                        TileId::playerFirearm : TileId::playerMelee;
    drawTile(tile, Panel::map, pos, Map::player->getClr(), clrBlack);
  } else {
    drawGlyph('@', Panel::map, pos, Map::player->getClr(), true, clrBlack);
  }
  const int LIFE_BAR_LENGTH = getLifebarLength(*Map::player);
  if(LIFE_BAR_LENGTH != -1) {
    drawLifeBar(pos, LIFE_BAR_LENGTH);
  }
  drawPlayerShockExclMarks();
}

} //Render

