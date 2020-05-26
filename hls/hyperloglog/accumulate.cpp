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
