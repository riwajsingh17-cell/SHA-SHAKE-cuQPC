#include <vector>
#include <iomanip>
#include <stdio.h>
#include <iostream>

#include <hash.hpp>

using namespace cupqc;

/* 
 * This example shows to utilize the generate_sub_tree API function to utilize multiple blocks to generate a Larger Merkle Tree.
*/

using HASH   = decltype(POSEIDON2_BB_8_16() + Thread());
using MERKLE = decltype(MERKLE_FIELD_2097152() + BlockDim<256>());
using SUB_MERKLE = decltype(MERKLE_FIELD_2048() + BlockDim<256>());
using FINAL_MERKLE = decltype(MERKLE_FIELD_1024() + BlockDim<256>());

__global__ void create_leaves_kernel(tree<MERKLE::Size, HASH, uint32_t> merkle, const uint32_t* msg, size_t inbuf_len)
{
    HASH hash{};
    size_t i = threadIdx.x + blockIdx.x * blockDim.x;
    if(i < MERKLE::Size) {
            MERKLE().create_leaf(merkle.nodes + i * merkle.digest_size, msg + i * inbuf_len, hash, inbuf_len);
    }
}

__global__ void generate_sub_tree_kernel(tree<MERKLE::Size, HASH, uint32_t> merkle)
{
    HASH hash{};
    SUB_MERKLE().generate_sub_tree(hash, merkle, blockIdx.x);
}

__global__ void generate_final_tree_kernel(tree<MERKLE::Size, HASH, uint32_t> merkle, uint32_t* root)
{
    HASH hash;
    tree<FINAL_MERKLE::Size, HASH, uint32_t> merkle_final;
    const size_t left_overs = merkle.size - merkle_final.size;
    merkle_final.nodes      = merkle.nodes + left_overs * merkle.digest_size;
    
   __syncthreads();
   FINAL_MERKLE().generate_tree(hash, merkle_final);

   if(threadIdx.x == 0) {
       for(uint32_t i = 0; i < merkle.digest_size; i++) {
           root[i] = merkle.root()[i];
       }
   }
}

void generate_tree(const std::vector<uint32_t>& msg, size_t inbuf_len, tree<MERKLE::Size, HASH, uint32_t> merkle, uint32_t* d_root)
{
    uint32_t* d_msg;
    cudaMalloc(reinterpret_cast<void**>(&d_msg), msg.size() * sizeof(uint32_t)); //Here msg.size has the number of messages and the length of each message is inbuf_len
    cudaMemcpy(d_msg, msg.data(), msg.size() * sizeof(uint32_t), cudaMemcpyHostToDevice);
    create_leaves_kernel<<<MERKLE::Size / MERKLE::BlockDim.x, MERKLE::BlockDim>>>(merkle, d_msg, inbuf_len);
    generate_sub_tree_kernel<<<FINAL_MERKLE::Size , MERKLE::BlockDim>>>(merkle);
    generate_final_tree_kernel<<<1, MERKLE::BlockDim>>>(merkle, d_root);
    cudaFree(d_msg); //Device side message is no longer needed
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {

    constexpr auto N = MERKLE::Size;

    constexpr size_t in_len = 64;
    std::vector<uint32_t> msg(in_len * N, 0); // Here we have 2048 messages, each of length in_len

    // Note that in_len is strictly less than BabyBearPrime, so we don't actually need to mod by BabyBearPrime.
    // This is just for illustration, as our Poseidon2 hash is built on the BabyBear field.
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < in_len; j++) {
            msg[i * in_len + j] = (i * j) % cupqc_common::BabyBearPrime; // Ensure that the message is within the field
        }
    }

    // To begin this sample, we will generate a Merkle tree from the messages.
    // The host function will handle device memory for messages, and generate the tree using a single kernel.
    tree<MERKLE::Size, HASH, uint32_t> merkle;
    merkle.allocate_tree(); // This will allocate the tree on the device
    uint32_t* d_root = nullptr, *root = nullptr;
    root = new uint32_t[merkle.digest_size];
    cudaMalloc(reinterpret_cast<void**>(&d_root), merkle.digest_size * sizeof(uint32_t));
    generate_tree(msg, in_len, merkle, d_root);
    cudaMemcpy(root, d_root, merkle.digest_size * sizeof(uint32_t), cudaMemcpyDeviceToHost);
    for(uint32_t i = 0; i < merkle.digest_size; i++) {
        std::cout << "Root[" << i << "]: " << root[i] << std::endl;
    }
    //We can now free the tree.
    merkle.free_tree();
    cudaFree(d_root);
    delete root;
    return 0;
}
