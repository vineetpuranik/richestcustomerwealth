#include <cstdio>
#include <vector>
#include <fstream>
#include <sstream>
#include <thread>
#include <CL/cl.h>
#include <iostream>
#include <algorithm>

// Parallel file reading
std::vector<std::vector<int>> read_accounts_from_file_parallel(const std::string& filename, int num_threads) {
    std::ifstream ifs(filename);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(ifs, line)) lines.push_back(line);

    std::vector<std::vector<int>> accounts(lines.size());
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

// OpenCL kernel as a string
const char* kernelSource = R"CLC(
__kernel void customer_wealth(
    __global const int* accounts,
    __global int* wealth,
    int accounts_per_user
) {
    int gid = get_global_id(0);
    int sum = 0;
    for (int i = 0; i < accounts_per_user; ++i) {
        sum += accounts[gid * accounts_per_user + i];
    }
    wealth[gid] = sum;
}
)CLC";

// Helper to check OpenCL errors
void checkErr(cl_int err, const char* name) {
    if (err != CL_SUCCESS) {
        std::cerr << "ERROR: " << name << " (" << err << ")" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void richest_customer_wealth_gpu(const std::vector<std::vector<int>>& accounts) {
    int num_users = accounts.size();
    int accounts_per_user = accounts[0].size();
    std::vector<int> flat_accounts(num_users * accounts_per_user);
    for (int i = 0; i < num_users; ++i)
        for (int j = 0; j < accounts_per_user; ++j)
            flat_accounts[i * accounts_per_user + j] = accounts[i][j];

    cl_int err;

    // 1. Get platform and device
    cl_platform_id platform;
    cl_device_id device;
    err = clGetPlatformIDs(1, &platform, nullptr); checkErr(err, "clGetPlatformIDs");
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr); checkErr(err, "clGetDeviceIDs");

    // 2. Create context and command queue
    cl_context context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err); checkErr(err, "clCreateContext");
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err); checkErr(err, "clCreateCommandQueue");

    // 3. Create buffers
    cl_mem accounts_buf = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                         sizeof(int) * flat_accounts.size(), flat_accounts.data(), &err); checkErr(err, "clCreateBuffer(accounts)");
    cl_mem wealth_buf = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                       sizeof(int) * num_users, nullptr, &err); checkErr(err, "clCreateBuffer(wealth)");

    // 4. Build kernel
    cl_program program = clCreateProgramWithSource(context, 1, &kernelSource, nullptr, &err); checkErr(err, "clCreateProgramWithSource");
    err = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS) {
        // Print build log on error
        size_t log_size;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size);
        std::vector<char> log(log_size);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log.data(), nullptr);
        std::cerr << "Build log:\n" << log.data() << std::endl;
        checkErr(err, "clBuildProgram");
    }
    cl_kernel kernel = clCreateKernel(program, "customer_wealth", &err); checkErr(err, "clCreateKernel");

    // 5. Set kernel arguments
    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &accounts_buf); checkErr(err, "clSetKernelArg 0");
    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &wealth_buf); checkErr(err, "clSetKernelArg 1");
    err = clSetKernelArg(kernel, 2, sizeof(int), &accounts_per_user); checkErr(err, "clSetKernelArg 2");

    // 6. Launch kernel
    size_t global_work_size = num_users;
    err = clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global_work_size, nullptr, 0, nullptr, nullptr); checkErr(err, "clEnqueueNDRangeKernel");

    // 7. Read back wealth
    std::vector<int> wealth(num_users);
    err = clEnqueueReadBuffer(queue, wealth_buf, CL_TRUE, 0, sizeof(int) * num_users, wealth.data(), 0, nullptr, nullptr); checkErr(err, "clEnqueueReadBuffer");

    // 8. Find max wealth on host
    int richest = *std::max_element(wealth.begin(), wealth.end());
    printf("GPU richest customer wealth: %d\n", richest);

    // 9. Cleanup
    clReleaseMemObject(accounts_buf);
    clReleaseMemObject(wealth_buf);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
}

int main() {
    const std::string filename = "accounts.txt";
    int num_threads = std::min(8, int(std::thread::hardware_concurrency())); // Limit threads for efficiency
    printf("Reading test data from file in parallel...\n");
    clock_t start = clock();
    std::vector<std::vector<int>> accounts = read_accounts_from_file_parallel(filename, num_threads);
    richest_customer_wealth_gpu(accounts);
    clock_t end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;
    printf("GPU time taken (including parallel file read): %.6f seconds\n", elapsed);
    return 0;
}