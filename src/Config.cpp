#include "Config.h"

#include <fstream>
#include <iostream>

#include "Converters.h"
#include "Engine.h"

using namespace std;

Config::Config() :
	GAME_VERSION(" v13.0, 2012.09.15"),

	TILES_IMAGE_NAME("images/gfx_16x24.bmp"),

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

	MAINSCREEN_WIDTH = MAP_X_CELLS * CELL_WIDTH_MAP;
	MAINSCREEN_HEIGHT = MAP_Y_CELLS * CELL_HEIGHT_MAP;

	LOG_X_OFFSET = LOG_X_CELLS_OFFSET * CELL_WIDTH_TEXT;
	LOG_Y_OFFSET = LOG_Y_CELLS_OFFSET * CELL_HEIGHT_TEXT;

	LOG_WIDTH = LOG_X_CELLS * CELL_WIDTH_TEXT;
	LOG_HEIGHT = CELL_HEIGHT_TEXT;

	MAINSCREEN_Y_OFFSET = MAINSCREEN_Y_CELLS_OFFSET * CELL_HEIGHT_TEXT;
	CHARACTER_LINES_Y_OFFSET = LOG_Y_OFFSET + LOG_HEIGHT + MAINSCREEN_HEIGHT;

	CHARACTER_LINES_HEIGHT = CHARACTER_LINES_Y_CELLS * CELL_HEIGHT_TEXT;

	//SCREEN_X_CELLS = MAP_X_CELLS;
	//SCREEN_Y_CELLS = MAINSCREEN_Y_CELLS_OFFSET + MAP_Y_CELLS + CHARACTER_LINES_Y_CELLS;

	SCREEN_WIDTH = MAP_X_CELLS * CELL_WIDTH_MAP; //SCREEN_X_CELLS * CELL_WIDTH;
	SCREEN_HEIGHT = CHARACTER_LINES_Y_OFFSET + CHARACTER_LINES_HEIGHT; //SCREEN_Y_CELLS * CELL_HEIGHT;
	//SCREEN_X_CELLS_HALF = SCREEN_X_CELLS / 2;
	//SCREEN_Y_CELLS_HALF = SCREEN_Y_CELLS / 2;
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
			CELL_WIDTH_TEXT = 8;
			CELL_HEIGHT_TEXT = 12;
		}
		break;
		case 1: {
			FONT_IMAGE_NAME = "images/11x19.bmp";
			CELL_WIDTH_TEXT = 11;
			CELL_HEIGHT_TEXT = 19;
		}
		break;
		case 2: {
			FONT_IMAGE_NAME = "images/16x24.bmp";
			CELL_WIDTH_TEXT = 16;
			CELL_HEIGHT_TEXT = 24;
		}
		break;
		case 3: {
			FONT_IMAGE_NAME = "images/16x24_typewriter.bmp";
			CELL_WIDTH_TEXT = 16;
			CELL_HEIGHT_TEXT = 24;
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
			CELL_WIDTH_MAP = CELL_WIDTH_TEXT;
			CELL_HEIGHT_MAP = CELL_HEIGHT_TEXT;
		} else {
			if(CELL_WIDTH_TEXT != CELL_WIDTH_MAP || CELL_HEIGHT_TEXT != CELL_HEIGHT_MAP) {
				USE_TILE_SET = true;
				FONT_IMAGE_NAME = "images/16x24_typewriter.bmp";
				CELL_WIDTH_TEXT = 16;
				CELL_HEIGHT_TEXT = 24;
				CELL_WIDTH_MAP = 16;
				CELL_HEIGHT_MAP = 24;
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

//	varName = "ALWAYS_ASK_DIRECTION=";
//	foundPos = line.find(varName);
//	if(foundPos != string::npos) {
//		line = line.substr(foundPos + varName.size());
//		ALWAYS_ASK_DIRECTION = line.at(0) == '0' ? false : true;
//	}

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

