#include "../mem_utils.hpp"

void fetch_tuples(  hls::stream<ap_uint<96> >&  s_axis_param,
                    hls::stream<memCmd>&        m_axis_dma_cmd);

void carmen_tx(hls::stream<ap_uint<96> >&      s_axis_param,
               hls::stream<memCmd>&            m_axis_dma_read_cmd);