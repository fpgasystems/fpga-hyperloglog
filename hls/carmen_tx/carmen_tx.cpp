/**
 * Copyright (c) 2020, Systems Group, ETH Zurich
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
