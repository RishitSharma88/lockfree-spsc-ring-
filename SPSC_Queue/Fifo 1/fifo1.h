/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/file.h to edit this template
 */

/* 
 * File:   fifo1.h
 * Author: rishitsharma
 *
 * Created on 19 March, 2026, 10:59 PM
 */

#ifndef FIFO1_H
#define FIFO1_H
#include <iostream>
#include <memory>
template <typename T, typename Alloc = std::allocator<T>>
class Fifo1 : private Alloc
{
    std::size_t capacity_;
    T* ring_;
    std::size_t pushCursor_{};
    std::size_t popCursor_{};

public:

    Fifo1(std::size_t capacity, Alloc const& alloc = Alloc{})
        : Alloc{alloc},
          capacity_{capacity},
          ring_{std::allocator_traits<Alloc>::allocate(*this, capacity)}
    {}

    ~Fifo1()
    {
        while(!empty())
        {
            ring_[popCursor_ % capacity_].~T();
            ++popCursor_;
        }

        std::allocator_traits<Alloc>::deallocate(*this, ring_, capacity_);
    }

    auto capacity() const { return capacity_; }

    auto size() const { return pushCursor_ - popCursor_; }

    auto empty() const { return size() == 0; }

    auto full() const { return size() == capacity_; }

    bool push(const T& value);

    bool pop(T* value);
};

template<typename T, typename Alloc>
bool Fifo1<T, Alloc>::push(const T& value)
{
    if(full())
        return false;

    new(&ring_[pushCursor_ % capacity_]) T(value);

    ++pushCursor_;

    return true;
}

template<typename T, typename Alloc>
bool Fifo1<T, Alloc>::pop(T* value)
{
    if(empty())
        return false;

    *value = ring_[popCursor_ % capacity_];

    ring_[popCursor_ % capacity_].~T();

    ++popCursor_;

    return true;
}


#endif /* FIFO1_H */

