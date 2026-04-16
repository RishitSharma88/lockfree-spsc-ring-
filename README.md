# SPSC Lock-Free Ring Buffer — From the Ground Up

This project is my implementation of a Single Producer Single Consumer (SPSC) lock-free ring buffer in C++. The whole idea behind this was to understand how lock-free data structures work at a low level — from the very basics of why atomics are needed, all the way to building a high-performance zero-copy queue.

This project is heavily inspired by the CppCon 2023 talk — [*"Single Producer Single Consumer Lock-free FIFO From the Ground Up" by Charles Frasch*](https://www.youtube.com/watch?v=K3P_Lmq6pw0) and his [repository](https://github.com/CharlesFrasch/cppcon2023). The talk does an amazing job of building the queue step by step, optimizing one bottleneck at a time, and I followed the same approach here while writing everything from scratch to make sure I actually understood what was going on under the hood.

## What I Built

The project is structured into 5 versions of the queue (Fifo 1 through Fifo 5), each one building on top of the previous one and introducing a new optimization. The idea was to not just write a fast queue, but to understand *why* each optimization matters and what bottleneck it solves.

- **Fifo 1** — A basic ring buffer with no synchronization. This one intentionally crashes with data races. The point was to see what happens when you don't use atomics at all.
- **Fifo 2** — Introduces the `<atomic>` library with acquire/release memory ordering. This gives us our first working lock-free SPSC queue.
- **Fifo 3** — Adds cache-line padding using `alignas` to eliminate false sharing. This was a massive performance jump.
- **Fifo 4** — Introduces cached cursors to batch atomic loads. Instead of hitting the other thread's atomic variable on every push/pop, we cache it locally and only reload when needed.
- **Fifo 5** — Implements a zero-copy proxy architecture using `pusher_t` and `popper_t` RAII proxy objects. This avoids unnecessary copies entirely and uses `rdtsc` for benchmarking.

## What I Learnt

This project taught me a lot about low-level concurrency and performance optimization in C++. Some of the key things I picked up along the way:

- Why normal operations like `++` and `--` are not atomic and why that matters in multithreaded code.
- How memory ordering works — `relaxed`, `acquire`, `release` — and when to use each one.
- What false sharing is and how cache-line alignment can drastically reduce overhead.
- How cursor caching reduces the frequency of expensive atomic loads across threads.
- How RAII proxy objects can be used to implement zero-copy semantics in a queue.
- How to use thread sanitizers (`-fsanitize=thread`) to detect data races.
- How to benchmark properly using `rdtsc` and why `std::chrono` isn't always the best choice for microbenchmarks.

## Benchmarks

All benchmarks were run with `-O3` optimization. The queue capacity was set to a power of 2. Two payload sizes were tested — `int` (4 bytes) and a `BigItem` struct (256 bytes) — to see how each optimization handles both small and large payloads.

| Version | Key Optimization | Payload | Throughput (ops/s) | Latency (ns/op) |
|---------|------------------|---------|--------------------|------------------|
| Fifo 2 | Atomics + Acquire/Release Memory Ordering | Int (4B) | ~12,800,000 | ~78.00 ns |
| Fifo 3 | Cache-Line Padding (`alignas`) | Int (4B) | ~599,880,000 | ~1.66 ns |
| Fifo 3 | Baseline for large payloads | BigItem (256B) | ~40,180,000 | ~24.88 ns |
| Fifo 4 | Cached Cursors (Batching atomic loads) | Int (4B) | ~724,010,000 | ~1.38 ns |
| Fifo 4 | Memcpy bottleneck reached | BigItem (256B) | ~44,085,000 | ~22.68 ns |
| Fifo 5 | Zero-Copy Proxy Architecture (`rdtsc`) | Int (4B) | ~935,187,000 | 1.07 ns |
| Fifo 5 | Zero-Copy Proxy Architecture (`rdtsc`) | BigItem (256B) | ~61,863,000 | 16.16 ns |

The jump from Fifo 2 to Fifo 3 is the most dramatic — eliminating false sharing alone took us from ~78 ns/op down to ~1.66 ns/op for `int`. From there, each version squeezes out more performance, with Fifo 5 reaching sub-nanosecond latency for small payloads.

## Project Structure

```
SPSC_Queue/
├── Fifo 1/          # No atomics — demonstrates data races
├── Fifo 2/          # Atomics + memory ordering
├── Fifo 3/          # Cache-line padding
├── Fifo 4/          # Cached cursors
├── Fifo 5/          # Zero-copy proxy architecture
└── README.md        # You are here
```

Each folder contains its own `README.md` with a detailed walkthrough of that version, a `fifo[N].h` header file with the queue implementation, and a `main.cpp` file to run and benchmark it.

## How to Build and Run

Each version can be compiled and run independently. For example, to run Fifo 5:

```bash
g++ -O3 -std=c++17 main.cpp -o fifo5 -lpthread
./fifo5
```

To run with thread sanitizer (useful for Fifo 1 to see the data race):

```bash
g++ -fsanitize=thread -g -O3 main.cpp -o fifo_tsan -lpthread
./fifo_tsan
```
