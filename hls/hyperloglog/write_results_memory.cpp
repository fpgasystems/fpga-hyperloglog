#include "hyperloglog.hpp"

void write_results_memory(
		hls::stream<float> & cardinality,
        hls::stream<memCmd>&   m_axis_write_cmd,
        hls::stream<hll_out >&  m_axis_write_data,
        ap_uint<64>  regBaseAddr
		)
{
#pragma HLS PIPELINE II=1
#pragma HLS INLINE off

	float  tempData;
	hll_out  dataIn;

    if(!cardinality.empty()){
        tempData = cardinality.read();
        dataIn.data = tempData;
		dataIn.keep = 0xF;
		dataIn.last = 0x1;        
        m_axis_write_cmd.write(memCmd(regBaseAddr, 4));
        m_axis_write_data.write(dataIn);
    }          
}
