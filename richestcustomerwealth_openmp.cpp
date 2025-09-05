#include <cstdio>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>
#include <omp.h>

// Read accounts sequentially (I/O is not OpenMP-friendly)
std::vector<std::vector<int>> read_accounts_from_file(const std::string& filename) {
    std::vector<std::vector<int>> accounts;
    std::ifstream ifs(filename);
    std::string line;
    while (std::getline(ifs, line)) {
        std::istringstream iss(line);
        std::vector<int> user;
        int val;
        while (iss >> val) user.push_back(val);
        accounts.push_back(user);
    }
    return accounts;
}

// Parallel wealth computation using OpenMP
int richest_customer_wealth_openmp(const std::vector<std::vector<int>>& accounts) {
    int richest = 0;
    int customers = accounts.size();

    #pragma omp parallel for reduction(max:richest)
    for (int i = 0; i < customers; i++) {
        int wealth = 0;
        for (int money : accounts[i]) wealth += money;
        if (wealth > richest) richest = wealth;  // reduction takes care of merging
    }
    return richest;
}

int main() {
    const std::string filename = "accounts.txt";
    printf("Reading test data from file (sequential)...\n");

    auto t_start = std::chrono::high_resolution_clock::now();
    auto t1 = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<int>> accounts = read_accounts_from_file(filename);
    auto t2 = std::chrono::high_resolution_clock::now();

    int richest = richest_customer_wealth_openmp(accounts);
    auto t3 = std::chrono::high_resolution_clock::now();

    double read_parse_time = std::chrono::duration<double>(t2 - t1).count();
    double compute_time = std::chrono::duration<double>(t3 - t2).count();
    double total_time = std::chrono::duration<double>(t3 - t_start).count();

    printf("OpenMP richest customer wealth: %d\n", richest);
    printf("OpenMP read & parse time: %.6f seconds\n", read_parse_time);
    printf("OpenMP compute time: %.6f seconds\n", compute_time);
    printf("OpenMP total time: %.6f seconds\n", total_time);
    printf("OpenMP threads used: %d\n", omp_get_max_threads());
    return 0;
}