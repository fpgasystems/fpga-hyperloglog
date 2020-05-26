#include "globals.hpp"
#include "../axi_utils.hpp"
#include "hyperloglog.hpp"

void zero_counter(
	hls::stream<aggrOutput>&	buck_val_in,
	hls::stream<rank_t>&		buck_val_out,
	hls::stream<bucket_cnt_t>&	zero_counter
) {

	#pragma HLS PIPELINE II=1
	#pragma HLS INLINE off

	static bucket_cnt_t zero_count = 0;

	if(!buck_val_in.empty()){
		aggrOutput data_in = buck_val_in.read();
		if(data_in.value==0){
			zero_count++;
		}
		buck_val_out.write(data_in.value);
		zero_counter.write(zero_count);
		if (data_in.last) {
			zero_count = 0;
		}
	}
}
