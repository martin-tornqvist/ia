#ifndef MAP_PARSING_H
#define MAP_PARSING_H

#include <vector>

#include "cmn_types.hpp"
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
    virtual bool is_checking_cells()          const {return false;}
    virtual bool is_checking_mobs()           const {return false;}
    virtual bool is_checking_actors()         const {return false;}
    virtual bool check(const Cell& c)       const {(void)c; return false;}
    virtual bool check(const Mob& f)        const {(void)f; return false;}
    virtual bool check(const Actor& a)      const {(void)a; return false;}
protected:
    Check() {}
};

class Blocks_los : public Check
{
public:
    Blocks_los() : Check() {}
    bool is_checking_cells()          const override {return true;}
    bool is_checking_mobs()           const override {return true;}
    bool check(const Cell& c)       const override;
    bool check(const Mob& f)        const override;
};

class Blocks_move_cmn : public Check
{
public:
    Blocks_move_cmn(bool is_actors_blocking) :
        Check(), IS_ACTORS_BLOCKING_(is_actors_blocking) {}
    bool is_checking_cells()          const override {return true;}
    bool is_checking_mobs()           const override {return true;}
    bool is_checking_actors()         const override {return IS_ACTORS_BLOCKING_;}
    bool check(const Cell& c)       const override;
    bool check(const Mob& f)        const override;
    bool check(const Actor& a)      const override;
private:
    const bool IS_ACTORS_BLOCKING_;
};

class Blocks_actor : public Check
{
public:
    Blocks_actor(Actor& actor, bool is_actors_blocking);
    bool is_checking_cells()          const override {return true;}
    bool is_checking_mobs()           const override {return true;}
    bool is_checking_actors()         const override {return IS_ACTORS_BLOCKING_;}
    bool check(const Cell& c)       const override;
    bool check(const Mob& f)        const override;
    bool check(const Actor& a)      const override;
private:
    const bool IS_ACTORS_BLOCKING_;
    bool actors_props_[size_t(Prop_id::END)];
};

class Blocks_projectiles : public Check
{
public:
    Blocks_projectiles() : Check() {}
    bool is_checking_cells()          const override {return true;}
    bool is_checking_mobs()           const override {return true;}
    bool check(const Cell& c)       const override;
    bool check(const Mob& f)        const override;
};

class Living_actors_adj_to_pos : public Check
{
public:
    Living_actors_adj_to_pos(const Pos& pos) :
        Check(), pos_(pos) {}
    bool is_checking_actors()         const override {return true;}
    bool check(const Actor& a)      const override;
    const Pos& pos_;
};

class Blocks_items : public Check
{
public:
    Blocks_items() : Check() {}
    bool is_checking_cells()          const override {return true;}
    bool is_checking_mobs()           const override {return true;}
    bool check(const Cell& c)       const override;
    bool check(const Mob& f)        const override;
};

//class Corridor : public Check {
//public:
//  Corridor() : Check() {}
//  bool is_checking_cells()          const override {return true;}
//  bool check(const Cell& c)       const override;
//};

// E.g. ##
//      #.
//      ##
//class Nook : public Check {
//public:
//  Nook() : Check() {}
//  bool is_checking_cells()          const override {return true;}
//  bool check(const Cell& c)       const override;
//};

class Is_feature : public Check
{
public:
    Is_feature(const Feature_id id) : Check(), feature_(id) {}
    bool is_checking_cells()          const override {return true;}
    bool check(const Cell& c)       const override;
private:
    const Feature_id feature_;
};

class Is_any_of_features : public Check
{
public:
    Is_any_of_features(const std::vector<Feature_id>& features) :
        Check(), features_(features) {}

    Is_any_of_features(const Feature_id id) :
        Check(), features_(std::vector<Feature_id> {id}) {}

    bool is_checking_cells()          const override {return true;}
    bool check(const Cell& c)       const override;
private:
    std::vector<Feature_id> features_;
};

class All_adj_is_feature : public Check
{
public:
    All_adj_is_feature(const Feature_id id) : Check(), feature_(id) {}
    bool is_checking_cells()          const override {return true;}
    bool check(const Cell& c)       const override;
private:
    const Feature_id feature_;
};

class All_adj_is_any_of_features : public Check
{
public:
    All_adj_is_any_of_features(const std::vector<Feature_id>& features) :
        Check(), features_(features) {}

    All_adj_is_any_of_features(const Feature_id id) :
        Check(), features_(std::vector<Feature_id> {id}) {}

    bool is_checking_cells()          const override {return true;}
    bool check(const Cell& c)       const override;
private:
    std::vector<Feature_id> features_;
};

class All_adj_is_not_feature : public Check
{
public:
    All_adj_is_not_feature(const Feature_id id) : Check(), feature_(id) {}
    bool is_checking_cells()          const override {return true;}
    bool check(const Cell& c)       const override;
private:
    const Feature_id feature_;
};

class All_adj_is_none_of_features : public Check
{
public:
    All_adj_is_none_of_features(const std::vector<Feature_id>& features) :
        Check(), features_(features) {}

    All_adj_is_none_of_features(const Feature_id id) :
        Check(), features_(std::vector<Feature_id> {id}) {}

    bool is_checking_cells()          const override {return true;}
    bool check(const Cell& c)       const override;
private:
    std::vector<Feature_id> features_;
};

} //cell_check

//NOTE: If append mode is used, the caller is responsible for initializing the array
//(typically with a previous parse call, with write rule set to "overwrite")
enum class Map_parse_mode {overwrite, append};

namespace map_parse
{

extern const Rect map_rect;

void run(const cell_check::Check& check, bool out[MAP_W][MAP_H],
         const map_parse_mode write_rule = map_parse_mode::overwrite,
         const Rect& area_to_check_cells = map_rect);

//Given a map array of booleans, this will fill a second map array of boolens
//where the cells are set to true if they are within the specified distance
//interval of the first array.
//This can be used for example to find all cells up to 3 steps from a wall.
void get_cells_within_dist_of_others(const bool in[MAP_W][MAP_H], bool out[MAP_W][MAP_H],
                                     const Range& dist_interval);

bool is_val_in_area(const Rect& area, const bool in[MAP_W][MAP_H], const bool VAL = true);

void append(bool base[MAP_W][MAP_H], const bool append[MAP_W][MAP_H]);

//Optimized for expanding with a distance of one
void expand(const bool in[MAP_W][MAP_H], bool out[MAP_W][MAP_H],
            const Rect& area_allowed_to_modify = Rect(0, 0, MAP_W, MAP_H));

//Slower version that can expand any distance
void expand(const bool in[MAP_W][MAP_H], bool out[MAP_W][MAP_H], const int DIST);

bool is_map_connected(const bool blocked[MAP_W][MAP_H]);

} //map_parse

//Function object for sorting STL containers by distance to a position
struct Is_closer_to_pos
{
public:
    Is_closer_to_pos(const Pos& p) : p_(p) {}
    bool operator()(const Pos& p1, const Pos& p2);
    Pos p_;
};

namespace flood_fill
{

void run(const Pos& p0, const bool blocked[MAP_W][MAP_H], int out[MAP_W][MAP_H],
         int travel_lmt, const Pos& p1, const bool ALLOW_DIAGONAL);

} //Flood_fill

namespace path_find
{

//NOTE: The path goes from target to origin, not including the origin.
//---------------------------------------------------------------------------------------
//RANDOMIZE_STEP_CHOICES: When true, for each step if there are multiple valid (nearer)
//                        choices, pick one at random. Otherwise iterate over a predefined
//                        list of offsets until a valid step is found. The second way is
//                        more optimized and is the default behavior (best for e.g. AI),
//                        while the randomized method can produces nicer results in some
//                        cases (e.g. corridors).
void run(const Pos& p0, const Pos& p1, bool blocked[MAP_W][MAP_H],
         std::vector<Pos>& out, const bool ALLOW_DIAGONAL = true,
         const bool RANDOMIZE_STEP_CHOICES = false);

} //path_find

#endif
