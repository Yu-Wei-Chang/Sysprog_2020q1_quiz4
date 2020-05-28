reset
set term png enhanced font 'Verdana,10'
set output 'bitcpy_time_mask_comp.png'
set title 'Bit copy time cost - Compare two different way to get MASK'
set xlabel "Bits for copy"
set ylabel "Copy time cost(ns)"
set grid

plot [0:32][0:50000000] 'static_var_mask.txt' using 1:2 with linespoints linewidth 2 title 'MASK \
store as static local variable', \
'macro_mask.txt' using 1:2 with linespoints linewidth 2 title 'MASK calculated in time'
