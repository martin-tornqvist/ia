#ifndef CREDITS_SCR_H
#define CREDITS_SCR_H

#include <vector>
#include <string>

using namespace std;

class Engine;

class Credits {
public:
  Credits(Engine& engine) : eng(engine) {readFile();}

  void run();

private:
  void readFile();
  void drawInterface();

  vector<string> lines;
  Engine& eng;
};

#endif
