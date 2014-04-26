#ifndef MANUAL_H
#define MANUAL_H

#include <vector>
#include <string>

using namespace std;



class Manual {
public:
  Manual() : eng() {
    readFile();
  }

  void run();

private:
  void readFile();

  void drawManualInterface();

  vector<string> lines;


};

#endif
