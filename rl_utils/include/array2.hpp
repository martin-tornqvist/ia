#ifndef RL_UTILS_ARRAY2_HPP
#define RL_UTILS_ARRAY2_HPP

#include <functional>

#include "pos.hpp"
#include "rect.hpp"

// Two dimensional dynamic array class
template<typename T>
class Array2
{
public:
        Array2(const P& dims) :
                data_(nullptr),
                dims_()
        {
                resize(dims);
        }

        Array2(const int w, const int h) :
                data_(nullptr),
                dims_()
        {
                resize(P(w, h));
        }

        Array2(const Array2<T>& other) :
                data_(nullptr),
                dims_()
        {
                resize_no_init(other.dims_);

                std::copy(
                        std::begin(other),
                        std::end(other),
                        std::begin(*this));
        }

        Array2(Array2<T>&& other) :
                data_(other.data_),
                dims_(other.dims_)
        {
                other.data_ = nullptr;
                other.dims_ = {0, 0};
        }

        ~Array2()
        {
                delete[] data_;
        }

        Array2<T>& operator=(const Array2<T>& other)
        {
                resize_no_init(other.dims_);

                std::copy(
                        std::begin(other),
                        std::end(other),
                        std::begin(*this));

                return *this;
        }

        Array2<T>& operator=(Array2<T>&& other)
        {
                if (&other == this)
                {
                        return *this;
                }

                delete[] data_;

                data_ = other.data_;
                dims_ = other.dims_;

                other.data_ = nullptr;
                other.dims_ = {0, 0};

                return *this;
        }

        T& at(const P& p) const
        {
                return get_element_ref(p);
        }

        T& at(const int x, const int y) const
        {
                return get_element_ref(P(x, y));
        }

        T& at(const size_t idx) const
        {
                return data_[idx];
        }

        T* begin() const
        {
                return data_;
        }

        T* end() const
        {
                return data_ + length();
        }

        void resize(const P& dims)
        {
                dims_ = dims;

                const size_t len = length();

                delete[] data_;

                data_ = nullptr;

                if (len > 0)
                {
                        data_ = new T[len]();
                }
        }

        void resize(const int w, const int h)
        {
                resize(P(w, h));
        }

        void resize(const P& dims, const T value)
        {
                resize_no_init(dims);

                std::fill_n(data_, length(), value);
        }

        void resize(const int w, const int h, const T value)
        {
                resize(P(w, h), value);
        }

        void rotate_cw()
        {
                const P my_dims(dims());

                Array2<T> rotated(my_dims.y, my_dims.x);

                for (int x = 0; x < my_dims.x; ++x)
                {
                        for (int y = 0; y < my_dims.y; ++y)
                        {
                                const size_t my_idx = pos_to_idx(x, y);

                                rotated.at(my_dims.y - 1 - y, x) =
                                        data_[my_idx];
                        }
                }

                *this = rotated;
        }

        void rotate_ccw()
        {
                const P my_dims(dims());

                Array2<T> rotated(my_dims.y, my_dims.x);

                for (int x = 0; x < my_dims.x; ++x)
                {
                        for (int y = 0; y < my_dims.y; ++y)
                        {
                                const size_t my_idx = pos_to_idx(x, y);

                                rotated.at(y, my_dims.x - 1 - x) =
                                        data_[my_idx];
                        }
                }

                *this = rotated;
        }

        void flip_hor()
        {
                const P d(dims());

                for (int x = 0; x < d.x / 2; ++x)
                {
                        for (int y = 0; y < d.y; ++y)
                        {
                                const size_t idx_1 = pos_to_idx(x, y);
                                const size_t idx_2 = pos_to_idx(d.x - 1 - x, y);

                                std::swap(data_[idx_1], data_[idx_2]);
                        }
                }
        }

        void flip_ver()
        {
                const P d(dims());

                for (int x = 0; x < d.x; ++x)
                {
                        for (int y = 0; y < d.y / 2; ++y)
                        {
                                const size_t idx_1 = pos_to_idx(x, y);
                                const size_t idx_2 = pos_to_idx(x, d.y - 1 - y);

                                std::swap(data_[idx_1], data_[idx_2]);
                        }
                }
        }

        void clear()
        {
                delete[] data_;

                dims_.set(0, 0);
        }

        size_t length() const
        {
                return dims_.x * dims_.y;
        }

        const P& dims() const
        {
                return dims_;
        }

        int w() const
        {
                return dims_.x;
        }

        int h() const
        {
                return dims_.y;
        }

        const R rect() const
        {
                return R({0, 0}, dims_ - 1);
        }

        T* data()
        {
                return data_;
        }

        const T* data() const
        {
                return data_;
        }

private:
        void resize_no_init(const P& dims)
        {
                dims_ = dims;

                const size_t len = length();

                delete[] data_;

                data_ = nullptr;

                if (len > 0)
                {
                        data_ = new T[len];
                }
        }

        T& get_element_ref(const P& p) const
        {
#ifndef NDEBUG
                check_pos(p);
#endif // NDEBUG

                const size_t idx = pos_to_idx(p);

                return data_[idx];
        }

        size_t pos_to_idx(const P& p) const
        {
                return (p.x * dims_.y) + p.y;
        }

        size_t pos_to_idx(const int x, const int y) const
        {
                return pos_to_idx(P(x, y));
        }

#ifndef NDEBUG
        void check_pos(const P& p) const
        {
                if ((p.x < 0) ||
                    (p.y < 0) ||
                    (p.x >= dims_.x) ||
                    (p.y >= dims_.y))
                {
                        ASSERT(false);
                }
        }
#endif // NDEBUG

        T* data_;
        P dims_;
};

#endif // RL_UTILS_ARRAY2_HPP
