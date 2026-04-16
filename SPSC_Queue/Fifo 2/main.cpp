// WITHOUT ATOMICS

#include "fifo2.h"
#include <exception>
#include <iostream>
#include <stdio.h>
#include <thread>
#include <vector>

int main() {
  std::cout << "Starting Fifo2 multithreaded SPSC test..." << std::endl;

  Fifo2<int> queue(1024);
  const int num_iterations = 1000000;
  std::thread producer([&]() {
    for (int i = 0; i < num_iterations; ++i) {
      while (!queue.push(i)) {
        std::this_thread::yield();
      }
    }
  });

  std::thread consumer([&]() {
    for (int i = 0; i < num_iterations; ++i) {
      int value = 0;
      while (!queue.pop(&value)) {
        std::this_thread::yield();
      }
      if (value != i) {
        std::cerr << "\nCRASH: Data race detected! Expected " << i
                  << " but got " << value << "\n";
        std::terminate();
      }
    }
  });

  producer.join();
  consumer.join();

  std::cout << "Test finished successfully! No data corruption detected."
            << std::endl;
  return 0;
}