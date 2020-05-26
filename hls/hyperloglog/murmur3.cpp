#include "murmur3.hpp"

// Implementations of murmur3<N> for N=32,64,128.


typedef ap_uint<128>	uint_128_t;
typedef ap_uint<64>		uint_64_t;
typedef ap_uint<32>		uint_32_t;
typedef ap_uint<8>		uint_8_t;
typedef ap_uint<1>		uint_1_t;

template<>
void murmur3<32>(
	hls::stream<dataItem<32>>&	dataFifo,
	hls::stream<dataItem<32>>&	hashFifo
) {
#pragma HLS PIPELINE II=1
#pragma HLS INLINE off
	const uint_32_t c1 = 0xcc9e2d51;
	const uint_32_t c2 = 0x1b873593;
	uint_32_t seed ;
	uint_32_t len;
	uint_32_t h1;
	uint_32_t k1_t;

	if (!dataFifo.empty()){
		dataItem<32> key = dataFifo.read();


		seed = 42;
		len = 4; //32-bit input (4 bytes)
		h1 = seed;

		// body
		k1_t =  key.data;
		k1_t *= c1;
		k1_t = (k1_t << 15) | (k1_t >> (32 - 15));
		k1_t *= c2;
		h1 ^= k1_t;
		h1 = (h1 << 13) | (h1 >> (32 - 13));
		h1 = h1*5+0xe6546b64;

		//finalization
		h1 ^= len;
		h1 ^= h1 >> 16;
		h1 *= 0x85ebca6b;
		h1 ^= h1 >> 13;
		h1 *= 0xc2b2ae35;
		h1 ^= h1 >> 16;

		hashFifo.write(dataItem<32>(h1, key.valid, key.last));
	}
} // murmur3<32>

template<>
void murmur3<64>(
	hls::stream<dataItem<32>>&	dataFifo,
	hls::stream<dataItem<64>>&	hashFifo
) {
#pragma HLS PIPELINE II=1
#pragma HLS INLINE off

	uint_32_t data;
	uint_64_t seed = 0xbaadf00d;
	uint_64_t len = 4;

	uint_64_t h1 = seed;
	uint_64_t h2 = seed;
	const uint_64_t c1 = 0x87c37b91114253d5;
	const uint_64_t c2 = 0x4cf5ad432745937f;
	const uint_64_t c3 = 0xff51afd7ed558ccd;
	const uint_64_t c4 = 0xc4ceb9fe1a85ec53;

	uint_64_t k1 = 0;
	uint_1_t last;

	if (!dataFifo.empty()){
		dataItem<32> key = dataFifo.read();
		data = key.data;

		//std::cout << std::fixed << data << std::endl;

		uint_64_t  t0 =  (uint_8_t) data;
		uint_64_t  t1 =  (uint_8_t)(data >> 8);
		uint_64_t  t2 =  (uint_8_t)(data >> 16);
		uint_64_t  t3 =  (uint_8_t)(data >> 24);

		k1 ^= t3 << 24;
		k1 ^= t2 << 16;
		k1 ^= t1 << 8;
		k1 ^= t0 << 0;
		k1 *= c1;
		k1 = (k1 << 31) | (k1 >> (64 - 31));
		k1 *= c2;
		h1 ^= k1;

		h1 ^= len;
		h2 ^= len;

		h1 += h2;
		h2 += h1;

		h1 ^= h1 >> 33;
		h1 *= c3;
		h1 ^= h1 >> 33;
		h1 *= c4;
		h1 ^= h1 >> 33;

		h2 ^= h2 >> 33;
		h2 *= c3;
		h2 ^= h2 >> 33;
		h2 *= c4;
		h2 ^= h2 >> 33;

		h1 += h2;
		h2 += h1;

		//std::cout << std::hex << h1 << std::endl;
		hashFifo.write(dataItem<64>(h1, key.valid, key.last));
	}
} // murmur3<64>

template<>
void murmur3<128>(
	hls::stream<dataItem<32>>&	dataFifo,
	hls::stream<dataItem<128>>&	hashFifo
) {
#pragma HLS PIPELINE II=1
#pragma HLS INLINE off

	uint_32_t data;
	uint_64_t seed = 0xbaadf00d;
	uint_64_t len = 4;

	uint_64_t h1 = seed;
	uint_64_t h2 = seed;
	const uint_64_t c1 = 0x87c37b91114253d5;
	const uint_64_t c2 = 0x4cf5ad432745937f;
	const uint_64_t c3 = 0xff51afd7ed558ccd;
	const uint_64_t c4 = 0xc4ceb9fe1a85ec53;

	uint_64_t k1 = 0;
	uint_1_t last;
	uint_128_t hash_val_128 = 0;

	if (!dataFifo.empty()){
		dataItem<32> key = dataFifo.read();
		data = key.data;

		uint_64_t  t0 =  (uint_8_t) data;
		uint_64_t  t1 =  (uint_8_t)(data >> 8);
		uint_64_t  t2 =  (uint_8_t)(data >> 16);
		uint_64_t  t3 =  (uint_8_t)(data >> 24);

		k1 ^= t3 << 24;
		k1 ^= t2 << 16;
		k1 ^= t1 << 8;
		k1 ^= t0 << 0;
		k1 *= c1;
		k1 = (k1 << 31) | (k1 >> (64 - 31));
		k1 *= c2;
		h1 ^= k1;

		//Finalization
		h1 ^= len;
		h2 ^= len;

		h1 += h2;
		h2 += h1;
		h1 ^= h1 >> 33;
		h1 *= c3;
		h1 ^= h1 >> 33;
		h1 *= c4;
		h1 ^= h1 >> 33;

		h2 ^= h2 >> 33;
		h2 *= c3;
		h2 ^= h2 >> 33;
		h2 *= c4;
		h2 ^= h2 >> 33;

		h1 += h2;
		h2 += h1;

		hash_val_128 = (((uint_128_t)h2 << 64) | ((uint_128_t)h1));

		hashFifo.write(dataItem<128>(hash_val_128, key.valid, key.last));
	}
} // murmur3<128>


