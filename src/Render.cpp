#include "Render.h"

#include <vector>
#include <iostream>

#include "Engine.h"
#include "Item.h"
#include "Interface.h"
#include "Marker.h"
#include "Timer.h"
#include "Map.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"

using namespace std;


Renderer::Renderer(Engine* engine) : eng(engine),
  m_glyphSheet(NULL), m_tileSheet(NULL), m_mainMenuLogo(NULL)  {

  SDL_init();
  setGfxAnVideoMode();
}

void Renderer::setGfxAnVideoMode() {
  if(m_glyphSheet != NULL) {
    SDL_FreeSurface(m_glyphSheet);
    m_glyphSheet = NULL;
  }

  if(m_tileSheet != NULL) {
    SDL_FreeSurface(m_tileSheet);
    m_tileSheet = NULL;
  }

  if(m_mainMenuLogo != NULL) {
    SDL_FreeSurface(m_mainMenuLogo);
    m_mainMenuLogo = NULL;
  }

  loadFontSheet();

  if(eng->config->USE_TILE_SET) {
    loadTiles();
    loadMainMenuLogo();
  }

  if(eng->config->FULLSCREEN) {
    m_screen = SDL_SetVideoMode(eng->config->SCREEN_WIDTH, eng->config->SCREEN_HEIGHT, 0, SDL_FULLSCREEN | SDL_SWSURFACE);
  } else {
    m_screen = SDL_SetVideoMode(eng->config->SCREEN_WIDTH, eng->config->SCREEN_HEIGHT, 0, SDL_SWSURFACE);
  }

  loadFontSheet();

  if(eng->config->USE_TILE_SET) {
    loadTiles();
    loadMainMenuLogo();
  }

  if(eng->config->FULLSCREEN) {
    SDL_ShowCursor(SDL_DISABLE);
  }
}

void Renderer::loadFontSheet() {
  tracer << "Renderer::loadFontSheet()..." << endl;
  const char* IMAGE_NAME = (eng->config->FONT_IMAGE_NAME).data();

  tracer << "Renderer: SDL_LoadBMP()..." << endl;
  SDL_Surface* loadedImage = SDL_LoadBMP(IMAGE_NAME);
  tracer << "Renderer: SDL_LoadBMP() [DONE]" << endl;
  if(loadedImage == NULL) {
    tracer << "[WARNING] NULL image returned from SDL_LoadBMP(), in Renderer::loadFontSheet()" << endl;
    return;
  }

  dsrect.x = 0;
  dsrect.y = static_cast<Sint16>(eng->config->MAINSCREEN_Y_OFFSET);
  dsrect.w = static_cast<Sint16>(eng->config->MAINSCREEN_WIDTH);
  dsrect.h = static_cast<Sint16>(eng->config->MAINSCREEN_HEIGHT);
  cell_rect.w = static_cast<Sint16>(eng->config->CELL_W);
  cell_rect.h = static_cast<Sint16>(eng->config->CELL_H);
  clipRect.w = cell_rect.w;
  clipRect.h = cell_rect.h;

  SDL_Surface* scaledImage = loadedImage;

  tracer << "Renderer: SDL_SetColorKey()..." << endl;
  SDL_SetColorKey(scaledImage, SDL_SRCCOLORKEY, SDL_MapRGB(scaledImage->format, 0xFF, 0xFF, 0xFF));
  tracer << "Renderer: SDL_SetColorKey() [DONE]" << endl;
  if(scaledImage == NULL) {
    tracer << "[WARNING] NULL image returned from SDL_SetColorKey(), in Renderer::loadFontSheet()" << endl;
    return;
  }

  tracer << "Renderer: SDL_DisplayFormat()..." << endl;
  m_glyphSheet = SDL_DisplayFormat(scaledImage);
  tracer << "Renderer: SDL_DisplayFormat [DONE]" << endl;

  SDL_FreeSurface(scaledImage);

  if(m_glyphSheet == NULL) {
    tracer << "[WARNING] NULL image returned from SDL_DisplayFormat(), in Renderer::loadFontSheet()" << endl;
    return;
  }

  tracer << "Image W,H: " << m_glyphSheet->w << "," << m_glyphSheet->h << endl;

  tracer << "Renderer::loadFontSheet() [DONE]" << endl;
}

void Renderer::loadTiles() {
  const char* IMAGE_NAME = (eng->config->TILES_IMAGE_NAME).data();

  tracer << "Loding image: " << IMAGE_NAME << endl;
  tracer << "SDL_LoadBMP()..." << endl;
  SDL_Surface* loadedImage = SDL_LoadBMP(IMAGE_NAME);
  tracer << "SDL_LoadBMP() [DONE]" << endl;

  tracer << "SDL_SetColorKey..." << endl;
  SDL_SetColorKey(loadedImage, SDL_SRCCOLORKEY, SDL_MapRGB(loadedImage->format, 0xB3, 0xB3, 0xB3));
  tracer << "SDL_SetColorKey [DONE]" << endl;

  tracer << "SDL_DisplayFormat()..." << endl;
  m_tileSheet = SDL_DisplayFormat(loadedImage);
  tracer << "SDL_DisplayFormat [DONE]" << endl;

  SDL_FreeSurface(loadedImage);
}

void Renderer::loadMainMenuLogo() {
  tracer << "Renderer::loadMainMenuLogo()..." << endl;

  const char* IMAGE_NAME = (eng->config->MAIN_MENU_LOGO_IMAGE_NAME).data();

  tracer << "Loding image: " << IMAGE_NAME << endl;

  tracer << "SDL_LoadBMP()..." << endl;
  SDL_Surface* loadedImage = SDL_LoadBMP(IMAGE_NAME);
  tracer << "SDL_LoadBMP() [DONE]" << endl;

  tracer << "SDL_SetColorKey..." << endl;
  SDL_SetColorKey(loadedImage, SDL_SRCCOLORKEY, SDL_MapRGB(loadedImage->format, 0x00, 0x00, 0x00));
  tracer << "SDL_SetColorKey [DONE]" << endl;

  tracer << "SDL_DisplayFormat()..." << endl;
  m_mainMenuLogo = SDL_DisplayFormat(loadedImage);
  tracer << "SDL_DisplayFormat [DONE]" << endl;

  SDL_FreeSurface(loadedImage);

  tracer << "Renderer::loadMainMenuLogo() [DONE]" << endl;
}

void Renderer::applySurface(int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip) {
  SDL_Rect offset;
  offset.x = static_cast<Sint16>(x);
  offset.y = static_cast<Sint16>(y);
  SDL_BlitSurface(source, clip, destination, &offset);
}

bool Renderer::SDL_init() {
  if(SDL_Init(SDL_INIT_EVERYTHING) == -1) {
    return false;
  }

  setKeyDelays();

  SDL_EnableUNICODE(1);

  SDL_WM_SetCaption(("IA " + eng->config->GAME_VERSION).c_str(), NULL);

  return true;
}

void Renderer::setKeyDelays() {
  SDL_EnableKeyRepeat(eng->config->KEY_REPEAT_DELAY, eng->config->KEY_REPEAT_INTERVAL);
}

//SDL_Surface* Renderer::scaleSurface(SDL_Surface* Surface, Uint16 Width, Uint16 Height)
//{
//  if(!Surface || !Width || !Height)
//    return 0;
//
//  SDL_Surface* _ret = SDL_CreateRGBSurface(Surface->flags, Width, Height, Surface->format->BitsPerPixel,
//                      Surface->format->Rmask, Surface->format->Gmask, Surface->format->Bmask, Surface->format->Amask);
//
//  double    _stretch_factor_x = (static_cast<double>(Width)  / static_cast<double>(Surface->w)),
//                                _stretch_factor_y = (static_cast<double>(Height) / static_cast<double>(Surface->h));
//
//  for(Sint32 y = 0; y < Surface->h; y++)
//    for(Sint32 x = 0; x < Surface->w; x++)
//      for(Sint32 o_y = 0; o_y < _stretch_factor_y; ++o_y)
//        for(Sint32 o_x = 0; o_x < _stretch_factor_x; ++o_x)
//          DrawPixel(_ret, static_cast<Sint32>(_stretch_factor_x * x) + o_x,
//                    static_cast<Sint32>(_stretch_factor_y * y) + o_y, ReadPixel(Surface, x, y));
//
//  return _ret;
//}
//
//
//Uint32 Renderer::ReadPixel(SDL_Surface* surface, int x, int y) {
//  int bpp = surface->format->BytesPerPixel;
//  Uint8* p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
//
//  switch(bpp) {
//  case 1:
//    return *p;
//    break;
//
//  case 2:
//    return *(Uint16 *)p;
//    break;
//
//  case 3:
//    if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
//      return p[0] << 16 | p[1] << 8 | p[2];
//    else
//      return p[0] | p[1] << 8 | p[2] << 16;
//    break;
//
//  case 4:
//    return *(Uint32 *)p;
//    break;
//
//  default:
//    return 0;
//  }
//}
//
//void Renderer::DrawPixel(SDL_Surface* surface, int x, int y, Uint32 pixel) {
//  int bpp = surface->format->BytesPerPixel;
//  Uint8* p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
//
//  switch(bpp) {
//  case 1:
//    *p = pixel;
//    break;
//
//  case 2:
//    *(Uint16 *)p = pixel;
//    break;
//
//  case 3:
//    if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
//      p[0] = (pixel >> 16) & 0xff;
//      p[1] = (pixel >> 8) & 0xff;
//      p[2] = pixel & 0xff;
//    } else {
//      p[0] = pixel & 0xff;
//      p[1] = (pixel >> 8) & 0xff;
//      p[2] = (pixel >> 16) & 0xff;
//    }
//    break;
//
//  case 4:
//    *(Uint32 *)p = pixel;
//    break;
//  }
//}

void Renderer::drawMainMenuLogo(const int Y_POS) {
  const int IMG_W = m_mainMenuLogo->w;
  const int X = (eng->config->SCREEN_WIDTH - IMG_W) / 2;
  const int Y = eng->config->CELL_H * Y_POS;
  applySurface(X, Y, m_mainMenuLogo, m_screen);
}

void Renderer::drawMarker(vector<coord> &trace, const int EFFECTIVE_RANGE) {
  drawMapAndInterface(false);

  if(trace.size() > 2) {
    for(unsigned int i = 1; i < trace.size() - 1; i++) {
      SDL_Color clr = clrGreenLight;

      if(EFFECTIVE_RANGE != -1) {
        const int CHEB_DIST = eng->basicUtils->chebyshevDistance(trace.at(0), trace.at(i));
        if(CHEB_DIST > EFFECTIVE_RANGE) {
          clr = clrYellow;
        }
      }
      if(eng->config->USE_TILE_SET) {
        drawTileInMap(tile_aimMarkerTrail, trace.at(i).x, trace.at(i).y, clr);
      } else {
        drawCharacter('*', renderArea_mainScreen, trace.at(i).x, trace.at(i).y, clr);
      }
    }
  }

  const coord xy = coord(eng->marker->getX(), eng->marker->getY());

  SDL_Color clr = clrGreenLight;

  if(trace.size() > 2) {
    if(EFFECTIVE_RANGE != -1) {
      const int CHEB_DIST = eng->basicUtils->chebyshevDistance(trace.at(0), xy);
      if(CHEB_DIST > EFFECTIVE_RANGE) {
        clr = clrYellow;
      }
    }
  }
  if(eng->config->USE_TILE_SET) {
    drawTileInMap(tile_aimMarkerHead, xy.x, xy.y, clr);
  } else {
    drawCharacter('X', renderArea_mainScreen, xy.x, xy.y, clr);
  }

  flip();
}

void Renderer::drawBlastAnimationAtField(const coord center, const int RADIUS, bool forbiddenCells[MAP_X_CELLS][MAP_Y_CELLS],
    const SDL_Color colorInner, const SDL_Color colorOuter, const int DURATION) {
  for(int y = max(1, center.y - RADIUS); y <= min(MAP_Y_CELLS - 2, center.y + RADIUS); y++) {
    for(int x = max(1, center.x - RADIUS); x <= min(MAP_X_CELLS - 2, center.x + RADIUS); x++) {
      if(forbiddenCells[x][y] == false) {
        const bool IS_OUTER = x == center.x - RADIUS || x == center.x + RADIUS || y == center.y - RADIUS || y == center.y + RADIUS;
        const SDL_Color color = IS_OUTER ? colorOuter : colorInner;
        drawTileInMap(tile_blastAnimation1, x, y, color);
      }
    }
  }
  flip();
  Timer t;
  t.start();
  while(t.get_ticks() < DURATION / 2) {
  }
  drawMapAndInterface();
  for(int y = max(1, center.y - RADIUS); y <= min(MAP_Y_CELLS - 2, center.y + RADIUS); y++) {
    for(int x = max(1, center.x - RADIUS); x <= min(MAP_X_CELLS - 2, center.x + RADIUS); x++) {
      if(forbiddenCells[x][y] == false) {
        const bool IS_OUTER = x == center.x - RADIUS || x == center.x + RADIUS || y == center.y - RADIUS || y == center.y + RADIUS;
        const SDL_Color color = IS_OUTER ? colorOuter : colorInner;
        drawTileInMap(tile_blastAnimation2, x, y, color);
      }
    }
  }
  flip();
  t.start();
  while(t.get_ticks() < DURATION / 2) {
  }
}

void Renderer::drawBlastAnimationAt(const coord pos, const SDL_Color color, const int DURATION) {
  drawTileInMap(tile_blastAnimation1, pos.x, pos.y, color);
  flip();
  Timer t;
  t.start();
  while(t.get_ticks() < DURATION / 2) {
  }
  drawTileInMap(tile_blastAnimation2, pos.x, pos.y, color);
  flip();
  t.stop();
  t.start();
  while(t.get_ticks() < DURATION / 2) {
  }
}


void Renderer::drawTileInMap(const Tile_t tile, const int X, const int Y, const SDL_Color clr) {
  coord tileCoords = eng->art->getTileCoords(tile, eng);

  const int W_PIXEL = eng->config->CELL_W;
  const int H_PIXEL = eng->config->CELL_H;

  const int X_PIXEL = X * W_PIXEL;
  const int Y_PIXEL = Y * H_PIXEL + eng->config->MAINSCREEN_Y_OFFSET;

//  clearTileAtPixel(X_PIXEL, Y_PIXEL);

  clipRect.x = static_cast<Sint16>(tileCoords.x);
  clipRect.y = static_cast<Sint16>(tileCoords.y);

  cell_rect.x = static_cast<Sint16>(X_PIXEL);
  cell_rect.y = static_cast<Sint16>(Y_PIXEL);

  SDL_FillRect(m_screen, &cell_rect, SDL_MapRGB(m_screen->format, clr.r, clr.g, clr.b));

  applySurface(X_PIXEL, Y_PIXEL, m_tileSheet, m_screen, &clipRect);
}

void Renderer::drawCharacterAtPixel(const char CHARACTER, const int X, const int Y, const SDL_Color clr) {
  coord glyphCoords = eng->art->getGlyphCoords(CHARACTER, eng);

//  clearCharacterAtPixel(X, Y);

  clipRect.x = static_cast<Sint16>(glyphCoords.x);
  clipRect.y = static_cast<Sint16>(glyphCoords.y);

  cell_rect.x = static_cast<Sint16>(X);
  cell_rect.y = static_cast<Sint16>(Y);

  SDL_FillRect(m_screen, &cell_rect, SDL_MapRGB(m_screen->format, clr.r, clr.g, clr.b));

  applySurface(X, Y, m_glyphSheet, m_screen, &clipRect);
}

coord Renderer::getPixelCoordsForCharacter(const RenderArea_t renderArea, const int X, const int Y) {
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

void Renderer::drawCharacter(const char CHARACTER, const RenderArea_t renderArea, const int X, const int Y, const SDL_Color clr) {
  const coord pixelCoord = getPixelCoordsForCharacter(renderArea, X, Y);
  clearCharacterAtPixel(pixelCoord.x, pixelCoord.y);
  drawCharacterAtPixel(CHARACTER, pixelCoord.x, pixelCoord.y, clr);
}

void Renderer::drawText(const string str, const RenderArea_t renderArea, const int X, const int Y, const SDL_Color clr) {
  coord pixelCoord = getPixelCoordsForCharacter(renderArea, X, Y);

  if(pixelCoord.y < 0 || pixelCoord.y >= eng->config->SCREEN_HEIGHT) {
    return;
  }

  for(unsigned int i = 0; i < str.size(); i++) {
    if(pixelCoord.x < 0 || pixelCoord.x >= eng->config->SCREEN_WIDTH) {
      return;
    }
    drawCharacterAtPixel(str.at(i), pixelCoord.x, pixelCoord.y, clr);
    pixelCoord.x += eng->config->CELL_W;
  }
}

//TODO drawTextCentered must be able to draw centered on the actual screen
void Renderer::drawTextCentered(const string str, const RenderArea_t renderArea, const int X, const int Y, const SDL_Color clr) {
  const int X_POS_LEFT = X - (str.size() / 2);
  drawText(str, renderArea, X_POS_LEFT, Y, clr);
}

void Renderer::clearRenderArea(const RenderArea_t renderArea) {
  const coord x0y0Pixel = getPixelCoordsForCharacter(renderArea, 0, 0);
  switch(renderArea) {
  case renderArea_characterLines: {
    clearAreaPixel(x0y0Pixel.x, x0y0Pixel.y, eng->config->SCREEN_WIDTH, eng->config->CHARACTER_LINES_HEIGHT);
  }
  break;
  case renderArea_log: {
    clearAreaPixel(0, 0, eng->config->SCREEN_WIDTH, eng->config->LOG_HEIGHT + eng->config->LOG_Y_OFFSET);
  }
  break;
  case renderArea_mainScreen: {
    clearAreaPixel(x0y0Pixel.x, x0y0Pixel.y, eng->config->SCREEN_WIDTH, eng->config->MAINSCREEN_HEIGHT);
  }
  break;
  case renderArea_screen: {
    clearAreaPixel(0, 0, eng->config->SCREEN_WIDTH, eng->config->SCREEN_HEIGHT);
  }
  break;

  }
}

void Renderer::clearAreaWithTextDimensions(const RenderArea_t renderArea, const int X, const int Y, const int W, const int H) {
  const coord x0y0 = getPixelCoordsForCharacter(renderArea, X, Y);
  clearAreaPixel(x0y0.x, x0y0.y, W * eng->config->CELL_W, H * eng->config->CELL_H);
}

void Renderer::clearAreaPixel(const int X, const int Y, const int W, const int H) {
  drawRectangle(X, Y, W, H, clrBlack);
}

void Renderer::clearCellInMap(const int X, const int Y) {
  const int CELL_W = eng->config->CELL_W;
  const int CELL_H = eng->config->CELL_H;
  clearAreaPixel(X * CELL_W, eng->config->MAINSCREEN_Y_OFFSET + Y * CELL_H, CELL_W, CELL_H);
}

void Renderer::drawLineVertical(const int x0, const int y0, const int h, SDL_Color clr) {
  drawRectangle(x0, y0, 1, h, clr);
}

void Renderer::drawLineHorizontal(const int x0, const int y0, const int w, SDL_Color clr) {
  drawRectangle(x0, y0, w, 1, clr);
}

void Renderer::drawRectangle(const int x, const int y, const int w, const int h, SDL_Color clr) {
  SDL_Rect curRect;

  curRect.x = static_cast<Sint16>(x);
  curRect.y = static_cast<Sint16>(y);
  curRect.w = static_cast<Sint16>(w);
  curRect.h = static_cast<Sint16>(h);

  SDL_FillRect(m_screen, &curRect, SDL_MapRGB(m_screen->format, clr.r, clr.g, clr.b));
}

void Renderer::clearCharacterAtPixel(const int X, const int Y) {
  clearAreaPixel(X, Y, eng->config->CELL_W, eng->config->CELL_H);
}

void Renderer::clearTileAtPixel(const int X, const int Y) {
  clearAreaPixel(X, Y, eng->config->CELL_W, eng->config->CELL_H);
}

void Renderer::drawMapAndInterface(const bool FLIP_SCREEN) {
  if(eng->config->USE_TILE_SET) {
    drawTiles();
  } else {
    drawASCII();
  }

  eng->interfaceRenderer->drawInfoLines();

  if(FLIP_SCREEN) {
    flip();
  }
}

void Renderer::drawASCII() {
  clearRenderArea(renderArea_mainScreen);

  GlyphAndColor* currentDrw = NULL;
  GlyphAndColor tempDrw;

  //-------------------------------------------- INSERT FEATURES AND BLOOD INTO TILE ARRAY
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {

      currentDrw = &renderArray[x][y];
      currentDrw->color = clrBlack;
      currentDrw->glyph = ' ';
      currentDrw->underscoreClr = clrBlack;

      if(eng->map->playerVision[x][y] == true) {
        //Static features
        char goreGlyph = ' ';
        if(eng->map->featuresStatic[x][y]->canHaveGore()) {
          goreGlyph = eng->map->featuresStatic[x][y]->getGoreGlyph();
        }
        if(goreGlyph == ' ') {
          currentDrw->glyph = eng->map->featuresStatic[x][y]->getGlyph();
          const SDL_Color featureStdClr = eng->map->featuresStatic[x][y]->getColor();
          currentDrw->color = eng->map->featuresStatic[x][y]->hasBlood() ? clrRedLight : featureStdClr;
        } else {
          currentDrw->glyph = goreGlyph;
          currentDrw->color = clrRed;
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
    if(actor->deadState == actorDeadState_corpse && actor->getInstanceDefinition()->glyph != ' ' && eng->map->playerVision[xPos][yPos]) {
      currentDrw = &renderArray[xPos][yPos];
      currentDrw->color = actor->getColor();
      currentDrw->glyph = actor->getGlyph();
    }
  }

  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      currentDrw = &renderArray[x][y];
      if(eng->map->playerVision[x][y] == true) {
        //-------------------------------------------- INSERT ITEMS INTO TILE ARRAY
        if(eng->map->items[x][y] != NULL) {
          currentDrw->color = eng->map->items[x][y]->getColor();
          currentDrw->glyph = eng->map->items[x][y]->getGlyph();
        }

        //COPY ARRAY TO PLAYER MEMORY (BEFORE LIVING ACTORS AND TIME ENTITIES)
        renderArrayActorsOmitted[x][y] = renderArray[x][y];
      }
    }
  }

  //-------------------------------------------- INSERT MOBILE FEATURES INTO TILE ARRAY
  const unsigned int SIZE_OF_FEAT_MOB = eng->gameTime->getFeatureMobsSize();
  for(unsigned int i = 0; i < SIZE_OF_FEAT_MOB; i++) {
    FeatureMob* feature = eng->gameTime->getFeatureMobAt(i);
    xPos = feature->getX();
    yPos = feature->getY();
    if(feature->getGlyph() != ' ' && eng->map->playerVision[xPos][yPos] == true) {
      currentDrw = &renderArray[xPos][yPos];
      currentDrw->color = feature->getColor();
      currentDrw->glyph = feature->getGlyph();
    }
  }

  //-------------------------------------------- INSERT LIVING ACTORS INTO TILE ARRAY
  for(unsigned int i = 0; i < LOOP_SIZE; i++) {
    actor = eng->gameTime->getActorAt(i);
    xPos = actor->pos.x;
    yPos = actor->pos.y;
    if(actor->deadState == actorDeadState_alive && actor->getGlyph() != ' ' && eng->player->checkIfSeeActor(*actor, NULL)) {
      currentDrw = &renderArray[xPos][yPos];
      currentDrw->color = actor->getColor();
      currentDrw->glyph = actor->getGlyph();

      if(actor != eng->player) {
        const Monster* const monster = dynamic_cast<const Monster*>(actor);
        if(monster->leader == eng->player) {
          currentDrw->underscoreClr = clrGreen;
        } else {
          if(monster->playerAwarenessCounter == 0) {
            currentDrw->underscoreClr = clrBlueLight;
          }
        }
      }
    }
  }

  //-------------------------------------------- DRAW THE TILE GRID
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {

      tempDrw = renderArray[x][y];

      //If outside FOV and explored, draw player memory instead
      if(eng->map->playerVision[x][y] == false && eng->map->explored[x][y] == true) {
        renderArray[x][y] = eng->map->playerVisualMemory[x][y];
        tempDrw = renderArray[x][y];

        tempDrw.color.r /= 3;
        tempDrw.color.g /= 3;
        tempDrw.color.b /= 3;
      }

      drawCharacter(tempDrw.glyph, renderArea_mainScreen, x, y, tempDrw.color);

      if(tempDrw.underscoreClr.r != 0 || tempDrw.underscoreClr.g != 0 || tempDrw.underscoreClr.b != 0) {
        drawLineHorizontal(
          x * eng->config->CELL_W, eng->config->MAINSCREEN_Y_OFFSET + (y + 1) * eng->config->CELL_H - 2,
          eng->config->CELL_W, tempDrw.underscoreClr);
      }
    }
  }
}

void Renderer::drawTiles() {
  clearRenderArea(renderArea_mainScreen);

  TileAndColor* currentDrw = NULL;
  TileAndColor tempDrw;

  //-------------------------------------------- INSERT STATIC FEATURES AND BLOOD INTO TILE ARRAY
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {

      currentDrw = &renderArrayTiles[x][y];
      currentDrw->color = clrBlack;
      currentDrw->tile = tile_empty;
      currentDrw->underscoreClr = clrBlack;

      if(eng->map->playerVision[x][y] == true) {

        //Static features
        Tile_t goreTile = tile_empty;
        if(eng->map->featuresStatic[x][y]->canHaveGore()) {
          goreTile = eng->map->featuresStatic[x][y]->getGoreTile();
        }
        if(goreTile == tile_empty) {
          currentDrw->tile = eng->map->featuresStatic[x][y]->getTile();
          const SDL_Color featStdClr = eng->map->featuresStatic[x][y]->getColor();
          currentDrw->color = eng->map->featuresStatic[x][y]->hasBlood() ? clrRedLight : featStdClr;
        } else {
          currentDrw->tile = goreTile;
          currentDrw->color = clrRed;
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
      currentDrw->color = actor->getColor();
      currentDrw->tile = actor->getTile();
    }
  }

  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      currentDrw = &renderArrayTiles[x][y];
      if(eng->map->playerVision[x][y] == true) {
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
    if(feature->getGlyph() != ' ' && eng->map->playerVision[xPos][yPos] == true) {
      currentDrw = &renderArrayTiles[xPos][yPos];
      currentDrw->color = feature->getColor();
      currentDrw->tile = feature->getTile();
    }
  }

  //-------------------------------------------- INSERT LIVING ACTORS INTO TILE ARRAY
  for(unsigned int i = 0; i < LOOP_SIZE; i++) {
    actor = eng->gameTime->getActorAt(i);
    xPos = actor->pos.x;
    yPos = actor->pos.y;
    if(actor->deadState == actorDeadState_alive && actor->getTile() != tile_empty && eng->player->checkIfSeeActor(*actor, NULL)) {
      currentDrw = &renderArrayTiles[xPos][yPos];
      currentDrw->color = actor->getColor();
      if(actor == eng->player) {
        bool isWieldingRangedWeapon = false;
        Item* item = eng->player->getInventory()->getItemInSlot(slot_wielded);
        if(item != NULL) {
          isWieldingRangedWeapon = item->getInstanceDefinition().isRangedWeapon;
        }
        currentDrw->tile = isWieldingRangedWeapon ? tile_playerFirearm : tile_playerMelee;
      } else {
        currentDrw->tile = actor->getTile();
        const Monster* const monster = dynamic_cast<const Monster*>(actor);
        if(monster->leader == eng->player) {
          currentDrw->underscoreClr = clrGreen;
        } else {
          if(monster->playerAwarenessCounter == 0) {
            currentDrw->underscoreClr = clrBlueLight;
          }
        }
      }
    }
  }

  //-------------------------------------------- DRAW THE TILE GRID
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      tempDrw = renderArrayTiles[x][y];

      //If outside FOV and explored, draw player memory instead
      if(eng->map->playerVision[x][y] == false && eng->map->explored[x][y] == true) {
        renderArrayTiles[x][y] = eng->map->playerVisualMemoryTiles[x][y];
        tempDrw = renderArrayTiles[x][y];

        tempDrw.color.r /= 3;
        tempDrw.color.g /= 3;
        tempDrw.color.b /= 3;
      }

      /*
       * Walls are given an illusion of perspective.
       * If the tile to be set is a (top) wall tile, check the tile beneath it. If the tile beneath is not
       * a front or (top) wall tile, and that cell is explored, change the current tile to wall front
       */
      if(y < MAP_Y_CELLS - 1 && tempDrw.tile == tile_wall) {
        if(eng->map->explored[x][y + 1]) {

          const Tile_t tileBelowSeen = renderArrayActorsOmittedTiles[x][y + 1].tile;
          const Tile_t tileBelowMem = eng->map->playerVisualMemoryTiles[x][y + 1].tile;
          const bool IS_CELL_BELOW_SEEN = eng->map->playerVision[x][y + 1];

          const bool TILE_BELOW_IS_WALL_FRONT = IS_CELL_BELOW_SEEN ? tileBelowSeen == tile_wallFront : tileBelowMem == tile_wallFront;
          const bool TILE_BELOW_IS_WALL_TOP = IS_CELL_BELOW_SEEN ? tileBelowSeen == tile_wall : tileBelowMem == tile_wall;

          if(!TILE_BELOW_IS_WALL_FRONT && !TILE_BELOW_IS_WALL_TOP) {
            tempDrw.tile = tile_wallFront;
          }
        }
      }

      drawTileInMap(tempDrw.tile, x, y, tempDrw.color);

      if(tempDrw.underscoreClr.r != 0 || tempDrw.underscoreClr.g != 0 || tempDrw.underscoreClr.b != 0) {
        drawLineHorizontal(
          x * eng->config->CELL_W, eng->config->MAINSCREEN_Y_OFFSET + (y + 1) * eng->config->CELL_H - 2,
          eng->config->CELL_W, tempDrw.underscoreClr);
      }
    }
  }
}

