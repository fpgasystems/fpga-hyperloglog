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


int main (){
	hls::stream<ap_uint<16> > listenPort;
	hls::stream<bool> listenPortStatus;
	hls::stream<appNotification> notifications;
	hls::stream<appReadRequest> readRequest;
	hls::stream<ap_uint<16> > rxMetaData;
	hls::stream<net_axis<line_width> > rxData("rxData");

	hls::stream<memCmd>  m_axis_write_cmd;
	hls::stream<net_axis<line_width> >  m_axis_write_data("m_axis_write_data");

	ap_uint<64>   regBaseAddr = 0xAAAAAAAA;
	uint32_t sizeInTuples = 10000;

	//create input
	for (int i = 0; i < sizeInTuples; i+= (1400/4))
	{
		uint32_t packetSize = sizeInTuples - i;
		if (packetSize > (1400/4))
			packetSize = 1400/4;

		notifications.write(appNotification(13, (ap_uint<16>) packetSize*4, (ap_uint<32>)0x01010101, (ap_uint<16>)0x1389));
		net_axis<line_width> inputWord;
		inputWord.keep = 0;
		int idx = 0;
		for (int j = 0; j < packetSize; j++)
		{
			inputWord.data(idx*32+31, idx*32) = i+j+1;
			inputWord.keep(idx*4+3, idx*4) = 0xF;
			idx++;
			if (idx % 10 == 0 || j+1 == packetSize)
			{
				inputWord.last = (j+1 == packetSize);
				rxData.write(inputWord);
				inputWord.keep = 0;
				idx = 0;
			}
		}
	}

	int count = 0;
	while (count < 100000)
	{
		hyperloglog_tcp(m_axis_write_cmd,
							m_axis_write_data,
							regBaseAddr,
							sizeInTuples,
							listenPort,
							listenPortStatus,
							notifications,
							readRequest,
							rxMetaData,
							rxData);
		if (!listenPort.empty())
		{
			listenPort.read();
			listenPortStatus.write(true);
		}
		if (!readRequest.empty())
		{
			appReadRequest request = readRequest.read();
			rxMetaData.write(request.sessionID);
		}
		count++;
	}


	uint64_t totalLength = 0;
	while (!m_axis_write_cmd.empty())
	{
		memCmd cmd = m_axis_write_cmd.read();
		totalLength += cmd.len;
	}
	if (totalLength != sizeInTuples*4 + 4)
	{
		std::cerr << "Expected length not correct" << std::endl;
	}


	uint32_t resultTupleCounter = 0;
	bool receivedAll = false;
	while (!m_axis_write_data.empty())
	{
		net_axis<line_width> word = m_axis_write_data.read();
		uint16_t length = keepToLen<320>(word.keep);
		if (!receivedAll)
		{
			resultTupleCounter += length/4;
		}
		else
		{
			conversion cardinality;
			cardinality.value = word.data(31, 0);
			printf("\n The estimated cardinality is: %f \n", cardinality.f);

			float std_error = ((cardinality.f - (float)sizeInTuples) / (float)sizeInTuples)*100;
			printf("\n The standard error: %f%% \n", std_error);
		}
		

		if (resultTupleCounter == sizeInTuples)
		{
			receivedAll = true;
		}


	}
    return 0;
}

