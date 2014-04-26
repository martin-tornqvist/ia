#ifndef SAVE_HANDLER
#define SAVE_HANDLER

#include <vector>
#include <string>

#include "MenuBrowser.h"

using namespace std;

class SaveHandler {
public:
  SaveHandler() {}

  void load();
  void save();

  bool isSaveAvailable();

private:
  void collectLinesFromGame(vector<string>& lines);
  void setupGameFromLines(vector<string>& lines) const;
  void readFile(vector<string>& lines);
  void writeFile(const vector<string>& lines) const;
};

#endif
