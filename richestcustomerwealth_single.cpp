#include <cstdio>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>

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

int richest_customer_wealth(const std::vector<std::vector<int>>& accounts) {
    int richest = 0;
    for (const auto& customer : accounts) {
        int wealth = 0;
        for (int money : customer) wealth += money;
        if (wealth > richest) richest = wealth;
    }
    return richest;
}

int main() {
    const std::string filename = "accounts.txt";
    printf("Reading test data from file (single-threaded)...\n");

    auto t_start = std::chrono::high_resolution_clock::now();

    // File read & parse
    auto t1 = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<int>> accounts = read_accounts_from_file(filename);
    auto t2 = std::chrono::high_resolution_clock::now();

    // Richest calculation
    int richest = richest_customer_wealth(accounts);
    auto t3 = std::chrono::high_resolution_clock::now();

    double total_time   = std::chrono::duration<double>(t3 - t_start).count();
    double read_time    = std::chrono::duration<double>(t2 - t1).count();
    double compute_time = std::chrono::duration<double>(t3 - t2).count();

    printf("Single-threaded richest customer wealth: %d\n", richest);
    printf("Time to read & parse file: %.6f seconds\n", read_time);
    printf("Time to compute richest:  %.6f seconds\n", compute_time);
    printf("Total time:               %.6f seconds\n", total_time);

    return 0;
}
