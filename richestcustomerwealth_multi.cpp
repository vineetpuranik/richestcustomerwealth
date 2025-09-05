#include <cstdio>
#include <vector>
#include <fstream>
#include <sstream>
#include <thread>
#include <algorithm>
#include <chrono>

// Parallel file reading & parsing
std::vector<std::vector<int>> read_accounts_from_file_parallel(const std::string& filename, int num_threads) {
    std::vector<std::vector<int>> accounts;
    std::ifstream ifs(filename);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(ifs, line)) lines.push_back(line);

    accounts.resize(lines.size());
    auto worker = [&](int tid, int start, int end) {
        for (int i = start; i < end; ++i) {
            std::istringstream iss(lines[i]);
            int val;
            while (iss >> val) accounts[i].push_back(val);
        }
    };

    int chunk = lines.size() / num_threads;
    int remainder = lines.size() % num_threads;
    int start = 0;
    std::vector<std::thread> threads;
    for (int t = 0; t < num_threads; ++t) {
        int end = start + chunk + (t < remainder ? 1 : 0);
        threads.emplace_back(worker, t, start, end);
        start = end;
    }
    for (auto& th : threads) th.join();
    return accounts;
}

// Parallel wealth computation
int richest_customer_wealth_parallel(const std::vector<std::vector<int>>& accounts, int num_threads) {
    int customers = accounts.size();
    std::vector<int> max_wealths(num_threads, 0);

    auto worker = [&](int tid, int start, int end) {
        int local_max = 0;
        for (int i = start; i < end; ++i) {
            int wealth = 0;
            for (int money : accounts[i]) wealth += money;
            if (wealth > local_max) local_max = wealth;
        }
        max_wealths[tid] = local_max;
    };

    int chunk = customers / num_threads;
    int remainder = customers % num_threads;
    int start = 0;
    std::vector<std::thread> threads;
    for (int t = 0; t < num_threads; ++t) {
        int end = start + chunk + (t < remainder ? 1 : 0);
        threads.emplace_back(worker, t, start, end);
        start = end;
    }
    for (auto& th : threads) th.join();
    return *std::max_element(max_wealths.begin(), max_wealths.end());
}

int main() {
    const std::string filename = "accounts.txt";
    int num_threads = std::min(12, int(std::thread::hardware_concurrency()));
    printf("Hardware concurrency (CPU threads available): %d\n", std::thread::hardware_concurrency());

    auto t_start = std::chrono::high_resolution_clock::now();

    // File read + parse
    auto t1 = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<int>> accounts = read_accounts_from_file_parallel(filename, num_threads);
    auto t2 = std::chrono::high_resolution_clock::now();

    // Richest calculation
    int richest = richest_customer_wealth_parallel(accounts, num_threads);
    auto t3 = std::chrono::high_resolution_clock::now();

    double total_time   = std::chrono::duration<double>(t3 - t_start).count();
    double read_time    = std::chrono::duration<double>(t2 - t1).count();
    double compute_time = std::chrono::duration<double>(t3 - t2).count();

    printf("Multi-threaded richest customer wealth: %d\n", richest);
    printf("Time to read & parse file: %.6f seconds\n", read_time);
    printf("Time to compute richest:  %.6f seconds\n", compute_time);
    printf("Total time:               %.6f seconds\n", total_time);

    return 0;
}
