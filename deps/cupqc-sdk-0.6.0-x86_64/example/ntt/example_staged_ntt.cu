#include <ntt.hpp>
#include <vector>
#include <iostream>

using namespace cupqc;

/*
 * Staged NTT decomposition parameters.
 *
 * SubSize<M> splits an N-point NTT into two successive kernel passes:
 *   K = N / M
 *
 *   Forward pass 1 (M blocks): K independent sub-NTTs on consecutive
 *                               groups of K elements.
 *   Forward pass 2 (K blocks): M independent sub-NTTs on strided groups
 *                               of M elements.
 *
 * This is useful when N exceeds what fits in a single block's shared
 * memory: each stage's working set is bounded by max(K, M) elements.
 *
 * Here N = 16384, M = 256, K = 64 over the KoalaBear prime field.
 */
constexpr uint32_t NTT_N = 16384;
constexpr uint32_t NTT_M = 256;
constexpr uint32_t NTT_K = NTT_N / NTT_M;  // 64

using ForwardNTT = decltype(Algorithm<algorithm::NTT>()
                            + Direction<nttDirection::FORWARD>()
                            + Precision<uint32_t>()
                            + Size<NTT_N>()
                            + SubSize<NTT_M>()
                            + Block()
                            + BlockDim<128>());

using InverseNTT = decltype(Algorithm<algorithm::NTT>()
                            + Direction<nttDirection::INVERSE>()
                            + Precision<uint32_t>()
                            + Size<NTT_N>()
                            + SubSize<NTT_M>()
                            + Block()
                            + BlockDim<128>());

// Host-side modular exponentiation: base^exp mod m.
static uint32_t host_modpow(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t result = 1;
    base %= mod;
    while (exp > 0) {
        if (exp & 1) result = result * base % mod;
        base = base * base % mod;
        exp >>= 1;
    }
    return static_cast<uint32_t>(result);
}

// Twiddle generation — identical to single-stage NTT.
template<class NTT>
__global__ void make_twiddles_kernel(uint32_t* twiddles, const uint32_t p, const uint32_t g) {
    NTT().make_twiddles(twiddles, p, g);
}
template<class NTT>
__global__ void transform_twiddles_to_mont_kernel(uint32_t* twiddles, const uint32_t p) {
    NTT().transform_twiddles_to_mont(twiddles, p);
}

/*
 * Forward NTT stage kernels.
 *
 * stage_1: launched with M blocks, K-element shared memory per block.
 * stage_2: launched with K blocks, M-element shared memory per block.
 *
 * Both stages operate on the same polynomial pointer; stage_2 reads the
 * output written by stage_1.
 */
__global__ void fwd_stage_1_kernel(uint32_t* data, const uint32_t* twiddles, const nttConst<uint32_t> scheme_const, const int batch_id) {
    extern __shared__ uint32_t sdata[];
    ForwardNTT().stage_1_load_to_mont(sdata, data + batch_id * ForwardNTT::Size, blockIdx.x, scheme_const);
    __syncthreads();
    ForwardNTT().stage_1_execute(sdata, twiddles, scheme_const.p);
    __syncthreads();
    ForwardNTT().stage_1_store(sdata, data + batch_id * ForwardNTT::Size, blockIdx.x);
}

__global__ void fwd_stage_2_kernel(uint32_t* data, const uint32_t* twiddles, const nttConst<uint32_t> scheme_const, const int batch_id) {
    extern __shared__ uint32_t sdata[];
    ForwardNTT().stage_2_load(sdata, data + batch_id * ForwardNTT::Size, blockIdx.x);
    __syncthreads();
    ForwardNTT().stage_2_execute(sdata, twiddles, scheme_const.p);
    __syncthreads();
    ForwardNTT().stage_2_store_from_mont(sdata, data + batch_id * ForwardNTT::Size, blockIdx.x, scheme_const);
}

/*
 * Inverse NTT stage kernels.
 *
 * stage_1: launched with K blocks, M-element shared memory per block.
 * stage_2: launched with M blocks, K-element shared memory per block.
 *          Applies the 1/N normalisation via N_inv.
 */
__global__ void inv_stage_1_kernel(uint32_t* data, const uint32_t* inv_twiddles, const nttConst<uint32_t> scheme_const, const int batch_id) {
    extern __shared__ uint32_t sdata[];
    InverseNTT().stage_1_load_to_mont(sdata, data + batch_id * InverseNTT::Size, blockIdx.x, scheme_const);
    __syncthreads();
    InverseNTT().stage_1_execute(sdata, inv_twiddles, scheme_const.p);
    __syncthreads();
    InverseNTT().stage_1_store(sdata, data + batch_id * InverseNTT::Size, blockIdx.x);
}

__global__ void inv_stage_2_kernel(uint32_t* data, const uint32_t* inv_twiddles,
                                    const nttConst<uint32_t> scheme_const, const uint32_t N_inv, const int batch_id) {
    extern __shared__ uint32_t sdata[];
    InverseNTT().stage_2_load(sdata, data + batch_id * InverseNTT::Size, blockIdx.x);
    __syncthreads();
    InverseNTT().stage_2_execute(sdata, inv_twiddles, scheme_const.p, N_inv);
    __syncthreads();
    InverseNTT().stage_2_store_from_mont(sdata, data + batch_id * InverseNTT::Size, blockIdx.x, scheme_const);
}

/*
 * Runs a batched staged NTT round-trip and verifies INTT(NTT(f)) == f.
 *
 * Twiddle tables are generated once and shared across the entire batch.
 * Each polynomial is transformed by launching four kernels (fwd stage 1,
 * fwd stage 2, inv stage 1, inv stage 2) with a pointer offset for that
 * polynomial.
 */
void staged_ntt_round_trip(const unsigned int batch,
                            const uint32_t p,
                            const uint32_t g,
                            const uint32_t g_inv,
                            const uint32_t N_inv)
{
    constexpr uint32_t N = ForwardNTT::Size;
    constexpr uint32_t M = NTT_M;
    constexpr uint32_t K = NTT_K;

    // Shared-memory sizes differ per stage and per direction.
    constexpr size_t fwd_s1_smem = fwd_stage_1_ntt_shared_workspace_size<N, M, uint32_t>();
    constexpr size_t fwd_s2_smem = fwd_stage_2_ntt_shared_workspace_size<N, M, uint32_t>();
    constexpr size_t inv_s1_smem = inv_stage_1_ntt_shared_workspace_size<N, M, uint32_t>();
    constexpr size_t inv_s2_smem = inv_stage_2_ntt_shared_workspace_size<N, M, uint32_t>();

    /*
     * Initialise batch polynomials with values 0, 1, 2, ... (mod p).
     */
    std::vector<uint32_t> h_data(N * batch);
    for (unsigned int b = 0; b < batch; b++) {
        for (uint32_t i = 0; i < N; i++) {
            h_data[b * N + i] = (b * N + i) % p;
        }
    }
    const std::vector<uint32_t> reference = h_data;

    /*
     * Allocate device memory.
     */
    uint32_t* d_data        = nullptr;
    uint32_t* d_twiddles     = nullptr;
    uint32_t* d_inv_twiddles = nullptr;

    cudaMalloc(reinterpret_cast<void**>(&d_data),        N * batch * sizeof(uint32_t));
    cudaMalloc(reinterpret_cast<void**>(&d_twiddles),     N * sizeof(uint32_t));
    cudaMalloc(reinterpret_cast<void**>(&d_inv_twiddles), N * sizeof(uint32_t));

    cudaMemcpy(d_data, h_data.data(), N * batch * sizeof(uint32_t), cudaMemcpyHostToDevice);

    const nttConst<uint32_t> scheme_const(p);

    /*
     * Generate twiddle tables once — same API as single-stage NTT.
     */
    make_twiddles_kernel<ForwardNTT><<<1, 1>>>(d_twiddles,     p, g);
    make_twiddles_kernel<InverseNTT><<<1, 1>>>(d_inv_twiddles, p, g_inv);
    transform_twiddles_to_mont_kernel<ForwardNTT><<<1, ForwardNTT::BlockDim>>>(d_twiddles, p);
    transform_twiddles_to_mont_kernel<InverseNTT><<<1, InverseNTT::BlockDim>>>(d_inv_twiddles, p);

    /*
     * Process each polynomial in the batch.
     *
     * Forward NTT — two kernel launches per polynomial:
     *   stage 1: M blocks, each performing a sub-NTT on K consecutive elements.
     *   stage 2: K blocks, each performing a sub-NTT on M strided elements.
     *
     * Inverse NTT — two kernel launches per polynomial (reverse stage ordering):
     *   stage 1: K blocks, M-element shared memory per block.
     *   stage 2: M blocks, K-element shared memory, applies 1/N normalisation.
     */
    for (unsigned int batch_id = 0; batch_id < batch; batch_id++) {
        fwd_stage_1_kernel<<<M, ForwardNTT::BlockDim, fwd_s1_smem>>>(d_data, d_twiddles, scheme_const, batch_id);
        fwd_stage_2_kernel<<<K, ForwardNTT::BlockDim, fwd_s2_smem>>>(d_data, d_twiddles, scheme_const, batch_id);

        inv_stage_1_kernel<<<K, InverseNTT::BlockDim, inv_s1_smem>>>(d_data, d_inv_twiddles, scheme_const, batch_id);
        inv_stage_2_kernel<<<M, InverseNTT::BlockDim, inv_s2_smem>>>(d_data, d_inv_twiddles, scheme_const, N_inv, batch_id);
    }

    cudaMemcpy(h_data.data(), d_data, N * batch * sizeof(uint32_t), cudaMemcpyDeviceToHost);

    cudaFree(d_data);
    cudaFree(d_twiddles);
    cudaFree(d_inv_twiddles);

    unsigned int errors = 0;
    for (size_t i = 0; i < reference.size(); i++) {
        if (h_data[i] != reference[i]) errors++;
    }
    if (errors == 0) {
        std::cout << "Round-trip OK: " << batch << " polynomial(s) of degree " << N << " (M=" << M << " K=" << K << ") verified." << std::endl;
    } else {
        std::cout << "Round-trip FAILED: " << errors << " coefficient(s) did not match." << std::endl;
    }
}

/*
 * This example demonstrates a batched staged NTT round-trip over the KoalaBear
 * prime field.  The staged API decomposes a large N-point NTT into two passes
 * using SubSize<M>, keeping each pass's per-block working set to K = N/M or M
 * elements rather than the full N.
 *
 * KoalaBear prime p = 2^31 - 2^24 + 1 supports NTT sizes up to 2^24.
 * Here N = 16384 = 2^14, split as M = 256, K = 64.
 *
 * In a real application the two forward stages would bracket pointwise
 * operations on the NTT-domain coefficients before the inverse stages.
 */
int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    constexpr uint32_t p = cupqc::KoalaBear;               // KoalaBear prime
    constexpr uint32_t g = cupqc::KoalaBear_primitive_root_14; // primitive 2^14-th root of unity mod p

    const uint32_t g_inv = host_modpow(g, p - 2, p);  // g^{-1} mod p
    const uint32_t N_inv = 2130576385; // (2^14)^{-1} mod p

    const unsigned int batch = 4;
    staged_ntt_round_trip(batch, p, g, g_inv, N_inv);
}
