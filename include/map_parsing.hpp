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

class Blocks_los : public Check
{
public:
    Blocks_los() : Check() {}
    bool is_checking_cells()        const override {return true;}
    bool is_checking_mobs()         const override {return true;}
    bool check(const Cell& c)       const override;
    bool check(const Mob& f)        const override;
};

class Blocks_move_cmn : public Check
{
public:
    Blocks_move_cmn(bool is_actors_blocking) :
        Check(),
        is_actors_blocking_(is_actors_blocking) {}
    bool is_checking_cells()        const override {return true;}
    bool is_checking_mobs()         const override {return true;}
    bool is_checking_actors()       const override {return is_actors_blocking_;}
    bool check(const Cell& c)       const override;
    bool check(const Mob& f)        const override;
    bool check(const Actor& a)      const override;
private:
    const bool is_actors_blocking_;
};

class Blocks_actor : public Check
{
public:
    Blocks_actor(Actor& actor, bool is_actors_blocking) :
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

class Blocks_projectiles : public Check
{
public:
    Blocks_projectiles() : Check() {}
    bool is_checking_cells()        const override {return true;}
    bool is_checking_mobs()         const override {return true;}
    bool check(const Cell& c)       const override;
    bool check(const Mob& f)        const override;
};

class Living_actors_adj_to_pos : public Check
{
public:
    Living_actors_adj_to_pos(const P& pos) :
        Check(), pos_(pos) {}
    bool is_checking_actors()       const override {return true;}
    bool check(const Actor& a)      const override;
    const P& pos_;
};

class Blocks_items : public Check
{
public:
    Blocks_items() : Check() {}
    bool is_checking_cells()        const override {return true;}
    bool is_checking_mobs()         const override {return true;}
    bool check(const Cell& c)       const override;
    bool check(const Mob& f)        const override;
};

class Is_feature : public Check
{
public:
    Is_feature(const Feature_id id) : Check(), feature_(id) {}
    bool is_checking_cells()        const override {return true;}
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

    bool is_checking_cells()        const override {return true;}
    bool check(const Cell& c)       const override;
private:
    std::vector<Feature_id> features_;
};

class All_adj_is_feature : public Check
{
public:
    All_adj_is_feature(const Feature_id id) : Check(), feature_(id) {}
    bool is_checking_cells()        const override {return true;}
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

    bool is_checking_cells()        const override {return true;}
    bool check(const Cell& c)       const override;
private:
    std::vector<Feature_id> features_;
};

class All_adj_is_not_feature : public Check
{
public:
    All_adj_is_not_feature(const Feature_id id) : Check(), feature_(id) {}
    bool is_checking_cells()        const override {return true;}
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

    bool is_checking_cells()        const override {return true;}
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

const R map_rect(0, 0, map_w - 1, map_h - 1);

void run(const cell_check::Check& method,
         bool out[map_w][map_h],
         const Map_parse_mode write_rule = Map_parse_mode::overwrite,
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
struct Is_closer_to_pos
{
public:
    Is_closer_to_pos(const P& p) :
        p_(p) {}

    bool operator()(const P& p1, const P& p2);

    P p_;
};

#endif
