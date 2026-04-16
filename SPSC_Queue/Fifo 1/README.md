# Fifo 1

## Introduction

So now, initially when we begin working with SPSC queues, we need to understand fundamentally 2 things:

1. What is the need of atomics.
2. How do we implement a basic SPSC queue structure without the use of atomics.

So to understand 1, we can take the example of a simple queue where we have a producer and a consumer and both are accessing the same queue. Producer and consumer in our case would be 2 threads, one trying to access the queue and push values into it and the other trying to access the queue and pop values from it. Now let's implement this simple architecture first without the use of atomics and see what happens.

The code for the same is in the file `fifo1.h`. The header file implements the basic structural API which allows the producer-consumer in the `main.cpp` file to push and pop values from the queue.

## Architecture

Firstly, let's understand the architecture of our queue which is written in `fifo1.h`.

The queue uses a Template to store values of any type. This allows us to push multiple types into the queue like `int`, `float`, `char`, etc. and even custom structures into it.

### Class Members

Stepping into the class, we find 4 members defined:

1. `std::size_t capacity` : Stores the capacity of the queue.
2. `T* ring_` : A pointer of type `T`, which is defined in the template and `T` can hold any type of data. This points to the actual memory where the values are stored.
3. `std::size_t pushCursor_` : A counter to maintain the count of number of values pushed into the queue by the producer.
4. `std::size_t popCursor_` : A counter to maintain the count of number of values popped from the queue by the consumer.

### Constructor & Destructor

Now, once we have declared some crucial variables, we will define the constructor with a custom allocator option in the arguments. The user can use their custom allocators or we will assign them a default allocator defined in the constructor. We use `std::allocator<T>` as the default allocator. The constructor allocates memory for the queue using the allocator and assigns it to the `ring_` pointer.

The destructor, on the other hand, runs for all the allocated blocks of memory provided to you for the queue and deallocates them.

### APIs

The next part of the architecture is APIs which will be used by the producer-consumer. The APIs have a straightforward name — `push()` and `pop()`. These are used to push structures or objects into the queue and pop structures or objects from the queue respectively.

The `ring_` uses the modulus operator to wrap around the indices of the queue. It is less efficient than the bitwise AND operator but it is more efficient than using if-else statements to check if the index is greater than the capacity of the queue. But we will use optimization techniques in the later versions of the queue.

## Running

Now to run this queue, we will use the `main.cpp` file.

We will run the file with this command:

```bash
g++ -fsanitize=thread -g -O3 main.cpp -o fifo_tsan -lpthread
```

We use `-fsanitize=thread` to enable the thread sanitizer, which will help us detect data races.

## Expected Output

As the output, we expect a data race as there is no synchronization between the producer and consumer. So if a producer pushes a value into the queue and the consumer tries to pop it at the same time, it will lead to a data race, which means multiple threads reaching for the same resource at the same time.

## Next

Next version of the queue which is Fifo 2 introduces the atomics library for concurrency.