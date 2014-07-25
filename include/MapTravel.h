#ifndef MAP_TRAVEL_H
#define MAP_TRAVEL_H

#include <vector>
#include <string>

using namespace std;

namespace MapTravel {

void init();

void storeToSaveLines(std::vector<std::string>& lines);
void setupFromSaveLines(std::vector<std::string>& lines);

void tryUseDownStairs();

void goToNxt();

} //MapTravel

#endif
