#pragma once

#include "../axi_utils.hpp"
#include "../mem_utils.hpp"
#include "../hyperloglog/globals.hpp"
#include "../hyperloglog/pipeline.hpp"

//TCP structs
struct appNotification
{
	ap_uint<16>			sessionID;
	ap_uint<16>			length;
	ap_uint<32>			ipAddress;
	ap_uint<16>			dstPort;
	bool				closed;
	appNotification() {}
	appNotification(ap_uint<16> id, ap_uint<16> len, ap_uint<32> addr, ap_uint<16> port)
				:sessionID(id), length(len), ipAddress(addr), dstPort(port), closed(false) {}
	appNotification(ap_uint<16> id, bool closed)
				:sessionID(id), length(0), ipAddress(0),  dstPort(0), closed(closed) {}
	appNotification(ap_uint<16> id, ap_uint<32> addr, ap_uint<16> port, bool closed)
				:sessionID(id), length(0), ipAddress(addr),  dstPort(port), closed(closed) {}
	appNotification(ap_uint<16> id, ap_uint<16> len, ap_uint<32> addr, ap_uint<16> port, bool closed)
			:sessionID(id), length(len), ipAddress(addr), dstPort(port), closed(closed) {}
};


struct appReadRequest
{
	ap_uint<16> sessionID;
	//ap_uint<16> address;
	ap_uint<16> length;
	appReadRequest() {}
	appReadRequest(ap_uint<16> id, ap_uint<16> len)
			:sessionID(id), length(len) {}
};

struct dmaMeta
{
	ap_uint<16>	length;
	bool			isLast;
	dmaMeta() {}
	dmaMeta(ap_uint<16> length, bool isLast)
		:length(length), isLast(isLast) {}
};

union conversion
{
	float		f;
	uint32_t	value;
};


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
		hls::stream<net_axis<line_width> >& rxData);
