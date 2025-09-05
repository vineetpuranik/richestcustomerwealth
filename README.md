# Richest Customer Wealth Benchmarking 🏦

## What Are We Trying To Do? 🤔

We are benchmarking different approaches to efficiently find the "richest customer" from a large dataset of bank account balances. The goal is to understand the impact of parallelism and I/O on overall performance.

## How Is the Current Accounts Data Structured? 📄

- The data is stored in a file named `accounts.txt`.
- Each line in the file represents a single customer.
- Each line contains space-separated integers, where each integer is the balance of one bank account for that customer.

## What Each of the `.cpp` Files Are Doing

- **richestcustomerwealth_single.cpp** 🐢  
  Reads and parses the file sequentially, computes each customer's total wealth in a single thread, and finds the maximum wealth.

- **richestcustomerwealth_multi.cpp** 🏎️  
  Reads and parses the file in parallel using multiple threads, computes each customer's wealth in parallel, and finds the maximum wealth using parallel reduction.

- **richestcustomerwealth_openmp.cpp** 🚀  
  Reads and parses the file sequentially (since file I/O is not OpenMP-friendly), then uses OpenMP to parallelize the computation of each customer's wealth and find the maximum.

## Benchmark Results 📊

### Single-threaded Implementation 🐢
- **Single-threaded richest customer wealth:** 67332
- **Time to read & parse file:** 49.28 seconds
- **Time to compute richest:** 0.55 seconds
- **Total time:** 49.83 seconds

### Multi-threaded Implementation 🏎️
- **Hardware concurrency (CPU threads available):** 8
- **Multi-threaded richest customer wealth:** 67332
- **Time to read & parse file:** 14.98 seconds
- **Time to compute richest:** 0.36 seconds
- **Total time:** 15.34 seconds

### OpenMP Implementation 🚀
- **OpenMP richest customer wealth:** 67332
- **OpenMP read & parse time:** 49.66 seconds
- **OpenMP compute time:** 0.31 seconds
- **OpenMP total time:** 49.97 seconds
- **OpenMP threads used:** 8

## Note

- **GPU implementation is in progress.** 🖥️⚡  
  The project will be extended to leverage OpenCL
