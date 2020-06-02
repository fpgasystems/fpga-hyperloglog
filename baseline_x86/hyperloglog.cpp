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
#include "hyperloglog.hpp"

#include <map>
#include <vector>
#include <algorithm>
#include <limits>
#include <cstring>
#include <cmath>

//---------------------------------------------------------------------------
// Utilities for hash_e enum
template<>
char const *name_of<hash_e>(hash_e  val) {
	static char const *LOOKUP[(unsigned)hash_e::end] = {
		"IDENT",
		"SIP",
		"MURMUR3_32",
		"MURMUR3_64",
#ifdef INCLUDE_AVX_HASHES
		"MURMUR3_32AVX",
		"MURMUR3_64AVX",
#endif
	};
	return	val < hash_e::end? LOOKUP[(unsigned)val] : "<undef>";
}

template<>
hash_e value_of<hash_e>(char const *name) {
	static std::map<char const*, hash_e, std::function<bool(char const*, char const*)>> const  LOOKUP {
		{
			{ "IDENT",				hash_e::IDENT },
			{ "SIP",				hash_e::SIP },
			{ "MURMUR3_32",			hash_e::MURMUR3_32 },
			{ "MURMUR3_64",			hash_e::MURMUR3_64 },
#ifdef INCLUDE_AVX_HASHES
			{ "MURMUR3_32AVX",		hash_e::MURMUR3_32AVX },
			{ "MURMUR3_64AVX",		hash_e::MURMUR3_64AVX },
#endif
		},
		[](char const *a, char const *b) { return  strcmp(a, b) < 0; }
	};
	auto const  res = LOOKUP.find(name);
	return	res != LOOKUP.end()? res->second : hash_e::end;
}

//---------------------------------------------------------------------------
// Hash-based Dispatch Table
std::array<HllCollector::dispatch_t, (unsigned)hash_e::end> const  HllCollector::DISPATCH {
	HllCollector::dispatch_t { hll_collect_ptr<hash_e::IDENT>,		hll_collect_fct<hash_e::IDENT> },
	HllCollector::dispatch_t { hll_collect_ptr<hash_e::SIP>,		hll_collect_fct<hash_e::SIP> },
	HllCollector::dispatch_t { hll_collect_ptr<hash_e::MURMUR3_32>,	hll_collect_fct<hash_e::MURMUR3_32> },
	HllCollector::dispatch_t { hll_collect_ptr<hash_e::MURMUR3_64>,	hll_collect_fct<hash_e::MURMUR3_64> },
#ifdef INCLUDE_AVX_HASHES
	HllCollector::dispatch_t { hll_collect_ptr<hash_e::MURMUR3_32AVX>,	hll_collect_fct<hash_e::MURMUR3_32AVX> },
	HllCollector::dispatch_t { hll_collect_ptr<hash_e::MURMUR3_64AVX>,	hll_collect_fct<hash_e::MURMUR3_64AVX> },
#endif
};

#include <iostream>

void HllCollector::merge0(HllCollector const& other) {
	if(this->m_p != other.m_p)  throw std::invalid_argument("Incompatible bucket sets.");

	size_t const  M = 1<<m_p;
	for(size_t  i = 0; i < M; i++) {
		unsigned *const  ref  = &this->m_buckets[i];
		unsigned  const  cand = other.m_buckets[i];
		if(*ref < cand)	*ref = cand;
	}
}

double HllCollector::estimate_cardinality() {
	size_t const  M = 1<<m_p;
	double const  ALPHA = (0.7213*M)/(M+1.079);

	// Raw Estimate and Zero Count
	size_t zeros	= 0;
	double rawest	= 0.0;
	for(unsigned  i = 0; i < M; i++) {
		unsigned const  rank = m_buckets[i];
		if(rank == 0)	zeros++;
		if(std::numeric_limits<double>::is_iec559) {
			union { uint64_t  i; double  f; } d = { .i = (UINT64_C(1023)-rank)<<52 };
			rawest += d.f;
		}
		else rawest += 1.0/pow(2.0, rank);
	}
	rawest = (ALPHA * M * M) / rawest;

	// Refine Output
	if(rawest <= 2.5*M) {							// Small Range Correction
		// Linear counting
		if(zeros)	return	M * log((double)M / (double)zeros);
	}
// DROP Large Range Correction - It makes things worse instead of better.
//	else if(rawest > CONST_TWO_POWER_32/30.0) {		// Large Range Correction
//		return (-CONST_TWO_POWER_32) * log(1.0 - (rawest / CONST_TWO_POWER_32));
//	}
	return	rawest;
}
