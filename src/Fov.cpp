#include "Fov.h"

#include <math.h>
#include <iostream>

#include "Engine.h"
#include "FovPreCalc.h"
#include "ConstDungeonSettings.h"

void Fov::allUnseen(bool array[MAP_X_CELLS][MAP_Y_CELLS]) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
        for(int y = 0; y < MAP_Y_CELLS; y++) {
            array[x][y] = false;
        }
    }
}

void Fov::performCheck(bool obstructions[MAP_X_CELLS][MAP_Y_CELLS], int _checkX, int _checkY, int _originX, int _originY,
                       bool array[MAP_X_CELLS][MAP_Y_CELLS], const bool AFFECTED_BY_DARKNESS) {
    const coord origin(_originX, _originY);

    const coord deltaToTarget(_checkX - _originX, _checkY - _originY);
    vector<coord> pathDeltas;
    eng->fovPreCalc->getLineTravelVector(pathDeltas, deltaToTarget);

    const bool TARGET_IS_LIGHT = eng->map->light[_checkX][_checkY];

    coord curPos;
    const unsigned int PATH_SIZE = pathDeltas.size();

    if(eng->basicUtils->chebyshevDistance(_originX, _originY, _checkX, _checkY) <= FOV_RADI_INT) {
        for(unsigned int i = 0; i < PATH_SIZE; i++) {
            curPos.set(origin + pathDeltas.at(i));
            if(i > 1) {
                const bool CURRENT_CELL_IS_DARK = eng->map->darkness[curPos.x][curPos.y];
                const bool CURRENT_CELL_IS_LIGHT = eng->map->light[curPos.x][curPos.y];
                if(CURRENT_CELL_IS_LIGHT == false && TARGET_IS_LIGHT == false && CURRENT_CELL_IS_DARK && AFFECTED_BY_DARKNESS) {
                    return;
                }
            }
            if(curPos == coord(_checkX, _checkY)) {
                array[_checkX][_checkY] = true;
                return;
            }
            if(i > 0) {
                if(obstructions[curPos.x][curPos.y]) {
                    return;
                }
            }
        }
    }
}

bool Fov::checkOneCell(bool obstructions[MAP_X_CELLS][MAP_Y_CELLS], const int checkX, const int checkY, const int _originX, const int _originY,
                       const bool AFFECTED_BY_DARKNESS) {
    const coord origin(_originX, _originY);

    if(eng->basicUtils->chebyshevDistance(_originX, _originY, checkX, checkY) > FOV_RADI_INT) {
        return false;
    }

    const coord deltaToTarget(checkX - _originX, checkY - _originY);
    vector<coord> pathDeltas;
    eng->fovPreCalc->getLineTravelVector(pathDeltas, deltaToTarget);

    const bool TARGET_IS_LIGHT = eng->map->light[checkX][checkY];

    coord curPos;
    const unsigned int PATH_SIZE = pathDeltas.size();

    for(unsigned int i = 0; i < PATH_SIZE; i++) {
        curPos.set(origin + pathDeltas.at(i));
        if(i > 1) {
            const bool CURRENT_CELL_IS_DARK = eng->map->darkness[curPos.x][curPos.y];
            const bool CURRENT_CELL_IS_LIGHT = eng->map->light[curPos.x][curPos.y];
            if(CURRENT_CELL_IS_LIGHT == false && TARGET_IS_LIGHT == false && CURRENT_CELL_IS_DARK && AFFECTED_BY_DARKNESS) {
                return false;
            }
        }
        if(curPos == coord(checkX, checkY)) {
            return true;
        }
        if(i > 0) {
            if(obstructions[curPos.x][curPos.y]) {
                return false;
            }
        }
    }
    return false;
}

void Fov::runFovOnArray(bool obstructions[MAP_X_CELLS][MAP_Y_CELLS], int originX, int originY, bool array[MAP_X_CELLS][MAP_Y_CELLS], const bool AFFECTED_BY_DARKNESS) {
    int checkX, checkY;

    allUnseen(array);

    array[originX][originY] = true;

    const int checkX_end = originX + FOV_RADI_INT;
    const int checkY_end = originY + FOV_RADI_INT;

    checkX = originX - FOV_RADI_INT;

    while(checkX <= checkX_end) {
        checkY = originY - FOV_RADI_INT;

        while(checkY <= checkY_end) {
            performCheck(obstructions, checkX, checkY, originX, originY, array, AFFECTED_BY_DARKNESS);
            checkY++;
        }
        checkX++;
    }
}

void Fov::runPlayerFov(bool obstructions[MAP_X_CELLS][MAP_Y_CELLS], const int originX, const int originY) {
    int checkX, checkY;

    allUnseen(eng->map->playerVision);

    eng->map->playerVision[originX][originY] = true;

    const int checkX_end = originX + static_cast<int> (FOV_RADI);
    const int checkY_end = originY + static_cast<int> (FOV_RADI);

    checkX = originX - static_cast<int> (FOV_RADI);

    while(checkX <= checkX_end) {
        checkY = originY - static_cast<int> (FOV_RADI);

        while(checkY <= checkY_end) {
            performCheck(obstructions, checkX, checkY, originX, originY, eng->map->playerVision, true);
            checkY++;
        }
        checkX++;
    }
}

