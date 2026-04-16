// Testing FIFO 5 or FINAL QUEUE

// #include "fifo5.h"
// #include <iostream>
// #include <iomanip>
// #include <thread>
// #include <ctime>
// #include <cstdint>
//
// struct BigItem {
//     int v{0};
//     char pad[252];
// };
//
// double elapsed_seconds(struct timespec start, struct timespec end) {
//     return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
// }
//
// int main() {
//     std::cout << "Starting Fifo5 Zero-Copy Performance Benchmark..." <<
//     std::endl;
//
//     // Must be a power of 2 for the mask_ optimization
//     Fifo5<BigItem> queue(131072);
//     const uint64_t iterations = 1000000; // 1 Million 256-byte items
//
//     struct timespec start, end;
//     clock_gettime(CLOCK_MONOTONIC, &start);
//
//     std::thread producer([&]() {
//         for (uint64_t i = 0; i < iterations; ++i) {
//             while (true) {
//                 // Request a direct memory slot from the queue
//                 auto pusher = queue.push();
//
//                 if (pusher) { // If there is space...
//                     // Write data DIRECTLY into the queue's memory. Zero
//                     copies! pusher->v = static_cast<int>(i); break;
//                     // 'pusher' goes out of scope here. The destructor fires
//                     and atomicly publishes the cursor!
//                 }
//             }
//         }
//     });
//
//     std::thread consumer([&]() {
//         for (uint64_t i = 0; i < iterations; ++i) {
//             while (true) {
//                 // Request access to read the next item
//                 auto popper = queue.pop();
//
//                 if (popper) { // If data is available...
//                     // Read the data DIRECTLY from the queue's memory
//                     volatile int val = popper->v;
//                     (void)val;
//                     break;
//                     // 'popper' goes out of scope here, freeing the slot for
//                     the producer.
//                 }
//             }
//         }
//     });
//
//     producer.join();
//     consumer.join();
//     clock_gettime(CLOCK_MONOTONIC, &end);
//
//     double elapsed = elapsed_seconds(start, end);
//     double ops = iterations / elapsed;
//
//     std::cout << "\n[Fifo5] BigItem (256 bytes) Zero-Copy" << std::endl;
//     std::cout << "  Items:   " << iterations << std::endl;
//     std::cout << "  Time:    " << std::fixed << std::setprecision(4) <<
//     elapsed << " s" << std::endl; std::cout.imbue(std::locale("")); std::cout
//     << "  ops/s:   " << static_cast<uint64_t>(ops) << std::endl;
//
//     return 0;
// }

// Testing FIFO 5 with rdtsc

#include "fifo5.h"
#include <cassert>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <thread>

// ─── rdtsc ──────────────────────────────────────────────────────────────────

inline uint64_t rdtsc() {
  uint32_t lo, hi;
  __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
  return (static_cast<uint64_t>(hi) << 32) | lo;
}

inline uint64_t rdtscp() {
  uint32_t lo, hi, aux;
  __asm__ __volatile__("rdtscp" : "=a"(lo), "=d"(hi), "=c"(aux));
  return (static_cast<uint64_t>(hi) << 32) | lo;
}

double tsc_frequency() {
  struct timespec t1, t2;
  clock_gettime(CLOCK_MONOTONIC, &t1);
  uint64_t tsc1 = rdtsc();

  struct timespec target = t1;
  target.tv_nsec += 100000000;
  if (target.tv_nsec >= 1000000000) {
    target.tv_sec++;
    target.tv_nsec -= 1000000000;
  }
  do {
    clock_gettime(CLOCK_MONOTONIC, &t2);
  } while (t2.tv_sec < target.tv_sec ||
           (t2.tv_sec == target.tv_sec && t2.tv_nsec < target.tv_nsec));

  uint64_t tsc2 = rdtscp();
  double elapsed = (t2.tv_sec - t1.tv_sec) + (t2.tv_nsec - t1.tv_nsec) / 1e9;
  return (tsc2 - tsc1) / elapsed;
}

// ─── helpers ────────────────────────────────────────────────────────────────

void print_result(const char *label, uint64_t iterations, uint64_t ticks,
                  double freq) {
  double elapsed = ticks / freq;
  double ops = iterations / elapsed;
  double ns_per_op = (ticks / static_cast<double>(iterations)) / (freq / 1e9);

  std::cout << label << "\n";
  std::cout << "  Items:      " << iterations << "\n";
  std::cout << "  Ticks:      " << ticks << "\n";
  std::cout << "  Time:       " << std::fixed << std::setprecision(4) << elapsed
            << " s\n";
  std::cout << "  Latency:    " << std::fixed << std::setprecision(2)
            << ns_per_op << " ns/op\n";
  std::cout.imbue(std::locale(""));
  std::cout << "  Throughput: " << static_cast<uint64_t>(ops) << " ops/s\n\n";
}

// ─── Test 1: int ─────────────────────────────────────────────────────────────

void test_int(uint64_t iterations, double freq) {
  Fifo5<int> queue(131072);

  uint64_t t1 = rdtsc();

  std::thread producer([&]() {
    for (uint64_t i = 0; i < iterations; ++i) {
      auto pusher = queue.push();
      while (!pusher) {
        pusher = queue.push();
      }
      new (pusher.get()) int(static_cast<int>(i));
    }
  });

  std::thread consumer([&]() {
    using TT = int;
    for (uint64_t i = 0; i < iterations; ++i) {
      auto popper = queue.pop();
      while (!popper) {
        popper = queue.pop();
      }
      popper.get()->~TT();
    }
  });

  producer.join();
  consumer.join();

  uint64_t t2 = rdtscp();
  print_result("[Test 1] int (4 bytes)", iterations, t2 - t1, freq);
}

// ─── Test 2: 256-byte struct ─────────────────────────────────────────────────

void test_large_struct(uint64_t iterations, double freq) {
  struct BigItem {
    int v{0};
    char pad[252];
  };

  Fifo5<BigItem> queue(131072);

  uint64_t t1 = rdtsc();

  std::thread producer([&]() {
    for (uint64_t i = 0; i < iterations; ++i) {
      auto pusher = queue.push();
      while (!pusher) {
        pusher = queue.push();
      }
      new (pusher.get()) BigItem();
      pusher->v = static_cast<int>(i);
    }
  });

  std::thread consumer([&]() {
    for (uint64_t i = 0; i < iterations; ++i) {
      auto popper = queue.pop();
      while (!popper) {
        popper = queue.pop();
      }
      popper.get()->~BigItem();
    }
  });

  producer.join();
  consumer.join();

  uint64_t t2 = rdtscp();
  print_result("[Test 2] BigItem (256 bytes)", iterations, t2 - t1, freq);
}

// ─── Test 3: correctness ─────────────────────────────────────────────────────

void test_correctness() {
  std::cout << "[Test 3] Correctness check...\n";
  Fifo5<int> queue(1024);
  const int N = 100000;
  bool passed = true;

  std::thread producer([&]() {
    for (int i = 0; i < N; ++i) {
      auto pusher = queue.push();
      while (!pusher) {
        pusher = queue.push();
      }
      new (pusher.get()) int(i);
    }
  });

  std::thread consumer([&]() {
    using TT = int;
    for (int i = 0; i < N; ++i) {
      auto popper = queue.pop();
      while (!popper) {
        popper = queue.pop();
      }
      if (*popper.get() != i)
        passed = false;
      popper.get()->~TT();
    }
  });

  producer.join();
  consumer.join();
  std::cout << "  Result: " << (passed ? "PASS ✓" : "FAIL ✗") << "\n\n";
}

// ─── Test 4: full queue
// ───────────────────────────────────────────────────────

void test_full_queue() {
  std::cout << "[Test 4] Full queue returns false...\n";
  Fifo5<int> queue(4);

  // Each pusher_t must be destroyed (go out of scope) before the next push,
  // because the destructor is what advances pushState_.cursor.
  // Without scoping, cursor stays at 0 and push() never sees a full queue.
  {
    auto p = queue.push();
    assert(p);
    new (p.get()) int(1);
  }
  {
    auto p = queue.push();
    assert(p);
    new (p.get()) int(2);
  }
  {
    auto p = queue.push();
    assert(p);
    new (p.get()) int(3);
  }
  {
    auto p = queue.push();
    assert(p);
    new (p.get()) int(4);
  }

  auto p5 = queue.push();
  assert(!p5 && "should be full");

  std::cout << "  Result: PASS ✓\n\n";
}

// ─── Test 5: empty queue
// ──────────────────────────────────────────────────────

void test_empty_queue() {
  std::cout << "[Test 5] Empty queue returns false...\n";
  Fifo5<int> queue(4);

  auto p = queue.pop();
  assert(!p && "should be empty");

  std::cout << "  Result: PASS ✓\n\n";
}

// ─── main ────────────────────────────────────────────────────────────────────

int main() {
  std::cout << "========================================\n";
  std::cout << "        Fifo5 Test Suite (rdtsc)\n";
  std::cout << "========================================\n\n";

  std::cout << "Calibrating TSC frequency...\n";
  double freq = tsc_frequency();
  std::cout << "  TSC frequency: " << std::fixed << std::setprecision(2)
            << freq / 1e9 << " GHz\n\n";

  test_correctness();
  test_full_queue();
  test_empty_queue();

  test_int(100000000, freq);
  test_large_struct(1000000, freq);

  std::cout << "========================================\n";
  std::cout << "All tests complete.\n";
  std::cout << "========================================\n";

  return 0;
}