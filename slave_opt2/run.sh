# rm -f log/*
# # naive
# make clean benchmark-naive
# # 512 512 512 7 48 16
# bsub -b -I -q q_sw_cpc_2 -host_stack 1024 -share_size 4096 -n 16 -cgsp 64 ./benchmark-naive 7 512 512 512 48 /home/export/online1/cpc/pre/stencil_data_7_512x512x512_48steps /home/export/online1/cpc/pre/stencil_answer_7_512x512x512_48steps 2>&1 | tee log/run_naive_1.log
# # 512 512 512 27 16 16
# bsub -b -I -q q_sw_cpc_2 -host_stack 1024 -share_size 4096 -n 16 -cgsp 64 ./benchmark-naive 27 512 512 512 16 /home/export/online1/cpc/pre/stencil_data_27_512x512x512_16steps /home/export/online1/cpc/pre/stencil_answer_27_512x512x512_16steps 2>&1 | tee log/run_naive_2.log
# # 768 768 768 7 64 64
# # bsub -b -I -q q_sw_cpc_2 -host_stack 1024 -share_size 4096 -n 64 -cgsp 64 ./benchmark-naive 7 768 768 768 64 /home/export/online1/cpc/pre/stencil_data_7_768x768x768_64steps /home/export/online1/cpc/pre/stencil_answer_7_768x768x768_64steps 2>&1 | tee log/run_naive_3.log
# # 768 768 768 27 16 64
# # bsub -b -I -q q_sw_cpc_2 -host_stack 1024 -share_size 4096 -n 64 -cgsp 64 ./benchmark-naive 27 768 768 768 16 /home/export/online1/cpc/pre/stencil_data_27_768x768x768_16steps /home/export/online1/cpc/pre/stencil_answer_27_768x768x768_16steps 2>&1 | tee log/run_naive_4.log
# # final
make clean benchmark-optimized
# # 512 512 512 7 48 16
bsub -b -I -q q_sw_cpc_2 -host_stack 1024 -share_size 4096 -n 16 -cgsp 64 ./benchmark-optimized 7 512 512 512 48 /home/export/online1/cpc/pre/stencil_data_7_512x512x512_48steps /home/export/online1/cpc/pre/stencil_answer_7_512x512x512_48steps 2>&1 | tee log/run_final_1.log
# # 512 512 512 27 16 16
bsub -b -I -q q_sw_cpc_2 -host_stack 1024 -share_size 4096 -n 16 -cgsp 64 ./benchmark-optimized 27 512 512 512 16 /home/export/online1/cpc/pre/stencil_data_27_512x512x512_16steps /home/export/online1/cpc/pre/stencil_answer_27_512x512x512_16steps 2>&1 | tee log/run_final_2.log
# # 768 768 768 7 64 64
bsub -b -I -q q_sw_cpc_2 -host_stack 1024 -share_size 4096 -n 64 -cgsp 64 ./benchmark-optimized 7 768 768 768 64 /home/export/online1/cpc/pre/stencil_data_7_768x768x768_64steps /home/export/online1/cpc/pre/stencil_answer_7_768x768x768_64steps 2>&1 | tee log/run_final_3.log
# # 768 768 768 27 16 64
bsub -b -I -q q_sw_cpc_2 -host_stack 1024 -share_size 4096 -n 64 -cgsp 64 ./benchmark-optimized 27 768 768 768 16 /home/export/online1/cpc/pre/stencil_data_27_768x768x768_16steps /home/export/online1/cpc/pre/stencil_answer_27_768x768x768_16steps 2>&1 | tee log/run_final_4.log