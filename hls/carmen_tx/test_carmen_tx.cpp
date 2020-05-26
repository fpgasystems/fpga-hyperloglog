#include "../mem_utils.hpp"
#include "carmen_tx.hpp"

int main(int argc, char* argv[])
{

hls::stream<ap_uint<96> >      s_axis_param("s_axis_param");
hls::stream<memCmd>            m_axis_dma_cmd("m_axis_dma_cmd");

ap_uint<96> parameter = 0x0000000000000002; 
ap_uint<64> address   = 0x0000000000000000;
ap_uint<32> length    = 0x00010000; 
memCmd      memoryCmd = memCmd(address,length);
                          
int count = 0;

s_axis_param.write(parameter);
m_axis_dma_cmd.write(memoryCmd); 

while (count < 10000)
{
	carmen_tx(s_axis_param, m_axis_dma_cmd);

    count++;
}
    return 0;
}
