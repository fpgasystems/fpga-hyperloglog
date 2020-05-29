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

#include "hyperloglog_tcp.hpp"


void hll_server(    hls::stream<ap_uint<16> >&      listenPort,
                hls::stream<bool>&              listenPortStatus,
                hls::stream<appNotification>&   notifications,
                hls::stream<appReadRequest>&    readRequest,
                hls::stream<bool>&              isLastPkgFifo,
                hls::stream<dmaMeta>&           dmaMetaFifo,
                ap_uint<32>                     sizeInTuples)
{
#pragma HLS PIPELINE II=1
#pragma HLS INLINE off

   enum listenFsmStateType {OPEN_PORT, WAIT_PORT_STATUS};
   static listenFsmStateType listenState = OPEN_PORT;
    enum consumeFsmStateType {WAIT_PKG, CONSUME};
    static consumeFsmStateType  serverFsmState = WAIT_PKG;
    #pragma HLS RESET variable=listenState

    static ap_uint<32> tupleCounter = 0;

    switch (listenState)
    {
    case OPEN_PORT:
        // Open Port 5001
        listenPort.write(5005);
        listenState = WAIT_PORT_STATUS;
        break;
    case WAIT_PORT_STATUS:
        if (!listenPortStatus.empty())
        {
            bool open = listenPortStatus.read();
            if (!open)
            {
                listenState = OPEN_PORT;
            }
        }
        break;
    }
    
    if (!notifications.empty())
    {
        appNotification notification = notifications.read();

        if (notification.length != 0)
        {
            readRequest.write(appReadRequest(notification.sessionID, notification.length));
            tupleCounter += (notification.length / 4);
            bool isLastPkg = (tupleCounter == sizeInTuples);
            isLastPkgFifo.write(isLastPkg);
            dmaMetaFifo.write(dmaMeta(notification.length, isLastPkg));
            if (isLastPkg)
            {
                tupleCounter = 0;
            }
        }
        
    }

}

void handle_tcp_packet( hls::stream<bool>&              isLastPkgFifo,
                    hls::stream<ap_uint<16> >&      rxMetaData,
                    hls::stream<net_axis<line_width> >& rxData,
                    hls::stream<net_axis<line_width> >& tcp2hllDataFifo,
                    hls::stream<net_axis<line_width> >& tcp2dmaDataFifo)
{
#pragma HLS PIPELINE II=1
#pragma HLS INLINE off

    static ap_uint<1> state = 0;
    static bool isLastPkg = false;

    switch (state)
    {
    case 0:
        if (!rxMetaData.empty() && !isLastPkgFifo.empty())
        {
            ap_uint<16> sessionID = rxMetaData.read();
            isLastPkgFifo.read(isLastPkg);
            state = 1;
        }
        break;
    case 1:
        if (!rxData.empty())
        {
            net_axis<line_width> word = rxData.read();
            tcp2dmaDataFifo.write(word);
            if (word.last)
            {
                word.last = isLastPkg;
                state = 0;
            }
            tcp2hllDataFifo.write(word);
        }
        break;
    }//switch
}

void dma_writer(hls::stream<dmaMeta>&   dmaMetaFifo,
                hls::stream<net_axis<line_width> >& tcp2dmaDataFifo,
                hls::stream<float>&     cardinalityFifo,
                hls::stream<memCmd>&  m_axis_write_cmd,
                hls::stream<net_axis<line_width> >&  m_axis_write_data,
                ap_uint<64>   regBaseAddr)
{
#pragma HLS PIPELINE II=1
#pragma HLS INLINE off

    static ap_uint<2> state = 0;
    static dmaMeta meta;
    static ap_uint<48> offset = 0;

    conversion cardinality;

    switch (state)
    {
    case 0:
        if (!dmaMetaFifo.empty() && !tcp2dmaDataFifo.empty())
        {
            meta = dmaMetaFifo.read();
            net_axis<line_width> word = tcp2dmaDataFifo.read();
            m_axis_write_cmd.write(memCmd(regBaseAddr+offset, meta.length));
            m_axis_write_data.write(word);
            offset += meta.length;

            state = 1;
            if (word.last)
            {
                state = 0;
                if (meta.isLast)
                {
                    state = 2;
                }
            }
        }
        break;
    case 1:
        if (!tcp2dmaDataFifo.empty())
        {
            net_axis<line_width> word = tcp2dmaDataFifo.read();
            m_axis_write_data.write(word);
            if (word.last)
            {
                state = 0;
                if (meta.isLast)
                {
                    state = 2;
                }
            }
        }
        break;
    case 2:
        if (!cardinalityFifo.empty())
        {
            cardinality.f = cardinalityFifo.read();
            net_axis<line_width> resultWord;
            resultWord.data(31, 0) = cardinality.value;
            resultWord.keep = 0xF;
            resultWord.last = 1;
            m_axis_write_cmd.write(memCmd(regBaseAddr+offset, 4));
            m_axis_write_data.write(resultWord);
            state = 0;
            offset = 0;
        }
        break;
    }//switch

}

void hyperloglog_tcp(
        hls::stream<memCmd>&  m_axis_write_cmd,
        hls::stream<net_axis<line_width> >&  m_axis_write_data,
        ap_uint<64>   regBaseAddr,
        ap_uint<32>   sizeInTuples,
        //TCP interface
        hls::stream<ap_uint<16> >& listenPort,
        hls::stream<bool>& listenPortStatus,
        hls::stream<appNotification>& notifications,
        hls::stream<appReadRequest>& readRequest,
        hls::stream<ap_uint<16> >& rxMetaData,
        hls::stream<net_axis<line_width> >& rxData
        ){

#pragma HLS DATAFLOW disable_start_propagation
#pragma HLS INTERFACE ap_ctrl_none port=return

    #pragma HLS INTERFACE axis port=m_axis_write_cmd
    #pragma HLS INTERFACE axis port=m_axis_write_data
    #pragma HLS DATA_PACK variable=m_axis_write_cmd

    #pragma HLS INTERFACE ap_stable register port=regBaseAddr
    #pragma HLS INTERFACE ap_stable register port=sizeInTuples

    #pragma HLS INTERFACE axis register port=listenPort name=m_axis_listen_port
    #pragma HLS INTERFACE axis register port=listenPortStatus name=s_axis_listen_port_status

    #pragma HLS INTERFACE axis register port=notifications name=s_axis_notifications
    #pragma HLS INTERFACE axis register port=readRequest name=m_axis_read_package
    #pragma HLS DATA_PACK variable=notifications
    #pragma HLS DATA_PACK variable=readRequest

    #pragma HLS INTERFACE axis register port=rxMetaData name=s_axis_rx_metadata
    #pragma HLS INTERFACE axis register port=rxData name=s_axis_rx_data


    static hls::stream<aggrOutput> aggr_out;
    #pragma HLS stream depth=8 variable=aggr_out
    #pragma HLS DATA_PACK variable=aggr_out

    static hls::stream<float>  accm;
    #pragma HLS stream depth=8 variable=accm

    static hls::stream<uint_1_t> done_accm;
    #pragma HLS stream depth=8 variable=done_accm

    static hls::stream<rank_t> numzeros_out;
    #pragma HLS stream depth=8 variable=numzeros_out

    static hls::stream<bucket_cnt_t> zero_count;
    #pragma HLS stream depth=8 variable=zero_count

    static hls::stream<uint_1_t> done_out;
    #pragma HLS stream depth=8 variable=done_out

    static hls::stream<dataItem<32> > dataFifo[NUM_PIPELINES];
    #pragma HLS stream depth=64 variable=dataFifo
    #pragma HLS DATA_PACK variable=dataFifo

    static hls::stream<rank_t> bucket_fifo[NUM_PIPELINES];
    #pragma HLS stream depth=8 variable=bucket_fifo

    static hls::stream<float> card_temp;
    #pragma HLS stream depth=2 variable=card_temp

    static hls::stream<bool> isLastPkgFifo("isLastPkgFifo");
    #pragma HLS stream depth=16 variable=isLastPkgFifo

    static hls::stream<net_axis<line_width> >   tcp2hllDataFifo("tcp2hll_dataFifo");
    static hls::stream<net_axis<line_width> >   tcp2dmaDataFifo("tcp2dma_dataFifo");
    #pragma HLS stream depth=2 variable=tcp2hllDataFifo
    #pragma HLS stream depth=2 variable=tcp2dmaDataFifo
    static hls::stream<dmaMeta> dmaMetaFifo("dmaMetaFifo");
    #pragma HLS stream depth=32 variable=dmaMetaFifo


    divide_data (
            tcp2hllDataFifo,
            dataFifo);


    pipeline<0>(dataFifo[0], bucket_fifo[0]);
    pipeline<1>(dataFifo[1], bucket_fifo[1]);
    pipeline<2>(dataFifo[2], bucket_fifo[2]);
    pipeline<3>(dataFifo[3], bucket_fifo[3]);
    pipeline<4>(dataFifo[4], bucket_fifo[4]);
    pipeline<5>(dataFifo[5], bucket_fifo[5]);
    pipeline<6>(dataFifo[6], bucket_fifo[6]);
    pipeline<7>(dataFifo[7], bucket_fifo[7]);
    pipeline<8>(dataFifo[8], bucket_fifo[8]);
    pipeline<9>(dataFifo[9], bucket_fifo[9]);
    pipeline<10>(dataFifo[10], bucket_fifo[10]);
    pipeline<11>(dataFifo[11], bucket_fifo[11]);
    pipeline<12>(dataFifo[12], bucket_fifo[12]);
    pipeline<13>(dataFifo[13], bucket_fifo[13]);
    pipeline<14>(dataFifo[14], bucket_fifo[14]);
    pipeline<15>(dataFifo[15], bucket_fifo[15]);

    aggr_bucket(bucket_fifo, aggr_out);

    //count zeros in the bucket
    zero_counter(aggr_out, numzeros_out, zero_count);

    accumulate (numzeros_out, accm, done_accm);

    //cardinality estimation
    estimate_cardinality<HASH_SIZE>(accm, zero_count, card_temp, done_accm);

    hll_server( listenPort,
                listenPortStatus,
                notifications,
                readRequest,
                isLastPkgFifo,
                dmaMetaFifo,
                sizeInTuples);

    handle_tcp_packet(  isLastPkgFifo,
                        rxMetaData,
                        rxData,
                        tcp2hllDataFifo,
                        tcp2dmaDataFifo);

    dma_writer( dmaMetaFifo,
                tcp2dmaDataFifo,
                card_temp,
                m_axis_write_cmd,
                m_axis_write_data,
                regBaseAddr);
}
