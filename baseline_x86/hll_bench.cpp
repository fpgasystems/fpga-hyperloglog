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
#include <memory>
#include <iostream>
#include <iomanip>
#include <chrono>

#include <cstdlib>
#include <cmath>

#include <vector>
#include <thread>

#include "hyperloglog.hpp"

int main(int argc, char* argv[]) {

	// Validate and Capture Arguments
	if (argc != 5) {
		std::cout << "Usage: " << argv[0] << " '<hash>' <num_items> <bucket_bits> <num_threads>\n\n  Hashes:\n";
		for(unsigned i = 0; i < (unsigned)hash_e::end; i++) std::cout << '\t' << name_of((hash_e)i) << '\n';
		std::cout << std::endl;
		return	1;
	}
	hash_e const  hash = value_of<hash_e>(argv[1]);
	if(hash == hash_e::end) {
		std::cerr << "Unknown hash '" << argv[1] << '\'' << std::endl;
		return	1;
	}
	unsigned const per_block = 256/32;

	size_t const num_items = strtoul(argv[2], nullptr, 0);
	if(num_items%per_block != 0) {
		std::cerr << "num_items must be a multiple of " << per_block << std::endl;
		return	1;
	}
	size_t const num_blocks = num_items/per_block;
	unsigned const num_threads = (unsigned)std::min(num_blocks, (size_t)strtoul(argv[4], nullptr, 0));
	unsigned const p_val = strtoul(argv[3], nullptr, 0);
	if((p_val < 4) || (16 < p_val)) {
		std::cerr << "bucket_bits out of valid range [4:16]." << std::endl;
		return	1;
	}
	unsigned const m_val = 1 << p_val;
	unsigned const num_cores = std::thread::hardware_concurrency();

	std::cout
		<< "H=" << name_of(hash)
		<< " B=" << p_val
		<< " T=" << num_threads << " (mod " << num_cores << " cores)" << std::endl;

	// Estimate Cardinality through HyperLogLog
	std::vector<HllCollector> collectors;
	for(unsigned i = 0; i < num_threads; i++)  collectors.emplace_back(p_val, hash);

	// Allocate & Populate Input Memory
	std::unique_ptr<uint32_t[]> input{new uint32_t[num_items]};
	for(unsigned i = 0; i < num_items; i++)	input[i] = i;

	// Threaded HLL Bucket Collection
	//	- split as evenly as possible at 32-byte boundaries
	auto const t0 = std::chrono::system_clock::now();
	{
		std::vector<std::thread> threads;

		size_t ofs = 0;
		for(unsigned i = 0; i < num_threads; i++) {
			size_t const  nofs  = (((i+1)*num_blocks)/num_threads) * per_block;

			size_t const  cnt = nofs-ofs;
			HllCollector& clct(collectors[i]);
			threads.emplace_back([&clct, data=&input[ofs], cnt](){ clct.collect(data, cnt); });
			cpu_set_t  cpus;
			CPU_ZERO(&cpus);
			CPU_SET(i%num_cores, &cpus);
			pthread_setaffinity_np(threads.back().native_handle(), sizeof(cpus), &cpus);
			ofs = nofs;
		}
		// Wait for all to finish
		for(std::thread& t : threads) t.join();
	}

	// Compact into first Bucket & Compute Cardinality Estimate
	auto const t1 = std::chrono::system_clock::now();
	for(unsigned  i = 1; i < num_threads; i++) {
		collectors[0].merge(collectors[i]);
	}

	double const cardest = collectors[0].estimate_cardinality();
	auto const t2 = std::chrono::system_clock::now();

	// Report Measurements
	double const std_error = (cardest/(double)num_items - 1.0) * 100.0;
	double const ref_error = 100.0*(1.04/sqrt(m_val));
	std::cout << std::fixed << std::setprecision(4)
		<< "  Estimated Cardinality: " << cardest << "\t[exp: " << num_items << ']' << std::endl
		<< "  Standard Error: " << std_error << "%\t[limit: " << ref_error << "%]";
	if(fabs(std_error) > ref_error)	std::cout << "\t! OUT OF RANGE !";

	float const d0 = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()/1000.f;
	float const d1 = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t0).count()/1000.f;
	std::cout
		<< std::endl << std::setprecision(3)
		<< "  Collection Time: " << d0 << " s" << "\t[" << (4*num_items)/d0/1000000.f << " MByte/s]" << std::endl
		<< "  Total Time: " << d1 << " s" << std::endl;

	return	0;
}
