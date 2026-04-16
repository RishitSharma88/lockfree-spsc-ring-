# Fifo 2

## Introduction

Now that we understand how Fifo 1 works, we can move on to Fifo 2.

Fifo 2 is a more optimized version of Fifo 1 as Fifo 1 crashed due to data races.

So, introduced in C++11, the atomics library allows us to perform atomic operations on variables. Atomics ensures that the operation is performed in a single step and cannot be interrupted by another thread. There are multiple functions as a part of the atomics library that will be used in this version of the queue. The ones we use are:

- `Atomics.load()` - This function is used to load the value of an atomic variable.
- `Atomics.store()` - This function is used to store the value of an atomic variable.

And some principles of memory ordering like:

- `std::memory_order_relaxed` : The weakest memory ordering constraint for atomic operations. It guarantees that an operation is atomic (cannot be partially completed), but it imposes no synchronization or ordering constraints relative to other memory accesses.
- `std::memory_order_acquire` : A memory ordering constraint in C++ used for synchronizing data between threads. It is primarily applied to atomic load operations to ensure that all memory writes that occurred before the write in the other thread are visible after the load.
- `std::memory_order_release` : A memory ordering constraint in C++ used for synchronizing data between threads. It is primarily applied to atomic store operations to ensure that all memory writes that occurred before the write in the other thread are visible after the store.

## Prerequisites

So before implementing them into the queue, there should be some basic understanding of memory ordering and the atomics library and why normal operations like `++`, `--`, etc. are not atomic. I referred to the CppCon lecture — *CppCon 2017: Fedor Pikus "C++ atomics, from basic to advanced. What do they really do?"*

This lecture provided a very strong base for understanding atomics and memory ordering.

## Implementation

Assuming we have a basic understanding of the atomics library and memory ordering, we can now implement them into the queue. The structure of the queue remains the same as Fifo 1, but we introduce atomics to the `pushCursor_` and `popCursor_` variables.

### `push()` Function

Inside the `push()` function:

- We use `std::memory_order_relaxed` to load the value of `pushCursor_` as we only care about the atomicity of the operation and not the synchronization with other threads.
- We use `std::memory_order_acquire` to load the value of `popCursor_` as we need to ensure that all memory writes that occurred before the write in the other thread are visible after the load.
- We use `std::memory_order_release` to store the value of `pushCursor_` as we need to ensure that all memory writes that occurred before the write in the other thread are visible after the store.

### `pop()` Function

Inside the `pop()` function:

- We use `std::memory_order_relaxed` to load the value of `popCursor_` as we only care about the atomicity of the operation and not the synchronization with other threads.
- We use `std::memory_order_acquire` to load the value of `pushCursor_` as we need to ensure that all memory writes that occurred before the write in the other thread are visible after the load.
- We use `std::memory_order_release` to store the value of `popCursor_` as we need to ensure that all memory writes that occurred before the write in the other thread are visible after the store.

## False Sharing — The New Bottleneck

But now there is a new problem. The new bottleneck is called False Sharing. False Sharing occurs when two threads access different variables that happen to be located on the same cache line. A cache line is a block of memory that is loaded into the cache of a CPU together. The CPU loads an entire chunk of memory, let's say 64 bytes, into its cache. So if two threads access different variables that happen to be located on the same cache line (within 64 bytes as per our example), they will both load the same cache line into their caches. This will cause a lot of overhead as the cache line will be constantly invalidated and reloaded.

## Next

There are multiple techniques to overcome false sharing. One of them is used in Fifo 3. Though the performance of Fifo 2 is fair, it can be further optimized by overcoming false sharing. But for now, we have our first working lock-free SPSC ring buffer.