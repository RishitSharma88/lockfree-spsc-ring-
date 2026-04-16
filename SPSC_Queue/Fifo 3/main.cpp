// FIFO3 TEST

#include "fifo3.h"
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <thread>

int main() {
  std::cout << "Starting Fifo3 Performance Benchmark..." << std::endl;

  struct Rishit {
    int v{0};
    char pad[252];
  };

  Fifo3<Rishit> queue(131072);
  const uint64_t num_iterations = 1000000;

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  std::thread producer([&]() {
    Rishit r;
    for (uint64_t i = 0; i < num_iterations; ++i) {
      r.v = static_cast<int>(i);
      while (!queue.push(r)) {
      }
    }
  });

  std::thread consumer([&]() {
    Rishit rc;
    for (uint64_t i = 0; i < num_iterations; ++i) {
      while (!queue.pop(&rc)) {
      }
    }
  });

  producer.join();
  consumer.join();

  clock_gettime(CLOCK_MONOTONIC, &end);

  double elapsed =
      (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

  double ops_per_second = num_iterations / elapsed;

  std::cout << "Successfully pushed and popped " << num_iterations
            << " items.\n";
  std::cout << "Time elapsed: " << std::fixed << std::setprecision(4) << elapsed
            << " seconds.\n";

  std::cout.imbue(std::locale(""));
  std::cout << "Fifo3: " << static_cast<uint64_t>(ops_per_second) << " ops/s\n";

  return 0;
}