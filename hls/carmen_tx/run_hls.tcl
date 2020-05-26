
open_project carmen_tx_prj

set_top carmen_tx

add_files carmen_tx.cpp
add_files ../carmen/carmen.hpp
add_files ../selection_filter_batch/selection_filter_batch.hpp
add_files ../selection_filter_batch/selection_filter_batch.cpp
add_files ../data_shuffling/data_shuffling.hpp
add_files ../data_shuffling/data_shuffling.cpp

add_files -tb test_carmen_tx.cpp

open_solution "solution1"
set_part {xcvu9p-flga2104-2L-e}
create_clock -period 6.4 -name default

csynth_design
export_design -format ip_catalog -display_name "Transmission selection and shuffling." -description "" -vendor "ethz.systems.fpga" -version "0.1"

exit
