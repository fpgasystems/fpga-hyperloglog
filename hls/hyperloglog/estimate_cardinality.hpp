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

#ifndef ESTIMATE_CARDINALITY_HPP
#define ESTIMATE_CARDINALITY_HPP

#include <cmath>

template<int HASH_SIZE>
void estimate_cardinality(
						hls::stream<float>& accm,
						hls::stream<ap_uint<17> >& zero_count,
						hls::stream<float>& cardinality,
						hls::stream<ap_uint<1> >& done
						)
{
#pragma HLS PIPELINE II=1
#pragma HLS INLINE off
	static float const	TWO_POWER_32 = 4294967296.f;
	static float const	alpha = 0.7213/(1+(1.079/num_buckets_m)); //for m >= 128, number of buckets

	if(!done.empty() && !zero_count.empty() && !accm.empty() ){
		ap_uint<1>	const	done_val	= done.read();
		ap_uint<17>	const	count_local	= zero_count.read();
		float		const	summation	= accm.read();

		if(done_val){
			//Conditioning the raw cardinality
			float const  raw_cardinality = (alpha * num_buckets_m * num_buckets_m) / summation;

			//estimated_cardinality = raw_cardinality;
			float estimated_cardinality;
			if (raw_cardinality <= 2.5*num_buckets_m){
				// Linear counting
				if(count_local != 0){
					estimated_cardinality = num_buckets_m*logf((float)num_buckets_m / (float)count_local);
				}
				else{
					estimated_cardinality = raw_cardinality;
				}
			}
			else if((HASH_SIZE < 36) && (raw_cardinality > 143165576.6)) {
				estimated_cardinality = (-TWO_POWER_32)*logf(1 -(raw_cardinality / TWO_POWER_32));
			}
			else {
				estimated_cardinality = raw_cardinality;
			}
			cardinality.write(estimated_cardinality);
		}
	}
}
#endif
