#ifndef MAP_CONTROLLER_HPP
#define MAP_CONTROLLER_HPP

#include <memory>

// -----------------------------------------------------------------------------
// MapController
// -----------------------------------------------------------------------------
class MapController
{
public:
        MapController() {}

        ~MapController() {}

        virtual void on_start() {}

        virtual void on_std_turn() {}
};

class MapControllerBoss: public MapController
{
public:
        MapControllerBoss() :
                MapController() {}

        void on_start() override;

        void on_std_turn() override;
};

// -----------------------------------------------------------------------------
// map_control
// -----------------------------------------------------------------------------
namespace map_control
{

extern std::unique_ptr<MapController> controller;

}

#endif // MAP_CONTROL_HPP
