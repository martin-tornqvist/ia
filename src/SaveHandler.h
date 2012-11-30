#ifndef SAVE_HANDLER
#define SAVE_HANDLER

#include <vector>
#include <string>

#include "MenuBrowser.h"

using namespace std;

class Engine;

class SaveHandler {
public:
  SaveHandler(Engine* engine) : eng(engine) {}

  void load();
  void save();

  bool isSaveAvailable();

private:
  void collectLinesFromGame(vector<string>& lines);
  void setGameParametersFromLines(vector<string>& lines) const;
  void readFile(vector<string>& lines);
  void writeFile(const vector<string>& lines) const;

  Engine* eng;
};

#endif
