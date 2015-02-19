#ifndef FEATURE_MOB_H
#define FEATURE_MOB_H

#include "Feature.h"

class Mob: public Feature
{
public:
    Mob(const Pos& pos) : Feature(pos) {}

    Mob() = delete;

    virtual ~Mob() {}

    virtual Feature_id   get_id()                         const override = 0;
    virtual std::string get_name(const Article article)  const override = 0;
    Clr                 get_clr()                        const override = 0;

    Clr get_clr_bg() const override final {return clr_black;}
};

class Smoke: public Mob
{
public:
    Smoke(const Pos& pos, const int NR_TURNS) :
        Mob(pos), nr_turns_left_(NR_TURNS) {}

    //Spawn-by-id compliant ctor (do not use for normal cases):
    Smoke(const Pos& pos) : Mob(pos), nr_turns_left_(-1) {}

    Smoke() = delete;
    ~Smoke() {}

    Feature_id get_id() const override {return Feature_id::smoke;}

    std::string get_name(const Article article)  const override;
    Clr         get_clr()                        const override;

    void on_new_turn() override;

protected:
    int nr_turns_left_;
};

class Lit_dynamite: public Mob
{
public:
    Lit_dynamite(const Pos& pos, const int NR_TURNS) :
        Mob(pos), nr_turns_left_(NR_TURNS) {}

    //Spawn-by-id compliant ctor (do not use for normal cases):
    Lit_dynamite(const Pos& pos) : Mob(pos), nr_turns_left_(-1) {}

    Lit_dynamite() = delete;

    ~Lit_dynamite() {}

    Feature_id get_id() const override {return Feature_id::lit_dynamite;}

    std::string get_name(const Article article)  const override;
    Clr         get_clr()                        const override;

    //TODO: Lit dynamite should add light on their own cell (just one cell)
    //void add_light(bool light[MAP_W][MAP_H]) const;

    void on_new_turn() override;

private:
    int nr_turns_left_;
};

class Lit_flare: public Mob
{
public:
    Lit_flare(const Pos& pos, const int NR_TURNS) :
        Mob(pos), nr_turns_left_(NR_TURNS) {}

    //Spawn-by-id compliant ctor (do not use for normal cases):
    Lit_flare(const Pos& pos) : Mob(pos), nr_turns_left_(-1) {}

    Lit_flare() = delete;

    ~Lit_flare() {}

    Feature_id get_id() const override {return Feature_id::lit_flare;}

    std::string get_name(const Article article)  const override;
    Clr         get_clr()                        const override;

    void on_new_turn() override;

    void add_light(bool light[MAP_W][MAP_H]) const;

private:
    int nr_turns_left_;
};

#endif
