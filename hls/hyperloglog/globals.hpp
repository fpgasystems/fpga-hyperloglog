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

#ifndef GLOBALS_HPP_
#define GLOBALS_HPP_

#include <ap_int.h>
#include "../bit_utils.hpp"

#define line_width 512

//---------------------------------------------------------------------------
// Design Geometry

// Length of Hash (currently supported values: 32, 64, 128)
unsigned const HASH_SIZE =  64;

// Prefix bits for bucket selection (practical values: 14, 15, 16)
unsigned const BUCKET_BITS = 16;

// Derived Constants
unsigned const num_buckets_m = (1 << BUCKET_BITS);

//---------------------------------------------------------------------------
// Types
typedef ap_uint<1>	uint_1_t;	// flags

/** Count of leading zeros - [0:HASH_SIZE-BUCKET_BITS] */
typedef ap_uint<btl::clog2<HASH_SIZE-BUCKET_BITS+1>::value> lzcnt_t;
/** Rank - [0:HASH_SIZE-BUCKET_BITS+1] */
typedef ap_uint<btl::clog2<HASH_SIZE-BUCKET_BITS+2>::value> rank_t;

/** Bucket ID: [0:2**BUCKET_BITS-1] */
typedef ap_uint<BUCKET_BITS>	bucket_id_t;
/** Bucket Count: [0:2**BUCKET_BITS] */
typedef ap_uint<BUCKET_BITS+1>	bucket_cnt_t;

template <int W>
struct dataItem
{
	ap_uint<W>	data;
	ap_uint<1>	valid;
	ap_uint<1>	last;
	dataItem() {}
	dataItem(ap_uint<W> data, ap_uint<1> valid, ap_uint<1> last)
		:data(data), valid(valid), last(last) {}
};

struct bucketMeta
{
	bucket_id_t	bucketNum;
	lzcnt_t		numZeros;
	ap_uint<1>	valid;
	ap_uint<1>	last;
	bucketMeta() {}
	bucketMeta(bucket_id_t bucketNum, lzcnt_t numZeros, ap_uint<1> valid, ap_uint<1> last)
		:bucketNum(bucketNum), numZeros(numZeros), valid(valid), last(last) {}
};


struct hll_out{
  float     	data;
  ap_uint<4> 		keep;
  ap_uint<1>       	last;
};


#endif // GLOBALS_H_ not defined

