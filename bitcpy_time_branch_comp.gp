reset
set term png enhanced font 'Verdana,10'
set output 'bitcpy_time_branch_comp.png'
set title 'Bit copy time cost - Compare original bitcpy and bitcpy_branch_less'
set xlabel "Bits for copy"
set ylabel "Copy time cost(ns)"
set grid

plot [0:7900][0:70000] 'bitcpy_time_comp.txt' using 1:2 with linespoints linewidth 1 title 'bitcpy', \
'' using 1:3 with linespoints linewidth 1 title 'bitcpy branch less'
