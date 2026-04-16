#include "fifo3.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>

int main() {
  std::cout << "Starting Fifo3 Performance Benchmark..." << std::endl;
  Fifo3<int> queue(131072);
  const uint64_t num_iterations = 100000000;
  auto start_time = std::chrono::high_resolution_clock::now();

  std::thread producer([&]() {
    for (uint64_t i = 0; i < num_iterations; ++i) {
      while (!queue.push(i)) {
      }
    }
  });

  std::thread consumer([&]() {
    for (uint64_t i = 0; i < num_iterations; ++i) {
      int value = 0;
      while (!queue.pop(&value)) {
      }
    }
  });

  producer.join();
  consumer.join();

  auto end_time = std::chrono::high_resolution_clock::now();

  std::chrono::duration<double> elapsed = end_time - start_time;

  double ops_per_second = num_iterations / elapsed.count();

  std::cout << "Successfully pushed and popped " << num_iterations << " items."
            << std::endl;
  std::cout << "Time elapsed:  " << std::fixed << std::setprecision(4)
            << elapsed.count() << " seconds." << std::endl;

  std::cout.imbue(std::locale(""));
  std::cout << "Fifo3: " << static_cast<uint64_t>(ops_per_second) << " ops/s"
            << std::endl;

  return 0;
}