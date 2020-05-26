#include "hyperloglog.hpp"
#include "globals.hpp"
void divide_data(
				  hls::stream<net_axis<line_width> >& rawDataIn,
				  hls::stream<dataItem<32> >	dataFifoOut[NUM_PIPELINES]
				  )
{
#pragma HLS PIPELINE II=1
#pragma HLS INLINE off

	if (!rawDataIn.empty())
	{
		net_axis<line_width> currentInput = rawDataIn.read();

		for (int i = 0; i < line_width/32; i++)
		{
			#pragma HLS UNROLL
			if(currentInput.keep(i*4+3,i*4) == 0xF){
				dataFifoOut[i].write(dataItem<32>(currentInput.data(i*32+31, i*32), 1, currentInput.last));
			}else {
				dataFifoOut[i].write(dataItem<32>(0, 0, currentInput.last));
			}

		}

	}

}
