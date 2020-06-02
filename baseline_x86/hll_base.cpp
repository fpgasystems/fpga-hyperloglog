/**
 * Copyright (c) 2020, Systems Group, ETH Zurich
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <cstddef>
#include <cstdint>

#include "hyperloglog.hpp"

#include <xmmintrin.h>
#include <immintrin.h>

//===========================================================================
// Hashes
template<hash_e HASH, typename TR> static inline TR hash(uint32_t const  key);

//---------------------------------------------------------------------------
// IDENT: *Not* a hash! Just returns the key.
template<> inline uint32_t hash<hash_e::IDENT, uint32_t>(uint32_t const  key) { return  key; }

//---------------------------------------------------------------------------
// MURMUR3_32
template<> inline uint32_t hash<hash_e::MURMUR3_32, uint32_t>(uint32_t const  key) {
	uint32_t const SEED = 42;
	uint32_t const LEN  = 4;
	uint32_t const c1 = 0xcc9e2d51;
	uint32_t const c2 = 0x1b873593;

	uint32_t h1 = SEED;
	uint32_t k1 = key;

	// body
	k1 =  key;
	k1 *= c1;
	k1 = (k1 << 15) | (k1 >> (32 - 15));
	k1 *= c2;
	h1 ^= k1;
	h1 = (h1 << 13) | (h1 >> (32 - 13));
	h1 = h1*5+0xe6546b64;

	// finalization
	h1 ^= LEN;
	h1 ^= h1 >> 16;
	h1 *= 0x85ebca6b;
	h1 ^= h1 >> 13;
	h1 *= 0xc2b2ae35;
	h1 ^= h1 >> 16;

	return	h1;
}

//---------------------------------------------------------------------------
// MURMUR3_64 (truncated 128-bit hash)
static inline uint64_t rotl64(uint64_t x, int8_t r) {
	return (x << r) | (x >> (64 - r));
}
static inline uint64_t fmix64(uint64_t k) {
	k ^= k >> 33;
	k *= UINT64_C(0xff51afd7ed558ccd);
	k ^= k >> 33;
	k *= UINT64_C(0xc4ceb9fe1a85ec53);
	k ^= k >> 33;

	return k;
}

template<> inline uint64_t hash<hash_e::MURMUR3_64, uint64_t>(uint32_t const  key) {
	uint32_t const  SEED = 0xbaadf00d;
	uint32_t const  LEN  = 4;

	uint64_t h1 = SEED;
	uint64_t h2 = SEED;

	uint64_t c1 = UINT64_C(0x87c37b91114253d5);
	uint64_t c2 = UINT64_C(0x4cf5ad432745937f);

	uint64_t k1 = key;
	k1 *= c1;
	k1  = rotl64(k1,31);
	k1 *= c2;
	h1 ^= k1;

	// finalization
	h1 ^= LEN;
	h2 ^= LEN;

	h1 += h2;
	h2 += h1;

	h1 = fmix64(h1);
	h2 = fmix64(h2);

	h1 += h2;
	h2 += h1;

	return  h2;
}

//---------------------------------------------------------------------------
// SIP
template<> inline uint64_t hash<hash_e::SIP, uint64_t>(uint32_t const  key) {
#define SIPROUND \
	do {                   \
	    v0 += v1;          \
	    v1 = rotl64(v1, 13); \
	    v1 ^= v0;          \
	    v0 = rotl64(v0, 32); \
	    v2 += v3;          \
	    v3 = rotl64(v3, 16); \
	    v3 ^= v2;          \
	    v0 += v3;          \
	    v3 = rotl64(v3, 21); \
	    v3 ^= v0;          \
	    v2 += v1;          \
	    v1 = rotl64(v1, 17); \
	    v1 ^= v2;          \
	    v2 = rotl64(v2, 32); \
	} while (0)

  static unsigned const  cROUNDS = 2;
  static unsigned const  dROUNDS = 4;

  uint64_t  v0 = UINT64_C(0x736f6d6570736575);
  uint64_t  v1 = UINT64_C(0x646f72616e646f6d);
  uint64_t  v2 = UINT64_C(0x6c7967656e657261);
  uint64_t  v3 = UINT64_C(0x7465646279746573);
  uint64_t  b = (UINT64_C(8)<<56) | key;
  v3 ^= b;
  for(unsigned i = 0; i < cROUNDS; i++)  SIPROUND;
  v0 ^= b;
  v2 ^= 0xFF;
  for(unsigned i = 0; i < dROUNDS; i++)  SIPROUND;
  return  v0^v1^v1^v2;
#undef SIPROUND
}

//---------------------------------------------------------------------------
// Exported Collection Functions
static inline unsigned clz_nz(uint32_t const  x) { return __builtin_clz(x); }
static inline unsigned clz_nz(uint64_t const  x) { return __builtin_clzl(x); }

template<hash_e HASH, typename T>
static inline void hll_collect_base(uint32_t const *data, size_t const num_items, unsigned *buckets, unsigned const p_val) {
	unsigned const rest_c = 8*sizeof(T) - p_val;
	for(size_t i = 0; i < num_items; i++) {
		T  const  hashv  = hash<HASH, T>(data[i]);
		unsigned *const  bucket = &buckets[hashv >> rest_c];
		unsigned  const  lzcnt  = clz_nz(((hashv+1)<<p_val)-1);
		if(__builtin_expect(lzcnt >= *bucket, 0)) *bucket = lzcnt + 1;
	}
}
template<hash_e HASH, typename T>
static inline void hll_collect_base(std::function<uint32_t()> src, size_t const num_items, unsigned *buckets, unsigned const p_val) {
	unsigned const rest_c = 8*sizeof(T) - p_val;
	for(size_t i = 0; i < num_items; i++) {
		T const  hashv = hash<HASH, T>(src());
		unsigned *const  bucket = &buckets[hashv >> rest_c];
		unsigned  const  lzcnt  = clz_nz(((hashv+1)<<p_val)-1);
		if(__builtin_expect(lzcnt >= *bucket, 0)) *bucket = lzcnt + 1;
	}
}

#define IMPLEMENT(HASH, W) \
template<> \
void hll_collect_ptr<hash_e::HASH>(uint32_t const *data, size_t const num_items, unsigned *buckets, unsigned const p_val) { \
	hll_collect_base<hash_e::HASH, uint##W##_t>(data, num_items, buckets, p_val); \
} \
template<> \
void hll_collect_fct<hash_e::HASH>(std::function<uint32_t()> src, size_t const num_items, unsigned *buckets, unsigned const p_val) { \
	hll_collect_base<hash_e::HASH, uint##W##_t>(src, num_items, buckets, p_val); \
}

IMPLEMENT(IDENT,		32)
IMPLEMENT(SIP,			64)
IMPLEMENT(MURMUR3_32,	32)
IMPLEMENT(MURMUR3_64,	64)
#undef IMPLEMENT
