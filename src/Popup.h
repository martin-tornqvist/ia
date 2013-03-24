#ifndef POPUP_H
#define POPUP_H

#include <vector>
#include <string>

using namespace std;

class Engine;

class Popup {
public:
  Popup(Engine* engine) :
    eng(engine) {
  }

  void showMessage(const string& message, const bool DRAW_MAP_AND_INTERFACE, const string title = "") const;

  unsigned int showMultiChoiceMessage(const string& message, const bool DRAW_MAP_AND_INTERFACE,
                                      const vector<string>& choices, const string title = "") const;
private:
  int printBoxAndReturnTitleYPos(const int TEXT_AREA_HEIGHT) const;

  void multiChoiceMessageDrawingHelper(const vector<string>& lines, const vector<string>& choices, const bool DRAW_MAP_AND_INTERFACE,
                                       const unsigned int currentChoice, const int TEXT_AREA_HEIGHT, const string title = "") const;
  Engine* eng;
};

#endif
