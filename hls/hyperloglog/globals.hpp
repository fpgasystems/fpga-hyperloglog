#ifndef GLOBALS_HPP_
#define GLOBALS_HPP_

#include <ap_int.h>
#include "../bit_utils.hpp"

#define line_width 512

//---------------------------------------------------------------------------
// Design Geometry

// Length of Hash (currently supported values: 32, 64, 128)
unsigned const HASH_SIZE =  64;

// Prefix bits for bucket selection (practical values: 14, 15, 16)
unsigned const BUCKET_BITS = 16;

// Derived Constants
unsigned const num_buckets_m = (1 << BUCKET_BITS);

//---------------------------------------------------------------------------
// Types
typedef ap_uint<1>	uint_1_t;	// flags

/** Count of leading zeros - [0:HASH_SIZE-BUCKET_BITS] */
typedef ap_uint<btl::clog2<HASH_SIZE-BUCKET_BITS+1>::value> lzcnt_t;
/** Rank - [0:HASH_SIZE-BUCKET_BITS+1] */
typedef ap_uint<btl::clog2<HASH_SIZE-BUCKET_BITS+2>::value> rank_t;

/** Bucket ID: [0:2**BUCKET_BITS-1] */
typedef ap_uint<BUCKET_BITS>	bucket_id_t;
/** Bucket Count: [0:2**BUCKET_BITS] */
typedef ap_uint<BUCKET_BITS+1>	bucket_cnt_t;

template <int W>
struct dataItem
{
	ap_uint<W>	data;
	ap_uint<1>	valid;
	ap_uint<1>	last;
	dataItem() {}
	dataItem(ap_uint<W> data, ap_uint<1> valid, ap_uint<1> last)
		:data(data), valid(valid), last(last) {}
};

struct bucketMeta
{
	bucket_id_t	bucketNum;
	lzcnt_t		numZeros;
	ap_uint<1>	valid;
	ap_uint<1>	last;
	bucketMeta() {}
	bucketMeta(bucket_id_t bucketNum, lzcnt_t numZeros, ap_uint<1> valid, ap_uint<1> last)
		:bucketNum(bucketNum), numZeros(numZeros), valid(valid), last(last) {}
};


struct hll_out{
  float     	data;
  ap_uint<4> 		keep;
  ap_uint<1>       	last;
};


#endif // GLOBALS_H_ not defined

