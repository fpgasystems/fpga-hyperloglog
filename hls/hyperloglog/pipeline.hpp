#pragma once

#include "../axi_utils.hpp"

#include "murmur3.hpp"
#include "bucket_num_zero_detector.hpp"
#include "fill_bucket.hpp"

template <int DUMMY>
void pipeline(
	hls::stream<dataItem<32> >&	line_data,
	hls::stream<rank_t>&		bucket_stream_fifo
) {
#pragma HLS INLINE

	static hls::stream<dataItem<HASH_SIZE>> hashFifo;
	#pragma HLS stream depth=2 variable=hashFifo

	static hls::stream<bucketMeta> bucketMetaFifo;
	#pragma HLS stream depth=2 variable=bucketMetaFifo

	murmur3(line_data, hashFifo);

	//extract bucket index and zero detection
	bz_detector(hashFifo, bucketMetaFifo);

	//call the fill_bucket
	fill_bucket<DUMMY>(bucketMetaFifo,
				bucket_stream_fifo);
}
