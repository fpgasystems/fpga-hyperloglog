#include "globals.hpp"

template <int DUMMY>
void fill_bucket(
	hls::stream<bucketMeta>&	bucketMetaFifoIn,
	hls::stream<rank_t>&		bucket_stream_fifo
) {
#pragma HLS PIPELINE II=1
#pragma HLS INLINE off

static rank_t buckets[num_buckets_m];
#pragma HLS RESOURCE variable=buckets core=RAM_T2P_BRAM
#pragma HLS DEPENDENCE variable=buckets inter false

	static bucket_cnt_t i = 0;
	//Readout the buckets if the hls::hls::streaming is done
	// This is a statemachine between filling and readout state.
	//Once the filling the buckets is done, the state is switched to the readout state.
	static enum fState {fill = 0, readout} state;

	static bucket_id_t prev_bucketNum  = 0;
	static bucket_id_t prev_prev_bucketNum  = 0;
	static bucket_id_t prev_prev_prev_bucketNum  = 0;
	static bucket_id_t prev_prev_prev_prev_bucketNum  = 0;

	static  rank_t	prev_rank = 0;
	static  rank_t	prev_prev_rank = 0;
	static  rank_t	prev_prev_prev_rank = 0;
	static  rank_t	prev_prev_prev_prev_rank = 0;

    switch(state){
		case fill:
			if (!bucketMetaFifoIn.empty()){

				bucketMeta	const	meta = bucketMetaFifoIn.read();
				rank_t		const	rank = meta.numZeros + 1;

				/* Handling the dependency -- Start*/
				rank_t	current_rank;
				if (meta.bucketNum == prev_bucketNum) {
					current_rank = prev_rank;
				}
				else if (meta.bucketNum == prev_prev_bucketNum) {
					current_rank = prev_prev_rank;
				}
				else if (meta.bucketNum == prev_prev_prev_bucketNum) {
					current_rank = prev_prev_prev_rank;
				}
				else if (meta.bucketNum == prev_prev_prev_prev_bucketNum) {
					current_rank = prev_prev_prev_prev_rank;
				}
				else {
					current_rank = buckets[meta.bucketNum];
				}
				prev_prev_prev_prev_rank = prev_prev_prev_rank;
				prev_prev_prev_rank = prev_prev_rank;
				prev_prev_rank = prev_rank;

				if (rank > current_rank) {
					buckets[meta.bucketNum] = rank;
					prev_rank = rank;
				}
				prev_prev_prev_prev_bucketNum = prev_prev_prev_bucketNum;
				prev_prev_prev_bucketNum = prev_prev_bucketNum;
				prev_prev_bucketNum = prev_bucketNum;
				prev_bucketNum = meta.bucketNum;
				/* Handling the dependency -- End*/

				if(meta.last){
					state = readout;
				}
			}
			else{
					state = fill;
				}
			break;



//    case fill:
//     if (!bucketMetaFifoIn.empty()){
//      bucketMeta meta = bucketMetaFifoIn.read();
//      rank = meta.numZeros+1;
//      if (meta.valid) {
//        if (rank > buckets[meta.bucketNum]) {
//          buckets[meta.bucketNum] = rank;
////          std::cout << "Rank = " << rank << std::endl;
////          std::cout << "buckets[meta.bucketNum] = " << buckets[meta.bucketNum] << std::endl;
////          std::cout << "meta.bucketNum = " << meta.bucketNum << std::endl;
//        }
//      }
//      if(meta.last){
//        state = readout;
//      }
//      }
//      else{
//        state = fill;
//        }
//      break;


    case readout:
		rank_t const	buckVal = buckets[i];
		bucket_stream_fifo.write(buckVal);
		buckets[i] = 0;
		i++;
		if(i == num_buckets_m){
			i = 0;
    		state = fill;
		}
		break;
    }
}
