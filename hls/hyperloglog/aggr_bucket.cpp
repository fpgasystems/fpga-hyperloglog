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

#include "hyperloglog.hpp"
#include "globals.hpp"
//#include <cmath>
#ifdef soft_compare

#include <fstream>
#include <cstdlib> // for exit function
#endif

using namespace std;

using namespace hls;

void aggr_bucket(
	hls::stream<rank_t> numZerosFifo[NUM_PIPELINES],
	hls::stream<aggrOutput>& zeros
) {
#pragma HLS PIPELINE II=1
#pragma HLS INLINE off

	static bucket_cnt_t iter = 0;
	ap_uint<NUM_PIPELINES> emptyMask = ~0;

#ifdef soft_compare
	  ofstream myfile;
	  myfile.open ("../../../bucket_output.txt", std::ios_base::app);
#endif

	//static enum fState_aggr {aggr = 0, zero_readout} state_aggr;
	{
		emptyMask = ~0;
		//Check all FIFOs if they are empty
		for (int i = 0; i < NUM_PIPELINES; i++)
		{
			#pragma HLS UNROLL
			emptyMask[i] = numZerosFifo[i].empty();
		}
		//If all FIFOs are empty read from them and determine the maximum value
		if (emptyMask == 0)
		{
			rank_t maxNumZeros = 0;
			for (int i = 0; i < NUM_PIPELINES; i++)
			{
				#pragma HLS UNROLL
				rank_t const  numZerosTemp = numZerosFifo[i].read();
				if (maxNumZeros < numZerosTemp)
					maxNumZeros = numZerosTemp;
			}
			iter++;
#ifdef soft_compare
			// write the zero value and the index to a file
			  myfile << iter << " " << maxNumZeros <<endl;
			//
#endif
			zeros.write(aggrOutput(maxNumZeros, iter == num_buckets_m));
			if (iter == num_buckets_m) {
				iter = 0;
#ifdef soft_compare

				myfile.close();
#endif
			}
		}
	}

}
