#ifndef NTT_CUH
#define NTT_CUH

#include "operators/direction.hpp"
#include "ntt_types.hpp"

namespace cupqc {
    namespace database {
        template<uint32_t N, typename Precision>
        __device__ void make_twiddle(Precision* twiddles, const Precision p, const Precision g);

        template<uint32_t N, uint32_t Prime>
        __device__ void make_twiddle(uint32_t* twiddles, const uint32_t g);

        template<uint32_t N, typename Precision, uint32_t NT>
        __device__ void twiddles_to_mont(Precision* twiddles, const Precision p);

        template<typename Precision>
        __device__ Precision to_montgomery(Precision x, const nttConst<Precision>& ntt_const);

        template<typename Precision>
        __device__ Precision from_montgomery(Precision x, const nttConst<Precision>& ntt_const);

        /* NTT and INTT */
        template<uint32_t N, typename Precision, uint32_t NT>
        __device__ void load(Precision* sdata, const Precision* data);

        template<uint32_t N, typename Precision, uint32_t NT>
        __device__ void store(const Precision* sdata, Precision* data);

        template<uint32_t N, typename Precision, uint32_t NT>
        __device__ void load_to_mont(Precision* sdata, const Precision* data, const nttConst<Precision>& ntt_const);

        template<uint32_t N, typename Precision, uint32_t NT>
        __device__ void store_from_mont(const Precision* sdata, Precision* data, const nttConst<Precision>& ntt_const);

        template<uint32_t N, typename Precision, uint32_t NT>
        __device__ void shared_ntt(Precision* sdata, const Precision* twiddles, const Precision p);

        template<uint32_t N, typename Precision, uint32_t NT>
        __device__ void shared_intt(Precision* sdata, const Precision* twiddles, const Precision p, const Precision n_inv);

        // Montgomery NTT (compile-time Prime as non-type template parameter)
        template<uint32_t N, uint32_t Prime, uint32_t NT>
        __device__ void load_montgomery(uint32_t* sdata, const uint32_t* data);

        template<uint32_t N, uint32_t Prime, uint32_t NT>
        __device__ void store_montgomery(const uint32_t* sdata, uint32_t* data);

        template<uint32_t N, uint32_t Prime, uint32_t NT>
        __device__ void shared_ntt_montgomery(uint32_t* sdata, const uint32_t* gm);

        template<uint32_t N, uint32_t Prime, uint32_t NT>
        __device__ void shared_intt_montgomery(uint32_t* sdata, const uint32_t* igm);

        // Addition, Multiplication and Subtraction after NTT
        template<uint32_t N, typename Precision, uint32_t NT>
        __device__ void shared_transformed_addition(Precision* data1, const Precision* data2, const Precision p);

        template<uint32_t N, typename Precision, uint32_t NT>
        __device__ void shared_transformed_multiplication(Precision* data1, const Precision* data2, const Precision p);

        template<uint32_t N, typename Precision, uint32_t NT>
        __device__ void shared_transformed_subtraction(Precision* data1, const Precision* data2, const Precision p);

        // Staged NTT
        template<uint32_t N, uint32_t M, typename Precision, uint32_t NT>
        __device__ void fwd_load_stage_1(Precision* sdata, const Precision* data, const int id);

        template<uint32_t N, uint32_t M, typename Precision, uint32_t NT>
        __device__ void fwd_store_stage_1(const Precision* sdata, Precision* data, const int id);

        template<uint32_t N, uint32_t M, typename Precision, uint32_t NT>
        __device__ void fwd_load_stage_2(Precision* sdata, const Precision* data, const int id);

        template<uint32_t N, uint32_t M, typename Precision, uint32_t NT>
        __device__ void fwd_store_stage_2(const Precision* sdata, Precision* data, const int id);

        template<uint32_t N, uint32_t M, typename Precision, uint32_t NT>
        __device__ void inv_load_stage_1(Precision* sdata, const Precision* data, const int id);

        template<uint32_t N, uint32_t M, typename Precision, uint32_t NT>
        __device__ void inv_store_stage_1(const Precision* sdata, Precision* data, const int id);

        template<uint32_t N, uint32_t M, typename Precision, uint32_t NT>
        __device__ void inv_load_stage_2(Precision* sdata, const Precision* data, const int id);

        template<uint32_t N, uint32_t M, typename Precision, uint32_t NT>
        __device__ void inv_store_stage_2(const Precision* sdata, Precision* data, const int id);

        template<uint32_t N, uint32_t M, typename Precision, uint32_t NT>
        __device__ void fwd_load_stage_1_to_mont(Precision* sdata, const Precision* data, const int id, const nttConst<Precision>& ntt_const);

        template<uint32_t N, uint32_t M, typename Precision, uint32_t NT>
        __device__ void fwd_store_stage_2_from_mont(const Precision* sdata, Precision* data, const int id, const nttConst<Precision>& ntt_const);

        template<uint32_t N, uint32_t M, typename Precision, uint32_t NT>
        __device__ void inv_load_stage_1_to_mont(Precision* sdata, const Precision* data, const int id, const nttConst<Precision>& ntt_const);

        template<uint32_t N, uint32_t M, typename Precision, uint32_t NT>
        __device__ void inv_store_stage_2_from_mont(const Precision* sdata, Precision* data, const int id, const nttConst<Precision>& ntt_const);

        template<uint32_t N, uint32_t M, typename Precision, uint32_t NT>
        __device__ void shared_ntt_stage_1(Precision* data, const Precision* twiddles, const Precision p);

        template<uint32_t N, uint32_t M, typename Precision, uint32_t NT>
        __device__ void shared_intt_stage_1(Precision* data, const Precision* twiddles, const Precision p);

        template<uint32_t N, uint32_t M, typename Precision, uint32_t NT>
        __device__ void shared_ntt_stage_2(Precision* data, const Precision* twiddles, const Precision p);

        template<uint32_t N, uint32_t M, typename Precision, uint32_t NT>
        __device__ void shared_intt_stage_2(Precision* data, const Precision* twiddles, const Precision p, const Precision n_inv);

        // Montgomery staged NTT (compile-time Prime as non-type template parameter)
        template<uint32_t N, uint32_t M, uint32_t Prime, uint32_t NT>
        __device__ void fwd_load_stage_1_montgomery(uint32_t* sdata, const uint32_t* data, const int id);

        template<uint32_t N, uint32_t M, uint32_t Prime, uint32_t NT>
        __device__ void fwd_store_stage_1_montgomery(const uint32_t* sdata, uint32_t* data, const int id);

        template<uint32_t N, uint32_t M, uint32_t Prime, uint32_t NT>
        __device__ void fwd_load_stage_2_montgomery(uint32_t* sdata, const uint32_t* data, const int id);

        template<uint32_t N, uint32_t M, uint32_t Prime, uint32_t NT>
        __device__ void fwd_store_stage_2_montgomery(const uint32_t* sdata, uint32_t* data, const int id);

        template<uint32_t N, uint32_t M, uint32_t Prime, uint32_t NT>
        __device__ void inv_load_stage_1_montgomery(uint32_t* sdata, const uint32_t* data, const int id);

        template<uint32_t N, uint32_t M, uint32_t Prime, uint32_t NT>
        __device__ void inv_store_stage_1_montgomery(const uint32_t* sdata, uint32_t* data, const int id);

        template<uint32_t N, uint32_t M, uint32_t Prime, uint32_t NT>
        __device__ void inv_load_stage_2_montgomery(uint32_t* sdata, const uint32_t* data, const int id);

        template<uint32_t N, uint32_t M, uint32_t Prime, uint32_t NT>
        __device__ void inv_store_stage_2_montgomery(const uint32_t* sdata, uint32_t* data, const int id);

        template<uint32_t N, uint32_t M, uint32_t Prime, uint32_t NT>
        __device__ void shared_ntt_montgomery_stage_1(uint32_t* sdata, const uint32_t* twiddles);

        template<uint32_t N, uint32_t M, uint32_t Prime, uint32_t NT>
        __device__ void shared_ntt_montgomery_stage_2(uint32_t* sdata, const uint32_t* twiddles);

        template<uint32_t N, uint32_t M, uint32_t Prime, uint32_t NT>
        __device__ void shared_intt_montgomery_stage_1(uint32_t* sdata, const uint32_t* igm);

        template<uint32_t N, uint32_t M, uint32_t Prime, uint32_t NT>
        __device__ void shared_intt_montgomery_stage_2(uint32_t* sdata, const uint32_t* igm);
    }
}   


#endif
