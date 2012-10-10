#include "Config.h"

#include <fstream>
#include <iostream>

#include "Converters.h"
#include "Engine.h"

using namespace std;

Config::Config() :
  GAME_VERSION("v14.0"),

  TILES_IMAGE_NAME("images/gfx_16x24.bmp"),

  MAIN_MENU_LOGO_IMAGE_NAME("images/main_menu_logo.bmp"),

  LOG_X_CELLS_OFFSET(1), LOG_Y_CELLS_OFFSET(1),

  LOG_X_CELLS(MAP_X_CELLS - LOG_X_CELLS_OFFSET),

  CHARACTER_LINES_Y_CELLS_OFFSET(LOG_Y_CELLS_OFFSET + 1 + MAP_Y_CELLS),

  CHARACTER_LINES_Y_CELLS(3),

  SCREEN_BPP(32),

  FRAMES_PER_SECOND(30),

  PLAYER_START_X(10),

  PLAYER_START_Y(MAP_Y_CELLS_HALF),

  BOT_PLAYING(false),

  MAINSCREEN_Y_CELLS_OFFSET(LOG_Y_CELLS_OFFSET + 1) {

  read();

  setCellDimDependentVariables();
}

void Config::setCellDimDependentVariables() {
  MAINSCREEN_WIDTH = MAP_X_CELLS * CELL_W;
  MAINSCREEN_HEIGHT = MAP_Y_CELLS * CELL_H;

  LOG_X_OFFSET = LOG_X_CELLS_OFFSET * CELL_W;
  LOG_Y_OFFSET = LOG_Y_CELLS_OFFSET * CELL_H;

  LOG_WIDTH = LOG_X_CELLS * CELL_W;
  LOG_HEIGHT = CELL_H;

  MAINSCREEN_Y_OFFSET = MAINSCREEN_Y_CELLS_OFFSET * CELL_H;
  CHARACTER_LINES_Y_OFFSET = LOG_Y_OFFSET + LOG_HEIGHT + MAINSCREEN_HEIGHT;

  CHARACTER_LINES_HEIGHT = CHARACTER_LINES_Y_CELLS * CELL_H;

  SCREEN_WIDTH = MAP_X_CELLS * CELL_W;
  SCREEN_HEIGHT = CHARACTER_LINES_Y_OFFSET + CHARACTER_LINES_HEIGHT;
}

void Config::trySetVariableFromLine(string line) {
  size_t foundPos;
  string varName;

  varName = "FONT_IMAGE=";
  foundPos = line.find(varName);
  if(foundPos != string::npos) {
    line = line.substr(foundPos + varName.size());
    switch(stringToInt(line)) {
    case 0: {
      FONT_IMAGE_NAME = "images/8x12.bmp";
      CELL_W = 8;
      CELL_H = 12;
    }
      break;
    case 1: {
      FONT_IMAGE_NAME = "images/11x19.bmp";
      CELL_W = 11;
      CELL_H = 19;
    }
      break;
    case 2: {
      FONT_IMAGE_NAME = "images/16x24.bmp";
      CELL_W = 16;
      CELL_H = 24;
    }
      break;
    case 3: {
      FONT_IMAGE_NAME = "images/16x24_typewriter.bmp";
      CELL_W = 16;
      CELL_H = 24;
    }
      break;
    }
  }

  varName = "USE_TILE_SET=";
  foundPos = line.find(varName);
  if(foundPos != string::npos) {
    line = line.substr(foundPos + varName.size());
    if(line.at(0) == '0') {
      USE_TILE_SET = false;
    } else {
      USE_TILE_SET = true;
      if(CELL_W != 16 || CELL_H != 24) {
	FONT_IMAGE_NAME = "images/16x24_typewriter.bmp";
	CELL_W = 16;
	CELL_H = 24;
      }
    }
  }

  varName = "FULLSCREEN=";
  foundPos = line.find(varName);
  if(foundPos != string::npos) {
    line = line.substr(foundPos + varName.size());
    FULLSCREEN = line.at(0) == '0' ? false : true;
  }

  varName = "WALL_SYMBOL_FULL_SQUARE=";
  foundPos = line.find(varName);
  if(foundPos != string::npos) {
    line = line.substr(foundPos + varName.size());
    WALL_SYMBOL_FULL_SQUARE = line.at(0) == '0' ? false : true;
  }

  varName = "SKIP_INTRO_LEVEL=";
  foundPos = line.find(varName);
  if(foundPos != string::npos) {
    line = line.substr(foundPos + varName.size());
    SKIP_INTRO_LEVEL = line.at(0) == '0' ? false : true;
  }

  varName = "RANGED_WPN_MELEE_PROMPT=";
  foundPos = line.find(varName);
  if(foundPos != string::npos) {
    line = line.substr(foundPos + varName.size());
    RANGED_WPN_MELEE_PROMPT = line.at(0) == '0' ? false : true;
  }

  varName = "KEY_REPEAT_DELAY=";
  foundPos = line.find(varName);
  if(foundPos != string::npos) {
    line = line.substr(foundPos + varName.size());
    KEY_REPEAT_DELAY = stringToInt(line);
  }

  varName = "KEY_REPEAT_INTERVAL=";
  foundPos = line.find(varName);
  if(foundPos != string::npos) {
    line = line.substr(foundPos + varName.size());
    KEY_REPEAT_INTERVAL = stringToInt(line);
  }

  varName = "DELAY_PROJECTILE_DRAW=";
  foundPos = line.find(varName);
  if(foundPos != string::npos) {
    line = line.substr(foundPos + varName.size());
    DELAY_PROJECTILE_DRAW = stringToInt(line);
  }

  varName = "DELAY_SHOTGUN=";
  foundPos = line.find(varName);
  if(foundPos != string::npos) {
    line = line.substr(foundPos + varName.size());
    DELAY_SHOTGUN = stringToInt(line);
  }

  varName = "DELAY_EXPLOSION=";
  foundPos = line.find(varName);
  if(foundPos != string::npos) {
    line = line.substr(foundPos + varName.size());
    DELAY_EXPLOSION = stringToInt(line);
  }
}

void Config::read() {
  string curLine;
  ifstream file("./config.txt");

  if(file.is_open()) {
    while(getline(file, curLine)) {
      if(curLine.size() > 0) {
	if(curLine.at(0) != '#') {
	  trySetVariableFromLine(curLine);
	}
      }
      curLine = "";
    }
  } else {
    tracer << "[WARNING] could not open config file, in Config::read()" << endl;
  }

  file.close();
}

