reset 
set ylabel 'time(sec)'
set xlabel 'N-threads'
set style fill solid 
set title "Benchmarks"
set key left box
set term png enhanced font 'Mono,10'
set output 'result.png'

plot 'exec_time.csv' using 1:2 title 'baseline' with linespoints , \
