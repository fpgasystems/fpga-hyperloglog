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
