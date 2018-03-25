#ifndef RL_UTILS_RECT_HPP
#define RL_UTILS_RECT_HPP

class R
{
public:
        R() :
                p0(P()),
                p1(P()) {}

        R(const P p0, const P p1) :
                p0(p0),
                p1(p1) {}

        R(const int x0, const int y0, const int x1, const int y1) :
                p0(P(x0, y0)),
                p1(P(x1, y1)) {}

        R(const R& r) :
                p0(r.p0),
                p1(r.p1) {}

        int w() const
        {
                return p1.x - p0.x + 1;
        }

        int h() const
        {
                return p1.y - p0.y + 1;
        }

        int area() const
        {
                return w() * h();
        }

        P dims() const
        {
                return P(w(), h());
        }

        int min_dim() const
        {
                return std::min(w(), h());
        }

        int max_dim() const
        {
                return std::max(w(), h());
        }

        P center() const
        {
                return P((p0.x + p1.x) / 2,
                         (p0.y + p1.y) / 2);
        }

        bool is_pos_inside(const P p) const
        {
                return
                        (p.x >= p0.x) &&
                        (p.y >= p0.y) &&
                        (p.x <= p1.x) &&
                        (p.y <= p1.y);
        }

        R with_offset(const P offset) const
        {
                return R(
                        p0 + P(offset.x, offset.y),
                        p1 + P(offset.x, offset.y));
        }

        R with_offset(const int x_offset, const int y_offset) const
        {
                return R(
                        p0 + P(x_offset, y_offset),
                        p1 + P(x_offset, y_offset));
        }

        R scaled_up(const int x_factor, const int y_factor) const
        {
                return R(
                        p0.scaled_up(x_factor, y_factor),
                        p1.scaled_up(x_factor, y_factor));
        }

        P p0;
        P p1;
};

#endif // RL_UTILS_RECT_HPP
