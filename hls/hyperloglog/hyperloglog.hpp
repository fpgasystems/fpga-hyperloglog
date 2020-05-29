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

#pragma once

#include "../axi_utils.hpp"
#include "../mem_utils.hpp"

#include "globals.hpp"

#include "pipeline.hpp"
#include "estimate_cardinality.hpp"

const unsigned NUM_PIPELINES = 16;

struct aggrOutput
{
	rank_t	value;
	ap_uint<1>	last;
	aggrOutput() {}
	aggrOutput(rank_t value, ap_uint<1> last)
		:value(value), last(last) {}
};


//forward declarations
void aggr_bucket(
	hls::stream<rank_t> numZerosFifo[NUM_PIPELINES],
	hls::stream<aggrOutput>& zeros
);


void divide_data(
		  hls::stream<net_axis<line_width> >& line_data,
		  hls::stream<dataItem<32> > data[NUM_PIPELINES]
		  	  	 );

void zero_counter(
	hls::stream<aggrOutput>&	buck_val_in,
	hls::stream<rank_t>&		buck_val_out,
	hls::stream<bucket_cnt_t>&	zero_counter
);

void accumulate (
	hls::stream<rank_t>&		buck_val,
	hls::stream<float>&			accm,
	hls::stream<ap_uint<1> >&	accm_done
);

		
void write_results_memory(
		hls::stream<float> & cardniality,
        hls::stream<memCmd>&   m_axis_write_cmd,
        hls::stream<hll_out >&  m_axis_write_data,
        ap_uint<64>  regBaseAddr
				);



void hyperloglog(
		hls::stream<net_axis<line_width> > & s_axis_input_tuple,
		hls::stream<memCmd>&  m_axis_write_cmd,
		hls::stream<hll_out>&  m_axis_write_data,
		ap_uint<64>   regBaseAddr
				);
