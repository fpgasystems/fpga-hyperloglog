##
# Copyright (c) 2020, Systems Group, ETH Zurich
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the names of its contributors
# may be used to endorse or promote products derived from this software
# without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
##
SHELL	:= bash
CXXFLAGS:= -Wall -O3 -march=native -DINCLUDE_AVX_HASHES
LDLIBS	:= -lpthread
LD		:= g++

TARGETS	:= hll_bench err_vs_card

.PHONY: all bench error clean
all: $(TARGETS)

hll_bench.o hyperloglog.o hll_base.o hll_avx.o err_vs_card.o: hyperloglog.hpp

#= HLL Benchmarking =========================================================
hll_bench: hll_bench.o hyperloglog.o hll_base.o hll_avx.o
	$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o$@

# Run Measurement over 1 GiByte data
bench: bench.log
bench.log: hll_bench
	set -e; i=0; declare -a logs; \
	for h in $$(./hll_bench | sed -ne'/Hashes:/{:a;n;p;ba;}'); do \
		./hll_bench "$$h" 8 4 1 >/dev/null; \
		for t in 1 2 4 8 16 32 64; do \
			log=bench$$((i++)).log; \
			./hll_bench "$$h" 100000000 16 $$t | tee $$log; logs+=($$log); echo; sync; \
		done; \
	done; \
	cat "$${logs[@]}" >$@; rm "$${logs[@]}"

#= Error Analysis ===========================================================
err_vs_card: err_vs_card.o hyperloglog.o hll_base.o hll_avx.o
	$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o$@

error: error.dat
error.dat: err_vs_card
	set -e; \
	./err_vs_card 'MURMUR3_32' 'MURMUR3_64' 'SIP' | tee error.dat

#= Clean up =================================================================
clean:
	rm -rf *.o $(TARGETS)
