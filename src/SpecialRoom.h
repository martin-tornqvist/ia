#ifndef SPECIAL_ROOM_H
#define SPECIAL_ROOM_H

#include <vector>
#include <string>

#include "ConstTypes.h"
#include "FeatureData.h"

using namespace std;

class Engine;

enum SpecialRoom_t {
	specialRoom_forest,
	specialRoom_statueGarden,
	specialRoom_ghoulStatueGarden,
	specialRoom_monster,
	specialRoom_spiderLair,
	specialRoom_tombs,
	specialRoom_highCeil,
	specialRoom_ritualChamber,
	specialRoom_shallowWater,
	specialRoom_shallowMud,
	specialRoom_poolOfBlood,
	specialRoom_pits,
	//specialRoom_temple,
//	specialRoom_armory,
//	specialRoom_library,

	endOfSpecialRooms
};

enum ForceDescriptPos_t {
	forceDescriptPos_first, forceDescriptPos_last, forceDescriptPos_any
};

//------------------------------------------------SPECIAL ROOM DESCRIPTION MODULES
struct RoomAndDescriptAssoc {
public:
	RoomAndDescriptAssoc(SpecialRoom_t roomId_, int chanceToOccur_, ForceDescriptPos_t forceDescriptPos_ = forceDescriptPos_any) :
		roomId(roomId_), chanceToOccur(chanceToOccur_), forceDescriptPos(forceDescriptPos_) {
	}

	RoomAndDescriptAssoc() {
	}

	SpecialRoom_t roomId;
	int chanceToOccur;
	ForceDescriptPos_t forceDescriptPos;
};

class RoomDescriptModule {
public:
	RoomDescriptModule() {
	}

	virtual RoomDescriptModule* copy() = 0;

	void initRoomData(const coord& x0y0_, const coord& x1y1_, Engine* engine) {
		x0y0 = x0y0_;
		x1y1 = x1y1_;
		eng = engine;
	}

	virtual bool isRoomDataAcceptedByModule(const Rectangle& coords) {
		(void) coords;
		return true;
	}

	void addRoomAssoc(const SpecialRoom_t roomId, const int CHANCE_TO_OCCUR, const ForceDescriptPos_t forceDescriptPos = forceDescriptPos_any) {
		roomsWhereAllowed.push_back(RoomAndDescriptAssoc(roomId, CHANCE_TO_OCCUR, forceDescriptPos));
	}

	RoomAndDescriptAssoc getRoomAssoc(const SpecialRoom_t roomId) {
		for(unsigned int i = 0; i < roomsWhereAllowed.size(); i++) {
			if(roomsWhereAllowed.at(i).roomId == roomId) {
				return roomsWhereAllowed.at(i);
			}
		}
		throw;
	}

	bool isAllowedInRoom(const SpecialRoom_t roomId) {
		for(unsigned int i = 0; i < roomsWhereAllowed.size(); i++) {
			if(roomsWhereAllowed.at(i).roomId == roomId && roomsWhereAllowed.at(i).chanceToOccur > 0) {
				return true;
			}
		}
		return false;
	}

	virtual void newTurn() {
	}
	virtual void makeRoom() {
	}

	virtual string getDescript() = 0;

	coord getRoomMid() {
		return coord((x1y1.x + x0y0.x) / 2, (x1y1.y + x0y0.y) / 2);
	}

protected:
	void clearGoreAt(const int X, const int Y);
	void clearGoreAt(const coord c);

	vector<RoomAndDescriptAssoc> roomsWhereAllowed;
	string descript;
	coord x0y0;
	coord x1y1;
	Engine* eng;
};

class RoomDescriptModuleForest: public RoomDescriptModule {
public:
	RoomDescriptModuleForest() :
		RoomDescriptModule() {
	}
	RoomDescriptModule* copy() {
		return new RoomDescriptModuleForest(*this);
	}
	void makeRoom();
	string getDescript() {
		return "";
	}
};

class RoomDescriptModuleStatueGarden: public RoomDescriptModule {
public:
	RoomDescriptModuleStatueGarden() :
		RoomDescriptModule() {
	}
	RoomDescriptModule* copy() {
		return new RoomDescriptModuleStatueGarden(*this);
	}
	void makeRoom();
	string getDescript() {
		return "";
	}
};

class RoomDescriptModuleGore: public RoomDescriptModule {
public:
	RoomDescriptModuleGore() :
		RoomDescriptModule() {
	}
	RoomDescriptModule* copy() {
		return new RoomDescriptModuleGore(*this);
	}
	void makeRoom();
	string getDescript() {
		return "";
	}
};

class RoomDescriptModuleDarkness: public RoomDescriptModule {
public:
	RoomDescriptModuleDarkness() :
		RoomDescriptModule() {
	}
	RoomDescriptModule* copy() {
		return new RoomDescriptModuleDarkness(*this);
	}
	void makeRoom();
	string getDescript() {
		return "";
	}
};

class RoomDescriptModuleSpiderLair: public RoomDescriptModule {
public:
	RoomDescriptModuleSpiderLair() :
		RoomDescriptModule() {
	}
	RoomDescriptModule* copy() {
		return new RoomDescriptModuleSpiderLair(*this);
	}
	void makeRoom();
	string getDescript() {
		return "";
	}
};

class RoomDescriptModuleGhoulStatueGarden: public RoomDescriptModule {
public:
	RoomDescriptModuleGhoulStatueGarden() :
		RoomDescriptModule() {
	}
	RoomDescriptModule* copy() {
		return new RoomDescriptModuleGhoulStatueGarden(*this);
	}
	void makeRoom();
	string getDescript() {
		return "";
	}
};

class RoomDescriptModuleTombs: public RoomDescriptModule {
public:
	RoomDescriptModuleTombs() :
		RoomDescriptModule() {
	}
	RoomDescriptModule* copy() {
		return new RoomDescriptModuleTombs(*this);
	}
	void makeRoom();
	string getDescript() {
		return "";
	}
};

class RoomDescriptModuleHighCeil: public RoomDescriptModule {
public:
	RoomDescriptModuleHighCeil() :
		RoomDescriptModule(), allowCreaturesComeDown(true), chanceCreaturesComeDown(0) {
	}
	RoomDescriptModule* copy() {
		return new RoomDescriptModuleHighCeil(*this);
	}

	void newTurn();
	void makeRoom();

	string getDescript() {
		return "The walls here reaches far upward towards a dark domed ceiling, where wooden beams extend across the room.";
	}
private:
	void creaturesComeDown();
	bool allowCreaturesComeDown;
	int chanceCreaturesComeDown;
};

class RoomDescriptModuleRitualChamber: public RoomDescriptModule {
public:
	RoomDescriptModuleRitualChamber() :
		RoomDescriptModule() {
	}
	RoomDescriptModule* copy() {
		return new RoomDescriptModuleRitualChamber(*this);
	}
	void makeRoom();
	string getDescript() {
		return "";
	}
};

class RoomDescriptModulePool: public RoomDescriptModule {
public:
	RoomDescriptModulePool() :
		RoomDescriptModule() {
	}

protected:
	void makePool();
	virtual Feature_t getPoolFeatureId() = 0;
};

class RoomDescriptModulePoolOfWater: public RoomDescriptModulePool {
public:
	RoomDescriptModulePoolOfWater() :
		RoomDescriptModulePool() {
	}
	RoomDescriptModule* copy() {
		return new RoomDescriptModulePoolOfWater(*this);
	}

	void makeRoom();
	void newTurn();

	string getDescript() {
		//return "There is a pond here.";
		return "";
	}
private:
	virtual Feature_t getPoolFeatureId() {
		return feature_shallowWater;
	}
};

class RoomDescriptModulePoolOfMud: public RoomDescriptModulePool {
public:
	RoomDescriptModulePoolOfMud() :
		RoomDescriptModulePool() {
	}
	RoomDescriptModule* copy() {
		return new RoomDescriptModulePoolOfMud(*this);
	}
	void makeRoom();
	string getDescript() {
		//return "There is a swamp here.";
		return "";
	}
private:
	virtual Feature_t getPoolFeatureId() {
		return feature_shallowMud;
	}
};

class RoomDescriptModulePoolOfBlood: public RoomDescriptModulePool {
public:
	RoomDescriptModulePoolOfBlood() :
		RoomDescriptModulePool() {
	}
	RoomDescriptModule* copy() {
		return new RoomDescriptModulePoolOfBlood(*this);
	}
	void makeRoom();
	string getDescript() {
		return "";
	}
private:
	virtual Feature_t getPoolFeatureId() {
		return feature_poolBlood;
	}
};

class RoomDescriptModulePits: public RoomDescriptModule {
public:
	RoomDescriptModulePits() :
		RoomDescriptModule() {
	}
	RoomDescriptModule* copy() {
		return new RoomDescriptModulePits(*this);
	}
	void makeRoom();

	void newTurn();

	string getDescript() {
		return "";
	}
};

class RoomDescriptModuleTemple: public RoomDescriptModule {
public:
	RoomDescriptModuleTemple() :
		RoomDescriptModule() {
	}
	RoomDescriptModule* copy() {
		return new RoomDescriptModuleTemple(*this);
	}
	void makeRoom();
	string getDescript() {
		return "[Temple]";
	}
};

class RoomDescriptModuleFloorPattern: public RoomDescriptModule {
public:
	RoomDescriptModuleFloorPattern() :
		RoomDescriptModule() {
	}
	RoomDescriptModule* copy() {
		return new RoomDescriptModuleFloorPattern(*this);
	}
	void makeRoom();
	string getDescript() {
		return "[Floor pattern]";
	}
};

class Door;

//class RoomDescriptModuleTreasureRoom: public RoomDescriptModule {
//public:
//	RoomDescriptModuleTreasureRoom() :
//		RoomDescriptModule(), hasTraps(false) {
//		doorsToRoom.resize(0);
//	}
//
//	bool isTreasureRoom() const {
//		return true;
//	}
//
//	void makeRoom();
//
//	virtual void treasureRoomSpecificMakeRoom() = 0;
//
//	bool isRoomDataAcceptedByModule();
//	string getDescript();
//protected:
//	void makeTraps();
//	bool hasTraps;
//	vector<Door*> doorsToRoom;
//};
//
//class RoomDescriptModuleArmory: public RoomDescriptModuleTreasureRoom {
//public:
//	RoomDescriptModuleArmory() :
//		RoomDescriptModuleTreasureRoom() {
//	}
//
//	RoomDescriptModuleArmory* copy() {
//		RoomDescriptModuleArmory* cpy = new RoomDescriptModuleArmory(*this);
//		cpy->doorsToRoom.resize(0);
//		cpy->doorsToRoom = doorsToRoom;
//		return cpy;
//	}
//
//	void treasureRoomSpecificMakeRoom();
//
//private:
//};
//
//class RoomDescriptModuleLibrary: public RoomDescriptModuleTreasureRoom {
//public:
//	RoomDescriptModuleLibrary() :
//		RoomDescriptModuleTreasureRoom() {
//	}
//
//	RoomDescriptModuleLibrary* copy() {
//		RoomDescriptModuleLibrary* cpy = new RoomDescriptModuleLibrary(*this);
//		cpy->doorsToRoom.resize(0);
//		cpy->doorsToRoom = doorsToRoom;
//		return cpy;
//	}
//
//	void treasureRoomSpecificMakeRoom();
//
//private:
//};

class SpecialRoom;
class MapArea;

//Responsible for converting rooms to special rooms, and to check if the player
//stands in one, to pop up text and ask the modules to run special events.
class SpecialRoomHandler {
public:
	SpecialRoomHandler(Engine* engine) :
		eng(engine) {
		makeModuleDataList();
	}

	~SpecialRoomHandler();

	void makeSpecialRooms(vector<MapArea*>& mapAreas);

	bool attemptMakeSpecialRoom(const SpecialRoom_t roomType, MapArea& mapArea);

	void runRoomEvents();

	void deleteAllSpecialRooms();

	const SpecialRoom* getRoomAtIndex(const unsigned int i) const {
		return roomsActive.at(i);
	}
	unsigned int getNrOfRooms() const {
		return roomsActive.size();
	}

private:
	//A list of ALL modules. When modules are added to a room, they are copied from
	//this list. They are first asked whether they fit for the current room.
	vector<RoomDescriptModule*> moduleData;
	void makeModuleDataList();

	void addAssocToBack(const SpecialRoom_t room, const int CHANCE_TO_OCCUR, const ForceDescriptPos_t forceDescriptPos = forceDescriptPos_any) {
		moduleData.back()->addRoomAssoc(room, CHANCE_TO_OCCUR, forceDescriptPos);
	}

	vector<SpecialRoom*> roomsActive;

	void addModulesToRoom(SpecialRoom* room, const coord roomX0Y0, const coord roomX1Y1);

	Engine* eng;
};

class SpecialRoom {
public:
	SpecialRoom() {
	}
	SpecialRoom(coord x0y0_, coord x1y1_, SpecialRoom_t roomType_, Engine* engine) :
		x0y0(x0y0_), x1y1(x1y1_), eng(engine), roomType(roomType_), hasShownDescript(false) {
	}

	~SpecialRoom() {
		for(unsigned int i = 0; i < modules.size(); i++) {
			delete modules.at(i);
		}
	}

	void addModule(RoomDescriptModule* const module) {
		modules.push_back(module);
	}

	void makeRoom() {
		for(unsigned int i = 0; i < modules.size(); i++) {
			modules.at(i)->makeRoom();
		}
	}

	void attemptRunEvents(); //Descriptions and new turn

	bool isCellInside(const coord& pos) const {
		return pos.x >= x0y0.x && pos.x <= x1y1.x && pos.y >= x0y0.y && pos.y <= x1y1.y;
	}

	unsigned int getNrOfModulesAdded() {
		return modules.size();
	}

	void spawnRoomMonsters();

	coord getX0Y0() const {
		return x0y0;
	}
	coord getX1Y1() const {
		return x1y1;
	}

	SpecialRoom_t getRoomType() const {
		return roomType;
	}

private:
	vector<RoomDescriptModule*> modules;

	coord x0y0;
	coord x1y1;

	Engine* eng;

	SpecialRoom_t roomType;

	bool hasShownDescript;
};

#endif

