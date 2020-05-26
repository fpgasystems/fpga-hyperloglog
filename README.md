# FPGA-based HyperLogLog Accelerator

## Build

Make sure Vivado 2019.1 is in your path = this is an example (your path might be different)
```
$ source /opt/Xilinx/Vivado/2019.1/settings64.sh
```

### Handling HLS IP cores

1. Setup build directory, e.g. for the hyperloglog module

```
$ cd hls/hyperloglog
$ mkdir build
$ cd build
$ cmake .. -DFPGA_PART=xcvu9p-flga2104-2L-e 
```

2. Run c-simulation
```
$ make csim
```

3. Run c-synthesis
```
$ make synthesis
```

4. Generate HLS IP core
```
$ make ip
```

5. Install HLS IP core in ip repository
```
$ make installip
```

After 'make installip' command the ip of the module could be found in ../iprepo 

## Contributors

- Amit Kulkarni amit.kulkarni@inf.ethz.ch
- Monica Chiosa monica.chiosa@inf.ethz.ch
- Thomas B. Preußer tpreusser@inf.ethz.ch
- Kaan Kara kaan.kara@inf.ethz.ch
- David Sidler david.sidler@inf.ethz.ch
- Gustavo Alonso alonso@inf.ethz.ch

## Publication(s)
- A. Kulkarni, M.Chiosa, T.B.Preu\{ss}er, K.Kara, D. Sidler, G. Alonso, 
*HyperLogLog Sketch Accleration on FPGA,* in FPL'20.


## License

We keep the BSD 3-Clause License for this project

```
BSD 3-Clause License

Copyright (c) 2020, 
Systems Group, ETH Zurich (systems.ethz.ch)
All rights reserved.


Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

<a name="citation"></a>

## Citation

If you use the HyperLogLog Accelerator in your project please cite the following paper and/or link to the github project:

```
@INPROCEEDINGS{hll2020,
    author={A. Kulkarni and M. Chiosa and T. B. Preu{\ss}er and K. Kara and D. Sidler and G. Alonso}, 
    booktitle={FPL},
    year={2020}, 
    title={{HyperLogLog Sketch Acceleration on FPGA}}, 
}
```
