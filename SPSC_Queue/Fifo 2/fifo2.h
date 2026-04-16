/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt
 * to change this license Click
 * nbfs://nbhost/SystemFileSystem/Templates/cppFiles/file.h to edit this
 * template
 */

/*
 * File:   fifo2.h
 * Author: rishitsharma
 *
 * Created on 19 March, 2026, 11:00 PM
 */

#ifndef FIFO2_H
#define FIFO2_H
#include <atomic>
#include <exception>
#include <iostream>
#include <memory>
#include <thread>

template <typename T, typename Alloc = std::allocator<T>>
class Fifo2 : private Alloc {
  std::size_t capacity_;
  T *ring_;

  std::atomic<std::size_t> pushCursor_{0};
  std::atomic<std::size_t> popCursor_{0};

public:
  explicit Fifo2(std::size_t capacity, Alloc const &alloc = Alloc{})
      : Alloc{alloc}, capacity_{capacity},
        ring_{std::allocator_traits<Alloc>::allocate(*this, capacity)} {}

  ~Fifo2() {
    auto popCursor = popCursor_.load(std::memory_order_relaxed);
    auto pushCursor = pushCursor_.load(std::memory_order_relaxed);

    while (popCursor != pushCursor) {
      ring_[popCursor % capacity_].~T();
      ++popCursor;
    }

    std::allocator_traits<Alloc>::deallocate(*this, ring_, capacity_);
  }

  auto capacity() const { return capacity_; }

  bool push(const T &value) {
    auto pushCursor = pushCursor_.load(std::memory_order_relaxed);
    auto popCursor = popCursor_.load(std::memory_order_acquire);

    if (pushCursor - popCursor == capacity_)
      return false; // Queue is full
    new (&ring_[pushCursor % capacity_]) T(value);

    pushCursor_.store(pushCursor + 1, std::memory_order_release);

    return true;
  }

  bool pop(T *value) {
    auto popCursor = popCursor_.load(std::memory_order_relaxed);

    auto pushCursor = pushCursor_.load(std::memory_order_acquire);

    if (pushCursor == popCursor)
      return false; // Queue is empty

    *value = ring_[popCursor % capacity_];
    ring_[popCursor % capacity_].~T();
    popCursor_.store(popCursor + 1, std::memory_order_release);

    return true;
  }
};

#endif /* FIFO2_H */
