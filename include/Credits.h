#ifndef CREDITS_SCR_H
#define CREDITS_SCR_H

#include <vector>
#include <string>

using namespace std;



class Credits {
public:
  Credits() : eng() {readFile();}

  void run();

private:
  void readFile();

  vector<string> lines;

};

#endif
