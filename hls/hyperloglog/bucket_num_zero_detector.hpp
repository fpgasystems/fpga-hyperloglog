#include "hyperloglog.hpp"

#include "../bit_utils.hpp"

template<int N>
void bz_detector(
		hls::stream<dataItem<N> >&	hashFifoIn,
		hls::stream<bucketMeta>&	bucketFifoOut
){
#pragma HLS PIPELINE II=1
#pragma HLS INLINE off
	unsigned const REST_C = HASH_SIZE - BUCKET_BITS;

	if(!hashFifoIn.empty()) {
		dataItem<N> const  hash_val = hashFifoIn.read();
		bucketFifoOut.write(bucketMeta(
			/* bucket number */	hash_val.data >> REST_C,
			/* leading zeros */	btl::clz(ap_uint<REST_C>(hash_val.data)),
			/* AXI flow */		hash_val.valid,
			/* AXI flow */		hash_val.last
		));
	}
}

