#include "../mem_utils.hpp"
#include "carmen_tx.hpp"

void fetch_tuples(  hls::stream<ap_uint<96> >&  s_axis_param,
                    hls::stream<memCmd>&        m_axis_dma_cmd)
{
    #pragma HLS PIPELINE II=1
    #pragma HLS INLINE off

    static ap_uint<64> address;
    static ap_uint<32> length;


    if (!s_axis_param.empty())
    {
        ap_uint<96> temp = s_axis_param.read();
        address = temp(63, 0);
        length = temp(95, 64);
        m_axis_dma_cmd.write(memCmd(address, length));
    }
}

void carmen_tx(hls::stream<ap_uint<96> >&      s_axis_param,
               hls::stream<memCmd>&            m_axis_dma_read_cmd)
{
    #pragma HLS DATAFLOW
    #pragma HLS INTERFACE ap_ctrl_none register port=return

    #pragma HLS resource core=AXI4Stream variable=s_axis_param metadata="-bus_bundle s_axis_param"
    #pragma HLS resource core=AXI4Stream variable=m_axis_dma_read_cmd metadata="-bus_bundle m_axis_dma_read_cmd"
    #pragma HLS DATA_PACK variable=m_axis_dma_read_cmd

    fetch_tuples(s_axis_param, m_axis_dma_read_cmd);

}
