#ifdef USE_CUPQC

#include <stddef.h>
#include <stdint.h>

#include <hash.hpp>

#include "params.h"
#include "sha256.h"

#ifdef SHA256

__device__ void dev_sha256_inc_init(dev_sha256_state *state)
{
	state->reset();
}

__device__ void dev_sha256_inc_blocks(dev_sha256_state *state, const void *in,
				      size_t inblocks)
{
	if (inblocks != 0) {
		state->update(static_cast<const uint8_t *>(in), 64 * inblocks);
	}
}

__device__ void dev_sha256_inc_finalize(uint8_t *out, dev_sha256_state *state,
					void *in, size_t inlen)
{
	alignas(8) uint8_t digest[SPX_SHA256_OUTPUT_BYTES];

	state->update(static_cast<const uint8_t *>(in), inlen);
	state->finalize();
	state->digest(digest, sizeof(digest));
	for (size_t i = 0; i < sizeof(digest); i++) {
		out[i] = digest[i];
	}
}

__device__ void dev_sha256(uint8_t *out, uint8_t *in, size_t inlen)
{
	dev_sha256_state state{};

	dev_sha256_inc_init(&state);
	dev_sha256_inc_finalize(out, &state, in, inlen);
}

__device__ void dev_seed_state(const unsigned char *pub_seed)
{
	(void)pub_seed;
}

#endif // ifdef SHA256
#endif // ifdef USE_CUPQC
