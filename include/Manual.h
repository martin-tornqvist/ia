#ifndef MANUAL_H
#define MANUAL_H

#include <vector>
#include <string>

class Manual {
public:
  Manual() : eng() {readFile();}

  void run();

private:
  void readFile();

  void drawManualInterface();

  std::vector<std::string> lines;


};

#endif
