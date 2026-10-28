#include <cuda_runtime.h>

#include <cstdint>
#include <cstdio>

#include <hash.hpp>

using CupqcSha256 = decltype(cupqc::SHA2_256() + cupqc::Thread());

__global__ void hash_abc(uint8_t *output)
{
	alignas(8) uint8_t digest[32];
	CupqcSha256 context{};

	context.reset();
	context.update(reinterpret_cast<const uint8_t *>("a"), 1);

	CupqcSha256 resumed = context;
	resumed.update(reinterpret_cast<const uint8_t *>("bc"), 2);
	resumed.finalize();
	resumed.digest(digest, sizeof(digest));

	for (size_t i = 0; i < sizeof(digest); i++) {
		output[i] = digest[i];
	}
}

int main()
{
	const uint8_t expected[32] = {
		0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea,
		0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
		0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c,
		0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad
	};
	uint8_t *device_output = nullptr;
	uint8_t host_output[sizeof(expected)];
	cudaError_t error = cudaMalloc(&device_output, sizeof(host_output));
	if (error != cudaSuccess) {
		std::fprintf(stderr, "cudaMalloc failed: %s\n", cudaGetErrorString(error));
		return 1;
	}

	hash_abc<<<1, 1>>>(device_output);
	error = cudaGetLastError();
	if (error == cudaSuccess) {
		error = cudaDeviceSynchronize();
	}
	if (error == cudaSuccess) {
		error = cudaMemcpy(host_output, device_output, sizeof(host_output),
				   cudaMemcpyDeviceToHost);
	}
	cudaFree(device_output);
	if (error != cudaSuccess) {
		std::fprintf(stderr, "CUDA probe failed: %s\n", cudaGetErrorString(error));
		return 1;
	}

	for (size_t i = 0; i < sizeof(expected); i++) {
		if (host_output[i] != expected[i]) {
			std::fprintf(stderr, "SHA-256 mismatch at byte %zu\n", i);
			return 1;
		}
	}
	std::puts("cuPQC incremental SHA-256 context-copy probe passed");
	return 0;
}
