#ifndef MAP_PARSING_HPP
#define MAP_PARSING_HPP

#include <vector>

#include "config.hpp"
#include "feature_data.hpp"

struct Cell;
class Mob;
class Actor;

namespace cell_check
{

class Check
{
public:
    virtual ~Check() {}
    virtual bool is_checking_cells()        const {return false;}
    virtual bool is_checking_mobs()         const {return false;}
    virtual bool is_checking_actors()       const {return false;}
    virtual bool check(const Cell& c)       const {(void)c; return false;}
    virtual bool check(const Mob& f)        const {(void)f; return false;}
    virtual bool check(const Actor& a)      const {(void)a; return false;}
protected:
    Check() {}
};

class BlocksLos : public Check
{
public:
    BlocksLos() : Check() {}
    bool is_checking_cells()        const override {return true;}
    bool is_checking_mobs()         const override {return true;}
    bool check(const Cell& c)       const override;
    bool check(const Mob& f)        const override;
};

class BlocksMoveCmn : public Check
{
public:
    BlocksMoveCmn(bool is_actors_blocking) :
        Check               (),
        is_actors_blocking_ (is_actors_blocking) {}
    bool is_checking_cells()        const override {return true;}
    bool is_checking_mobs()         const override {return true;}
    bool is_checking_actors()       const override {return is_actors_blocking_;}
    bool check(const Cell& c)       const override;
    bool check(const Mob& f)        const override;
    bool check(const Actor& a)      const override;
private:
    const bool is_actors_blocking_;
};

class BlocksActor : public Check
{
public:
    BlocksActor(Actor& actor, bool is_actors_blocking) :
        Check(),
        is_actors_blocking_ (is_actors_blocking),
        actor_              (actor) {}
    bool is_checking_cells()        const override {return true;}
    bool is_checking_mobs()         const override {return true;}
    bool is_checking_actors()       const override {return is_actors_blocking_;}
    bool check(const Cell& c)       const override;
    bool check(const Mob& f)        const override;
    bool check(const Actor& a)      const override;
private:
    const bool is_actors_blocking_;
    Actor& actor_;
};

class BlocksProjectiles : public Check
{
public:
    BlocksProjectiles() : Check() {}
    bool is_checking_cells()        const override {return true;}
    bool is_checking_mobs()         const override {return true;}
    bool check(const Cell& c)       const override;
    bool check(const Mob& f)        const override;
};

class LivingActorsAdjToPos : public Check
{
public:
    LivingActorsAdjToPos(const P& pos) :
        Check(), pos_(pos) {}
    bool is_checking_actors()       const override {return true;}
    bool check(const Actor& a)      const override;
    const P& pos_;
};

class BlocksItems : public Check
{
public:
    BlocksItems() : Check() {}
    bool is_checking_cells()        const override {return true;}
    bool is_checking_mobs()         const override {return true;}
    bool check(const Cell& c)       const override;
    bool check(const Mob& f)        const override;
};

class BlocksRigid : public Check
{
public:
    BlocksRigid() : Check() {}
    bool is_checking_cells()        const override {return true;}
    bool check(const Cell& c)       const override;
};

class IsFeature : public Check
{
public:
    IsFeature(const FeatureId id) : Check(), feature_(id) {}
    bool is_checking_cells()        const override {return true;}
    bool check(const Cell& c)       const override;
private:
    const FeatureId feature_;
};

class IsAnyOfFeatures : public Check
{
public:
    IsAnyOfFeatures(const std::vector<FeatureId>& features) :
        Check(), features_(features) {}

    IsAnyOfFeatures(const FeatureId id) :
        Check(), features_(std::vector<FeatureId> {id}) {}

    bool is_checking_cells()        const override {return true;}
    bool check(const Cell& c)       const override;
private:
    std::vector<FeatureId> features_;
};

class AllAdjIsFeature : public Check
{
public:
    AllAdjIsFeature(const FeatureId id) : Check(), feature_(id) {}
    bool is_checking_cells()        const override {return true;}
    bool check(const Cell& c)       const override;
private:
    const FeatureId feature_;
};

class AllAdjIsAnyOfFeatures : public Check
{
public:
    AllAdjIsAnyOfFeatures(const std::vector<FeatureId>& features) :
        Check(), features_(features) {}

    AllAdjIsAnyOfFeatures(const FeatureId id) :
        Check(), features_(std::vector<FeatureId> {id}) {}

    bool is_checking_cells()        const override {return true;}
    bool check(const Cell& c)       const override;
private:
    std::vector<FeatureId> features_;
};

class AllAdjIsNotFeature : public Check
{
public:
    AllAdjIsNotFeature(const FeatureId id) : Check(), feature_(id) {}
    bool is_checking_cells()        const override {return true;}
    bool check(const Cell& c)       const override;
private:
    const FeatureId feature_;
};

class AllAdjIsNoneOfFeatures : public Check
{
public:
    AllAdjIsNoneOfFeatures(const std::vector<FeatureId>& features) :
        Check(), features_(features) {}

    AllAdjIsNoneOfFeatures(const FeatureId id) :
        Check(), features_(std::vector<FeatureId> {id}) {}

    bool is_checking_cells()        const override {return true;}
    bool check(const Cell& c)       const override;
private:
    std::vector<FeatureId> features_;
};

} //cell_check

//NOTE: If append mode is used, the caller is responsible for initializing the array
//(typically with a previous parse call, with write rule set to "overwrite")
enum class MapParseMode {overwrite, append};

namespace map_parse
{

const R map_rect(0, 0, map_w - 1, map_h - 1);

void run(const cell_check::Check& method,
         bool out[map_w][map_h],
         const MapParseMode write_rule = MapParseMode::overwrite,
         const R& area_to_check_cells = map_rect);

bool cell(const cell_check::Check& method, const P& p);

//Given a map array of booleans, this will fill a second map array of boolens
//where the cells are set to true if they are within the specified distance
//interval of the first array.
//This can be used for example to find all cells up to 3 steps from a wall.
void cells_within_dist_of_others(const bool in[map_w][map_h],
                                 bool out[map_w][map_h],
                                 const Range& dist_interval);

void append(bool base[map_w][map_h], const bool append[map_w][map_h]);

//Optimized for expanding with a distance of one
void expand(const bool in[map_w][map_h],
            bool out[map_w][map_h],
            const R& area_allowed_to_modify = R(0, 0, map_w, map_h));

//Slower version that can expand any distance
void expand(const bool in[map_w][map_h],
            bool out[map_w][map_h],
            const int dist);

bool is_map_connected(const bool blocked[map_w][map_h]);

} //map_parse

//Function object for sorting STL containers by distance to a position
struct IsCloserToPos
{
public:
    IsCloserToPos(const P& p) :
        p_(p) {}

    bool operator()(const P& p1, const P& p2);

    P p_;
};

#endif
