//Note: This file must not define "ROOM_H" for the include guard. That name
//should be reserved for constants representing room height.
#ifndef ROOM_CLASS_H
#define ROOM_CLASS_H

#include <vector>

#include "CmnTypes.h"

enum class RoomType {
  //Standard rooms (standardized feature spawning and reshaping)
  plain,
  human,
  ritual,
  spider,
  crypt,
  monster,
  flooded,
  muddy,
  endOfStdRooms,

  //Exceptional areas
  river,
  cave,
  corridorJunction
};

class Room {
public:
  Room(Rect r, RoomType type = RoomType::plain) :
    descr_(""), r_(r), type_(type) {}
  Room() :
    descr_(""), r_(),  type_(RoomType::plain) {}

  virtual ~Room() {}

  virtual void onPreConnect();
  virtual void onPostConnect()  {}

  Pos getCenterPos() const {return (r_.p1 + r_.p0) / 2;}

  std::string descr_;
  Rect        r_;
  std::vector<Room*> roomsConTo_;
  RoomType    type_;
};

class RiverRoom: public Room {
public:
  RiverRoom(Rect r) : Room(r,       RoomType::river) {}
  RiverRoom()       : Room(Rect(),  RoomType::river) {}

  void onPreConnect() override;
};

#endif
