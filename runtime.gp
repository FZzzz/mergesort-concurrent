reset 
set ylabel 'exec\_time(s)'
set xlabel 'n th sample'
set style fill solid 
set title "100 samples of 128 threads"
set key left box
set term png enhanced font 'Mono,10'
set output 'runtime_compare.png'

plot 'sort_uniform.csv' using 1:6 with points title 'sort\_uniform', \
'sort.csv' using 1:6 with points title 'sort'
