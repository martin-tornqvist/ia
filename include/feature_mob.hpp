#ifndef FEATURE_MOB_HPP
#define FEATURE_MOB_HPP

#include "feature.hpp"

class Mob: public Feature
{
public:
    Mob(const P& feature_pos) : Feature(feature_pos) {}

    Mob() = delete;

    virtual ~Mob() {}

    virtual FeatureId id() const override = 0;

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
    Smoke(const P& feature_pos, const int nr_turns) :
        Mob(feature_pos), nr_turns_left_(nr_turns) {}

    //Spawn-by-id compliant ctor (do not use for normal cases):
    Smoke(const P& feature_pos) : Mob(feature_pos), nr_turns_left_(-1) {}

    Smoke() = delete;
    ~Smoke() {}

    FeatureId id() const override
    {
        return FeatureId::smoke;
    }

    std::string name(const Article article)  const override;
    Clr clr() const override;

    void on_new_turn() override;

protected:
    int nr_turns_left_;
};

class LitDynamite: public Mob
{
public:
    LitDynamite(const P& feature_pos, const int nr_turns) :
        Mob(feature_pos), nr_turns_left_(nr_turns) {}

    //Spawn-by-id compliant ctor (do not use for normal cases):
    LitDynamite(const P& feature_pos) : Mob(feature_pos), nr_turns_left_(-1) {}

    LitDynamite() = delete;

    ~LitDynamite() {}

    FeatureId id() const override
    {
        return FeatureId::lit_dynamite;
    }

    std::string name(const Article article) const override;

    Clr clr() const override;

    //TODO: Lit dynamite should add light on their own cell (just one cell)
    //void add_light(bool light[map_w][map_h]) const;

    void on_new_turn() override;

private:
    int nr_turns_left_;
};

class LitFlare: public Mob
{
public:
    LitFlare(const P& feature_pos, const int nr_turns) :
        Mob(feature_pos), nr_turns_left_(nr_turns) {}

    //Spawn-by-id compliant ctor (do not use for normal cases):
    LitFlare(const P& feature_pos) : Mob(feature_pos), nr_turns_left_(-1) {}

    LitFlare() = delete;

    ~LitFlare() {}

    FeatureId id() const override
    {
        return FeatureId::lit_flare;
    }

    std::string name(const Article article) const override;

    Clr clr() const override;

    void on_new_turn() override;

    void add_light(bool light[map_w][map_h]) const override;

private:
    int nr_turns_left_;
};

#endif
