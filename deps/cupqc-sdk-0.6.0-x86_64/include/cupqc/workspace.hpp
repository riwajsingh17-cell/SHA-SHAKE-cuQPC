// Copyright (c) 2024, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#ifndef CUPQC_WORKSPACE_HPP
#define CUPQC_WORKSPACE_HPP

#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

namespace cupqc {

template<class T>
uint8_t* make_workspace(size_t batch, cudaStream_t stream = 0) {
    cudaError_t error_code = cudaSuccess;
    if (T::workspace_size == 0) {
        return nullptr;
    } else {
        uint8_t* workspace;
        error_code = cudaMallocAsync(&workspace, T::workspace_size * batch, stream);
        if(error_code != cudaSuccess) {
            std::string error_what = cudaGetErrorString(error_code);
            throw std::runtime_error(error_what);
        }
        return workspace;
    }
}

template<class T>
uint8_t* get_entropy(size_t batch, cudaStream_t stream = 0) {
    cudaError_t error_code = cudaSuccess;
    if (T::entropy_size == 0) {
        return nullptr;
    } else {
        const size_t bytes = T::entropy_size * batch;

        uint8_t* d_entropy;
        cudaMallocAsync(&d_entropy, bytes, stream);

        std::vector<uint8_t> h_entropy(bytes);

        FILE* fp = fopen("/dev/urandom", "r+b");
        // TODO error if fp == NULL
        size_t read_bytes = fread(reinterpret_cast<void*>(h_entropy.data()), sizeof(uint8_t), bytes, fp);
        // TODO error if read_bytes != bytes
        fclose(fp);
        // TODO do PRNG on device instead of in syscall to /dev/urandom

        error_code = cudaMemcpyAsync(d_entropy, h_entropy.data(), bytes, cudaMemcpyDefault, stream);
        if(error_code != cudaSuccess) {
            std::string error_what = cudaGetErrorString(error_code);
            throw std::runtime_error(error_what);
        }

        // Cannot return until h_entropy has been fully read
        cudaStreamSynchronize(stream);

        return d_entropy;
    }
}

inline void release_entropy(uint8_t* entropy, cudaStream_t stream = 0) {
    if (entropy != nullptr) {
        cudaFreeAsync(entropy, stream);
    }
}

inline void destroy_workspace(uint8_t* workspace, cudaStream_t stream = 0) {
    if (workspace != nullptr) {
        cudaFreeAsync(workspace, stream);
    }
}


} // namespace

#endif // CUPQC_WORKSPACE_HPP
