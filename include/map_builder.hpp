#ifndef MAP_BUILDER_HPP
#define MAP_BUILDER_HPP

#include <memory>
#include <vector>

#include "rl_utils.hpp"
#include "map_templates.hpp"

class MapController;

enum class MapType
{
        intro,
        std,
        egypt,
        rat_cave,
        boss,
        trapez
};

// -----------------------------------------------------------------------------
// MapBuilder
// -----------------------------------------------------------------------------
class MapBuilder
{
public:
        MapBuilder() {}

        virtual ~MapBuilder() {}

        void build();

private:
        virtual bool build_specific() = 0;

        virtual std::unique_ptr<MapController> map_controller() const;
};

// -----------------------------------------------------------------------------
// MapBuilderTemplateLevel
// -----------------------------------------------------------------------------
class MapBuilderTemplateLevel: public MapBuilder
{
public:
        MapBuilderTemplateLevel() :
                MapBuilder(),
                template_(nullptr) {}

        virtual ~MapBuilderTemplateLevel() {}

protected:
        const Array2<char>* template_;

private:
        bool build_specific() override final;

        virtual LevelTemplId template_id() const = 0;

        virtual void handle_template_pos(const P& p, const char c) = 0;

        virtual void on_template_built() {}
};

// -----------------------------------------------------------------------------
// MapBuilderStd
// -----------------------------------------------------------------------------
class MapBuilderStd: public MapBuilder
{
public:
        MapBuilderStd() :
                MapBuilder() {}

        ~MapBuilderStd() {}

private:
        bool build_specific() override;
};

// -----------------------------------------------------------------------------
// MapBuilderIntroForest
// -----------------------------------------------------------------------------
class MapBuilderIntroForest: public MapBuilderTemplateLevel
{
public:
        MapBuilderIntroForest() :
                MapBuilderTemplateLevel(),
                possible_grave_positions_() {}

        ~MapBuilderIntroForest() {}

private:
        LevelTemplId template_id() const override
        {
                return LevelTemplId::intro_forest;
        }

        void handle_template_pos(const P& p, const char c) override;

        void on_template_built() override;

        std::vector<P> possible_grave_positions_;
};

// -----------------------------------------------------------------------------
// MapBuilderEgypt
// -----------------------------------------------------------------------------
class MapBuilderEgypt: public MapBuilderTemplateLevel
{
public:
        MapBuilderEgypt();

        ~MapBuilderEgypt() {}

private:
        LevelTemplId template_id() const override
        {
                return LevelTemplId::egypt;
        }

        void handle_template_pos(const P& p, const char c) override;

        void on_template_built() override;

        const char stair_symbol_;
};

// -----------------------------------------------------------------------------
// MapBuilderRatCave
// -----------------------------------------------------------------------------
class MapBuilderRatCave: public MapBuilderTemplateLevel
{
public:
        MapBuilderRatCave() :
                MapBuilderTemplateLevel() {}

        ~MapBuilderRatCave() {}

private:
        LevelTemplId template_id() const override
        {
                return LevelTemplId::rat_cave;
        }

        void handle_template_pos(const P& p, const char c) override;

        void on_template_built() override;
};

// -----------------------------------------------------------------------------
// MapBuilderBoss
// -----------------------------------------------------------------------------
class MapBuilderBoss: public MapBuilderTemplateLevel
{
public:
        MapBuilderBoss() :
                MapBuilderTemplateLevel() {}

        ~MapBuilderBoss() {}

private:
        LevelTemplId template_id() const override
        {
                return LevelTemplId::boss_level;
        }

        void handle_template_pos(const P& p, const char c) override;

        void on_template_built() override;

        std::unique_ptr<MapController> map_controller() const override;
};

// -----------------------------------------------------------------------------
// MapBuilderTrapez
// -----------------------------------------------------------------------------
class MapBuilderTrapez: public MapBuilderTemplateLevel
{
public:
        MapBuilderTrapez() :
                MapBuilderTemplateLevel() {}

        virtual ~MapBuilderTrapez() {}

private:
        LevelTemplId template_id() const override
        {
                return LevelTemplId::trapez_level;
        }

        void handle_template_pos(const P& p, const char c) override;
};

namespace map_builder
{

std::unique_ptr<MapBuilder> make(const MapType map_type);

} // map_builder

#endif // MAP_BUILDER_HPP
