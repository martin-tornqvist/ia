#include "Render.h"

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
#include "ActorMon.h"
#include "Log.h"
#include "Attack.h"
#include "FeatureMob.h"
#include "FeatureDoor.h"
#include "Inventory.h"
#include "Utils.h"
#include "CmnData.h"
#include "SdlWrapper.h"
#include "TextFormatting.h"

using namespace std;

namespace Render
{

CellRenderData  renderArray[MAP_W][MAP_H];
CellRenderData  renderArrayNoActors[MAP_W][MAP_H];
SDL_Surface*    screenSurface       = nullptr;
SDL_Surface*    mainMenuLogoSurface = nullptr;

namespace
{

const size_t PIXEL_DATA_W = 400;
const size_t PIXEL_DATA_H = 400;

bool tilePixelData_[PIXEL_DATA_W][PIXEL_DATA_H];
bool fontPixelData_[PIXEL_DATA_W][PIXEL_DATA_H];
bool contourPixelData_[PIXEL_DATA_W][PIXEL_DATA_H];

bool isInited()
{
  return screenSurface;
}

void loadMainMenuLogo()
{
  TRACE_FUNC_BEGIN;

  SDL_Surface* mainMenuLogoSurfaceTmp = IMG_Load(mainMenuLogoImgName.data());

  mainMenuLogoSurface = SDL_DisplayFormatAlpha(mainMenuLogoSurfaceTmp);

  SDL_FreeSurface(mainMenuLogoSurfaceTmp);

  TRACE_FUNC_END;
}

Uint32 getPixel(SDL_Surface* const surface, const int PIXEL_X, const int PIXEL_Y)
{
  int bpp = surface->format->BytesPerPixel;
  /* Here p is the address to the pixel we want to retrieve */
  Uint8* p = (Uint8*)surface->pixels + PIXEL_Y * surface->pitch + PIXEL_X * bpp;

  switch (bpp)
  {
    case 1:   return *p;          break;
    case 2:   return *(Uint16*)p; break;
    case 3:
    {
      if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
      {
        return p[0] << 16 | p[1] << 8 | p[2];
      }
      else
      {
        return p[0] | p[1] << 8 | p[2] << 16;
      }
    } break;
    case 4:   return *(Uint32*)p; break;
    default:  return -1;          break;
  }
  return -1;
}

inline void putPixel(SDL_Surface* const surface, const int PIXEL_X, const int PIXEL_Y,
                     Uint32 pixel)
{
  int bpp = surface->format->BytesPerPixel;
  //Here p is the address to the pixel we want to set
  Uint8* p = (Uint8*)surface->pixels + PIXEL_Y * surface->pitch + PIXEL_X * bpp;

  switch (bpp)
  {
    case 1:
      *p = pixel;
      break;

    case 2:
      *(Uint16*)p = pixel;
      break;

    case 3:
      if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
      {
        p[0] = (pixel >> 16) & 0xff;
        p[1] = (pixel >> 8)  & 0xff;
        p[2] = pixel & 0xff;
      }
      else //Little endian
      {
        p[0] = pixel & 0xff;
        p[1] = (pixel >> 8)  & 0xff;
        p[2] = (pixel >> 16) & 0xff;
      }
      break;

    case 4:
      *(Uint32*)p = pixel;
      break;

    default: {} break;
  }
}

void loadFont()
{
  TRACE_FUNC_BEGIN;

  SDL_Surface* fontSurfaceTmp = IMG_Load(Config::getFontName().data());

  Uint32 bgClr = SDL_MapRGB(fontSurfaceTmp->format, 0, 0, 0);

  for (int x = 0; x < fontSurfaceTmp->w; ++x)
  {
    for (int y = 0; y < fontSurfaceTmp->h; ++y)
    {
      fontPixelData_[x][y] = getPixel(fontSurfaceTmp, x, y) != bgClr;
    }
  }

  SDL_FreeSurface(fontSurfaceTmp);

  TRACE_FUNC_END;
}

void loadTiles()
{
  TRACE_FUNC_BEGIN;

  SDL_Surface* tileSurfaceTmp = IMG_Load(tilesImgName.data());

  Uint32 imgClr = SDL_MapRGB(tileSurfaceTmp->format, 255, 255, 255);
  for (int x = 0; x < tileSurfaceTmp->w; ++x)
  {
    for (int y = 0; y < tileSurfaceTmp->h; ++y)
    {
      tilePixelData_[x][y] = getPixel(tileSurfaceTmp, x, y) == imgClr;
    }
  }

  SDL_FreeSurface(tileSurfaceTmp);

  TRACE_FUNC_END;
}

void loadContour(const bool base[PIXEL_DATA_W][PIXEL_DATA_H])
{
  const Pos cellDims(Config::getCellW(), Config::getCellH());

  for (size_t pxX = 0; pxX < PIXEL_DATA_W; ++pxX)
  {
    for (size_t pxY = 0; pxY < PIXEL_DATA_H; ++pxY)
    {

      bool& curVal  = contourPixelData_[pxX][pxY];
      curVal        = false;

      //Only mark this pixel as contour if it's not marked on the base image
      if (!base[pxX][pxY])
      {
        //Position interval to check for this pixel is constrained within current image
        const Pos curImgPxP0((Pos(pxX, pxY) / cellDims) * cellDims);
        const Pos curImgPxP1(curImgPxP0 + cellDims - 1);
        const Pos pxP0(max(curImgPxP0.x, int(pxX - 1)), max(curImgPxP0.y, int(pxY - 1)));
        const Pos pxP1(min(curImgPxP1.x, int(pxX + 1)), min(curImgPxP1.y, int(pxY + 1)));

        for (int pxCheckX = pxP0.x; pxCheckX <= pxP1.x; ++pxCheckX)
        {
          for (int pxCheckY = pxP0.y; pxCheckY <= pxP1.y; ++pxCheckY)
          {
            if (base[pxCheckX][pxCheckY])
            {
              curVal = true;
              break;
            }
          }
          if (curVal) {break;}
        }
      }
    }
  }
}

void putPixelsOnScr(const bool pixelData[PIXEL_DATA_W][PIXEL_DATA_H],
                    const Pos& sheetPos, const Pos& scrPixelPos, const Clr& clr)
{
  if (isInited())
  {
    const int CLR_TO = SDL_MapRGB(screenSurface->format, clr.r, clr.g, clr.b);

    SDL_LockSurface(screenSurface);

    const int CELL_W      = Config::getCellW();
    const int CELL_H      = Config::getCellH();
    const int SHEET_PX_X0 = sheetPos.x * CELL_W;
    const int SHEET_PX_Y0 = sheetPos.y * CELL_H;
    const int SHEET_PX_X1 = SHEET_PX_X0 + CELL_W - 1;
    const int SHEET_PX_Y1 = SHEET_PX_Y0 + CELL_H - 1;
    const int SCR_PX_X0   = scrPixelPos.x;
    const int SCR_PX_Y0   = scrPixelPos.y;

    int scrPxX = SCR_PX_X0;
    int scrPxY = SCR_PX_Y0;

    for (int sheetPxX = SHEET_PX_X0; sheetPxX <= SHEET_PX_X1; sheetPxX++)
    {
      scrPxY = SCR_PX_Y0;
      for (int sheetPxY = SHEET_PX_Y0; sheetPxY <= SHEET_PX_Y1; sheetPxY++)
      {
        if (pixelData[sheetPxX][sheetPxY])
        {
          putPixel(screenSurface, scrPxX, scrPxY, CLR_TO);
        }
        ++scrPxY;
      }
      ++scrPxX;
    }

    SDL_UnlockSurface(screenSurface);
  }
}

void putPixelsOnScrForTile(const TileId tile, const Pos& scrPixelPos, const Clr& clr)
{
  putPixelsOnScr(tilePixelData_, Art::getTilePos(tile), scrPixelPos, clr);
}

void putPixelsOnScrForGlyph(const char GLYPH, const Pos& scrPixelPos, const Clr& clr)
{
  putPixelsOnScr(fontPixelData_, Art::getGlyphPos(GLYPH), scrPixelPos, clr);
}

Pos getPixelPosForCellInPanel(const Panel panel, const Pos& pos)
{
  const Pos cellDims(Config::getCellW(), Config::getCellH());

  switch (panel)
  {
    case Panel::screen:
      return Pos(pos.x * cellDims.x, pos.y * cellDims.y);

    case Panel::map:
      return (pos * cellDims) + Pos(0, Config::getMapPixelOffsetH());

    case Panel::log:
      return pos * cellDims;

    case Panel::charLines:
      return (pos * cellDims) + Pos(0, Config::getCharLinesPixelOffsetH());
  }
  return Pos();
}

int getLifebarLength(const Actor& actor)
{
  const int ACTOR_HP = max(0, actor.getHp());
  const int ACTOR_HP_MAX = actor.getHpMax(true);
  if (ACTOR_HP < ACTOR_HP_MAX)
  {
    int HP_PERCENT = (ACTOR_HP * 100) / ACTOR_HP_MAX;
    return ((Config::getCellW() - 2) * HP_PERCENT) / 100;
  }
  return -1;
}

void drawLifeBar(const Pos& pos, const int LENGTH)
{
  if (LENGTH >= 0)
  {
    const Pos cellDims(Config::getCellW(),  Config::getCellH());
    const int W_GREEN   = LENGTH;
    const int W_BAR_TOT = cellDims.x - 2;
    const int W_RED     = W_BAR_TOT - W_GREEN;
    const Pos pixelPos =
      getPixelPosForCellInPanel(Panel::map, pos + Pos(0, 1)) - Pos(0, 2);
    const int X0_GREEN  = pixelPos.x + 1;
    const int X0_RED    = X0_GREEN + W_GREEN;

    if (W_GREEN > 0)
    {
      drawLineHor(Pos(X0_GREEN, pixelPos.y), W_GREEN, clrGreenLgt);
    }
    if (W_RED > 0)
    {
      drawLineHor(Pos(X0_RED, pixelPos.y), W_RED, clrRedLgt);
    }
  }
}

void drawExclMarkAt(const Pos& pixelPos)
{
  drawRectangleSolid(pixelPos,  Pos(3, 12),     clrBlack);
  drawLineVer(pixelPos +        Pos(1,  1), 6,  clrMagentaLgt);
  drawLineVer(pixelPos +        Pos(1,  9), 2,  clrMagentaLgt);
}

void drawPlayerShockExclMarks()
{
  const double SHOCK  = Map::player->getPermShockTakenCurTurn();
  const int NR_EXCL   = SHOCK > 8 ? 3 : SHOCK > 3 ? 2 : SHOCK > 1 ? 1 : 0;

  if (NR_EXCL > 0)
  {
    const Pos& playerPos = Map::player->pos;
    const Pos pixelPosRight = getPixelPosForCellInPanel(Panel::map, playerPos);

    for (int i = 0; i < NR_EXCL; ++i)
    {
      drawExclMarkAt(pixelPosRight + Pos(i * 3, 0));
    }
  }
}

void drawGlyphAtPixel(const char GLYPH, const Pos& pixelPos, const Clr& clr,
                      const bool DRAW_BG_CLR, const Clr& bgClr = clrBlack)
{
  if (DRAW_BG_CLR)
  {
    const Pos cellDims(Config::getCellW(), Config::getCellH());

    drawRectangleSolid(pixelPos, cellDims, bgClr);

    //Only draw contour if neither the foreground or background is black
    if (!Utils::isClrEq(clr, clrBlack) && !Utils::isClrEq(bgClr, clrBlack))
    {
      putPixelsOnScr(contourPixelData_, Art::getGlyphPos(GLYPH), pixelPos, clrBlack);
    }
  }

  putPixelsOnScrForGlyph(GLYPH, pixelPos, clr);
}

} //Namespace

void init()
{
  TRACE_FUNC_BEGIN;
  cleanup();

  TRACE << "Setting up rendering window" << endl;
  const string title = "IA " + gameVersionStr;
  SDL_WM_SetCaption(title.data(), nullptr);

  const int W = Config::getScreenPixelW();
  const int H = Config::getScreenPixelH();
  if (Config::isFullscreen())
  {
    screenSurface = SDL_SetVideoMode(W, H, SCREEN_BPP,
                                     SDL_SWSURFACE | SDL_FULLSCREEN);
  }
  if (!Config::isFullscreen() || !screenSurface)
  {
    screenSurface = SDL_SetVideoMode(W, H, SCREEN_BPP, SDL_SWSURFACE);
  }

  if (!screenSurface)
  {
    TRACE << "Failed to create screen surface" << endl;
    assert(false);
  }

  loadFont();

  if (Config::isTilesMode())
  {
    loadTiles();
    loadMainMenuLogo();
  }

  loadContour(Config::isTilesMode() ? tilePixelData_ : fontPixelData_);

  TRACE_FUNC_END;
}

void cleanup()
{
  TRACE_FUNC_BEGIN;

  if (screenSurface)
  {
    SDL_FreeSurface(screenSurface);
    screenSurface = nullptr;
  }

  if (mainMenuLogoSurface)
  {
    SDL_FreeSurface(mainMenuLogoSurface);
    mainMenuLogoSurface = nullptr;
  }

  TRACE_FUNC_END;
}

void updateScreen()
{
  if (isInited()) {SDL_Flip(screenSurface);}
}

void clearScreen()
{
  if (isInited())
  {
    SDL_FillRect(screenSurface, nullptr, SDL_MapRGB(screenSurface->format, 0, 0, 0));
  }
}

void applySurface(const Pos& pixelPos, SDL_Surface* const src,
                  SDL_Rect* clip)
{
  if (isInited())
  {
    SDL_Rect offset;
    offset.x = pixelPos.x;
    offset.y = pixelPos.y;
    SDL_BlitSurface(src, clip, screenSurface, &offset);
  }
}

void drawMainMenuLogo(const int Y_POS)
{
  const Pos pos((Config::getScreenPixelW() - mainMenuLogoSurface->w) / 2,
                Config::getCellH() * Y_POS);
  applySurface(pos, mainMenuLogoSurface);
}

void drawMarker(const Pos& p, const vector<Pos>& trail, const int EFFECTIVE_RANGE)
{
  if (trail.size() > 2)
  {
    for (size_t i = 1; i < trail.size(); ++i)
    {
      const Pos& pos = trail[i];
      coverCellInMap(pos);

      Clr clr = clrGreenLgt;

      if (EFFECTIVE_RANGE != -1)
      {
        const int CHEB_DIST = Utils::kingDist(trail[0], pos);
        if (CHEB_DIST > EFFECTIVE_RANGE) {clr = clrYellow;}
      }
      if (Config::isTilesMode())
      {
        drawTile(TileId::aimMarkerTrail, Panel::map, pos, clr, clrBlack);
      }
      else
      {
        drawGlyph('*', Panel::map, pos, clr, true, clrBlack);
      }
    }
  }

  Clr clr = clrGreenLgt;

  if (trail.size() > 2)
  {
    if (EFFECTIVE_RANGE != -1)
    {
      const int CHEB_DIST = Utils::kingDist(trail[0], p);
      if (CHEB_DIST > EFFECTIVE_RANGE)
      {
        clr = clrYellow;
      }
    }
  }

  if (Config::isTilesMode())
  {
    drawTile(TileId::aimMarkerHead, Panel::map, p, clr, clrBlack);
  }
  else
  {
    drawGlyph('X', Panel::map, p, clr, true, clrBlack);
  }
}

void drawBlastAtField(const Pos& centerPos, const int RADIUS,
                      bool forbiddenCells[MAP_W][MAP_H], const Clr& clrInner,
                      const Clr& clrOuter)
{
  TRACE_FUNC_BEGIN;
  if (isInited())
  {
    drawMapAndInterface();

    bool isAnyBlastRendered = false;

    Pos pos;

    for (
      pos.y = max(1, centerPos.y - RADIUS);
      pos.y <= min(MAP_H - 2, centerPos.y + RADIUS);
      pos.y++)
    {
      for (
        pos.x = max(1, centerPos.x - RADIUS);
        pos.x <= min(MAP_W - 2, centerPos.x + RADIUS);
        pos.x++)
      {
        if (!forbiddenCells[pos.x][pos.y])
        {
          const bool IS_OUTER = pos.x == centerPos.x - RADIUS ||
                                pos.x == centerPos.x + RADIUS ||
                                pos.y == centerPos.y - RADIUS ||
                                pos.y == centerPos.y + RADIUS;
          const Clr clr = IS_OUTER ? clrOuter : clrInner;
          if (Config::isTilesMode())
          {
            drawTile(TileId::blast1, Panel::map, pos, clr, clrBlack);
          }
          else
          {
            drawGlyph('*', Panel::map, pos, clr, true, clrBlack);
          }
          isAnyBlastRendered = true;
        }
      }
    }
    updateScreen();
    if (isAnyBlastRendered) {SdlWrapper::sleep(Config::getDelayExplosion() / 2);}

    for (
      pos.y = max(1, centerPos.y - RADIUS);
      pos.y <= min(MAP_H - 2, centerPos.y + RADIUS);
      pos.y++)
    {
      for (
        pos.x = max(1, centerPos.x - RADIUS);
        pos.x <= min(MAP_W - 2, centerPos.x + RADIUS);
        pos.x++)
      {
        if (!forbiddenCells[pos.x][pos.y])
        {
          const bool IS_OUTER = pos.x == centerPos.x - RADIUS ||
                                pos.x == centerPos.x + RADIUS ||
                                pos.y == centerPos.y - RADIUS ||
                                pos.y == centerPos.y + RADIUS;
          const Clr clr = IS_OUTER ? clrOuter : clrInner;
          if (Config::isTilesMode())
          {
            drawTile(TileId::blast2, Panel::map, pos, clr, clrBlack);
          }
          else
          {
            drawGlyph('*', Panel::map, pos, clr, true, clrBlack);
          }
        }
      }
    }
    updateScreen();
    if (isAnyBlastRendered) {SdlWrapper::sleep(Config::getDelayExplosion() / 2);}
    drawMapAndInterface();
  }
  TRACE_FUNC_END;
}

void drawBlastAtCells(const vector<Pos>& positions, const Clr& clr)
{
  TRACE_FUNC_BEGIN;
  if (isInited())
  {
    drawMapAndInterface();

    for (const Pos& pos : positions)
    {
      if (Config::isTilesMode())
      {
        drawTile(TileId::blast1, Panel::map, pos, clr, clrBlack);
      }
      else
      {
        drawGlyph('*', Panel::map, pos, clr, true, clrBlack);
      }
    }
    updateScreen();
    SdlWrapper::sleep(Config::getDelayExplosion() / 2);

    for (const Pos& pos : positions)
    {
      if (Config::isTilesMode())
      {
        drawTile(TileId::blast2, Panel::map, pos, clr, clrBlack);
      }
      else
      {
        drawGlyph('*', Panel::map, pos, clr, true, clrBlack);
      }
    }
    updateScreen();
    SdlWrapper::sleep(Config::getDelayExplosion() / 2);
    drawMapAndInterface();
  }
  TRACE_FUNC_END;
}

void drawBlastAtSeenCells(const vector<Pos>& positions, const Clr& clr)
{
  vector<Pos> positionsWithVision;
  for (const Pos& p : positions)
  {
    if (Map::cells[p.x][p.y].isSeenByPlayer)
    {
      positionsWithVision.push_back(p);
    }
  }

  Render::drawBlastAtCells(positionsWithVision, clr);
}

void drawBlastAtSeenActors(const std::vector<Actor*>& actors, const Clr& clr)
{
  vector<Pos> positions;
  for (Actor* const actor : actors)
  {
    positions.push_back(actor->pos);
  }
  drawBlastAtSeenCells(positions, clr);
}

void drawTile(const TileId tile, const Panel panel, const Pos& pos, const Clr& clr,
              const Clr& bgClr)
{
  if (isInited())
  {
    const Pos pixelPos = getPixelPosForCellInPanel(panel, pos);
    const Pos cellDims(Config::getCellW(), Config::getCellH());

    drawRectangleSolid(pixelPos, cellDims, bgClr);

    if (!Utils::isClrEq(bgClr, clrBlack))
    {
      putPixelsOnScr(contourPixelData_, Art::getTilePos(tile), pixelPos, clrBlack);
    }

    putPixelsOnScrForTile(tile, pixelPos, clr);
  }
}

void drawGlyph(const char GLYPH, const Panel panel, const Pos& pos, const Clr& clr,
               const bool DRAW_BG_CLR, const Clr& bgClr)
{
  if (isInited())
  {
    const Pos pixelPos = getPixelPosForCellInPanel(panel, pos);
    drawGlyphAtPixel(GLYPH, pixelPos, clr, DRAW_BG_CLR, bgClr);
  }
}

void drawText(const string& str, const Panel panel, const Pos& pos, const Clr& clr,
              const Clr& bgClr)
{
  if (isInited())
  {
    Pos pixelPos = getPixelPosForCellInPanel(panel, pos);

    if (pixelPos.y < 0 || pixelPos.y >= Config::getScreenPixelH())
    {
      return;
    }

    const Pos cellDims(Config::getCellW(), Config::getCellH());
    const int LEN = str.size();
    drawRectangleSolid(pixelPos, Pos(cellDims.x * LEN, cellDims.y), bgClr);

    for (int i = 0; i < LEN; ++i)
    {
      if (pixelPos.x < 0 || pixelPos.x >= Config::getScreenPixelW())
      {
        return;
      }
      drawGlyphAtPixel(str[i], pixelPos, clr, false);
      pixelPos.x += cellDims.x;
    }
  }
}

int drawTextCentered(const string& str, const Panel panel, const Pos& pos,
                     const Clr& clr, const Clr& bgClr,
                     const bool IS_PIXEL_POS_ADJ_ALLOWED)
{
  const int LEN         = str.size();
  const int LEN_HALF    = LEN / 2;
  const int X_POS_LEFT  = pos.x - LEN_HALF;

  const Pos cellDims(Config::getCellW(), Config::getCellH());

  Pos pixelPos = getPixelPosForCellInPanel(panel, Pos(X_POS_LEFT, pos.y));

  if (IS_PIXEL_POS_ADJ_ALLOWED)
  {
    const int PIXEL_X_ADJ = LEN_HALF * 2 == LEN ? cellDims.x / 2 : 0;
    pixelPos += Pos(PIXEL_X_ADJ, 0);
  }

  const int W_TOT_PIXEL = LEN * cellDims.x;

  SDL_Rect sdlRect =
  {
    (Sint16)pixelPos.x, (Sint16)pixelPos.y,
    (Uint16)W_TOT_PIXEL, (Uint16)cellDims.y
  };
  SDL_FillRect(screenSurface, &sdlRect, SDL_MapRGB(screenSurface->format,
               bgClr.r, bgClr.g, bgClr.b));

  for (int i = 0; i < LEN; ++i)
  {
    if (pixelPos.x < 0 || pixelPos.x >= Config::getScreenPixelW())
    {
      return X_POS_LEFT;
    }
    drawGlyphAtPixel(str[i], pixelPos, clr, false, bgClr);
    pixelPos.x += cellDims.x;
  }
  return X_POS_LEFT;
}

void coverPanel(const Panel panel)
{
  const int SCREEN_PIXEL_W = Config::getScreenPixelW();

  switch (panel)
  {
    case Panel::charLines:
    {
      const Pos pixelPos = getPixelPosForCellInPanel(panel, Pos(0, 0));
      coverAreaPixel(pixelPos, Pos(SCREEN_PIXEL_W, Config::getCharLinesPixelH()));
    } break;

    case Panel::log:
    {
      coverAreaPixel(Pos(0, 0), Pos(SCREEN_PIXEL_W, Config::getLogPixelH()));
    } break;

    case Panel::map:
    {
      const Pos pixelPos = getPixelPosForCellInPanel(panel, Pos(0, 0));
      coverAreaPixel(pixelPos, Pos(SCREEN_PIXEL_W, Config::getMapPixelH()));
    } break;

    case Panel::screen: {clearScreen();} break;
  }
}

void coverArea(const Panel panel, const Rect& area)
{
  coverArea(panel, area.p0, area.p1 - area.p0 + 1);
}

void coverArea(const Panel panel, const Pos& pos, const Pos& dims)
{
  const Pos pixelPos = getPixelPosForCellInPanel(panel, pos);
  const Pos cellDims(Config::getCellW(), Config::getCellH());
  coverAreaPixel(pixelPos, dims * cellDims);
}

void coverAreaPixel(const Pos& pixelPos, const Pos& pixelDims)
{
  drawRectangleSolid(pixelPos, pixelDims, clrBlack);
}

void coverCellInMap(const Pos& pos)
{
  const Pos cellDims(Config::getCellW(), Config::getCellH());
  Pos pixelPos = getPixelPosForCellInPanel(Panel::map, pos);
  coverAreaPixel(pixelPos, cellDims);
}

void drawLineHor(const Pos& pixelPos, const int W, const Clr& clr)
{
  drawRectangleSolid(pixelPos, Pos(W, 2), clr);
}

void drawLineVer(const Pos& pixelPos, const int H, const Clr& clr)
{
  drawRectangleSolid(pixelPos, Pos(1, H), clr);
}

void drawRectangleSolid(const Pos& pixelPos, const Pos& pixelDims, const Clr& clr)
{
  if (isInited())
  {
    SDL_Rect sdlRect = {(Sint16)pixelPos.x, (Sint16)pixelPos.y,
                        (Uint16)pixelDims.x, (Uint16)pixelDims.y
                       };

    SDL_FillRect(screenSurface, &sdlRect,
                 SDL_MapRGB(screenSurface->format, clr.r, clr.g, clr.b));
  }
}

void drawProjectiles(vector<Projectile*>& projectiles,
                     const bool SHOULD_DRAW_MAP_BEFORE)
{

  if (SHOULD_DRAW_MAP_BEFORE) {drawMapAndInterface(false);}

  for (Projectile* p : projectiles)
  {
    if (!p->isDoneRendering && p->isVisibleToPlayer)
    {
      coverCellInMap(p->pos);
      if (Config::isTilesMode())
      {
        if (p->tile != TileId::empty)
        {
          drawTile(p->tile, Panel::map, p->pos, p->clr);
        }
      }
      else
      {
        if (p->glyph != -1) {drawGlyph(p->glyph, Panel::map, p->pos, p->clr);}
      }
    }
  }

  updateScreen();
}

void drawPopupBox(const Rect& border, const Panel panel, const Clr& clr,
                  const bool COVER_AREA)
{
  if (COVER_AREA) {coverArea(panel, border);}

  const bool IS_TILES = Config::isTilesMode();

  //Vertical bars
  const int Y0_VERT = border.p0.y + 1;
  const int Y1_VERT = border.p1.y - 1;
  for (int y = Y0_VERT; y <= Y1_VERT; ++y)
  {
    if (IS_TILES)
    {
      drawTile(TileId::popupVer, panel, Pos(border.p0.x, y), clr, clrBlack);
      drawTile(TileId::popupVer, panel, Pos(border.p1.x, y), clr, clrBlack);
    }
    else
    {
      drawGlyph('|', panel, Pos(border.p0.x, y), clr, true, clrBlack);
      drawGlyph('|', panel, Pos(border.p1.x, y), clr, true, clrBlack);
    }
  }

  //Horizontal bars
  const int X0_VERT = border.p0.x + 1;
  const int X1_VERT = border.p1.x - 1;
  for (int x = X0_VERT; x <= X1_VERT; ++x)
  {
    if (IS_TILES)
    {
      drawTile(TileId::popupHor, panel, Pos(x, border.p0.y), clr, clrBlack);
      drawTile(TileId::popupHor, panel, Pos(x, border.p1.y), clr, clrBlack);
    }
    else
    {
      drawGlyph('-', panel, Pos(x, border.p0.y), clr, true, clrBlack);
      drawGlyph('-', panel, Pos(x, border.p1.y), clr, true, clrBlack);
    }
  }

  const vector<Pos> corners
  {
    {border.p0.x, border.p0.y}, //Top left
    {border.p1.x, border.p0.y}, //Top right
    {border.p0.x, border.p1.y}, //Btm left
    {border.p1.x, border.p1.y}  //Brm right
  };

  //Corners
  if (IS_TILES)
  {
    drawTile(TileId::popupTopL, panel, corners[0], clr, clrBlack);
    drawTile(TileId::popupTopR, panel, corners[1], clr, clrBlack);
    drawTile(TileId::popupBtmL, panel, corners[2], clr, clrBlack);
    drawTile(TileId::popupBtmR, panel, corners[3], clr, clrBlack);
  }
  else
  {
    drawGlyph('+', panel, corners[0], clr, true, clrBlack);
    drawGlyph('+', panel, corners[1], clr, true, clrBlack);
    drawGlyph('+', panel, corners[2], clr, true, clrBlack);
    drawGlyph('+', panel, corners[3], clr, true, clrBlack);
  }
}

void drawDescrBox(const std::vector<StrAndClr>& lines)
{
  const int DESCR_Y0  = 2;
  const int DESCR_X1  = MAP_W - 1;
  coverArea(Panel::screen, Rect(DESCR_X0, DESCR_Y0, DESCR_X1, SCREEN_H - 1));

  const int MAX_W = DESCR_X1 - DESCR_X0 + 1;

  Pos p(DESCR_X0, DESCR_Y0);

  for (const auto& line : lines)
  {
    vector<string> formatted;
    TextFormatting::lineToLines(line.str, MAX_W, formatted);
    for (const auto& lineInFormatted : formatted)
    {
      drawText(lineInFormatted, Panel::screen, p, line.clr);
      ++p.y;
    }
    ++p.y;
  }
}

void drawMapAndInterface(const bool SHOULD_UPDATE_SCREEN)
{
  if (isInited())
  {
    clearScreen();

    drawMap();

    CharacterLines::drawInfoLines();
    CharacterLines::drawLocationInfo();
    Log::drawLog(false);

    if (SHOULD_UPDATE_SCREEN) {updateScreen();}
  }
}

void drawMap()
{
  if (!isInited()) {return;}

  CellRenderData* curDrw = nullptr;
  CellRenderData  tmpDrw;

  const bool IS_TILES = Config::isTilesMode();

  //---------------- INSERT RIGIDS AND BLOOD INTO ARRAY
  for (int x = 0; x < MAP_W; ++x)
  {
    for (int y = 0; y < MAP_H; ++y)
    {
      if (Map::cells[x][y].isSeenByPlayer)
      {

        renderArray[x][y]           = CellRenderData();
        curDrw                      = &renderArray[x][y];
        const auto* const f         = Map::cells[x][y].rigid;
        TileId            goreTile  = TileId::empty;
        char              goreGlyph = 0;

        if (f->canHaveGore())
        {
          goreTile  = f->getGoreTile();
          goreGlyph = f->getGoreGlyph();
        }
        if (goreTile == TileId::empty)
        {
          curDrw->tile  = f->getTile();
          curDrw->glyph = f->getGlyph();
          curDrw->clr   = f->getClr();
          const Clr& featureClrBg = f->getClrBg();
          if (!Utils::isClrEq(featureClrBg, clrBlack)) {curDrw->clrBg = featureClrBg;}
        }
        else
        {
          curDrw->tile  = goreTile;
          curDrw->glyph = goreGlyph;
          curDrw->clr   = clrRed;
        }
//        if(Map::cells[x][y].isLit && f->canMoveCmn())
//        {
//          curDrw->isMarkedLit = true;
//        }
      }
    }
  }

  int xPos, yPos;
  //---------------- INSERT DEAD ACTORS INTO ARRAY
  for (Actor* actor : GameTime::actors_)
  {
    xPos = actor->pos.x;
    yPos = actor->pos.y;
    if (actor->isCorpse()                       &&
        actor->getData().glyph != ' '           &&
        actor->getData().tile != TileId::empty  &&
        Map::cells[xPos][yPos].isSeenByPlayer)
    {
      curDrw        = &renderArray[xPos][yPos];
      curDrw->clr   = actor->getClr();
      curDrw->tile  = actor->getTile();
      curDrw->glyph = actor->getGlyph();
    }
  }

  for (int x = 0; x < MAP_W; ++x)
  {
    for (int y = 0; y < MAP_H; ++y)
    {
      curDrw = &renderArray[x][y];
      if (Map::cells[x][y].isSeenByPlayer)
      {
        //---------------- INSERT ITEMS INTO ARRAY
        const Item* const item = Map::cells[x][y].item;
        if (item)
        {
          curDrw->clr   = item->getClr();
          curDrw->tile  = item->getTile();
          curDrw->glyph = item->getGlyph();
        }

        //COPY ARRAY TO PLAYER MEMORY (BEFORE LIVING ACTORS AND MOBILE FEATURES)
        renderArrayNoActors[x][y] = renderArray[x][y];

        //COLOR CELLS MARKED AS LIT YELLOW
//        if(curDrw->isMarkedLit)
//        {
//          curDrw->clr = clrYellow;
//        }
      }
    }
  }

  //---------------- INSERT MOBILE FEATURES INTO ARRAY
  for (auto* mob : GameTime::mobs_)
  {
    const Pos& p            = mob->getPos();
    const TileId  mobTile   = mob->getTile();
    const char    mobGlyph  = mob->getGlyph();
    if (mobTile != TileId::empty && mobGlyph != ' ' &&
        Map::cells[p.x][p.y].isSeenByPlayer)
    {
      curDrw = &renderArray[p.x][p.y];
      curDrw->clr   = mob->getClr();
      curDrw->tile  = mobTile;
      curDrw->glyph = mobGlyph;
    }
  }

  //---------------- INSERT LIVING ACTORS INTO ARRAY
  for (auto* actor : GameTime::actors_)
  {
    if (actor != Map::player)
    {
      if (actor->isAlive())
      {
        const Pos& p  = actor->pos;
        curDrw        = &renderArray[p.x][p.y];

        const auto* const mon = static_cast<const Mon*>(actor);

        if (Map::player->isSeeingActor(*actor, nullptr))
        {
          if (actor->getTile()  != TileId::empty && actor->getGlyph() != ' ')
          {
            curDrw->clr   = actor->getClr();
            curDrw->tile  = actor->getTile();
            curDrw->glyph = actor->getGlyph();

            curDrw->lifebarLength         = getLifebarLength(*actor);
            curDrw->isLivingActorSeenHere = true;
            curDrw->isLightFadeAllowed    = false;

            if (Map::player->isLeaderOf(mon))
            {
              curDrw->clrBg = clrGreen;
            }
            else
            {
              if (mon->awareCounter_ <= 0) {curDrw->clrBg = clrBlue;}
            }
          }
        }
        else //Player is not seeing actor
        {
          if (mon->playerAwareOfMeCounter_ > 0 || Map::player->isLeaderOf(mon))
          {
            curDrw->isAwareOfMonHere  = true;
          }
        }
      }
    }
  }

  //---------------- DRAW THE GRID
  auto divClr = [](Clr & clr, const double DIV)
  {
    clr.r = double(clr.r) / DIV;
    clr.g = double(clr.g) / DIV;
    clr.b = double(clr.b) / DIV;
  };

  for (int x = 0; x < MAP_W; ++x)
  {
    for (int y = 0; y < MAP_H; ++y)
    {

      tmpDrw = renderArray[x][y];

      const Cell& cell = Map::cells[x][y];

      if (cell.isSeenByPlayer)
      {
        if (tmpDrw.isLightFadeAllowed)
        {
          const int DIST_FROM_PLAYER = Utils::kingDist(Map::player->pos, Pos(x, y));
          if (DIST_FROM_PLAYER > 1)
          {
            const double DIV = min(2.0, 1.0 + (double(DIST_FROM_PLAYER - 1) * 0.33));
            divClr(tmpDrw.clr,    DIV);
            divClr(tmpDrw.clrBg,  DIV);
          }

          if (cell.isDark && !cell.isLit)
          {
            const double DRK_DIV = 1.75;
            divClr(tmpDrw.clr,    DRK_DIV);
            divClr(tmpDrw.clrBg,  DRK_DIV);
          }
        }
      }
      else if (cell.isExplored)
      {
        bool isAwareOfMonHere   = tmpDrw.isAwareOfMonHere;
        renderArray[x][y]       = cell.playerVisualMemory;
        tmpDrw                  = renderArray[x][y];
        tmpDrw.isAwareOfMonHere = isAwareOfMonHere;

        const double DIV = 5.0;
        divClr(tmpDrw.clr,    DIV);
        divClr(tmpDrw.clrBg,  DIV);
      }

      if (IS_TILES)
      {
        //Walls are given perspective here. If the tile to be set is a (top) wall tile,
        //instead place a front wall tile on any of the current conditions:
        //(1) Cell below is explored, and its tile is not a front or top wall tile.
        //(2) Cell below is unexplored.
        if (!tmpDrw.isLivingActorSeenHere && !tmpDrw.isAwareOfMonHere)
        {
          const auto tileSeen     = renderArrayNoActors[x][y].tile;
          const auto tileMem      = cell.playerVisualMemory.tile;
          const bool IS_TILE_WALL = cell.isSeenByPlayer ?
                                    Wall::isTileAnyWallTop(tileSeen) :
                                    Wall::isTileAnyWallTop(tileMem);
          if (IS_TILE_WALL)
          {
            const auto* const f   = cell.rigid;
            const auto featureId  = f->getId();
            bool isHiddenDoor     = false;
            if (featureId == FeatureId::door)
            {
              isHiddenDoor = static_cast<const Door*>(f)->isSecret();
            }
            if (y < MAP_H - 1 && (featureId == FeatureId::wall || isHiddenDoor))
            {
              if (Map::cells[x][y + 1].isExplored)
              {
                const bool IS_SEEN_BELOW  = Map::cells[x][y + 1].isSeenByPlayer;
                const auto tileBelowSeen  = renderArrayNoActors[x][y + 1].tile;
                const auto tileBelowMem   = Map::cells[x][y + 1].playerVisualMemory.tile;

                const bool TILE_BELOW_IS_WALL_FRONT =
                  IS_SEEN_BELOW ? Wall::isTileAnyWallFront(tileBelowSeen) :
                  Wall::isTileAnyWallFront(tileBelowMem);

                const bool TILE_BELOW_IS_WALL_TOP =
                  IS_SEEN_BELOW ? Wall::isTileAnyWallTop(tileBelowSeen) :
                  Wall::isTileAnyWallTop(tileBelowMem);

                bool tileBelowIsRevealedDoor =
                  IS_SEEN_BELOW ? Door::isTileAnyDoor(tileBelowSeen) :
                  Door::isTileAnyDoor(tileBelowMem);

                if (TILE_BELOW_IS_WALL_FRONT  ||
                    TILE_BELOW_IS_WALL_TOP    ||
                    tileBelowIsRevealedDoor)
                {
                  if (featureId == FeatureId::wall)
                  {
                    const auto* const wall = static_cast<const Wall*>(f);
                    tmpDrw.tile = wall->getTopWallTile();
                  }
                }
                else if (featureId == FeatureId::wall)
                {
                  const auto* const wall  = static_cast<const Wall*>(f);
                  tmpDrw.tile             = wall->getFrontWallTile();
                }
                else if (isHiddenDoor)
                {
                  tmpDrw.tile = Config::isTilesWallFullSquare() ?
                                TileId::wallTop :
                                TileId::wallFront;
                }
              }
              else     //Cell below is not explored
              {
                const Rigid* wall = nullptr;
                if (isHiddenDoor)
                {
                  wall = static_cast<const Door*>(f)->getMimic();
                }
                else
                {
                  wall = f;
                }
                tmpDrw.tile = static_cast<const Wall*>(wall)->getFrontWallTile();
              }
            }
          }
        }
      }

      Pos pos(x, y);

      if (tmpDrw.isAwareOfMonHere)
      {
        drawGlyph('!', Panel::map, pos, clrBlack, true, clrNosfTealDrk);
      }
      else if (tmpDrw.tile != TileId::empty && tmpDrw.glyph != ' ')
      {
        if (IS_TILES)
        {
          drawTile(tmpDrw.tile, Panel::map, pos, tmpDrw.clr, tmpDrw.clrBg);
        }
        else
        {
          drawGlyph(tmpDrw.glyph, Panel::map, pos, tmpDrw.clr, true, tmpDrw.clrBg);
        }

        if (tmpDrw.lifebarLength != -1)
        {
          drawLifeBar(pos, tmpDrw.lifebarLength);
        }
      }

      if (!cell.isExplored) {renderArray[x][y] = CellRenderData();}
    }
  }

  //---------------- DRAW PLAYER CHARACTER
  bool        isRangedWpn = false;
  const Pos&  pos         = Map::player->pos;
  Item*       item        = Map::player->getInv().getItemInSlot(SlotId::wielded);
  if (item)
  {
    isRangedWpn = item->getData().ranged.isRangedWpn;
  }
  if (IS_TILES)
  {
    const TileId tile = isRangedWpn ? TileId::playerFirearm : TileId::playerMelee;
    drawTile(tile, Panel::map, pos, Map::player->getClr(), clrBlack);
  }
  else
  {
    drawGlyph('@', Panel::map, pos, Map::player->getClr(), true, clrBlack);
  }
  const int LIFE_BAR_LENGTH = getLifebarLength(*Map::player);
  if (LIFE_BAR_LENGTH != -1)
  {
    drawLifeBar(pos, LIFE_BAR_LENGTH);
  }
  drawPlayerShockExclMarks();
}

} //Render
