// cuPQC BigInt Add / ModMul Example
//
// A minimal, single-thread demonstration of two of the most common
// cuPQC-BigInt operations: plain addition and modular multiplication.

#include "cucheck.hpp"
#include "bigint.hpp"

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>

using namespace cupqc;

// 256-bit integers, one thread per value.
using BI256 = decltype(BitWidth<256>() + SM<800>() + Thread());

__global__ void add_mulmod_kernel(uint32_t* sums, uint32_t* products,
                                   const uint32_t* global_buf_a,
                                   const uint32_t* global_buf_b,
                                   const uint32_t* global_buf_m,
                                   unsigned int count)
{
    const unsigned int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index >= count) {
        return;
    }

    const BI256::bigint a(global_buf_a, index);
    const BI256::bigint b(global_buf_b, index);
    const BI256::bigint m(global_buf_m, index);

    // 256-bit integer addition (wraps on overflow).
    const auto sum = a + b;
    // (a * b) mod m.
    const auto product = a.mul_mod(b, m);

    sum.store(sums, index);
    product.store(products, index);
}

int main()
{
    // Small values keep host-side verification easy; the same layout
    // supports arbitrary 256-bit input values.
    const unsigned int count = 2;
    using host_limb_array = std::array<uint32_t, BI256::num_limbs>;
    static_assert(sizeof(host_limb_array) == BI256::num_limbs * sizeof(uint32_t));
    std::vector<host_limb_array> a(count, host_limb_array{});
    std::vector<host_limb_array> b(count, host_limb_array{});
    std::vector<host_limb_array> m(count, host_limb_array{});
    std::vector<host_limb_array> sums(count);
    std::vector<host_limb_array> products(count);

    a[0][0] = 7u;
    b[0][0] = 5u;
    m[0][0] = 11u;   // (7 + 5) = 12, (7 * 5) mod 11 = 2
    a[1][0] = 40u;
    b[1][0] = 9u;
    m[1][0] = 13u;   // (40 + 9) = 49, (40 * 9) mod 13 = 3

    uint32_t *d_a = nullptr, *d_b = nullptr, *d_m = nullptr;
    uint32_t *d_sums = nullptr, *d_products = nullptr;
    const size_t bytes = a.size() * sizeof(host_limb_array);
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_a), bytes));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_b), bytes));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_m), bytes));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_sums), bytes));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_products), bytes));
    CUDA_CHECK(cudaMemcpy(d_a, a.data(), bytes, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_b, b.data(), bytes, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_m, m.data(), bytes, cudaMemcpyHostToDevice));

    constexpr unsigned int kThreads = 128;
    add_mulmod_kernel<<<(count + kThreads - 1) / kThreads, kThreads>>>(
        d_sums, d_products, d_a, d_b, d_m, count);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaMemcpy(sums.data(), d_sums, bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(products.data(), d_products, bytes, cudaMemcpyDeviceToHost));

    CUDA_CHECK(cudaFree(d_a));
    CUDA_CHECK(cudaFree(d_b));
    CUDA_CHECK(cudaFree(d_m));
    CUDA_CHECK(cudaFree(d_sums));
    CUDA_CHECK(cudaFree(d_products));

    std::printf("OK: 7 + 5 = %u, (7 * 5) mod 11 = %u; "
                "40 + 9 = %u, (40 * 9) mod 13 = %u\n",
                sums[0][0], products[0][0], sums[1][0], products[1][0]);
    return EXIT_SUCCESS;
}
