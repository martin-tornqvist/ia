#ifndef DEBUG_MODE_STAT_PRINTER_H
#define DEBUG_MODE_STAT_PRINTER_H

#include <string>
#include <iostream>
#include <fstream>

using namespace std;

class Engine;

//IS_DEBUG_MODE

class DebugModeStatPrinter {
public:
  DebugModeStatPrinter(Engine* const engine) : eng(engine) {
  }

  ~DebugModeStatPrinter() {
  }

  void run();

private:
  void printLine(const string& line);

  ofstream statFile;

  Engine* const eng;
};


#endif
