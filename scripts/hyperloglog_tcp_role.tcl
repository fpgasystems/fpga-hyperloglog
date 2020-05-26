set hll_src_dir $root_dir/hdl

#Add sources
add_files $hll_src_dir

create_ip -name hyperloglog_tcp -vendor ethz.systems.fpga -library hls -version 0.1 -module_name hyperloglog_tcp_ip -dir $device_ip_dir
generate_target {instantiation_template} [get_files $device_ip_dir/hyperloglog_tcp_ip/hyperloglog_tcp_ip.xci]
update_compile_order -fileset sources_1

create_ip -name axis_dwidth_converter -vendor xilinx.com -library ip -version 1.1 -module_name axis_512_to_320_converter  -dir $device_ip_dir
set_property -dict [list CONFIG.S_TDATA_NUM_BYTES {64} CONFIG.M_TDATA_NUM_BYTES {40} CONFIG.HAS_TLAST {1} CONFIG.HAS_TKEEP {1} CONFIG.HAS_MI_TKEEP {1} CONFIG.Component_Name {axis_512_to_320_converter}] [get_ips axis_512_to_320_converter]
generate_target {instantiation_template} [get_files $device_ip_dir/axis_512_to_320_converter/axis_512_to_320_converter.xci]
update_compile_order -fileset sources_1

create_ip -name axis_dwidth_converter -vendor xilinx.com -library ip -version 1.1 -module_name axis_320_to_512_converter  -dir $device_ip_dir
set_property -dict [list CONFIG.S_TDATA_NUM_BYTES {40} CONFIG.M_TDATA_NUM_BYTES {64} CONFIG.HAS_TLAST {1} CONFIG.HAS_TKEEP {1} CONFIG.HAS_MI_TKEEP {1} CONFIG.Component_Name {axis_320_to_512_converter}] [get_ips axis_320_to_512_converter]
generate_target {instantiation_template} [get_files $device_ip_dir/axis_320_to_512_converter/axis_320_to_512_converter.xci]
update_compile_order -fileset sources_1


create_ip -name ila -vendor xilinx.com -library ip -version 6.2 -module_name ila_32_mixed  -dir $device_ip_dir
set_property -dict [list CONFIG.C_PROBE31_TYPE {1} CONFIG.C_PROBE30_TYPE {1} CONFIG.C_PROBE29_TYPE {1} CONFIG.C_PROBE28_TYPE {1} CONFIG.C_PROBE27_TYPE {1} CONFIG.C_PROBE26_TYPE {1} CONFIG.C_PROBE25_TYPE {1} CONFIG.C_PROBE24_TYPE {1} CONFIG.C_PROBE23_TYPE {1} CONFIG.C_PROBE22_TYPE {1} CONFIG.C_PROBE21_TYPE {1} CONFIG.C_PROBE20_TYPE {1} CONFIG.C_PROBE19_TYPE {1} CONFIG.C_PROBE18_TYPE {1} CONFIG.C_PROBE17_TYPE {1} CONFIG.C_PROBE16_TYPE {1} CONFIG.C_PROBE31_WIDTH {32} CONFIG.C_PROBE30_WIDTH {32} CONFIG.C_PROBE29_WIDTH {32} CONFIG.C_PROBE28_WIDTH {32} CONFIG.C_PROBE27_WIDTH {32} CONFIG.C_PROBE26_WIDTH {32} CONFIG.C_PROBE25_WIDTH {32} CONFIG.C_PROBE24_WIDTH {32} CONFIG.C_PROBE23_WIDTH {32} CONFIG.C_PROBE22_WIDTH {32} CONFIG.C_PROBE21_WIDTH {32} CONFIG.C_PROBE20_WIDTH {32} CONFIG.C_PROBE19_WIDTH {32} CONFIG.C_PROBE18_WIDTH {32} CONFIG.C_PROBE17_WIDTH {32} CONFIG.C_PROBE16_WIDTH {32} CONFIG.C_NUM_OF_PROBES {32} CONFIG.Component_Name {ila_32_mixed}] [get_ips ila_32_mixed]
generate_target {instantiation_template} [get_files $device_ip_dir/ila_32_mixed/ila_32_mixed.xci]
update_compile_order -fileset sources_1
