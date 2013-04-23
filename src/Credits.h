#ifndef CREDITS_H
#define CREDITS_H

#include <vector>
#include <string>

using namespace std;

class Engine;

class Credits {
public:
  Credits(Engine* engine) : eng(engine) {
    readFile();
  }

  void run();

private:
  void readFile();

  void drawInterface();

  vector<string> lines;

  Engine* eng;
};

#endif
