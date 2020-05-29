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

#include "globals.hpp"
#include "../axi_utils.hpp"

void accumulate (
	hls::stream<rank_t>&		buck_val,
	hls::stream<float>&			accm,
	hls::stream<ap_uint<1> >&	accm_done
) {

#pragma HLS PIPELINE II=1
#pragma HLS INLINE off
	/**
	 * Exact accumulation until float conversion for output:
	 *
	 * | <- BUCKET_BITS -> . <- HASH_SIZE-BUCKET_BITS+1 -> |
	 *
	 * Numeric Range:
	 *	0.0 .. (2^BUCKET_BITS)*1.0
	 *
	 * All values are represented exact except for the maximum final result
	 * in the case that *all* buckets report a rank of zero, i.e. empty. The
	 * accumulated sum will then saturate to one ulp lower.
	 */
	typedef ap_ufixed<HASH_SIZE+1, BUCKET_BITS, AP_RND_ZERO, AP_SAT> accu_t;

	static accu_t		summation = 0;
	static bucket_cnt_t	count = 0;
	if(!buck_val.empty()){
		rank_t const  rank = buck_val.read();
		accu_t  d = 0;		// d = 2^(-rank)
		d[HASH_SIZE-BUCKET_BITS+1 - rank] = 1;
		summation += d;
		accm.write(summation.to_float());
		count++;
		if(count == num_buckets_m){
			summation = 0;
			count = 0;
			accm_done.write(1);
		}else{
			accm_done.write(0);
		}
	}
}
