#ifndef ROOM_THEME_H
#define ROOM_THEME_H

#include <vector>

using namespace std;

class Engine;

enum RoomTheme_t {
  roomTheme_plain,
  roomTheme_human,
  roomTheme_ritual,
  roomTheme_spider,
  roomTheme_jail,
  roomTheme_tomb,
  roomTheme_monster,

  endOfRoomThemes
};

struct Room;

class RoomThemeMaker {
public:
  RoomThemeMaker(Engine* engine) : eng(engine) {
  }

  void run(const vector<Room*>& rooms);

private:
  Engine* eng;
};


#endif
