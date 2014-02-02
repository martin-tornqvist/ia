#include "Renderer.h"

#include <vector>
#include <iostream>

#include <SDL_image.h>

#include "Engine.h"
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

using namespace std;

Renderer::Renderer(Engine& engine) : eng(engine), screenSurface_(NULL),
  mainMenuLogoSurface_(NULL) {

  initAndClearPrev();
}

Renderer::~Renderer() {
  freeAssets();
}

void Renderer::freeAssets() {
  trace << "Renderer::freeAssets()..." << endl;

  if(screenSurface_ != NULL) {
    SDL_FreeSurface(screenSurface_);
    screenSurface_ = NULL;
  }

  if(mainMenuLogoSurface_ != NULL) {
    SDL_FreeSurface(mainMenuLogoSurface_);
    mainMenuLogoSurface_ = NULL;
  }

  trace << "Renderer::freeAssets() [DONE]" << endl;
}

void Renderer::initAndClearPrev() {
  trace << "Renderer::initAndClearPrev()..." << endl;
  freeAssets();

  trace << "Renderer: Setting up rendering window" << endl;
  const string title = "IA " + gameVersionStr;
  SDL_WM_SetCaption(title.data(), NULL);

  const int W = eng.config->screenPixelW;
  const int H = eng.config->screenPixelH;
  if(eng.config->isFullscreen) {
    screenSurface_ =
      SDL_SetVideoMode(W, H, SCREEN_BPP, SDL_SWSURFACE | SDL_FULLSCREEN);
  }
  if(eng.config->isFullscreen == false || screenSurface_ == NULL) {
    screenSurface_ =
      SDL_SetVideoMode(W, H, SCREEN_BPP, SDL_SWSURFACE);
  }

  if(screenSurface_ == NULL) {
    trace << "[WARNING] Failed to create screen surface, ";
    trace << "in Renderer::initAndClearPrev()" << endl;
  }

  loadFont();

  if(eng.config->isTilesMode) {
    loadTiles();
    loadMainMenuLogo();
  }

  trace << "Renderer::initAndClearPrev() [DONE]" << endl;
}

void Renderer::loadFont() {
  trace << "Renderer::loadFont()..." << endl;

  SDL_Surface* fontSurfaceTmp = IMG_Load(eng.config->fontBig.data());
//    IMG_LoadPNG_RW(SDL_RWFromFile(, "r"));

//  fontSurface_ = SDL_DisplayFormatAlpha(fontSurfaceTmp);

  Uint32 imgClr = SDL_MapRGB(fontSurfaceTmp->format, 255, 255, 255);

  for(int y = 0; y < fontSurfaceTmp->h; y++) {
    for(int x = 0; x < fontSurfaceTmp->w; x++) {
      fontPixelData_[x][y] = getPixel(fontSurfaceTmp, x, y) == imgClr;
    }
  }

  SDL_FreeSurface(fontSurfaceTmp);

  trace << "Renderer::loadFont() [DONE]" << endl;
}

void Renderer::loadTiles() {
  trace << "Renderer::loadTiles()..." << endl;

  SDL_Surface* tileSurfaceTmp = IMG_Load(tilesImgName.data());

  Uint32 imgClr = SDL_MapRGB(tileSurfaceTmp->format, 255, 255, 255);
  for(int y = 0; y < tileSurfaceTmp->h; y++) {
    for(int x = 0; x < tileSurfaceTmp->w; x++) {
      tilePixelData_[x][y] = getPixel(tileSurfaceTmp, x, y) == imgClr;
    }
  }

  SDL_FreeSurface(tileSurfaceTmp);

  trace << "Renderer::loadTiles() [DONE]" << endl;
}

void Renderer::loadMainMenuLogo() {
  trace << "Renderer::loadMainMenuLogo()..." << endl;

  SDL_Surface* mainMenuLogoSurfaceTmp = IMG_Load(mainMenuLogoImgName.data());

  mainMenuLogoSurface_ = SDL_DisplayFormatAlpha(mainMenuLogoSurfaceTmp);

  SDL_FreeSurface(mainMenuLogoSurfaceTmp);

  trace << "Renderer::loadMainMenuLogo() [DONE]" << endl;
}


void Renderer::putPixelsOnScreenForTile(const Tile_t tile, const Pos& pixelPos,
                                        const SDL_Color& clr) {
  const int CLR_TO = SDL_MapRGB(screenSurface_->format, clr.r, clr.g, clr.b);

  SDL_LockSurface(screenSurface_);

  const int CELL_W = eng.config->cellW;
  const int CELL_H = eng.config->cellH;

  const Pos sheetPoss = eng.art->getTilePoss(tile);
  const int SHEET_X0  = sheetPoss.x * CELL_W;
  const int SHEET_Y0  = sheetPoss.y * CELL_H;
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
        putPixel(screenSurface_, screenX, screenY, CLR_TO);
      }
      screenX++;
    }
    screenY++;
  }

  SDL_UnlockSurface(screenSurface_);
}

void Renderer::putPixelsOnScreenForGlyph(const char GLYPH, const Pos& pixelPos,
    const SDL_Color& clr) {
  const int CLR_TO = SDL_MapRGB(screenSurface_->format, clr.r, clr.g, clr.b);

  SDL_LockSurface(screenSurface_);

  const int CELL_W = eng.config->cellW;
  const int CELL_H = eng.config->cellH;

  const int SCALE = 1;

  const int CELL_W_SHEET = CELL_W / SCALE;
  const int CELL_H_SHEET = CELL_H / SCALE;

  const Pos sheetPoss = eng.art->getGlyphPoss(GLYPH);
  const int SHEET_X0  = sheetPoss.x * CELL_W_SHEET;
  const int SHEET_Y0  = sheetPoss.y * CELL_H_SHEET;
  const int SHEET_X1  = SHEET_X0 + CELL_W_SHEET - 1;
  const int SHEET_Y1  = SHEET_Y0 + CELL_H_SHEET - 1;
  const int SCREEN_X0 = pixelPos.x;
  const int SCREEN_Y0 = pixelPos.y;

  int screenX = SCREEN_X0;
  int screenY = SCREEN_Y0;

  for(int sheetY = SHEET_Y0; sheetY <= SHEET_Y1; sheetY++) {
    screenX = SCREEN_X0;
    for(int sheetX = SHEET_X0; sheetX <= SHEET_X1; sheetX++) {
      if(fontPixelData_[sheetX][sheetY]) {
        for(int dy = 0; dy < SCALE; dy++) {
          for(int dx = 0; dx < SCALE; dx++) {
            putPixel(screenSurface_, screenX + dx, screenY + dy, CLR_TO);
          }
        }
      }
      screenX += SCALE;
    }
    screenY += SCALE;
  }

  SDL_UnlockSurface(screenSurface_);
}

Uint32 Renderer::getPixel(SDL_Surface* const surface,
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

void Renderer::putPixel(SDL_Surface* const surface,
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

void Renderer::applySurface(const Pos& pixelPos, SDL_Surface* const src,
                            SDL_Rect* clip) {
  SDL_Rect offset;
  offset.x = pixelPos.x;
  offset.y = pixelPos.y;
  SDL_BlitSurface(src, clip, screenSurface_, &offset);
}

void Renderer::drawMainMenuLogo(const int Y_POS) {
  const Pos pos((eng.config->screenPixelW - mainMenuLogoSurface_->w) / 2,
                eng.config->cellH * Y_POS);
  applySurface(pos, mainMenuLogoSurface_);
}

void Renderer::drawMarker(const vector<Pos>& trail,
                          const int EFFECTIVE_RANGE) {
  if(trail.size() > 2) {
    for(size_t i = 1; i < trail.size(); i++) {
      const Pos& pos = trail.at(i);
      coverCellInMap(pos);

      SDL_Color clr = clrGreenLgt;

      if(EFFECTIVE_RANGE != -1) {
        const int CHEB_DIST = eng.basicUtils->chebyshevDist(trail.at(0), pos);
        if(CHEB_DIST > EFFECTIVE_RANGE) {clr = clrYellow;}
      }
      if(eng.config->isTilesMode) {
        drawTile(tile_aimMarkerTrail, panel_map, pos, clr, clrBlack);
      } else {
        drawGlyph('*', panel_map, pos, clr, true, clrBlack);
      }
    }
  }

  const Pos& headPos = eng.marker->getPos();

  SDL_Color clr = clrGreenLgt;

  if(trail.size() > 2) {
    if(EFFECTIVE_RANGE != -1) {
      const int CHEB_DIST = eng.basicUtils->chebyshevDist(trail.at(0), headPos);
      if(CHEB_DIST > EFFECTIVE_RANGE) {
        clr = clrYellow;
      }
    }
  }

  if(eng.config->isTilesMode) {
    drawTile(tile_aimMarkerHead, panel_map, headPos, clr, clrBlack);
  } else {
    drawGlyph('X', panel_map, headPos, clr, true, clrBlack);
  }
}

void Renderer::drawBlastAnimationAtField(const Pos& center, const int RADIUS,
    bool forbiddenCells[MAP_W][MAP_H], const SDL_Color& colorInner,
    const SDL_Color& colorOuter) {
  trace << "Renderer::drawBlastAnimationAtField()..." << endl;

  drawMapAndInterface();

  bool isAnyBlastRendered = false;

  Pos pos;

  for(
    pos.y = max(1, center.y - RADIUS);
    pos.y <= min(MAP_H - 2, center.y + RADIUS);
    pos.y++) {
    for(
      pos.x = max(1, center.x - RADIUS);
      pos.x <= min(MAP_W - 2, center.x + RADIUS);
      pos.x++) {
      if(forbiddenCells[pos.x][pos.y] == false) {
        const bool IS_OUTER = pos.x == center.x - RADIUS ||
                              pos.x == center.x + RADIUS ||
                              pos.y == center.y - RADIUS ||
                              pos.y == center.y + RADIUS;
        const SDL_Color color = IS_OUTER ? colorOuter : colorInner;
        if(eng.config->isTilesMode) {
          drawTile(tile_blast1, panel_map, pos, color, clrBlack);
        } else {
          drawGlyph('*', panel_map, pos, color, true, clrBlack);
        }
        isAnyBlastRendered = true;
      }
    }
  }
  updateScreen();
  if(isAnyBlastRendered) {eng.sleep(eng.config->delayExplosion / 2);}

  for(
    pos.y = max(1, center.y - RADIUS);
    pos.y <= min(MAP_H - 2, center.y + RADIUS);
    pos.y++) {
    for(
      pos.x = max(1, center.x - RADIUS);
      pos.x <= min(MAP_W - 2, center.x + RADIUS);
      pos.x++) {
      if(forbiddenCells[pos.x][pos.y] == false) {
        const bool IS_OUTER = pos.x == center.x - RADIUS ||
                              pos.x == center.x + RADIUS ||
                              pos.y == center.y - RADIUS ||
                              pos.y == center.y + RADIUS;
        const SDL_Color color = IS_OUTER ? colorOuter : colorInner;
        if(eng.config->isTilesMode) {
          drawTile(tile_blast2, panel_map, pos, color, clrBlack);
        } else {
          drawGlyph('*', panel_map, pos, color, true, clrBlack);
        }
      }
    }
  }
  updateScreen();
  if(isAnyBlastRendered) {eng.sleep(eng.config->delayExplosion / 2);}
  drawMapAndInterface();

  trace << "Renderer::drawBlastAnimationAtField() [DONE]" << endl;
}

void Renderer::drawBlastAnimationAtPositions(
  const vector<Pos>& positions, const SDL_Color& color) {
  trace << "Renderer::drawBlastAnimationAtPositions()..." << endl;

  drawMapAndInterface();

  for(unsigned int i = 0; i < positions.size(); i++) {
    const Pos& pos = positions.at(i);
    if(eng.config->isTilesMode) {
      drawTile(tile_blast1, panel_map, pos, color, clrBlack);
    } else {
      drawGlyph('*', panel_map, pos, color, true, clrBlack);
    }
  }
  updateScreen();
  eng.sleep(eng.config->delayExplosion / 2);

  for(unsigned int i = 0; i < positions.size(); i++) {
    const Pos& pos = positions.at(i);
    if(eng.config->isTilesMode) {
      drawTile(tile_blast2, panel_map, pos, color, clrBlack);
    } else {
      drawGlyph('*', panel_map, pos, color, true, clrBlack);
    }
  }
  updateScreen();
  eng.sleep(eng.config->delayExplosion / 2);
  drawMapAndInterface();

  trace << "Renderer::drawBlastAnimationAtPositions() [DONE]" << endl;
}

void Renderer::drawBlastAnimationAtPositionsWithPlayerVision(
  const vector<Pos>& positions, const SDL_Color& clr) {

  vector<Pos> positionsWithVision;
  for(unsigned int i = 0; i < positions.size(); i++) {
    const Pos& pos = positions.at(i);
    if(eng.map->cells[pos.x][pos.y].isSeenByPlayer) {
      positionsWithVision.push_back(pos);
    }
  }

  eng.renderer->drawBlastAnimationAtPositions(positionsWithVision, clr);
}

void Renderer::drawTile(const Tile_t tile, const Panel_t panel, const Pos& pos,
                        const SDL_Color& clr, const SDL_Color& bgClr) {
  const Pos pixelPos = getPixelPosForCellInPanel(panel, pos);
  const Pos cellDims(eng.config->cellW, eng.config->cellH);
  drawRectangleSolid(pixelPos, cellDims, bgClr);
  putPixelsOnScreenForTile(tile, pixelPos, clr);
}

void Renderer::drawGlyphAtPixel(const char GLYPH, const Pos& pixelPos,
                                const SDL_Color& clr, const bool DRAW_BG_CLR,
                                const SDL_Color& bgClr) {
  if(DRAW_BG_CLR) {
    const Pos cellDims(eng.config->cellW, eng.config->cellH);
    drawRectangleSolid(pixelPos, cellDims, bgClr);
  }

  putPixelsOnScreenForGlyph(GLYPH, pixelPos, clr);
}

Pos Renderer::getPixelPosForCellInPanel(
  const Panel_t panel, const Pos& pos) const {
  const Pos cellDims(eng.config->cellW, eng.config->cellH);

  switch(panel) {
    case panel_screen: {
      return Pos(pos.x * cellDims.x, pos.y * cellDims.y);
    } break;

    case panel_map: {
      return (pos * cellDims) + Pos(0, eng.config->mapPixelOffsetH);
    } break;

    case panel_log: {
      return pos * cellDims;
    } break;

    case panel_char: {
      return (pos * cellDims) + Pos(0, eng.config->charLinesPixelOffsetH);
    } break;
  }
  return Pos();
}

void Renderer::drawGlyph(const char GLYPH, const Panel_t panel, const Pos& pos,
                         const SDL_Color& clr, const bool DRAW_BG_CLR,
                         const SDL_Color& bgClr) {
  const Pos pixelPos = getPixelPosForCellInPanel(panel, pos);
  drawGlyphAtPixel(GLYPH, pixelPos, clr, DRAW_BG_CLR, bgClr);
}

void Renderer::drawText(const string& str, const Panel_t panel,
                        const Pos& pos, const SDL_Color& clr,
                        const SDL_Color& bgClr) {
  Pos pixelPos = getPixelPosForCellInPanel(panel, pos);

  if(pixelPos.y < 0 || pixelPos.y >= eng.config->screenPixelH) {
    return;
  }

  const Pos cellDims(eng.config->cellW, eng.config->cellH);
  const int LEN = str.size();
  drawRectangleSolid(pixelPos, Pos(cellDims.x * LEN, cellDims.y), bgClr);

  for(int i = 0; i < LEN; i++) {
    if(pixelPos.x < 0 || pixelPos.x >= eng.config->screenPixelW) {
      return;
    }
    drawGlyphAtPixel(str.at(i), pixelPos, clr, false);
    pixelPos.x += cellDims.x;
  }
}

int Renderer::drawTextCentered(const string& str, const Panel_t panel,
                               const Pos& pos, const SDL_Color& clr,
                               const SDL_Color& bgClr,
                               const bool IS_PIXEL_POS_ADJ_ALLOWED) {
  const int LEN         = str.size();
  const int LEN_HALF    = LEN / 2;
  const int X_POS_LEFT  = pos.x - LEN_HALF;

  const Pos cellDims(eng.config->cellW, eng.config->cellH);

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
  SDL_FillRect(screenSurface_, &sdlRect, SDL_MapRGB(screenSurface_->format,
               bgClr.r, bgClr.g, bgClr.b));

  for(int i = 0; i < LEN; i++) {
    if(pixelPos.x < 0 || pixelPos.x >= eng.config->screenPixelW) {
      return X_POS_LEFT;
    }
    drawGlyphAtPixel(str.at(i), pixelPos, clr, false);
    pixelPos.x += cellDims.x;
  }
  return X_POS_LEFT;
}

void Renderer::coverPanel(const Panel_t panel) {
  const Config& conf = *eng.config;
  const int SCREEN_PIXEL_W = conf.screenPixelW;

  switch(panel) {
    case panel_char: {
      const Pos pixelPos = getPixelPosForCellInPanel(panel, Pos(0, 0));
      coverAreaPixel(pixelPos, Pos(SCREEN_PIXEL_W, conf.charLinesPixelH));
    } break;

    case panel_log: {
      coverAreaPixel(Pos(0, 0), Pos(SCREEN_PIXEL_W, conf.logPixelH));
    } break;

    case panel_map: {
      const Pos pixelPos = getPixelPosForCellInPanel(panel, Pos(0, 0));
      coverAreaPixel(pixelPos, Pos(SCREEN_PIXEL_W, conf.mapPixelH));
    } break;

    case panel_screen: {clearScreen();} break;
  }
}

void Renderer::coverArea(const Panel_t panel, const Pos& pos, const Pos& dims) {
  const Pos pixelPos = getPixelPosForCellInPanel(panel, pos);
  const Pos cellDims(eng.config->cellW, eng.config->cellH);
  coverAreaPixel(pixelPos, dims * cellDims);
}

void Renderer::coverAreaPixel(const Pos& pixelPos, const Pos& pixelDims) {
  drawRectangleSolid(pixelPos, pixelDims, clrBlack);
}

void Renderer::coverCellInMap(const Pos& pos) {
  const Pos cellDims(eng.config->cellW, eng.config->cellH);
  Pos pixelPos = getPixelPosForCellInPanel(panel_map, pos);
  coverAreaPixel(pixelPos, cellDims);
}

void Renderer::drawLineHor(const Pos& pixelPos, const int W,
                           const SDL_Color& clr) const {
  const int SCALE = 1;
  const Pos offset(0, 1 - SCALE);
  drawRectangleSolid(pixelPos + offset, Pos(W, 2 * SCALE), clr);
}

void Renderer::drawLineVer(const Pos& pixelPos, const int H,
                           const SDL_Color& clr) const {
  drawRectangleSolid(pixelPos, Pos(1, H), clr);
}

void Renderer::drawRectangleSolid(const Pos& pixelPos, const Pos& pixelDims,
                                  const SDL_Color& clr) const {
  SDL_Rect sdlRect = {(Sint16)pixelPos.x, (Sint16)pixelPos.y,
                      (Uint16)pixelDims.x, (Uint16)pixelDims.y
                     };
  SDL_FillRect(screenSurface_, &sdlRect,
               SDL_MapRGB(screenSurface_->format, clr.r, clr.g, clr.b));
}

void Renderer::coverGlyphAtPixel(const Pos& pixelPos) {
  coverAreaPixel(pixelPos, Pos(eng.config->cellW, eng.config->cellH));
}

void Renderer::coverTileAtPixel(const Pos& pixelPos) {
  coverAreaPixel(pixelPos, Pos(eng.config->cellW, eng.config->cellH));
}

void Renderer::drawProjectiles(vector<Projectile*>& projectiles) {
  drawMapAndInterface(false);

  for(unsigned int i = 0; i < projectiles.size(); i++) {
    Projectile* const p = projectiles.at(i);
    if(p->isDoneRendering == false && p->isVisibleToPlayer) {
      coverCellInMap(p->pos);
      if(eng.config->isTilesMode) {
        if(p->tile != tile_empty) {
          drawTile(p->tile, panel_map, p->pos, p->clr);
        }
      } else {
        if(p->glyph != -1) {
          drawGlyph(p->glyph, panel_map, p->pos, p->clr);
        }
      }
    }
  }

  updateScreen();
}

void Renderer::drawPopupBox(const Rect& border, const Panel_t panel,
                            const SDL_Color& clr) {
  const bool IS_TILES = eng.config->isTilesMode;

  //Vertical bars
  const int Y0_VERT = border.x0y0.y + 1;
  const int Y1_VERT = border.x1y1.y - 1;
  for(int y = Y0_VERT; y <= Y1_VERT; y++) {
    if(IS_TILES) {
      drawTile(tile_popupVerticalBar,
               panel, Pos(border.x0y0.x, y), clr, clrBlack);
      drawTile(tile_popupVerticalBar,
               panel, Pos(border.x1y1.x, y), clr, clrBlack);
    } else {
      drawGlyph('|',
                panel, Pos(border.x0y0.x, y), clr, true, clrBlack);
      drawGlyph('|',
                panel, Pos(border.x1y1.x, y), clr, true, clrBlack);
    }
  }

  //Horizontal bars
  const int X0_VERT = border.x0y0.x + 1;
  const int X1_VERT = border.x1y1.x - 1;
  for(int x = X0_VERT; x <= X1_VERT; x++) {
    if(IS_TILES) {
      drawTile(tile_popupHorizontalBar,
               panel, Pos(x, border.x0y0.y), clr, clrBlack);
      drawTile(tile_popupHorizontalBar,
               panel, Pos(x, border.x1y1.y), clr, clrBlack);
    } else {
      drawGlyph('-', panel, Pos(x, border.x0y0.y), clr, true, clrBlack);
      drawGlyph('-', panel, Pos(x, border.x1y1.y), clr, true, clrBlack);
    }
  }

  //Corners
  if(IS_TILES) {
    drawTile(tile_popupCornerTopLeft,
             panel, Pos(border.x0y0.x, border.x0y0.y), clr, clrBlack);
    drawTile(tile_popupCornerTopRight,
             panel, Pos(border.x1y1.x, border.x0y0.y), clr, clrBlack);
    drawTile(tile_popupCornerBottomLeft,
             panel, Pos(border.x0y0.x, border.x1y1.y), clr, clrBlack);
    drawTile(tile_popupCornerBottomRight,
             panel, Pos(border.x1y1.x, border.x1y1.y), clr, clrBlack);
  } else {
    drawGlyph(
      '+', panel, Pos(border.x0y0.x, border.x0y0.y), clr, true, clrBlack);
    drawGlyph(
      '+', panel, Pos(border.x1y1.x, border.x0y0.y), clr, true, clrBlack);
    drawGlyph(
      '+', panel, Pos(border.x0y0.x, border.x1y1.y), clr, true, clrBlack);
    drawGlyph(
      '+', panel, Pos(border.x1y1.x, border.x1y1.y), clr, true, clrBlack);
  }
}

void Renderer::drawMapAndInterface(const bool SHOULD_UPDATE_SCREEN) {
  clearScreen();

  drawMap();

  CharacterLines::drawInfoLines(eng);
  CharacterLines::drawLocationInfo(eng);
  eng.log->drawLog(false);

  if(SHOULD_UPDATE_SCREEN) {updateScreen();}
}

int Renderer::getLifebarLength(const Actor& actor) const {
  const int ACTOR_HP = max(0, actor.getHp());
  const int ACTOR_HP_MAX = actor.getHpMax(true);
  if(ACTOR_HP < ACTOR_HP_MAX) {
    int HP_PERCENT = (ACTOR_HP * 100) / ACTOR_HP_MAX;
    return ((eng.config->cellW - 2) * HP_PERCENT) / 100;
  }
  return -1;
}

void Renderer::drawLifeBar(const Pos& pos, const int LENGTH) {
  if(LENGTH >= 0) {
    const Pos cellDims(eng.config->cellW,  eng.config->cellH);
    const int W_GREEN   = LENGTH;
    const int W_BAR_TOT = cellDims.x - 2;
    const int W_RED     = W_BAR_TOT - W_GREEN;
    const Pos pixelPos =
      getPixelPosForCellInPanel(panel_map, pos + Pos(0, 1)) - Pos(0, 2);
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

void Renderer::drawExclMarkAt(const Pos& pixelPos) const {
  drawRectangleSolid(pixelPos,  Pos(3, 12),     clrBlack);
  drawLineVer(pixelPos +        Pos(1,  1), 6,  clrMagentaLgt);
  drawLineVer(pixelPos +        Pos(1,  9), 2,  clrMagentaLgt);
}

void Renderer::drawPlayerShockExclMarks() const {
  const double SHOCK  = eng.player->getPermShockTakenCurTurn();
  const int NR_EXCL   = SHOCK > 8 ? 3 : SHOCK > 3 ? 2 : SHOCK > 1 ? 1 : 0;

  if(NR_EXCL > 0) {
    const Pos& playerPos = eng.player->pos;
    const Pos pixelPosRight =
      getPixelPosForCellInPanel(panel_map, playerPos);

    for(int i = 0; i < NR_EXCL; i++) {
      drawExclMarkAt(pixelPosRight + Pos(i * 3, 0));
    }
  }
}

void Renderer::drawMap() {
  CellRenderData* curDrw = NULL;
  CellRenderData tmpDrw;

  const bool IS_TILES = eng.config->isTilesMode;

  //---------------- INSERT STATIC FEATURES AND BLOOD INTO ARRAY
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {

      if(eng.map->cells[x][y].isSeenByPlayer) {

        curDrw = &renderArray[x][y];
        curDrw->clear();

        const FeatureStatic* const f = eng.map->cells[x][y].featureStatic;

        Tile_t  goreTile  = tile_empty;
        char    goreGlyph = ' ';
        if(f->canHaveGore()) {
          goreTile  = f->getGoreTile();
          goreGlyph = f->getGoreGlyph();
        }
        if(goreTile == tile_empty) {
          curDrw->tile  = f->getTile();
          curDrw->glyph = f->getGlyph();
          const SDL_Color& featureClr   = f->getColor();
          const SDL_Color& featureClrBg = f->getColorBg();
          curDrw->clr = f->hasBlood() ? clrRedLgt : featureClr;
          if(eng.basicUtils->isClrEq(featureClrBg, clrBlack) == false) {
            curDrw->clrBg = featureClrBg;
          }
        } else {
          curDrw->tile  = goreTile;
          curDrw->glyph = goreGlyph;
          curDrw->clr   = clrRed;
        }
        if(eng.map->cells[x][y].isLight) {
          if(f->canMoveCmn()) {
            curDrw->isMarkedAsLit = true;
          }
        }
      }
    }
  }

  int xPos, yPos;
  //---------------- INSERT DEAD ACTORS INTO ARRAY
  for(Actor * actor : eng.gameTime->actors_) {
    xPos = actor->pos.x;
    yPos = actor->pos.y;
    if(
      actor->deadState == actorDeadState_corpse &&
      actor->getData().glyph != ' ' &&
      actor->getData().tile != tile_empty &&
      eng.map->cells[xPos][yPos].isSeenByPlayer) {
      curDrw = &renderArray[xPos][yPos];
      curDrw->clr   = clrRed;
      curDrw->tile  = actor->getTile();
      curDrw->glyph = actor->getGlyph();
    }
  }

  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      curDrw = &renderArray[x][y];
      if(eng.map->cells[x][y].isSeenByPlayer) {
        //---------------- INSERT ITEMS INTO ARRAY
        const Item* const item = eng.map->cells[x][y].item;
        if(item != NULL) {
          curDrw->clr   = item->getColor();
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
  for(FeatureMob * mob : eng.gameTime->featureMobs_) {
    xPos = mob->getX();
    yPos = mob->getY();
    const Tile_t  mobTile   = mob->getTile();
    const char    mobGlyph  = mob->getGlyph();
    if(
      mobTile != tile_empty && mobGlyph != ' ' &&
      eng.map->cells[xPos][yPos].isSeenByPlayer) {
      curDrw = &renderArray[xPos][yPos];
      curDrw->clr = mob->getColor();
      curDrw->tile  = mobTile;
      curDrw->glyph = mobGlyph;
    }
  }

  //---------------- INSERT LIVING ACTORS INTO ARRAY
  for(Actor * actor : eng.gameTime->actors_) {
    if(actor != eng.player) {
      xPos = actor->pos.x;
      yPos = actor->pos.y;

      if(actor->deadState == actorDeadState_alive) {

        curDrw = &renderArray[xPos][yPos];

        const Monster* const monster = dynamic_cast<const Monster*>(actor);

        if(eng.player->checkIfSeeActor(*actor, NULL)) {

          if(
            actor->getTile()  != tile_empty &&
            actor->getGlyph() != ' ') {

            curDrw->clr   = actor->getColor();
            curDrw->tile  = actor->getTile();
            curDrw->glyph = actor->getGlyph();

            curDrw->lifebarLength = getLifebarLength(*actor);

            curDrw->isFadeEffectAllowed = false;

            if(monster->leader == eng.player) {
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

      if(eng.map->cells[x][y].isSeenByPlayer) {
        if(tmpDrw.isFadeEffectAllowed) {
          const int DIST_FROM_PLAYER =
            eng.basicUtils->chebyshevDist(eng.player->pos, Pos(x, y));
          if(DIST_FROM_PLAYER > 1) {
            const double DIST_FADE_DIV =
              min(2.0, 1.0 + (double(DIST_FROM_PLAYER - 1) * 0.33));
            tmpDrw.clr.r /= DIST_FADE_DIV;
            tmpDrw.clr.g /= DIST_FADE_DIV;
            tmpDrw.clr.b /= DIST_FADE_DIV;
          }
        }
      } else if(eng.map->cells[x][y].isExplored) {
        bool isAwareOfMonsterHere = tmpDrw.isAwareOfMonsterHere;
        renderArray[x][y] = eng.map->cells[x][y].playerVisualMemory;
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
          const Tile_t tileSeen = renderArrayNoActors[x][y].tile;
          const Tile_t tileMem  = eng.map->cells[x][y].playerVisualMemory.tile;
          const bool IS_TILE_WALL =
            eng.map->cells[x][y].isSeenByPlayer ?
            Wall::isTileAnyWallTop(tileSeen) :
            Wall::isTileAnyWallTop(tileMem);
          if(IS_TILE_WALL) {
            const Feature* const f = eng.map->cells[x][y].featureStatic;
            const Feature_t featureId = f->getId();
            bool isHiddenDoor = false;
            if(featureId == feature_door) {
              isHiddenDoor = dynamic_cast<const Door*>(f)->isSecret();
            }
            if(
              y < MAP_H - 1 &&
              (featureId == feature_stoneWall || isHiddenDoor)) {
              if(eng.map->cells[x][y + 1].isExplored) {
                const bool IS_CELL_BELOW_SEEN =
                  eng.map->cells[x][y + 1].isSeenByPlayer;

                const Tile_t tileBelowSeen =
                  renderArrayNoActors[x][y + 1].tile;

                const Tile_t tileBelowMem =
                  eng.map->cells[x][y + 1].playerVisualMemory.tile;

                const bool TILE_BELOW_IS_WALL_FRONT =
                  IS_CELL_BELOW_SEEN ? Wall::isTileAnyWallFront(tileBelowSeen) :
                  Wall::isTileAnyWallFront(tileBelowMem);

                const bool TILE_BELOW_IS_WALL_TOP =
                  IS_CELL_BELOW_SEEN ? Wall::isTileAnyWallTop(tileBelowSeen) :
                  Wall::isTileAnyWallTop(tileBelowMem);

                bool tileBelowIsRevealedDoor =
                  IS_CELL_BELOW_SEEN ? Door::isTileAnyDoor(tileBelowSeen) :
                  Door::isTileAnyDoor(tileBelowMem);

                if(
                  TILE_BELOW_IS_WALL_FRONT  ||
                  TILE_BELOW_IS_WALL_TOP    ||
                  tileBelowIsRevealedDoor) {
                  if(featureId == feature_stoneWall) {
                    const Wall* const wall = dynamic_cast<const Wall*>(f);
                    tmpDrw.tile = wall->getTopWallTile();
                  }
                } else if(featureId == feature_stoneWall) {
                  const Wall* const wall = dynamic_cast<const Wall*>(f);
                  tmpDrw.tile = wall->getFrontWallTile();
                } else if(isHiddenDoor) {
                  tmpDrw.tile = eng.config->isTilesWallSymbolFullSquare ?
                                tile_wallTop :
                                tile_wallFront;
                }
              }
            }
          }
        }
      }

      Pos pos(x, y);

      if(tmpDrw.isAwareOfMonsterHere) {
        drawGlyph('!', panel_map, pos, clrBlack, true, clrNosfTealDrk);
      } else if(tmpDrw.tile != tile_empty && tmpDrw.glyph != ' ') {
        if(IS_TILES) {
          drawTile(tmpDrw.tile, panel_map, pos, tmpDrw.clr, tmpDrw.clrBg);
        } else {
          drawGlyph(tmpDrw.glyph, panel_map, pos, tmpDrw.clr, true,
                    tmpDrw.clrBg);
        }

        if(tmpDrw.lifebarLength != -1) {
          drawLifeBar(pos, tmpDrw.lifebarLength);
        }
      }

      if(eng.map->cells[x][y].isExplored == false) {
        renderArray[x][y].clear();
      }
    }
  }

  //---------------- DRAW PLAYER CHARACTER
  bool isRangedWpn = false;
  const Pos& pos = eng.player->pos;
  Item* item = eng.player->getInv().getItemInSlot(slot_wielded);
  if(item != NULL) {
    isRangedWpn = item->getData().isRangedWeapon;
  }
  if(IS_TILES) {
    const Tile_t tile = isRangedWpn ? tile_playerFirearm : tile_playerMelee;
    drawTile(tile, panel_map, pos, eng.player->getColor(), clrBlack);
  } else {
    drawGlyph('@', panel_map, pos, eng.player->getColor(), true, clrBlack);
  }
  const int LIFE_BAR_LENGTH = getLifebarLength(*eng.player);
  if(LIFE_BAR_LENGTH != -1) {
    drawLifeBar(pos, LIFE_BAR_LENGTH);
  }
  drawPlayerShockExclMarks();
}
