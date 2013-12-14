#ifndef DEBUG_MODE_STAT_PRINTER_H
#define DEBUG_MODE_STAT_PRINTER_H

#include <string>
#include <iostream>
#include <fstream>

using namespace std;

class Engine;

class DebugModeStatPrinter {
public:
  DebugModeStatPrinter(Engine& engine) : eng(engine) {
  }

  ~DebugModeStatPrinter() {
  }

  void run();

private:
  void printLine(const string& line);

  ofstream statFile;

  Engine& eng;
};


#endif
