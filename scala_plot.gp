reset 
set ylabel 'time(sec)'
set xlabel 'thread id'
set style fill solid 
set title "Benchmarks"
set key left box
set term png enhanced font 'Mono,10'
set output 'scalability.png'

plot 'scalability.txt' using 2:xtic(1) title 'points' with histogram \
