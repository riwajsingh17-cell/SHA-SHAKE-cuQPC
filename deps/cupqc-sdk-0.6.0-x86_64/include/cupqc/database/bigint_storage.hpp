// Copyright (c) 2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#ifndef CUPQC_BIGINT_STORAGE_HPP
#define CUPQC_BIGINT_STORAGE_HPP

#include "../operators/bigint_on_error.hpp"

#include <cstdint>
#include <type_traits>

namespace cupqc {
    namespace database {


        // Forward declarations
        template<unsigned int N_LIMBS, unsigned int TPI, class ErrorPolicy = default_bigint_error_policy>
        class bigint_impl;

        template<unsigned int N_LIMBS, unsigned int TPI, class ErrorPolicy = default_bigint_error_policy>
        class wide_bigint_impl;

        template<unsigned int N_LIMBS, class ErrorPolicy = default_bigint_error_policy, unsigned int TPI = 1>
        class montgomery_modulus;

        /// Whether div_rem supports QUOTIENT_LIMBS for a DIVIDEND_LIMBS-wide dividend.
        template<unsigned int DIVIDEND_LIMBS, unsigned int QUOTIENT_LIMBS>
        inline constexpr bool div_rem_quotient_width_supported_v =
            QUOTIENT_LIMBS == DIVIDEND_LIMBS ||
            2 * QUOTIENT_LIMBS == DIVIDEND_LIMBS;

        namespace detail {

            template<unsigned int DIVIDEND_LIMBS, unsigned int QUOTIENT_LIMBS>
            struct div_rem_quotient_width_check {
                static_assert(
                    div_rem_quotient_width_supported_v<DIVIDEND_LIMBS, QUOTIENT_LIMBS>,
                    "div_rem requires quotient width to equal or be half the dividend width");
                static constexpr int value = 0;
            };

        } // namespace detail

        /// ---- Templated bigint storage (parameterized by instance limb count and TPI)
        /// N_LIMBS is the number of valid limbs in the entire bigint instance.
        /// Developer note: this differs from most core-function N_LIMBS symbols, which usually mean the number of limbs allocated to each thread.
        template<unsigned int N_LIMBS, unsigned int TPI>
        class bigint_storage {
        private:
            static constexpr uint32_t THREAD_ALLOC_LIMBS = (N_LIMBS + TPI - 1) / TPI;

        protected:

            /*
            - The thread with the lowest index has all the least significant limbs.
            - If there is padding, it begins at a certain limb of a certain thread. Then the rest of that thread's limbs are padding and all limbs of all subsequent threads are padding.
                - For example, if there is some small number of padding limbs, they're all with one thread, not one each over a small number of threads.
            - See bigint_limb_dist.cuh for more information about limb distribution metadata.
            */
            uint32_t limbs[THREAD_ALLOC_LIMBS];

        public:
            template<unsigned int, unsigned int, class> friend class bigint_impl;
            template<unsigned int, unsigned int, class> friend class wide_bigint_impl;

            __device__ bigint_storage();
            __device__ explicit bigint_storage(const uint32_t* ptr);
            __device__ explicit bigint_storage(const uint32_t* ptr, unsigned int index);
            __device__ bigint_storage(const bigint_storage& other);
            __device__ bigint_storage& operator=(const bigint_storage& other);
            __device__ void store(uint32_t* ptr) const;
            __device__ void store(uint32_t* ptr, unsigned int index) const;

            // Currently only accesses a thread's local limbs.
            template<unsigned int U = TPI, std::enable_if_t<U == 1 && U == TPI, int> = 0>
            __device__ uint32_t& operator[](unsigned int idx);

            template<unsigned int U = TPI, std::enable_if_t<U == 1 && U == TPI, int> = 0>
            __device__ const uint32_t& operator[](unsigned int idx) const;

        };

        /// ---- The bigint implementation with arithmetic operations
        /// N_LIMBS is the number of valid limbs in the entire bigint instance.
        /// Developer note: this differs from most core-function N_LIMBS symbols, which usually mean the number of limbs allocated to each thread.

        template<unsigned int N_LIMBS, unsigned int TPI, class ErrorPolicy>
        class bigint_impl : public bigint_storage<N_LIMBS, TPI> {
        private:

            using storage_type = bigint_storage<N_LIMBS, TPI>;
            using this_t = bigint_impl;
            using this_wide_t = wide_bigint_impl<N_LIMBS, TPI, ErrorPolicy>;

            template<unsigned int A, unsigned int B, class E>
            friend class bigint_impl;

        public:
            using storage_type::storage_type;

            __device__ explicit bigint_impl(uint32_t value);

            template<unsigned int M = N_LIMBS, std::enable_if_t<M == 1, int> = 0>
            __device__ explicit bigint_impl(uint64_t value) = delete;

            template<unsigned int M = N_LIMBS, std::enable_if_t<M != 1, int> = 0>
            __device__ explicit bigint_impl(uint64_t value);

            __device__ uint32_t to_uint32() const;
            __device__ uint64_t to_uint64() const;

            __device__ this_t add_scalar(const uint32_t limb) const;
            __device__ this_t& operator+=(const uint32_t limb);
            __device__ this_t operator+(const uint32_t limb) const;

            __device__ this_t& operator+=(const this_t& other);
            __device__ this_t operator+(const this_t& other) const;

            __device__ int clz() const;

            __device__ this_t& operator<<=(unsigned int shift);
            __device__ this_t operator<<(unsigned int shift) const;

            __device__ this_t& operator>>=(unsigned int shift);
            __device__ this_t operator>>(unsigned int shift) const;

            __device__ this_t& operator-=(const this_t& other);
            __device__ this_t operator-(const this_t& other) const;

            __device__ this_t sub_scalar(const uint32_t limb) const;
            __device__ this_t& operator-=(const uint32_t limb);
            __device__ this_t operator-(const uint32_t limb) const;

            __device__ this_t operator&(const this_t& other) const;
            __device__ this_t operator|(const this_t& other) const;
            __device__ this_t operator^(const this_t& other) const;
            __device__ this_t operator~() const;

            __device__ __forceinline__ this_t& operator&=(const this_t& other) { return *this = (*this & other); }
            __device__ __forceinline__ this_t& operator|=(const this_t& other) { return *this = (*this | other); }
            __device__ __forceinline__ this_t& operator^=(const this_t& other) { return *this = (*this ^ other); }

            // Performs half the work of mul_wide.
            __device__ this_t mul_low(const this_t& other) const;

            // Performs the same work as mul_wide, simply discarding the lower half.
            __device__ this_t mul_high(const this_t& other) const;

            __device__ this_wide_t mul_wide(const this_t& other) const;

            __device__ this_wide_t operator*(const this_t& other) const;

            __device__ this_t mul_scalar(const uint32_t limb) const;

            __device__ this_t& operator*=(const uint32_t limb);
            __device__ this_t operator*(const uint32_t limb) const;

            /// Returns success, divide_by_zero, or quotient_overflow.
            __device__ bigint_error div_rem(const this_t& divisor, this_t& q, this_t& r) const;

            /// Returns success, divide_by_zero, or quotient_overflow.
            /// The quotient width is independent; remainder width matches the divisor.
            template<unsigned int DIVISOR_LIMBS, unsigned int QUOTIENT_LIMBS,
                     int = detail::div_rem_quotient_width_check<
                         N_LIMBS, QUOTIENT_LIMBS>::value>
            __device__ bigint_error div_rem(
                const bigint_impl<DIVISOR_LIMBS, TPI, ErrorPolicy>& divisor,
                bigint_impl<QUOTIENT_LIMBS, TPI, ErrorPolicy>& q,
                bigint_impl<DIVISOR_LIMBS, TPI, ErrorPolicy>& r) const;

            __device__ this_t operator/(const this_t& divisor) const;
            __device__ this_t operator%(const this_t& divisor) const;

            __device__ static void setup_barrett(const this_t& den, this_t& approx_out, int& den_clz_out);
            __device__ static void reduce_barrett(this_t& rem_out, const this_wide_t& num, const this_t& den, const this_t& approx, const int den_clz);

            __device__ this_wide_t square() const;

            /// Three-way comparison. Returns a value less than, equal to, or
            /// greater than zero when this is respectively less than, equal
            /// to, or greater than other.
            __device__ int compare(const this_t& other) const;
            __device__ int compare(uint32_t other) const;
            __device__ int compare(uint64_t other) const;

            /// Returns true when this and other have identical values.
            __device__ bool operator==(const this_t& other) const { return compare(other) == 0; }
            __device__ bool operator==(uint32_t other) const { return compare(other) == 0; }
            __device__ bool operator==(uint64_t other) const { return compare(other) == 0; }

            __device__ bool operator!=(const this_t& other) const { return compare(other) != 0; }
            __device__ bool operator!=(uint32_t other) const { return compare(other) != 0; }
            __device__ bool operator!=(uint64_t other) const { return compare(other) != 0; }
            __device__ bool operator<(const this_t& other) const { return compare(other) < 0; }
            __device__ bool operator<(uint32_t other) const { return compare(other) < 0; }
            __device__ bool operator<(uint64_t other) const { return compare(other) < 0; }
            __device__ bool operator<=(const this_t& other) const { return compare(other) <= 0; }
            __device__ bool operator<=(uint32_t other) const { return compare(other) <= 0; }
            __device__ bool operator<=(uint64_t other) const { return compare(other) <= 0; }

            __device__ bool operator>=(const this_t& other) const { return compare(other) >= 0; }
            __device__ bool operator>=(uint32_t other) const { return compare(other) >= 0; }
            __device__ bool operator>=(uint64_t other) const { return compare(other) >= 0; }
            __device__ bool operator>(const this_t& other) const { return compare(other) > 0; }
            __device__ bool operator>(uint32_t other) const { return compare(other) > 0; }
            __device__ bool operator>(uint64_t other) const { return compare(other) > 0; }

            __device__ this_t mod(const this_t& divisor) const;

            __device__ this_t add_mod(const this_t& other, const this_t& modulus) const;

            /// (this - other) mod modulus. Precondition: this < modulus and other < modulus.
            __device__ this_t sub_mod(const this_t& other, const this_t& modulus) const;

            /// (this * other) mod modulus using a full multiply then remainder (not Montgomery).
            __device__ this_t mul_mod(const this_t& other, const this_t& modulus) const;

            /// (this^exponent) mod modulus. Zero exponent yields 1 mod modulus (when modulus > 0).
            __device__ this_t pow_mod(uint64_t exponent, const this_t& modulus) const;
            __device__ this_t pow_mod(const this_t& exponent, const this_t& modulus) const;

            /// Multiplicative inverse: sets out to this^{-1} mod modulus.
            /// The input must already be reduced (this < modulus) when modulus is valid.
            /// The modulus must be greater than 1. Both scalar and cooperative
            /// instances support odd and even moduli.
            /// Returns success, inv_mod_not_invertible, inv_mod_invalid_modulus,
            /// or inv_mod_zero_input.
            __device__ bigint_error inv_mod(const this_t& modulus, this_t& out) const;

            /// Montgomery transformation: compute (this * R) mod modulus,
            /// where R = 2^(N_LIMBS*32). Precondition: this < modulus.
            __device__ this_t to_montgomery(const montgomery_modulus<N_LIMBS, ErrorPolicy, TPI>& modulus) const;

            // Montgomery inverse: compute (this * R^-1) mod modulus where R = 2^(N_LIMBS*32)
            __device__ this_t from_montgomery(const montgomery_modulus<N_LIMBS, ErrorPolicy, TPI>& modulus) const;

            // Montgomery multiplication: (this * other) * R^-1 mod modulus; returns normal-width abR mod M
            __device__ this_t mul_montgomery(const this_t& other, const montgomery_modulus<N_LIMBS, ErrorPolicy, TPI>& modulus) const;

        };

        template<class Scalar, unsigned int N_LIMBS, unsigned int TPI, class ErrorPolicy,
                 std::enable_if_t<std::is_same<Scalar, uint32_t>::value
                                  || std::is_same<Scalar, uint64_t>::value, int> = 0>
        __device__ bool operator==(Scalar lhs,
                                   const bigint_impl<N_LIMBS, TPI, ErrorPolicy>& rhs) {
            return rhs == lhs;
        }

        template<class Scalar, unsigned int N_LIMBS, unsigned int TPI, class ErrorPolicy,
                 std::enable_if_t<std::is_same<Scalar, uint32_t>::value
                                  || std::is_same<Scalar, uint64_t>::value, int> = 0>
        __device__ bool operator!=(Scalar lhs,
                                   const bigint_impl<N_LIMBS, TPI, ErrorPolicy>& rhs) {
            return rhs != lhs;
        }

        template<class Scalar, unsigned int N_LIMBS, unsigned int TPI, class ErrorPolicy,
                 std::enable_if_t<std::is_same<Scalar, uint32_t>::value
                                  || std::is_same<Scalar, uint64_t>::value, int> = 0>
        __device__ bool operator<(Scalar lhs,
                                  const bigint_impl<N_LIMBS, TPI, ErrorPolicy>& rhs) {
            return rhs > lhs;
        }

        template<class Scalar, unsigned int N_LIMBS, unsigned int TPI, class ErrorPolicy,
                 std::enable_if_t<std::is_same<Scalar, uint32_t>::value
                                  || std::is_same<Scalar, uint64_t>::value, int> = 0>
        __device__ bool operator<=(Scalar lhs,
                                   const bigint_impl<N_LIMBS, TPI, ErrorPolicy>& rhs) {
            return rhs >= lhs;
        }

        template<class Scalar, unsigned int N_LIMBS, unsigned int TPI, class ErrorPolicy,
                 std::enable_if_t<std::is_same<Scalar, uint32_t>::value
                                  || std::is_same<Scalar, uint64_t>::value, int> = 0>
        __device__ bool operator>=(Scalar lhs,
                                   const bigint_impl<N_LIMBS, TPI, ErrorPolicy>& rhs) {
            return rhs <= lhs;
        }

        template<class Scalar, unsigned int N_LIMBS, unsigned int TPI, class ErrorPolicy,
                 std::enable_if_t<std::is_same<Scalar, uint32_t>::value
                                  || std::is_same<Scalar, uint64_t>::value, int> = 0>
        __device__ bool operator>(Scalar lhs,
                                  const bigint_impl<N_LIMBS, TPI, ErrorPolicy>& rhs) {
            return rhs < lhs;
        }

        /*
            For some padded environment: TPI = 4, valid limbs = 18.
            Legend: (1) Number in square = instance limb index (2) P = padding
            Within each thread rectangle, the rightmost element is array index 0.
            ==========================================================================================================

            TPI > 1 "Regular" distributed limbs:

            Array for each thread.
            thread 3                   thread 2                   thread 1                   thread 0
            +----+----+----+----+----+ +----+----+----+----+----+ +----+----+----+----+----+ +----+----+----+----+----+
            | P  | P  | 17 | 16 | 15 | | 14 | 13 | 12 | 11 | 10 | |  9 |  8 |  7 |  6 |  5 | |  4 |  3 |  2 |  1 |  0 |
            +----+----+----+----+----+ +----+----+----+----+----+ +----+----+----+----+----+ +----+----+----+----+----+

            ==========================================================================================================

            TPI > 1 "Wide" distributed limbs: total valid limbs = 36

            low array for each thread:
            thread 3                   thread 2                   thread 1                   thread 0
            +----+----+----+----+----+ +----+----+----+----+----+ +----+----+----+----+----+ +----+----+----+----+----+
            | P  | P  | 17 | 16 | 15 | | 14 | 13 | 12 | 11 | 10 | |  9 |  8 |  7 |  6 |  5 | |  4 |  3 |  2 |  1 |  0 |
            +----+----+----+----+----+ +----+----+----+----+----+ +----+----+----+----+----+ +----+----+----+----+----+

            high array for each thread:
            thread 3                   thread 2                   thread 1                   thread 0
            +----+----+----+----+----+ +----+----+----+----+----+ +----+----+----+----+----+ +----+----+----+----+----+
            | P  | P  | 35 | 34 | 33 | | 32 | 31 | 30 | 29 | 28 | | 27 | 26 | 25 | 24 | 23 | | 22 | 21 | 20 | 19 | 18 |
            +----+----+----+----+----+ +----+----+----+----+----+ +----+----+----+----+----+ +----+----+----+----+----+
        
        */

        // Wide big int. Holds 2 * N_LIMBS limbs. Split across separate low and high arrays.
        // N_LIMBS is the number of valid limbs in each regular-width bigint instance (lo or hi), so the complete wide instance has 2 * N_LIMBS valid limbs.
        // Developer note: this differs from most core-function N_LIMBS symbols, which usually mean the number of limbs allocated to each thread.
        // We don't template on N_LIMBS_WIDE because the bigint implementation
        // units instantiate a consistent single template symbol. It simplifies
        // type naming.
        template<unsigned int N_LIMBS, unsigned int TPI, class ErrorPolicy>
        class wide_bigint_impl {
        public:
            bigint_impl<N_LIMBS, TPI, ErrorPolicy> lo;
            bigint_impl<N_LIMBS, TPI, ErrorPolicy> hi;

            __device__ wide_bigint_impl();
            __device__ explicit wide_bigint_impl(const uint32_t* src_wide_int);
            __device__ explicit wide_bigint_impl(const uint32_t* src_wide_int_arr, unsigned int group_src_wide_int_idx);
            __device__ wide_bigint_impl(const wide_bigint_impl& other);
            __device__ wide_bigint_impl& operator=(const wide_bigint_impl& other);
            __device__ void store(uint32_t* dst_wide_int) const;
            __device__ void store(uint32_t* dst_wide_int_arr, unsigned int group_dst_wide_int_idx) const;

            // Montgomery reduction of this split 2N-limb value.
            __device__ bigint_impl<N_LIMBS, TPI, ErrorPolicy> reduce_montgomery(
                const montgomery_modulus<N_LIMBS, ErrorPolicy, TPI>& modulus) const;

            template<unsigned int DIVISOR_LIMBS, unsigned int QUOTIENT_LIMBS,
                     int = detail::div_rem_quotient_width_check<
                         2 * N_LIMBS, QUOTIENT_LIMBS>::value>
            __device__ bigint_error div_rem(
                const bigint_impl<DIVISOR_LIMBS, TPI, ErrorPolicy>& divisor,
                bigint_impl<QUOTIENT_LIMBS, TPI, ErrorPolicy>& quotient,
                bigint_impl<DIVISOR_LIMBS, TPI, ErrorPolicy>& remainder) const;

            template<unsigned int U = TPI, std::enable_if_t<U == 1 && U == TPI, int> = 0>
            __device__ uint32_t& operator[](unsigned int idx);

            template<unsigned int U = TPI, std::enable_if_t<U == 1 && U == TPI, int> = 0>
            __device__ const uint32_t& operator[](unsigned int idx) const;
        };



        /// ---- Montgomery modulus with precomputed constants
        /// Precomputes m' = -M^-1 mod 2^32 for efficient Montgomery reduction
        template<unsigned int N_LIMBS, class ErrorPolicy, unsigned int TPI>
        class montgomery_modulus : public bigint_impl<N_LIMBS, TPI, ErrorPolicy> {
        private:
            using base_t = bigint_impl<N_LIMBS, TPI, ErrorPolicy>;
            uint32_t m_prime_;  // -M^-1 mod 2^32

        public:
            // Construct from bigint and precompute Montgomery constant
            __device__ montgomery_modulus(const base_t& modulus);

            // Construct from raw pointer
            __device__ explicit montgomery_modulus(const uint32_t* ptr);

            // Construct from raw pointer with batch index
            __device__ montgomery_modulus(const uint32_t* ptr, unsigned int index);

            // Get precomputed Montgomery constant: -M^-1 mod 2^32
            __device__ uint32_t m_prime() const { return m_prime_; }

        private:
            // Compute the Montgomery constant from the modulus
            __device__ void compute_m_prime();
        };

        // Extern template declarations to prevent implicit instantiation

        extern template class bigint_storage<8, 1>;
        extern template class bigint_storage<16, 1>;

        extern template class bigint_impl<8, 1, OnErrorNone>;
        extern template class bigint_impl<8, 1, OnErrorTrap>;
        extern template class bigint_impl<8, 1, OnErrorPrintTrap>;

        extern template class wide_bigint_impl<8, 1, OnErrorNone>;
        extern template class wide_bigint_impl<8, 1, OnErrorTrap>;
        extern template class wide_bigint_impl<8, 1, OnErrorPrintTrap>;

    } // namespace database
} // namespace cupqc

#endif // CUPQC_BIGINT_STORAGE_HPP
