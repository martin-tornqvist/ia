#ifndef RL_UTILS_ARRAY2_HPP
#define RL_UTILS_ARRAY2_HPP

#include <functional>

#include "pos.hpp"

// Two dimensional array class
template<typename T>
class Array2
{
public:
        Array2() :
                data_(nullptr),
                dims_(0, 0) {}

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

        ~Array2()
        {
                delete[] data_;
        }

        Array2(const Array2<T>& other) :
                data_(nullptr),
                dims_()
        {
                resize(other.dims_);

                const size_t size = nr_elements();

                for (size_t idx = 0; idx < size; ++idx)
                {
                        data_[idx] = other.data_[idx];
                }
        }

        Array2<T>& operator=(const Array2<T>& other)
        {
                resize(other.dims_);

                const size_t size = nr_elements();

                for (size_t idx = 0; idx < size; ++idx)
                {
                        data_[idx] = other.data_[idx];
                }

                return *this;
        }

        void resize(const P& dims)
        {
                dims_ = dims;

                const size_t size = nr_elements();

                delete[] data_;

                data_ = new T[size];
        }

        void resize(const int w, const int h)
        {
                resize(P(w, h));
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

                                rotated(my_dims.y - 1 - y, x) = data_[my_idx];
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

                                rotated(y, my_dims.x - 1 - x) = data_[my_idx];
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

        T& at(const P& p) const
        {
                return get_element_ref(p);
        }

        T& at(const int x, const int y) const
        {
                return get_element_ref(P(x, y));
        }

        T& operator()(const P& p) const
        {
                return get_element_ref(p);
        }

        T& operator()(const int x, const int y) const
        {
                return get_element_ref(P(x, y));
        }

        void for_each(std::function<void(T& v)> func)
        {
                const size_t size = nr_elements();

                for (size_t idx = 0; idx < size; ++idx)
                {
                        func(data_[idx]);
                }
        }

        void clear()
        {
                delete[] data_;

                dims_.set(0, 0);
        }

        const P& dims() const
        {
                return dims_;
        }

private:
        T& get_element_ref(const P& p) const
        {
                check_pos(p);

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

        size_t nr_elements() const
        {
                return dims_.x * dims_.y;
        }

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

        T* data_;
        P dims_;
};

#endif // RL_UTILS_ARRAY2_HPP
