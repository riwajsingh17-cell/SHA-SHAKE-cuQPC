// cuPQC BigInt VecAdd Example
// 
// Vector addition is the "hello world" of CUDA kernel programming.
//
// This kernel adds two vectors of 1024-bit integers, with each addition using
// 4 threads cooperating per integer operation (TPI=4).

#include "cucheck.hpp"
#include "bigint.hpp"

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>

using namespace cupqc;

// 1024-bit integers, with four threads cooperating on each integer.
using BI1024 = decltype(BitWidth<1024>() + SM<800>() + TPI<4>() + Warp());

__global__ void add_kernel(uint32_t* sums,
                           const uint32_t* global_buf_a,
                           const uint32_t* global_buf_b,
                           unsigned int count)
{
    const unsigned int thread_id = blockIdx.x * blockDim.x + threadIdx.x;
    // each group of 4 threads owns one 1024-bit integer
    const unsigned int bigint_index = thread_id / BI1024::tpi;
    if (bigint_index >= count) {
        return;
    }

    // The indexed constructor reads the integer from the packed limb buffer
    const BI1024::bigint a(global_buf_a, bigint_index);
    const BI1024::bigint b(global_buf_b, bigint_index);
    // 1024-bit integer addition (wraps on overflow)
    const auto c = a + b;
    // store the result in the packed output limb buffer
    c.store(sums, bigint_index);
}

int main()
{
    // Two small values make host-side verification easy; the same layout
    // supports arbitrary 1024-bit input values.
    const unsigned int count = 2;
    using host_limb_array = std::array<uint32_t, BI1024::num_limbs>;
    static_assert(sizeof(host_limb_array) == BI1024::num_limbs * sizeof(uint32_t));
    std::vector<host_limb_array> a(count);
    std::vector<host_limb_array> b(count);
    std::vector<host_limb_array> sums(count);
    a[0][0] = 2u;
    a[1][0] = 10u;
    b[0][0] = 3u;
    b[1][0] = 20u;

    uint32_t *d_a = nullptr, *d_b = nullptr, *d_sums = nullptr;
    const size_t bytes = a.size() * sizeof(host_limb_array);
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_a), bytes));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_b), bytes));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_sums), bytes));
    CUDA_CHECK(cudaMemcpy(d_a, a.data(), bytes, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_b, b.data(), bytes, cudaMemcpyHostToDevice));

    constexpr unsigned int kThreads = 128;
    add_kernel<<<(count * BI1024::tpi + kThreads - 1) / kThreads, kThreads>>>(
        d_sums, d_a, d_b, count);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaMemcpy(sums.data(), d_sums, bytes, cudaMemcpyDeviceToHost));

    CUDA_CHECK(cudaFree(d_a));
    CUDA_CHECK(cudaFree(d_b));
    CUDA_CHECK(cudaFree(d_sums));

    std::printf("OK: 2 + 3 = %u; 10 + 20 = %u\n", sums[0][0], sums[1][0]);
    return EXIT_SUCCESS;
}
