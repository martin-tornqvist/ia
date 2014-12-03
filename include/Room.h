//Note: This file must not define "ROOM_H" for the include guard. That name should be
//reserved for constants representing room height.
#ifndef MAPGEN_ROOM_H
#define MAPGEN_ROOM_H

#include <vector>

#ifdef MACOSX
#include <numeric>
#endif

#include "CmnTypes.h"
#include "CmnData.h"

//---------------------------------------------------------------------------------------
// Room theming occurs both before and after rooms are connected (pre/post-connect).
//   > In pre-connect reshaping is done. The reshaping is called from MapGenUtils
//     (e.g. plus-shape, cavern-shape, pillars, etc)
//
//     When pre-connect starts, it is assumed that all (standard) rooms are rectangular
//     with unbroken walls.
//
//   > In post-connect, auto-features such as chests and altars are placed (this is
//     configured in the feature data), as well as room-specific stuff like trees, altars,
//     etc. It can then be verified for each feature that the map is still connected.
//
// As a rule of thumb, place walkable features in the pre-connect step, and blocking
// features in the post-connect step.
//---------------------------------------------------------------------------------------

struct  FeatureDataT;
class   Room;

enum class RoomType
{
  //Standard rooms (standardized feature spawning and reshaping)
  plain, //Note: "plain" must be the first type
  human,
  ritual,
  spider,
  crypt,
  monster,
  flooded,
  muddy,
  cave,
  chasm,
  forest,
  END_OF_STD_ROOMS,

  //Exceptional room types
  corrLink,
  crumbleRoom,
  river
};

namespace RoomFactory
{

void initRoomBucket();

//Note: These functions do not make rooms on the map, they merely create Room objects.
Room* mk(const RoomType type, const Rect& r);

Room* mkRandomAllowedStdRoom(const Rect& r, const bool IS_SUBROOM);

} //RoomFactory

class Room
{
public:
  Room(Rect r, RoomType type);

  Room() = delete;

  virtual ~Room() {}

  virtual void onPreConnect (bool doorProposals[MAP_W][MAP_H]) = 0;
  virtual void onPostConnect(bool doorProposals[MAP_W][MAP_H]) = 0;

  Rect                r_;
  const RoomType      type_;
  bool                isSubRoom_;
  std::vector<Room*>  roomsConTo_;
  std::vector<Room*>  subRooms_;

protected:
  void mkDrk() const;
};

class StdRoom : public Room
{
public:
  StdRoom(Rect r, RoomType type) : Room(r, type) {}

  virtual ~StdRoom() {}

  void onPreConnect (bool doorProposals[MAP_W][MAP_H]) override final;
  void onPostConnect(bool doorProposals[MAP_W][MAP_H]) override final;

  virtual bool isAllowed() const {return true;}

protected:
  virtual Range getNrAutoFeaturesAllowed()  const = 0;
  virtual int   getBasePctChanceDrk()      const = 0;

  size_t tryGetAutoFeaturePlacement(
    const std::vector<Pos>& adjToWalls, const std::vector<Pos>& awayFromWalls,
    const std::vector<const FeatureDataT*>& featureDataBucket, Pos& posRef) const;

  int placeAutoFeatures();

  virtual void onPreConnect_ (bool doorProposals[MAP_W][MAP_H]) = 0;
  virtual void onPostConnect_(bool doorProposals[MAP_W][MAP_H]) = 0;
};

class PlainRoom: public StdRoom
{
public:
  PlainRoom(Rect r) : StdRoom(r, RoomType::plain) {}

  ~PlainRoom() {}

protected:
  Range   getNrAutoFeaturesAllowed()                        const override;
  int     getBasePctChanceDrk()                             const override;
  void    onPreConnect_ (bool doorProposals[MAP_W][MAP_H])        override;
  void    onPostConnect_(bool doorProposals[MAP_W][MAP_H])        override;
};

class HumanRoom: public StdRoom
{
public:
  HumanRoom(Rect r) : StdRoom(r, RoomType::human) {}

  ~HumanRoom() {}

  bool    isAllowed()                                       const override;

protected:
  Range   getNrAutoFeaturesAllowed()                        const override;
  int     getBasePctChanceDrk()                             const override;
  void    onPreConnect_ (bool doorProposals[MAP_W][MAP_H])        override;
  void    onPostConnect_(bool doorProposals[MAP_W][MAP_H])        override;
};

class RitualRoom: public StdRoom
{
public:
  RitualRoom(Rect r) : StdRoom(r, RoomType::ritual) {}

  ~RitualRoom() {}

  bool    isAllowed()                                       const override;

protected:
  Range   getNrAutoFeaturesAllowed()                        const override;
  int     getBasePctChanceDrk()                             const override;
  void    onPreConnect_ (bool doorProposals[MAP_W][MAP_H])        override;
  void    onPostConnect_(bool doorProposals[MAP_W][MAP_H])        override;
};

class SpiderRoom: public StdRoom
{
public:
  SpiderRoom(Rect r) : StdRoom(r, RoomType::spider) {}

  ~SpiderRoom() {}

  bool    isAllowed()                                       const override;

protected:
  Range   getNrAutoFeaturesAllowed()                        const override;
  int     getBasePctChanceDrk()                             const override;
  void    onPreConnect_ (bool doorProposals[MAP_W][MAP_H])        override;
  void    onPostConnect_(bool doorProposals[MAP_W][MAP_H])        override;
};

class CryptRoom: public StdRoom
{
public:
  CryptRoom(Rect r) : StdRoom(r, RoomType::crypt) {}

  ~CryptRoom() {}

  bool    isAllowed()                                       const override;

protected:
  Range   getNrAutoFeaturesAllowed()                        const override;
  int     getBasePctChanceDrk()                             const override;
  void    onPreConnect_ (bool doorProposals[MAP_W][MAP_H])        override;
  void    onPostConnect_(bool doorProposals[MAP_W][MAP_H])        override;
};

class MonsterRoom: public StdRoom
{
public:
  MonsterRoom(Rect r) : StdRoom(r, RoomType::monster) {}

  ~MonsterRoom() {}

  bool    isAllowed()                                       const override;

protected:
  Range   getNrAutoFeaturesAllowed()                        const override;
  int     getBasePctChanceDrk()                             const override;
  void    onPreConnect_ (bool doorProposals[MAP_W][MAP_H])        override;
  void    onPostConnect_(bool doorProposals[MAP_W][MAP_H])        override;
};

class FloodedRoom: public StdRoom
{
public:
  FloodedRoom(Rect r) : StdRoom(r, RoomType::flooded) {}

  ~FloodedRoom() {}

  bool    isAllowed()                                       const override;

protected:
  Range   getNrAutoFeaturesAllowed()                        const override;
  int     getBasePctChanceDrk()                             const override;
  void    onPreConnect_ (bool doorProposals[MAP_W][MAP_H])        override;
  void    onPostConnect_(bool doorProposals[MAP_W][MAP_H])        override;
};

class MuddyRoom: public StdRoom
{
public:
  MuddyRoom(Rect r) : StdRoom(r, RoomType::muddy) {}

  ~MuddyRoom() {}

  bool    isAllowed()                                       const override;

protected:
  Range   getNrAutoFeaturesAllowed()                        const override;
  int     getBasePctChanceDrk()                             const override;
  void    onPreConnect_ (bool doorProposals[MAP_W][MAP_H])        override;
  void    onPostConnect_(bool doorProposals[MAP_W][MAP_H])        override;
};

class CaveRoom: public StdRoom
{
public:
  CaveRoom(Rect r) : StdRoom(r, RoomType::cave) {}

  ~CaveRoom() {}

  bool    isAllowed()                                       const override;

protected:
  Range   getNrAutoFeaturesAllowed()                        const override;
  int     getBasePctChanceDrk()                             const override;
  void    onPreConnect_ (bool doorProposals[MAP_W][MAP_H])        override;
  void    onPostConnect_(bool doorProposals[MAP_W][MAP_H])        override;
};

class ChasmRoom: public StdRoom
{
public:
  ChasmRoom(Rect r) : StdRoom(r, RoomType::chasm) {}

  ~ChasmRoom() {}

  bool    isAllowed()                                       const override;

protected:
  Range   getNrAutoFeaturesAllowed()                        const override;
  int     getBasePctChanceDrk()                             const override;
  void    onPreConnect_ (bool doorProposals[MAP_W][MAP_H])        override;
  void    onPostConnect_(bool doorProposals[MAP_W][MAP_H])        override;
};

class ForestRoom: public StdRoom
{
public:
  ForestRoom(Rect r) : StdRoom(r, RoomType::forest) {}

  ~ForestRoom() {}

  bool    isAllowed()                                       const override;

protected:
  Range   getNrAutoFeaturesAllowed()                        const override;
  int     getBasePctChanceDrk()                             const override;
  void    onPreConnect_ (bool doorProposals[MAP_W][MAP_H])        override;
  void    onPostConnect_(bool doorProposals[MAP_W][MAP_H])        override;
};

class CorrLinkRoom: public Room
{
public:
  CorrLinkRoom(const Rect& r) : Room(r, RoomType::corrLink) {}

  ~CorrLinkRoom() {}

  void onPreConnect(bool doorProposals[MAP_W][MAP_H]) override
  {
    (void)doorProposals;
  }

  void onPostConnect(bool doorProposals[MAP_W][MAP_H]) override
  {
    (void)doorProposals;
  }
};

class CrumbleRoom: public Room
{
public:
  CrumbleRoom(const Rect& r) : Room(r, RoomType::crumbleRoom) {}

  ~CrumbleRoom() {}

  void onPreConnect(bool doorProposals[MAP_W][MAP_H]) override
  {
    (void)doorProposals;
  }

  void onPostConnect(bool doorProposals[MAP_W][MAP_H]) override
  {
    (void)doorProposals;
  }
};

class RiverRoom: public Room
{
public:
  RiverRoom(const Rect& r) : Room(r, RoomType::river), dir_(HorizontalVertical::hor) {}

  ~RiverRoom() {}

  void onPreConnect(bool doorProposals[MAP_W][MAP_H]) override;

  void onPostConnect(bool doorProposals[MAP_W][MAP_H]) override
  {
    (void)doorProposals;
  }

  HorizontalVertical dir_;
};

#endif
