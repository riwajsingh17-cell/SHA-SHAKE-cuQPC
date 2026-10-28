// Copyright (c) 2026, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// Raw RSA bigint example. This generates a non-secret 2048-bit demonstration
// key on the GPU and performs a raw-RSA round trip. It is not production RSA:
// xorshift, probable-prime testing, and this variable-time private operation
// are deliberately instructional only. Use vetted key generation, OAEP,
// secure key handling, and side-channel protections in real applications.

#include "cucheck.hpp"
#include "bigint.hpp"

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace {

constexpr unsigned int kBits = 2048;
constexpr unsigned int kWords = kBits / 32;
constexpr uint64_t kPublicExponent = 65537;
constexpr unsigned int kMillerRabinRounds = 64;
constexpr unsigned int kMaxPrimeCandidates = 16384;

// Limb zero is least significant, matching cuPQC bigint's little-endian layout.
constexpr uint32_t kPlaintext[kWords] = {42u};

constexpr uint32_t kKeygenSuccess = 0u;
constexpr uint32_t kKeygenFailure = 1u;

// The first 168 primes, through 997. This inexpensive sieve avoids most
// expensive Miller-Rabin tests; candidates are always much larger than them.
__device__ __constant__ uint16_t kSmallPrimes[] = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53,
    59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113,
    127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181,
    191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251,
    257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317,
    331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397,
    401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463,
    467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547, 557,
    563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619,
    631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701,
    709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787,
    797, 809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 863,
    877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953,
    967, 971, 977, 983, 991, 997,
};
constexpr unsigned int kSmallPrimeCount = sizeof(kSmallPrimes) / sizeof(kSmallPrimes[0]);

template<unsigned int BitWidth, uint64_t PublicExponent = kPublicExponent>
struct RSAPublicKeyDevice {
    using bigint = typename decltype(cupqc::BitWidth<BitWidth>() + cupqc::SM<800>()
                                     + cupqc::TPI<32>() + cupqc::Warp())::bigint;

    bigint modulus;

    __device__ explicit RSAPublicKeyDevice(const uint32_t* modulus_ptr)
        : modulus(modulus_ptr) {}

    __device__ void encrypt(uint32_t* ciphertext, const uint32_t* message) const
    {
        const bigint message_value(message);
        message_value.pow_mod(PublicExponent, modulus).store(ciphertext);
    }
};

template<unsigned int BitWidth>
struct RSAPrivateKeyDevice {
    using bigint = typename decltype(cupqc::BitWidth<BitWidth>() + cupqc::SM<800>()
                                     + cupqc::TPI<32>() + cupqc::Warp())::bigint;

    bigint modulus;
    bigint exponent;

    __device__ RSAPrivateKeyDevice(const uint32_t* modulus_ptr, const uint32_t* exponent_ptr)
        : modulus(modulus_ptr), exponent(exponent_ptr) {}

    __device__ void decrypt(uint32_t* recovered, const uint32_t* ciphertext) const
    {
        const bigint ciphertext_value(ciphertext);
        ciphertext_value.pow_mod(exponent, modulus).store(recovered);
    }
};

__device__ uint64_t xorshift64star(uint64_t& state)
{
    state ^= state >> 12;
    state ^= state << 25;
    state ^= state >> 27;
    return state * 0x2545f4914f6cdd1dULL;
}

template<unsigned int BitWidth>
__device__ void generate_half_width_value(uint32_t* value, uint64_t& rng_state)
{
    constexpr unsigned int kWordCount = BitWidth / 32;
    constexpr unsigned int kPrimeWordCount = kWordCount / 2;
    const unsigned int lane = threadIdx.x & 31u;

    static_assert(BitWidth == 2048, "Only the instantiated 2048-bit TPI<32> path is supported");
    value[lane] = static_cast<uint32_t>(xorshift64star(rng_state));
    value[lane + kPrimeWordCount] = 0u;
    __syncwarp();
}

template<unsigned int BitWidth>
__device__ void generate_prime_candidate(uint32_t* candidate, uint64_t& rng_state)
{
    constexpr unsigned int kPrimeWordCount = BitWidth / 64;
    generate_half_width_value<BitWidth>(candidate, rng_state);
    if ((threadIdx.x & 31u) == 0u) {
        candidate[0] |= 1u;
        candidate[kPrimeWordCount - 1] |= 0x80000000u;
    }
    __syncwarp();
}

template<unsigned int BitWidth>
__device__ bool passes_small_prime_sieve(const uint32_t* candidate)
{
    constexpr unsigned int kPrimeWordCount = BitWidth / 64;
    bool passes = true;
    if ((threadIdx.x & 31u) == 0u) {
        for (unsigned int prime_index = 0; prime_index < kSmallPrimeCount; ++prime_index) {
            const uint16_t prime = kSmallPrimes[prime_index];
            uint64_t remainder = 0;
            for (int word = kPrimeWordCount - 1; word >= 0; --word) {
                remainder = ((remainder << 32) + candidate[word]) % prime;
            }
            if (remainder == 0u) {
                passes = false;
                break;
            }
        }
    }
    return __shfl_sync(0xffffffffu, passes, 0);
}

template<unsigned int BitWidth>
__device__ unsigned int trailing_zeroes_in_candidate_minus_one(const uint32_t* candidate)
{
    constexpr unsigned int kPrimeWordCount = BitWidth / 64;
    unsigned int count = 0;
    if ((threadIdx.x & 31u) == 0u) {
        uint32_t word = candidate[0] - 1u;
        for (unsigned int i = 0; i < kPrimeWordCount; ++i) {
            if (word != 0u) {
                count += __ffs(word) - 1u;
                break;
            }
            count += 32u;
            if (i + 1u < kPrimeWordCount) {
                word = candidate[i + 1u];
            }
        }
    }
    return __shfl_sync(0xffffffffu, count, 0);
}

template<unsigned int BitWidth>
__device__ bool is_probable_prime(const uint32_t* candidate_ptr, uint32_t* base_ptr,
                                  uint64_t& rng_state)
{
    using bigint = typename RSAPublicKeyDevice<BitWidth>::bigint;
    const bigint candidate(candidate_ptr);
    const bigint candidate_minus_one = candidate - 1u;
    const unsigned int s = trailing_zeroes_in_candidate_minus_one<BitWidth>(candidate_ptr);
    const bigint d = candidate_minus_one >> s;

    for (unsigned int round = 0; round < kMillerRabinRounds; ++round) {
        // Each lane supplies one limb of a full-width pseudorandom base.
        generate_half_width_value<BitWidth>(base_ptr, rng_state);
        bigint base(base_ptr);
        base = base.mod(candidate);

        // A base of zero or one is uninformative; try a fresh base instead.
        for (unsigned int retry = 0; retry < 4
             && (base == 0u || base == 1u); ++retry) {
            generate_half_width_value<BitWidth>(base_ptr, rng_state);
            base = bigint(base_ptr).mod(candidate);
        }
        if (base == 0u || base == 1u) {
            return false;
        }

        bigint witness = base.pow_mod(d, candidate);
        if (witness == 1u || witness == candidate_minus_one) {
            continue;
        }

        bool passed = false;
        for (unsigned int r = 1; r < s; ++r) {
            witness = witness.mul_mod(witness, candidate);
            if (witness == candidate_minus_one) {
                passed = true;
                break;
            }
            if (witness == 1u) {
                break;
            }
        }
        if (!passed) {
            return false;
        }
    }
    return true;
}

template<unsigned int BitWidth>
__device__ uint32_t generate_key(uint32_t* modulus, uint32_t* private_exponent,
                                  uint32_t* p, uint32_t* q, uint32_t* base,
                                  uint64_t seed)
{
    using bigint = typename RSAPublicKeyDevice<BitWidth>::bigint;
    const unsigned int lane = threadIdx.x & 31u;
    uint64_t rng_state = seed ^ (0x9e3779b97f4a7c15ULL * (lane + 1u));
    if (rng_state == 0u) {
        rng_state = lane + 1u;
    }

    unsigned int p_attempts_remaining = kMaxPrimeCandidates;
    while (true) {
        if (p_attempts_remaining == 0u) {
            return kKeygenFailure;
        }
        --p_attempts_remaining;
        generate_prime_candidate<BitWidth>(p, rng_state);
        if (passes_small_prime_sieve<BitWidth>(p)
            && is_probable_prime<BitWidth>(p, base, rng_state)) {
            break;
        }
    }

    const bigint public_exponent(static_cast<uint32_t>(kPublicExponent));
    const bigint p_value(p);
    unsigned int q_attempts_remaining = kMaxPrimeCandidates;
    while (true) {
        if (q_attempts_remaining == 0u) {
            return kKeygenFailure;
        }
        --q_attempts_remaining;
        generate_prime_candidate<BitWidth>(q, rng_state);
        if (!passes_small_prime_sieve<BitWidth>(q)
            || !is_probable_prime<BitWidth>(q, base, rng_state)) {
            continue;
        }

        const bigint q_value(q);
        if (p_value == q_value) {
            continue;
        }
        const bigint phi = (p_value - 1u).mul_low(q_value - 1u);
        bigint d;
        if (public_exponent.inv_mod(phi, d) == cupqc::bigint_error::success) {
            p_value.mul_low(q_value).store(modulus);
            d.store(private_exponent);
            return kKeygenSuccess;
        }
    }
}

template<unsigned int BitWidth>
__global__ void rsa_keygen_round_trip_kernel(uint32_t* ciphertext, uint32_t* recovered,
                                             const uint32_t* message, uint32_t* modulus,
                                             uint32_t* private_exponent, uint32_t* p,
                                             uint32_t* q, uint32_t* base, uint32_t* status,
                                             uint64_t seed)
{
    // One full warp cooperates on every TPI=32 bigint operation.
    const uint32_t result = generate_key<BitWidth>(modulus, private_exponent, p, q, base, seed);
    if ((threadIdx.x & 31u) == 0u) {
        *status = result;
    }
    if (result != kKeygenSuccess) {
        return;
    }

    RSAPublicKeyDevice<BitWidth> public_key{modulus};
    RSAPrivateKeyDevice<BitWidth> private_key{modulus, private_exponent};
    public_key.encrypt(ciphertext, message);
    private_key.decrypt(recovered, ciphertext);
}

bool parse_seed(int argc, char** argv, uint64_t& seed)
{
    if (argc == 1) {
        FILE* random = std::fopen("/dev/urandom", "rb");
        if (random == nullptr || std::fread(&seed, sizeof(seed), 1, random) != 1) {
            std::fprintf(stderr, "Unable to read a seed from /dev/urandom\n");
            if (random != nullptr) {
                std::fclose(random);
            }
            return false;
        }
        std::fclose(random);
        return true;
    }
    if (argc != 3 || std::strcmp(argv[1], "--seed") != 0) {
        std::fprintf(stderr, "Usage: %s [--seed <integer>]\n", argv[0]);
        return false;
    }

    char* end = nullptr;
    errno = 0;
    seed = std::strtoull(argv[2], &end, 0);
    if (errno != 0 || end == argv[2] || *end != '\0') {
        std::fprintf(stderr, "Invalid seed: %s\n", argv[2]);
        return false;
    }
    return true;
}

bool verify_plaintext(const std::vector<uint32_t>& actual)
{
    for (unsigned int i = 0; i < kWords; ++i) {
        if (actual[i] != kPlaintext[i]) {
            std::fprintf(stderr, "FAIL: recovered plaintext limb[%u] = 0x%08x, expected 0x%08x\n",
                         i, actual[i], kPlaintext[i]);
            return false;
        }
    }
    return true;
}

} // namespace

int main(int argc, char** argv)
{
    uint64_t seed = 0;
    if (!parse_seed(argc, argv, seed)) {
        return EXIT_FAILURE;
    }

    std::vector<uint32_t> plaintext(kPlaintext, kPlaintext + kWords);
    std::vector<uint32_t> recovered(kWords);
    uint32_t status = kKeygenFailure;

    uint32_t *d_message = nullptr, *d_modulus = nullptr, *d_private_exponent = nullptr;
    uint32_t *d_ciphertext = nullptr, *d_recovered = nullptr, *d_p = nullptr, *d_q = nullptr;
    uint32_t *d_base = nullptr, *d_status = nullptr;
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_message), kWords * sizeof(uint32_t)));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_modulus), kWords * sizeof(uint32_t)));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_private_exponent), kWords * sizeof(uint32_t)));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_ciphertext), kWords * sizeof(uint32_t)));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_recovered), kWords * sizeof(uint32_t)));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_p), kWords * sizeof(uint32_t)));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_q), kWords * sizeof(uint32_t)));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_base), kWords * sizeof(uint32_t)));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_status), sizeof(uint32_t)));
    CUDA_CHECK(cudaMemcpy(d_message, plaintext.data(), kWords * sizeof(uint32_t), cudaMemcpyHostToDevice));

    cudaEvent_t start = nullptr;
    cudaEvent_t stop = nullptr;
    CUDA_CHECK(cudaEventCreate(&start));
    CUDA_CHECK(cudaEventCreate(&stop));
    CUDA_CHECK(cudaEventRecord(start));
    rsa_keygen_round_trip_kernel<kBits><<<1, 32>>>(
        d_ciphertext, d_recovered, d_message, d_modulus, d_private_exponent, d_p, d_q,
        d_base, d_status, seed);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaEventRecord(stop));
    CUDA_CHECK(cudaEventSynchronize(stop));

    float elapsed_ms = 0.0f;
    CUDA_CHECK(cudaEventElapsedTime(&elapsed_ms, start, stop));
    CUDA_CHECK(cudaMemcpy(&status, d_status, sizeof(status), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(recovered.data(), d_recovered, kWords * sizeof(uint32_t), cudaMemcpyDeviceToHost));

    CUDA_CHECK(cudaEventDestroy(start));
    CUDA_CHECK(cudaEventDestroy(stop));
    CUDA_CHECK(cudaFree(d_message));
    CUDA_CHECK(cudaFree(d_modulus));
    CUDA_CHECK(cudaFree(d_private_exponent));
    CUDA_CHECK(cudaFree(d_ciphertext));
    CUDA_CHECK(cudaFree(d_recovered));
    CUDA_CHECK(cudaFree(d_p));
    CUDA_CHECK(cudaFree(d_q));
    CUDA_CHECK(cudaFree(d_base));
    CUDA_CHECK(cudaFree(d_status));

    if (status != kKeygenSuccess) {
        std::fprintf(stderr, "FAIL: RSA key generation failed\n");
        return EXIT_FAILURE;
    }
    const bool plaintext_ok = verify_plaintext(recovered);
    std::printf("RSA 2048-bit key generation and raw round trip: %s (%.2f ms)\n",
                plaintext_ok ? "OK" : "FAIL", elapsed_ms);
    return plaintext_ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
