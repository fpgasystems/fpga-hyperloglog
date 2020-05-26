set hll_src_dir $root_dir/hdl

#Add sources
add_files $hll_src_dir

create_ip -name carmen_tx -vendor ethz.systems.fpga -library hls -version 0.1 -module_name carmen_tx_ip -dir $device_ip_dir
generate_target {instantiation_template} [get_files $device_ip_dir/carmen_tx_ip/carmen_tx_ip.xci]
update_compile_order -fileset sources_1

create_ip -name hyperloglog -vendor ethz.systems.fpga -library hls -version 0.1 -module_name hyperloglog_ip -dir $device_ip_dir
generate_target {instantiation_template} [get_files $device_ip_dir/hyperloglog_ip/hyperloglog_ip.xci]
update_compile_order -fileset sources_1

create_ip -name axis_dwidth_converter -vendor xilinx.com -library ip -version 1.1 -module_name axis_512_to_320_converter  -dir $device_ip_dir
set_property -dict [list CONFIG.S_TDATA_NUM_BYTES {64} CONFIG.M_TDATA_NUM_BYTES {40} CONFIG.HAS_TLAST {1} CONFIG.HAS_TKEEP {1} CONFIG.HAS_MI_TKEEP {1} CONFIG.Component_Name {axis_512_to_320_converter}] [get_ips axis_512_to_320_converter]
generate_target {instantiation_template} [get_files $device_ip_dir/axis_512_to_320_converter/axis_512_to_320_converter.xci]
update_compile_order -fileset sources_1
