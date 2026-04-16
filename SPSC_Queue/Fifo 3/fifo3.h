/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/file.h to edit this template
 */

/* 
 * File:   fifo3.h
 * Author: rishitsharma
 *
 * Created on 14 April, 2026, 12:58 AM
 */

#ifndef FIFO3_H
#define FIFO3_H
#include<iostream>
#include<atomic>
#include<memory>
#include<thread>
#include<exception>
#include<new>

template <typename T, typename Alloc = std::allocator<T>>

class Fifo3 : private Alloc
{
    std::size_t capacity_;
    T* ring_;
    
    struct alignas(std::hardware_destructive_interference_size) Cursor{
        std::atomic<size_t> value{0};
    };
    Cursor pushCursor_;
    Cursor popCursor_;
    public:
        
        explicit Fifo3(std::size_t capacity, Alloc const& alloc = Alloc{})
        : Alloc{alloc},
          capacity_{capacity},
          ring_{std::allocator_traits<Alloc>::allocate(*this, capacity)}
          {}
         ~Fifo3()
         {
            auto popCursor = popCursor_.value.load(std::memory_order_relaxed);
            auto pushCursor = pushCursor_.value.load(std::memory_order_relaxed);
            while (popCursor != pushCursor)
            {
                ring_[popCursor % capacity_].~T();
                ++popCursor;
            }

            std::allocator_traits<Alloc>::deallocate(*this, ring_, capacity_);
         }
         auto capacity() const { return capacity_; }
         bool push(const T& value)
         {
             auto pushCursor = pushCursor_.value.load(std::memory_order_relaxed);
             auto popCursor = popCursor_.value.load(std::memory_order_acquire);
             if(pushCursor-popCursor == capacity_)
             {
                 return false;
             }
             new(&ring_[pushCursor%capacity_]) T(value);
             pushCursor_.value.store(pushCursor+1,std::memory_order_release);
             return true;
         }
         bool pop(T* value)
         {
             auto popCursor = popCursor_.value.load(std::memory_order_relaxed);
             auto pushCursor = pushCursor_.value.load(std::memory_order_acquire);
             if(pushCursor==popCursor)
             {
                 return false;
             }
             *value = std::move(ring_[popCursor%capacity_]);
             ring_[popCursor%capacity_].~T();
             popCursor_.value.store(popCursor+1,std::memory_order_release);
             return true;
         }
};
#endif /* FIFO3_H */

