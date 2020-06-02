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
#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>
#include <map>
#include <random>

#include <cmath>
#include <cstring>

#include "hyperloglog.hpp"

template<typename T>
class Sequence {
public:
	virtual ~Sequence() {}
	T operator()() { return  next(); }
public:
	virtual void reset() {}
	virtual T next() = 0;
};
template<typename T>
class Naturals : public Sequence<T> {
	T m_val = 0;
public:
	Naturals() : m_val(0) {}
public:
	void reset() { m_val = 0; }
	T next() { return  m_val++; }
};
template<typename T>
class MT19937 : public Sequence<T> {};
template<>
class MT19937<uint32_t> : public Sequence<uint32_t> {
	std::mt19937  m_src;
public:
	MT19937() : m_src(42) {}
public:
	void reset() { m_src.seed(42); }
	uint32_t next() { return  m_src(); }
};

int main(int const  argc, char *const argv[]) {
	size_t   const  num_items = 0x100000000uL;
	unsigned const  p_val = 14;
	bool random_input = false;

	// Parse Requested Hashes
	std::vector<hash_e>  runs;
	for(int  i = 1; i < argc; i++) {
		char const *const  arg  = argv[i];
		hash_e const  hash = value_of<hash_e>(arg);
		if(hash != hash_e::end) {
			runs.emplace_back(hash);
			continue;
		}
		else if(strcmp(arg, "-rnd") == 0) {
			random_input = true;
			continue;
		}
		std::cerr << argv[0] << " [-rnd] '<hash>' ..." << std::endl;
		return	1;
	}

	// Collection of Estimates
	std::map<size_t, std::vector<double>>	estimates;

	{ // Run all Hashes over Input
		std::cout << std::fixed << std::setprecision(4) << "Cardinality\tExpected";
		std::unique_ptr<Sequence<uint32_t>>	seq{
			random_input? (Sequence<uint32_t>*)new MT19937<uint32_t>() : new Naturals<uint32_t>()
		};

		for(hash_e const  hash : runs) {
			std::cout << '\t' << name_of(hash) << std::flush;

			HllCollector	collector(p_val, hash);
			size_t ofs = 0;
			for(size_t nofs = 100000; nofs < num_items; nofs = (size_t)(1.15478197*nofs)) {
				// Collect from `ofs` up to `nofs`
				collector.collect(std::ref(*seq), nofs-ofs);

				// Compute and collect Estimates
				estimates[nofs].push_back(collector.estimate_cardinality());

				ofs = nofs;
			}
			seq->reset();
		}
	}
	std::cout << std::endl;

	// Output Results
	double const ref_error = 100.0*(1.04/sqrt(1<<p_val));
	for(auto const& kvp : estimates) {
		double const  ref = (double)kvp.first;
		std::cout << kvp.first << '\t' << ref_error;
		for(auto const& est : kvp.second) std::cout << '\t' << 100.0*(est/ref - 1.0);
		std::cout << std::endl;
	}

	return  0;
}
