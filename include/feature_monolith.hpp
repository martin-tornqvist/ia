#ifndef FEATURE_MONOLITH_HPP
#define FEATURE_MONOLITH_HPP

#include "feature_rigid.hpp"

class Monolith: public Rigid
{
public:
    Monolith(const P& p);
    Monolith() = delete;
    ~Monolith() {}

    FeatureId id() const override
    {
        return FeatureId::monolith;
    }

    std::string name(const Article article) const override;

    void bump(Actor& actor_bumping) override;

private:
    Clr clr_default() const override;

    void on_hit(const DmgType dmg_type,
                const DmgMethod dmg_method,
                Actor* const actor) override;

    void activate();

    void spawn_monsters();

    bool is_activated_;
};

#endif //FEATURE_MONOLITH_HPP
