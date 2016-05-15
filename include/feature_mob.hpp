#ifndef FEATURE_MOB_HPP
#define FEATURE_MOB_HPP

#include "feature.hpp"

class Mob: public Feature
{
public:
    Mob(const P& feature_pos) : Feature(feature_pos) {}

    Mob() = delete;

    virtual ~Mob() {}

    virtual Feature_id id() const override = 0;

    virtual std::string name(const Article article) const override = 0;

    Clr clr() const override = 0;

    Clr clr_bg() const override final
    {
        return clr_black;
    }
};

class Smoke: public Mob
{
public:
    Smoke(const P& feature_pos, const int NR_TURNS) :
        Mob(feature_pos), nr_turns_left_(NR_TURNS) {}

    //Spawn-by-id compliant ctor (do not use for normal cases):
    Smoke(const P& feature_pos) : Mob(feature_pos), nr_turns_left_(-1) {}

    Smoke() = delete;
    ~Smoke() {}

    Feature_id id() const override
    {
        return Feature_id::smoke;
    }

    std::string name(const Article article)  const override;
    Clr clr() const override;

    void on_new_turn() override;

protected:
    int nr_turns_left_;
};

class Lit_dynamite: public Mob
{
public:
    Lit_dynamite(const P& feature_pos, const int NR_TURNS) :
        Mob(feature_pos), nr_turns_left_(NR_TURNS) {}

    //Spawn-by-id compliant ctor (do not use for normal cases):
    Lit_dynamite(const P& feature_pos) : Mob(feature_pos), nr_turns_left_(-1) {}

    Lit_dynamite() = delete;

    ~Lit_dynamite() {}

    Feature_id id() const override
    {
        return Feature_id::lit_dynamite;
    }

    std::string name(const Article article) const override;

    Clr clr() const override;

    //TODO: Lit dynamite should add light on their own cell (just one cell)
    //void add_light(bool light[MAP_W][MAP_H]) const;

    void on_new_turn() override;

private:
    int nr_turns_left_;
};

class Lit_flare: public Mob
{
public:
    Lit_flare(const P& feature_pos, const int NR_TURNS) :
        Mob(feature_pos), nr_turns_left_(NR_TURNS) {}

    //Spawn-by-id compliant ctor (do not use for normal cases):
    Lit_flare(const P& feature_pos) : Mob(feature_pos), nr_turns_left_(-1) {}

    Lit_flare() = delete;

    ~Lit_flare() {}

    Feature_id id() const override
    {
        return Feature_id::lit_flare;
    }

    std::string name(const Article article) const override;

    Clr clr() const override;

    void on_new_turn() override;

    void add_light(bool light[MAP_W][MAP_H]) const;

private:
    int nr_turns_left_;
};

#endif
