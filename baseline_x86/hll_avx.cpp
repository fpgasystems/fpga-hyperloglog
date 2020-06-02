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
#include <type_traits>

#include <xmmintrin.h>
#include <immintrin.h>

//===========================================================================
// Overloaded AVX Helpers for unified type differentiation

// Load / Store
static __m256i const ls_mask = _mm256_set1_epi32(0xFFFFFFFF);
static inline __m256i load_avx(int32_t const *const  src) { return _mm256_maskload_epi32(src, ls_mask); }
static inline __m256i load_avx(int64_t const *const  src) { return _mm256_maskload_epi64((long long const*)src, ls_mask); }
static inline void store_avx(int32_t *const  dst, __m256i const  val) { _mm256_maskstore_epi32(dst, ls_mask, val); }
static inline void store_avx(int64_t *const  dst, __m256i const  val) { _mm256_maskstore_epi64((long long*)dst, ls_mask, val); }

// Right Shift by Scalar
template<typename T>
static inline __m256i srli_avx(__m256i const  val, int s);
template<> inline __m256i srli_avx<uint32_t>(__m256i const  v, int const  s) { return _mm256_srli_epi32(v, s); }
template<> inline __m256i srli_avx<uint64_t>(__m256i const  v, int const  s) { return _mm256_srli_epi64(v, s); }

//===========================================================================
// Hashes

template<typename T>
static inline __m256i murmur3(uint32_t const *keys);

//---------------------------------------------------------------------------
// MURMUR3 32-bit
template<>
inline __m256i murmur3<uint32_t>(uint32_t const *keys) {
    const int seed = 42;
    const __m256i len = _mm256_set1_epi32(4);
    const __m256i c1 = _mm256_set1_epi32(0xcc9e2d51);
    const __m256i c2 = _mm256_set1_epi32(0x1b873593);
    const __m256i c3 = _mm256_set1_epi32(5);
    const __m256i c4 = _mm256_set1_epi32(0xe6546b64);
    const __m256i c5 = _mm256_set1_epi32(0x85ebca6b);
    const __m256i c6 = _mm256_set1_epi32(0xc2b2ae35);

    const int left_shift1 = 15;
    const int right_shift1 = 32-left_shift1;
    const int left_shift2 = 13;
    const int right_shift2 = 32-left_shift2;

    __m256i h1 = _mm256_set1_epi32 (seed);

    __m256i k1_t = load_avx(reinterpret_cast<int32_t const*>(keys));
    k1_t = _mm256_mullo_epi32(k1_t, c1);
    k1_t = _mm256_or_si256(_mm256_slli_epi32 (k1_t, left_shift1), _mm256_srli_epi32(k1_t, right_shift1));
    k1_t = _mm256_mullo_epi32(k1_t, c2);
    h1 = _mm256_xor_si256(h1, k1_t);
    h1 = _mm256_or_si256(_mm256_slli_epi32 (h1, left_shift2), _mm256_srli_epi32(h1, right_shift2));
    h1 = _mm256_add_epi32(_mm256_mullo_epi32(h1, c3), c4);

    h1 = _mm256_xor_si256(h1, len);
    h1 = _mm256_xor_si256(h1, _mm256_srli_epi32(h1, 16));
    h1 = _mm256_mullo_epi32(h1, c5);
    h1 = _mm256_xor_si256(h1, _mm256_srli_epi32(h1, 13));
    h1 = _mm256_mullo_epi32(h1, c6);
    h1 = _mm256_xor_si256(h1, _mm256_srli_epi32(h1, 16));

    return h1;
}

static inline void leading_zero_avx(int32_t *const  dst, __m256i  x, unsigned const  p_val) {
    // Strip Bucket Selector
    __m256i const  fill_ones = _mm256_set1_epi32((1 << p_val)-1);
    x = _mm256_slli_epi32(x, p_val);
    x = _mm256_or_si256(x, fill_ones);

#if defined(__AVX512VL__) || defined(__AVX512CD__)
    // AVX512 supports the leading-zero count natively
    store_avx(dst, _mm256_lzcnt_epi32(x));
#elif 1
    // Scalar native leading-zero count
    for(unsigned i = 0; i < 8; i++)  dst[i] = __builtin_clz(_mm256_extract_epi32(x, i));
#else
    __m256i const threshold16 = _mm256_set1_epi32 (0x0000ffff);
    __m256i const threshold8 = _mm256_set1_epi32 (0x00ffffff);
    __m256i const threshold4 = _mm256_set1_epi32 (0x0fffffff);
    __m256i const threshold2 = _mm256_set1_epi32 (0x3fffffff);
    __m256i const zeros = _mm256_set1_epi32 (0);

    // Logarithmic Decent Consuming Zero Prefixes
    __m256i n;

    // Length 16
    __m256i const  msk16 = _mm256_cmpeq_epi32(_mm256_andnot_si256(threshold16, x), zeros);
    __m256i const  res16 = _mm256_and_si256(msk16, _mm256_set1_epi32(16));
    x = _mm256_sllv_epi32(x, res16);
    n = res16;

    // Length 8
    __m256i const  msk8 = _mm256_cmpeq_epi32(_mm256_andnot_si256(threshold8, x), zeros);
    __m256i const  res8 = _mm256_and_si256(msk8, _mm256_set1_epi32(8));
    x = _mm256_sllv_epi32(x, res8);
    n = _mm256_add_epi32(n, res8);

    // Length 4
    __m256i const  msk4 = _mm256_cmpeq_epi32(_mm256_andnot_si256(threshold4, x), zeros);
    __m256i const  res4 = _mm256_and_si256(msk4, _mm256_set1_epi32(4));
    x = _mm256_sllv_epi32(x, res4);
    n = _mm256_add_epi32(n, res4);

    // Length 2
    __m256i const  msk2 = _mm256_cmpeq_epi32(_mm256_andnot_si256(threshold2, x), zeros);
    __m256i const  res2 = _mm256_and_si256(msk2, _mm256_set1_epi32(2));
    x = _mm256_sllv_epi32(x, res2);
    n = _mm256_add_epi32(n, res2);

    // Length 1
    __m256i const  msk1 = _mm256_cmpgt_epi32(zeros, x);
    __m256i const  res1 = _mm256_andnot_si256(msk1, _mm256_set1_epi32(1));
    n = _mm256_add_epi32(n, res1);

    store_avx(dst, n);
#endif
} 

//---------------------------------------------------------------------------
// MURMUR3 64-bit
static inline __m256i multiply_epi64(__m256i const  in1, __m256i const  in2) {
	__m256i const  mask_0_FFFFFFFF = _mm256_set1_epi64x(0x00000000FFFFFFFF);
	__m256i const  mask_FFFFFFFF_0 = _mm256_set1_epi64x(0xFFFFFFFF00000000);

    __m256i const  in1_low = _mm256_and_si256(in1, mask_0_FFFFFFFF);
    __m256i const  in1_high = _mm256_srli_epi64(_mm256_and_si256(in1, mask_FFFFFFFF_0), 32);
    __m256i const  in2_low = _mm256_and_si256(in2, mask_0_FFFFFFFF);
    __m256i const  in2_high = _mm256_srli_epi64 (_mm256_and_si256(in2, mask_FFFFFFFF_0), 32);
    __m256i const  low_low = _mm256_mul_epu32(in1_low, in2_low);
    __m256i const  high_low = _mm256_slli_epi64(_mm256_mul_epu32(in1_high, in2_low), 32);
    __m256i const  low_high = _mm256_slli_epi64(_mm256_mul_epu32(in1_low, in2_high), 32);
    
    __m256i const  result = _mm256_add_epi64(high_low, low_low);
    return  _mm256_add_epi64(result, low_high);
}

static inline __m256i rotl31_epi64(__m256i const  in) {
    return _mm256_or_si256(_mm256_slli_epi64(in, 31), _mm256_srli_epi64(in, 33));
}

static inline __m256i fmix64_epi64(__m256i const  in) {
	__m256i const  c3 = _mm256_set1_epi64x (0xff51afd7ed558ccd);
	__m256i const  c4 = _mm256_set1_epi64x (0xc4ceb9fe1a85ec53);

    __m256i k = in;
    k = _mm256_xor_si256(k, _mm256_srli_epi64(k, 33));
    k = multiply_epi64(k, c3);
    k = _mm256_xor_si256(k, _mm256_srli_epi64(k, 33));
    k = multiply_epi64(k, c4);
    k = _mm256_xor_si256(k, _mm256_srli_epi64(k, 33));
    return k;
}

template<>
inline __m256i murmur3<uint64_t>(uint32_t const *keys) {
    __m256i const  seed = _mm256_set1_epi64x(0x00000000baadf00d);
	__m256i const  c1   = _mm256_set1_epi64x(0x87c37b91114253d5);
	__m256i const  c2   = _mm256_set1_epi64x(0x4cf5ad432745937f);
	__m256i const  len  = _mm256_set1_epi64x(4);

    __m256i k1 = _mm256_cvtepu32_epi64(_mm_maskload_epi32(reinterpret_cast<int32_t const*>(keys), _mm_set1_epi32(0xFFFFFFFF)));;

    __m256i h1 = seed;
    __m256i h2 = seed;

    k1 = multiply_epi64(k1, c1);
    k1 = rotl31_epi64(k1);
    k1 = multiply_epi64(k1, c2);
    h1 = _mm256_xor_si256(h1, k1);

    h1 = _mm256_xor_si256(h1, len);
    h2 = _mm256_xor_si256(h2, len);
    h1 = _mm256_add_epi64(h1, h2);
    h2 = _mm256_add_epi64(h2, h1);

    h1 = fmix64_epi64(h1);
    h2 = fmix64_epi64(h2);
    h1 = _mm256_add_epi64(h1, h2);
    h2 = _mm256_add_epi64(h2, h1);

    return  h2;
}

static inline void leading_zero_avx(int64_t *dst, __m256i x, unsigned const  p_val) {
     // Strip Bucket Selector
    const __m256i fill_ones = _mm256_set1_epi64x((1 << p_val)-1);
    x = _mm256_slli_epi64(x, p_val);
    x = _mm256_or_si256(x, fill_ones);

#if defined(__AVX512VL__) || defined(__AVX512CD__)
    // Native AVX512 support for leading-zero count
    store_avx(dst, _mm256_lzcnt_epi64(x));
#elif 1
    // Scalar native leading-zero count
    for(unsigned i = 0; i < 4; i++)  dst[i] = __builtin_clzl(_mm256_extract_epi64(x, i));
#else
    // Logarithmic Decent Consuming Zero Prefixes
    const __m256i threshold32 = _mm256_set1_epi64x (0x00000000ffffffff);
    const __m256i threshold16 = _mm256_set1_epi64x (0x0000ffffffffffff);
    const __m256i threshold8 = _mm256_set1_epi64x (0x00ffffffffffffff);
    const __m256i threshold4 = _mm256_set1_epi64x (0x0fffffffffffffff);
    const __m256i threshold2 = _mm256_set1_epi64x (0x3fffffffffffffff);
    const __m256i threshold1 = _mm256_set1_epi64x (0x7fffffffffffffff);
    const __m256i zeros = _mm256_set1_epi64x (0);

    __m256i n;

    // Length 32
    __m256i const  msk32 = _mm256_cmpeq_epi64(_mm256_andnot_si256(threshold32, x), zeros);
    __m256i const  res32 = _mm256_and_si256(msk32, _mm256_set1_epi64x(32));
    x = _mm256_sllv_epi64(x, res32);
    n = res32;

    // Length 16
    __m256i const  msk16 = _mm256_cmpeq_epi64(_mm256_andnot_si256(threshold16, x), zeros);
    __m256i const  res16 = _mm256_and_si256(msk16, _mm256_set1_epi64x(16));
    x = _mm256_sllv_epi64(x, res16);
    n = _mm256_add_epi64(n, res16);

    // Length 8
    __m256i const  msk8 = _mm256_cmpeq_epi64(_mm256_andnot_si256(threshold8, x), zeros);
    __m256i const  res8 = _mm256_and_si256(msk8, _mm256_set1_epi64x(8));
    x = _mm256_sllv_epi64(x, res8);
    n = _mm256_add_epi64(n, res8);

    // Length 4
    __m256i const  msk4 = _mm256_cmpeq_epi64(_mm256_andnot_si256(threshold4, x), zeros);
    __m256i const  res4 = _mm256_and_si256(msk4, _mm256_set1_epi64x(4));
    x = _mm256_sllv_epi64(x, res4);
    n = _mm256_add_epi64(n, res4);

    // Length 2
    __m256i const  msk2 = _mm256_cmpeq_epi64(_mm256_andnot_si256(threshold2, x), zeros);
    __m256i const  res2 = _mm256_and_si256(msk2, _mm256_set1_epi64x(2));
    x = _mm256_sllv_epi64(x, res2);
    n = _mm256_add_epi64(n, res2);

    // Length 1
    __m256i const  msk1 = _mm256_cmpgt_epi64(zeros, x);
    __m256i const  res1 = _mm256_andnot_si256(msk1, _mm256_set1_epi64x(1));
    n = _mm256_add_epi64(n, res1);

    store_avx(dst, n);
#endif
} 

//---------------------------------------------------------------------------
// Exports
template<typename T>
static inline void hll_collect_avx(uint32_t const *data, size_t const num_items, unsigned *buckets, unsigned const p_val) {
	typedef typename std::make_signed<T>::type TS;
	unsigned const N = 32/sizeof(T);
    unsigned const rest_c = 8*sizeof(T) - p_val;

    TS buckid[N];
    TS lzcnts[N];
    for(size_t i = 0; i < num_items; i += N) {
        __m256i const  hashes = murmur3<T>(&data[i]);
        store_avx(buckid, srli_avx<T>(hashes, rest_c));
        leading_zero_avx(lzcnts, hashes, p_val);
        for (unsigned j = 0; j < N; j++) {
            unsigned *const  bucket = &buckets[buckid[j]];
            unsigned  const  lzcnt  = lzcnts[j];
            if(__builtin_expect(lzcnt >= *bucket, 0)) *bucket = lzcnt + 1;
        }
    }
}
template<typename T>
static inline void hll_collect_avx(std::function<uint32_t()> src, size_t const num_items, unsigned *buckets, unsigned const p_val) {
	typedef typename std::make_signed<T>::type TS;
	unsigned const N = 32/sizeof(T);
    unsigned const rest_c = 8*sizeof(T) - p_val;

    uint32_t  data[N];
    TS buckid[N];
    TS lzcnts[N];
    for(size_t i = 0; i < num_items; i += N) {
        for(unsigned j = 0; j < N; j++) data[j] = src();
        __m256i const  hashes = murmur3<T>(data);
        store_avx(buckid, srli_avx<T>(hashes, rest_c));
        leading_zero_avx(lzcnts, hashes, p_val);
        for (unsigned j = 0; j < N; j++) {
        	unsigned *const  bucket = &buckets[buckid[j]];
        	unsigned  const  lzcnt  = lzcnts[j];
        	if(__builtin_expect(lzcnt >= *bucket, 0)) *bucket = lzcnt + 1;
        }
    }
}

#define IMPLEMENT(HASH, W) \
template<> \
void hll_collect_ptr<hash_e::HASH>(uint32_t const *data, size_t const num_items, unsigned *buckets, unsigned const p_val) { \
    hll_collect_avx<uint##W##_t>(data, num_items, buckets, p_val); \
} \
template<> \
void hll_collect_fct<hash_e::HASH>(std::function<uint32_t()> src, size_t const num_items, unsigned *buckets, unsigned const p_val) { \
    hll_collect_avx<uint##W##_t>(src, num_items, buckets, p_val); \
}
IMPLEMENT(MURMUR3_32AVX, 32)
IMPLEMENT(MURMUR3_64AVX, 64)
#undef IMPLEMENT
