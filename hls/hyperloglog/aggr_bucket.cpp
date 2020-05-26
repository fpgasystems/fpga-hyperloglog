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
