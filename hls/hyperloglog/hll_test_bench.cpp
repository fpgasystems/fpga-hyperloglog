//
// Created by Amit Kulkarni on 06-Apr-19.
// Test bench for HLL
//
#include "hyperloglog.hpp"

#define max_count 100000
ap_uint<320> data_merge (
					uint32_t inp1,
					uint32_t inp2					,
					uint32_t inp3,
					uint32_t inp4,
					uint32_t inp5,
					uint32_t inp6,
					uint32_t inp7,
					uint32_t inp8,
					uint32_t inp9,
					uint32_t inp10
){
	return
			(((ap_uint<320>) inp10) << 288) |
			(((ap_uint<320>) inp9)  << 256) |
			(((ap_uint<320>) inp8)  << 224) |
			(((ap_uint<320>) inp7)  << 192) |
			(((ap_uint<320>) inp6)  << 160) |
			(((ap_uint<320>) inp5)  << 128) |
			(((ap_uint<320>) inp4)  << 96) |
			(((ap_uint<320>) inp3)  << 64) |
			(((ap_uint<320>) inp2)  << 32) |
			((ap_uint<320>) inp1);
}

int main (){
	hls::stream<net_axis<line_width> > s_axis_input_tuple;
	hls::stream<memCmd>  m_axis_write_cmd;
	hls::stream<hll_out>  m_axis_write_data;

	ap_uint<64>   regBaseAddr = 0xAAAAAAAA;

	net_axis<line_width> data_in;
	hll_out cardinality;

    float std_error = 0;
    int actual_count = 0;
    printf("\n Hyperloglog..\n");
    uint32_t i = 1;
	uint32_t j;
    ap_uint<320> data_sent;
    for(i=1; i<=max_count; i=i+10){
    	data_sent = data_merge(i,
    						   i+1,
    						   i+2,
    						   i+3,
    						   i+4,
    						   i+5,
    						   i+6,
    						   i+7,
    						   i+8,
    						   i+9
    	);

    	data_in.data = data_sent;
    	data_in.last = 0;
    	data_in.keep = 0xFFFFFFFFFF;
    	s_axis_input_tuple.write(data_in);

        hyperloglog(s_axis_input_tuple,
        		m_axis_write_cmd,
				m_axis_write_data,
				regBaseAddr);
    }

    	//The last
  		data_in.data = data_sent;
    	data_in.last = 1;
    	s_axis_input_tuple.write(data_in);
        hyperloglog(s_axis_input_tuple, m_axis_write_cmd, m_axis_write_data, regBaseAddr);

    	data_in.keep = 0;

    for(i=1; i<=2*num_buckets_m;i++){
        hyperloglog(s_axis_input_tuple, m_axis_write_cmd, m_axis_write_data, regBaseAddr);
    }

//    for(i=1; i<=max_count; i++){
//    	data_in.data = i;
//    	data_in.last = 0;
//    	data_in.keep = 0xF;
//    	s_axis_input_tuple.write(data_in);
//
//        hyperloglog(s_axis_input_tuple,
//        		m_axis_write_cmd,
//				m_axis_write_data,
//				regBaseAddr);
//    }
//
//    	//The last
//  		data_in.data = i;
//    	data_in.last = 1;
//    	s_axis_input_tuple.write(data_in);
//        hyperloglog(s_axis_input_tuple, m_axis_write_cmd, m_axis_write_data, regBaseAddr);
//
//    	data_in.keep = 0;
//
////    for(i=1; i<=2*num_buckets_m;i++){
////        hyperloglog(s_axis_input_tuple, m_axis_write_cmd, m_axis_write_data, regBaseAddr);
////    }

    if(!m_axis_write_data.empty())
    {
    	cardinality = m_axis_write_data.read();
    }

    float card = cardinality.data;
    printf("\n The estimated cardinality is: %f \n", card);

    assert(max_count != 0);
    //std_error = ((card - (float)1) / (float)1)*100;

    std_error = ((card - (float)max_count) / (float)max_count)*100;
    printf("\n The standard error: %f%% \n\n\n", std_error);

    return 0;
}

