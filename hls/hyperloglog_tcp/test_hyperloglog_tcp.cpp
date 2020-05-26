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

