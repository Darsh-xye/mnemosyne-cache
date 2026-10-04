# Mnemosyne-Cache

Mnemosyne-Cache is a small Redis-compatible in-memory key-value cache written in C++17.

The main goal of the project is simple:

> Take the Mnemosyne custom allocator out of an isolated benchmark and test it inside a real networked application.

The server uses Linux non-blocking sockets and `epoll`, supports a small RESP subset, and works with `redis-cli` and `redis-benchmark`.

## Architecture

```text
redis-cli / redis-benchmark
          │
          ▼
     TCP + RESP
          │
          ▼
   epoll TCP server
          │
          ▼
      RESP parser
          │
          ▼
 PING / SET / GET / DEL
          │
          ▼
 std::unordered_map
          │
          ▼
      CacheValue
     char* + size
          │
          ▼
   allocator layer
      /       \
Mnemosyne    malloc
```

## Features

- C++17
- Linux sockets
- non-blocking I/O with `epoll`
- per-client request buffering
- minimal RESP support
- `PING`, `SET`, `GET`, `DEL`
- `redis-cli` compatible
- `redis-benchmark` compatible
- switchable Mnemosyne / glibc `malloc` backend
- cache churn benchmarks
- Docker support

## Why Mnemosyne-Cache?

Caches naturally create repeated allocation and deallocation:

```text
SET
 ↓
allocate

DEL
 ↓
free

SET
 ↓
reuse
```

This makes a cache a good workload for understanding how Mnemosyne behaves outside a standalone allocator benchmark.

The server stores cache values explicitly:

```cpp
struct CacheValue {
    char* data;
    std::size_t size;
};
```

Value memory is obtained through:

```text
cache_alloc()
cache_free()
```

so the rest of the server stays identical whether the backend is Mnemosyne or `malloc`.

## Building

### malloc version

```bash
cmake -S . -B build-malloc \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_MNEMOSYNE=OFF

cmake --build build-malloc -j$(nproc)
```

### Mnemosyne version

Keep both repositories next to each other:

```text
projects/
├── mnemosyne/
└── mnemosyne-cache/
```

Then:

```bash
cmake -S . -B build-mnemosyne \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_MNEMOSYNE=ON

cmake --build build-mnemosyne -j$(nproc)
```

## Usage

Start the server:

```bash
./build-mnemosyne/mnemosyne-cache
```

Then from another terminal:

```bash
redis-cli -p 6379 PING
redis-cli -p 6379 SET name darsh
redis-cli -p 6379 GET name
redis-cli -p 6379 DEL name
```

## Benchmark observations

The benchmarks showed that Mnemosyne is workload-dependent rather than universally faster than `malloc`.

It performs best when:

- objects are small
- allocation sizes are predictable
- values stay within Mnemosyne's slab classes
- the workload repeatedly allocates and frees memory

In direct cache-churn testing, some slab-sized workloads performed better than glibc `malloc`. The strongest observed result was around the 512-byte size class.

For larger values, especially above the 512-byte slab boundary, performance dropped sharply because allocations move to Mnemosyne's large-object path.

## Good use cases for Mnemosyne

Based on the benchmark behavior, Mnemosyne fits workloads such as:

- small in-memory cache entries
- request/session objects
- networking metadata
- fixed-size message objects
- temporary objects with short lifetimes
- object pools
- systems with frequent allocate/free cycles

It is most useful when object sizes are predictable and memory can be reused aggressively.

## Where it is weaker

Mnemosyne is less suitable for:

- large allocations
- unpredictable object sizes
- workloads dominated by objects larger than its slab classes
- applications where allocation is only a tiny part of total execution time

For example, `GET` performance changed very little between the two allocators because most of the work is hash lookup, networking, and response handling rather than allocation.

## Docker

Build from the directory containing both repositories:

```bash
docker build \
    -f mnemosyne-cache/Dockerfile \
    -t mnemosyne-cache .
```

Run:

```bash
docker run --rm \
    -p 6379:6379 \
    mnemosyne-cache
```

Then:

```bash
redis-cli -p 6379 PING
```

## Project structure

```text
mnemosyne-cache/
├── include/
│   ├── allocator.hpp
│   ├── cache.hpp
│   ├── resp.hpp
│   └── server.hpp
├── src/
│   ├── allocator.cpp
│   ├── cache.cpp
│   ├── main.cpp
│   ├── resp.cpp
│   └── server.cpp
├── tests/
│   └── cache_test.cpp
├── benchmark/
│   ├── cache_churn.cpp
│   └── churn.py
├── CMakeLists.txt
├── Dockerfile
└── README.md
```

## Related project

Mnemosyne allocator:

https://github.com/Darsh-xye/mnemosyne
