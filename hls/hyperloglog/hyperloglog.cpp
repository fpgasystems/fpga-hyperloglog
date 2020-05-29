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

void hyperloglog(
		hls::stream<net_axis<line_width> > & s_axis_input_tuple,
        hls::stream<memCmd>&  m_axis_write_cmd,
        hls::stream<hll_out>&  m_axis_write_data,
        ap_uint<64>   regBaseAddr){

#pragma HLS INLINE off
#pragma HLS DATAFLOW

#pragma HLS INTERFACE axis port=s_axis_input_tuple
#pragma HLS INTERFACE axis port=m_axis_write_cmd
#pragma HLS INTERFACE axis port=m_axis_write_data
#pragma HLS INTERFACE ap_stable register port=regBaseAddr

#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS DATA_PACK variable=m_axis_write_cmd

    static hls::stream<aggrOutput> aggr_out;
#pragma HLS stream depth=8 variable=aggr_out
#pragma HLS DATA_PACK variable=aggr_out

    static hls::stream<float>  accm;
#pragma HLS stream depth=8 variable=accm

    static hls::stream<uint_1_t> done_accm;
#pragma HLS stream depth=8 variable=done_accm

	static hls::stream<rank_t> numzeros_out;
#pragma HLS stream depth=8 variable=numzeros_out

    static hls::stream<bucket_cnt_t> zero_count;
#pragma HLS stream depth=8 variable=zero_count

    static hls::stream<uint_1_t> done_out;
#pragma HLS stream depth=8 variable=done_out

	static hls::stream<dataItem<32> > dataFifo[NUM_PIPELINES];
	#pragma HLS stream depth=8 variable=dataFifo	
	#pragma HLS DATA_PACK variable=dataFifo
	

	static hls::stream<rank_t> bucket_fifo[NUM_PIPELINES];
	#pragma HLS stream depth=8 variable=bucket_fifo
	

	static hls::stream<float> card_temp;
	#pragma HLS stream depth=2 variable=card_temp

	divide_data (
			s_axis_input_tuple,
			dataFifo);

	pipeline<0>(dataFifo[0], bucket_fifo[0]);
	pipeline<1>(dataFifo[1], bucket_fifo[1]);
	pipeline<2>(dataFifo[2], bucket_fifo[2]);
	pipeline<3>(dataFifo[3], bucket_fifo[3]);
	pipeline<4>(dataFifo[4], bucket_fifo[4]);
	pipeline<5>(dataFifo[5], bucket_fifo[5]);
	pipeline<6>(dataFifo[6], bucket_fifo[6]);
	pipeline<7>(dataFifo[7], bucket_fifo[7]);
	pipeline<8>(dataFifo[8], bucket_fifo[8]);
	pipeline<9>(dataFifo[9], bucket_fifo[9]);
	pipeline<10>(dataFifo[10], bucket_fifo[10]);
	pipeline<11>(dataFifo[11], bucket_fifo[11]);
	pipeline<12>(dataFifo[12], bucket_fifo[12]);
	pipeline<13>(dataFifo[13], bucket_fifo[13]);
	pipeline<14>(dataFifo[14], bucket_fifo[14]);
	pipeline<15>(dataFifo[15], bucket_fifo[15]);

	aggr_bucket(bucket_fifo,
					aggr_out);

		//count zeros in the bucket
		zero_counter(aggr_out,
						 numzeros_out,
						 zero_count);

		accumulate (numzeros_out,
				accm,
				done_accm);

		//cardinality estimation
		estimate_cardinality<HASH_SIZE>(accm,
				zero_count,
				card_temp,
				done_accm);

		write_results_memory(card_temp,
				m_axis_write_cmd,
				m_axis_write_data,
				regBaseAddr);

}
