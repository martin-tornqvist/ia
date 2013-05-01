//#include "Viewport.h"
//
//#include "Engine.h"
//
//Rect Viewport::getArea() {
//  const int VIEWPORT_X_CELLS = eng->config->TILES_VIEWPORT_X_CELLS;
//  const int VIEWPORT_Y_CELLS = eng->config->TILES_VIEWPORT_Y_CELLS;
//
//  const int X1 = pos_.x + VIEWPORT_X_CELLS - 1;
//  const int Y1 = pos_.y + VIEWPORT_Y_CELLS - 1;
//
//  return Rect(pos_, coord(X1, Y1));
//}
//
//void Viewport::centerAt(const coord& posToCenterAt) {
//  const int VIEWPORT_X_CELLS = eng->config->TILES_VIEWPORT_X_CELLS;
//  const int VIEWPORT_Y_CELLS = eng->config->TILES_VIEWPORT_Y_CELLS;
//
//  const int VIEWPORT_X_CELLS_HALF = VIEWPORT_X_CELLS / 2;
//  const int VIEWPORT_Y_CELLS_HALF = VIEWPORT_Y_CELLS / 2;
//
//  pos_.x = posToCenterAt.x - VIEWPORT_X_CELLS_HALF;
//  pos_.y = posToCenterAt.y - VIEWPORT_Y_CELLS_HALF;
//
//  const int X_MAX = MAP_X_CELLS - VIEWPORT_X_CELLS;
//  const int Y_MAX = MAP_Y_CELLS - VIEWPORT_Y_CELLS;
//
//  pos_.x = max(0, min(X_MAX, pos_.x));
//  pos_.y = max(0, min(Y_MAX, pos_.y));
//}
