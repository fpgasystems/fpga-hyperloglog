#ifndef MURMUR3_HPP
#define MURMUR3_HPP

#include "globals.hpp"

#include <hls_stream.h>

template<int N>
void murmur3(
	hls::stream<dataItem<32>>&	dataFifo,
	hls::stream<dataItem<N>>&	hashFifo
);

#endif