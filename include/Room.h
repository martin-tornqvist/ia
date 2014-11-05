//Note: This file must not define "ROOM_H" for the include guard. That name
//should be reserved for constants representing room height.
#ifndef ROOM_CLASS_H
#define ROOM_CLASS_H

#include <vector>
#ifdef MACOSX
#include <numeric>
#endif

#include "CmnTypes.h"
#include "CmnData.h"

enum class RoomType
{
  //Standard rooms (standardized feature spawning and reshaping)
  plain,
  human,
  ritual,
  spider,
  crypt,
  monster,
  flooded,
  muddy,
  END_OF_STD_ROOMS,

  //Exceptional areas
  //Note: Keep these after the standard rooms in this list! The room list gets
  //sorted according to RoomType before "onPreConnect" etc is called. The
  //standard rooms assume that they are rectangular with unbroken walls when
  //their generic reshaping runs (called from "onPreConnect").
  river,
  cave,
  corridorJunction,
  crumbleRoom
};

class Room
{
public:
  Room(Rect r, RoomType type = RoomType::plain) :
    descr_(""), r_(r), type_(type), isSubRoom_(false) {}

  virtual ~Room() {}

  virtual void onPreConnect(bool doorProposals[MAP_W][MAP_H]);
  virtual void onPostConnect(bool doorProposals[MAP_W][MAP_H]) {(void)doorProposals;}

  Pos getCenterPos() const {return (r_.p1 + r_.p0) / 2;}

  std::string descr_;
  Rect        r_;
  std::vector<Room*> roomsConTo_;
  RoomType    type_;
  bool        isSubRoom_;

private:
  Room() : descr_(""), r_(),  type_(RoomType::plain) {}
};

class RiverRoom: public Room
{
public:
  RiverRoom(Rect r, HorizontalVertical dir) : Room(r, RoomType::river), dir_(dir) {}

  void onPreConnect(bool doorProposals[MAP_W][MAP_H]) override;

private:
  RiverRoom() : Room(Rect(), RoomType::river) {}

  HorizontalVertical dir_;
};

#endif
