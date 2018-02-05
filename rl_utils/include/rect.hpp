#ifndef RL_UTILS_RECT_HPP
#define RL_UTILS_RECT_HPP

class R
{
public:
    R() :
        p0(P()),
        p1(P()) {}

    R(const P& p0, const P& p1) :
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
        return ((p1 + p0) / 2);
    }

    bool is_p_inside(const P& p) const
    {
        return p >= p0 &&
               p <= p1;
    }

    R& operator+=(const P& p)
    {
        p0 += p;
        p1 += p;
        return *this;
    }

    R& operator-=(const P& p)
    {
        p0 -= p;
        p1 -= p;
        return *this;
    }

    // Useful for e.g. converting map cell dimensions to pixel dimensions
    R operator*(const P& p)
    {
        return R(p0 * p, p1 * p);
    }

    P p0;
    P p1;
};

#endif // RL_UTILS_RECT_HPP
