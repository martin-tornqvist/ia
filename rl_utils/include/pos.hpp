#ifndef RL_UTILS_POS_HPP
#define RL_UTILS_POS_HPP

#include "direction.hpp"

class P
{
public:
        P() :
                x(0),
                y(0) {}

        P(const int x, const int y) :
                x(x),
                y(y) {}

        P(const P& p) :
                x(p.x),
                y(p.y) {}

        // Construct from a direction -> offsets (e.g. 1, -1)
        explicit P(const Dir dir);

        P& operator=(const P p)
        {
                x = p.x;
                y = p.y;

                return *this;
        }

        // Assign from a direction -> offsets (e.g. 1, -1)
        P& operator=(const Dir dir);

        P& operator+=(const P p)
        {
                x += p.x;
                y += p.y;

                return *this;
        }

        // Add a direction offset (e.g. 1, -1)
        P& operator+=(const Dir dir);

        P& operator-=(const P p)
        {
                x -= p.x;
                y -= p.y;

                return *this;
        }

        P& operator++()
        {
                ++x;
                ++y;

                return *this;
        }

        P& operator--()
        {
                --x;
                --y;

                return *this;
        }

        P operator+(const P p) const
        {
                return P(x + p.x, y + p.y);
        }

        P operator+(const int v) const
        {
                return P(x + v, y + v);
        }

        P operator+(const Dir dir) const;

        P operator-(const P p) const
        {
                return P(x - p.x, y - p.y);
        }

        P operator-(const int v) const
        {
                return P(x - v, y - v);
        }

        P with_offsets(const int x_offset, const int y_offset) const
        {
                return P(x + x_offset, y + y_offset);
        }

        P with_x_offset(const int offset) const
        {
                return P(x + offset, y);
        }

        P with_y_offset(const int offset) const
        {
                return P(x, y + offset);
        }

        P scaled_up(const P p) const
        {
                return P(x * p.x, y * p.y);
        }

        P scaled_up(const int x_factor, const int y_factor) const
        {
                return P(x * x_factor, y * y_factor);
        }

        P scaled_up(const int v) const
        {
                return P(x * v, y * v);
        }

        P scaled_down(const int x_denom, const int y_denom) const
        {
                return P(x / x_denom, y / y_denom);
        }

        P scaled_down(const int v) const
        {
                return P(x / v, y / v);
        }

        P scaled_down(const P denoms) const
        {
                return P(x / denoms.x, y / denoms.y);
        }

        bool operator==(const P p) const
        {
                return
                        (x == p.x) &&
                        (y == p.y);
        }

        bool operator!=(const P p) const
        {
                return
                        (x != p.x) ||
                        (y != p.y);
        }

        bool operator!=(const int v) const
        {
                return
                        (x != v) ||
                        (y != v);
        }

        P signs() const
        {
                return P((x == 0) ? 0 : (x > 0) ? 1 : -1,
                         (y == 0) ? 0 : (y > 0) ? 1 : -1);
        }

        void set(const int new_x, const int new_y)
        {
                x = new_x;
                y = new_y;
        }

        void set(const P p)
        {
                x = p.x;
                y = p.y;
        }

        void swap(P& p)
        {
                P tmp(p);

                p = *this;

                set(tmp);
        }

        bool is_adjacent(const P p)
        {
                // Do not count the same position as adjacent
                if (p == *this)
                {
                        return false;
                }

                const auto d = *this - p;

                const bool x_adj = (d.x >= -1) && (d.x <= 1);
                const bool y_adj = (d.y >= -1) && (d.y <= 1);

                return x_adj && y_adj;
        }

        // NOTE: This assumes that both x and y is -1, 0, or 1
        Dir to_dir();

        int x, y;
};

struct PosVal
{
        PosVal() :
                pos(P()),
                val(-1) {}

        PosVal(const P pos_, const int val_) :
                pos(pos_),
                val(val_) {}

        PosVal(const PosVal& o) :
                pos(o.pos),
                val(o.val) {}

        P pos;
        int val;
};

#endif // RL_UTILS_POS_HPP
