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
#ifndef HYPERLOG_HPP
#define HYPERLOG_HPP

#include <cstdint>
#include <memory>
#include <functional>

enum class hash_e : unsigned {
	IDENT,
	SIP,
	MURMUR3_32,
	MURMUR3_64,
#ifdef INCLUDE_AVX_HASHES
	MURMUR3_32AVX,
	MURMUR3_64AVX,
#endif
	end
};

template<typename T> char const *name_of(T  val);
template<>			 char const *name_of<hash_e>(hash_e  val);
template<typename T> T		value_of(char const *name);
template<>			 hash_e	value_of<hash_e>(char const *name);

// Hll Collector Backends
template<hash_e HASH>
void hll_collect_ptr(uint32_t const *data, size_t const num_items, unsigned *buckets, unsigned const p_val);
template<hash_e HASH>
void hll_collect_fct(std::function<uint32_t()> src, size_t const num_items, unsigned *buckets, unsigned const p_val);


class HllCollector {

	struct dispatch_t {
		void (*f_ptr)(uint32_t const*, size_t, unsigned*, unsigned);
		void (*f_fct)(std::function<uint32_t()>, size_t, unsigned*, unsigned);
	};
	static std::array<dispatch_t, (unsigned)hash_e::end> const  DISPATCH;

	unsigned const  			m_p;
	std::unique_ptr<unsigned[]>	m_buckets;
	dispatch_t const *const		m_dispatch;

public:
	HllCollector(unsigned const  p_val, hash_e const  hash)
	 : m_p(p_val), m_buckets(new unsigned[1<<p_val]()), m_dispatch(&DISPATCH.at((unsigned)hash)) {}

	HllCollector(HllCollector&& o)
	 : m_p(o.m_p), m_buckets(std::move(o.m_buckets)), m_dispatch(o.m_dispatch) {}

	~HllCollector() {}

public:
	void collect(uint32_t const *data,          size_t  n) { m_dispatch->f_ptr(data, n, &m_buckets[0], m_p); }
	void collect(std::function<uint32_t()> src, size_t  n) { m_dispatch->f_fct(src,  n, &m_buckets[0], m_p); }

private:
	void merge0(HllCollector const& other);
public:
	HllCollector& merge(HllCollector const& other) {
		merge0(other);
		return *this;
	}

public:
	double estimate_cardinality();
};
#endif
