#ifndef CREDITS_SCR_H
#define CREDITS_SCR_H

#include <vector>
#include <string>

class Credits {
public:
  Credits() {readFile();}

  void run();

private:
  void readFile();

  std::vector<std::string> lines;

};

#endif
