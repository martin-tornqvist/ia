#ifndef INV_HANDLING_HPP
#define INV_HANDLING_HPP

#include "inventory.hpp"
#include "state.hpp"
#include "browser.hpp"

enum class InvScr
{
        inv,
        equip,
        apply,
        none
};

class InvState: public State
{
public:
        InvState();

        virtual ~InvState() {}

        StateId id() override;

protected:
        void draw_slot(
                const SlotId id,
                const int y,
                const char key,
                const bool is_marked,
                const ItemRefAttInf att_info) const;

        void draw_backpack_item(
                const size_t backpack_idx,
                const int y,
                const char key,
                const bool is_marked,
                const ItemRefAttInf att_info) const;

        void activate(const size_t backpack_idx);

        MenuBrowser browser_;

        void draw_weight_pct_and_dots(
                const P item_pos,
                const size_t item_name_len,
                const Item& item,
                const Color& item_name_color_id,
                const bool is_marked) const;

        // void draw_item_symbol(const Item& item, const P& p) const;

        void draw_detailed_item_descr(const Item* const item) const;
};

class BrowseInv: public InvState
{
public:
        BrowseInv() :
                InvState() {}

        void on_start() override;

        void on_resume() override;

        void draw() override;

        void update() override;
};

class Apply: public InvState
{
public:
        Apply() :
                InvState(),
                filtered_backpack_indexes_() {}

        void on_start() override;

        void draw() override;

        void update() override;

private:
        std::vector<size_t> filtered_backpack_indexes_;
};

class Drop: public InvState
{
public:
        Drop() :
                InvState(),
                filtered_slots_() {}

        void on_start() override;

        void draw() override;

        void update() override;

private:
        std::vector<SlotId> filtered_slots_;
};

class Equip: public InvState
{
public:
        Equip(InvSlot& slot) :
                InvState(),
                filtered_backpack_indexes_(),
                slot_to_equip_(slot) {}

        void on_start() override;

        void draw() override;

        void update() override;

private:
        std::vector<size_t> filtered_backpack_indexes_;

        InvSlot& slot_to_equip_;
};

class SelectThrow: public InvState
{
public:
        SelectThrow() :
                InvState(),
                filtered_slots_(),
                filtered_backpack_indexes_() {}

        void on_start() override;

        void draw() override;

        void update() override;

private:
        std::vector<SlotId> filtered_slots_;
        std::vector<size_t> filtered_backpack_indexes_;
};

class SelectIdentify: public InvState
{
public:
        SelectIdentify() :
                InvState(),
                filtered_slots_(),
                filtered_backpack_indexes_() {}

        void on_start() override;

        void draw() override;

        void update() override;

private:
        std::vector<SlotId> filtered_slots_;
        std::vector<size_t> filtered_backpack_indexes_;
};

#endif // INV_HANDLING_HPP
