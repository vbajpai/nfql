#!/bin/sh -e

# sync
# sudo sysctl vm.drop_caches=3 > /dev/null # drop pagecache, dentries and inodes

# compiling flowy filter
#gcc -Wall -O3 -Werror -I ../flowy2 ../flowy2/ftreader.o ../flowy2/auto_comps.o filter-hard.c -lft -lz -o filter-hard
#gcc -Wall -O3 -Werror -I ../flowy2 ../flowy2/ftreader.o ../flowy2/auto_comps.o filter-medium.c -lft -lz -o filter-medium
#gcc -Wall -O3 -Werror -I ../flowy2 ../flowy2/ftreader.o ../flowy2/auto_comps.o filter-easy.c -lft -lz -o filter-easy

#TRACES="traces-103189.flow traces-336536.flow traces-656108.flow traces-867538.flow traces-971133.flow traces-1161133.flow"
#NUMS="traces-103189.nfdump traces-336536.nfdump traces-656108.nfdump traces-867538.nfdump traces-971133.nfdump traces-1161133.nfdump"

NUMS="500000 1000000 1500000 2000000 2500000 3000000 3500000 4000000 4500000 5000000 5500000 6000000 6500000 7000000 7500000 8000000 8500000 9000000 9500000 10000000"

echo "benchmarking nfdump easy"

for trace in $NUMS; do
	echo -n "$trace\t"
	result="0"
	trace="traces-$trace.nfdump"
	cat $trace > /dev/null
	for i in `seq 1 10`; do
		result="$result+`time -f "%e" nfdump -r $trace -f filter-easy.nf 2>&1 > /dev/null`"
	done
	echo "scale=2;($result)/10" | bc
done

echo "benchmarking nfdump medium"

for trace in $NUMS; do
	echo -n "$trace\t"
	result="0"
	trace="traces-$trace.nfdump"
	cat $trace > /dev/null
	for i in `seq 1 10`; do
		result="$result+`time -f "%e" nfdump -r $trace -f filter-medium.nf 2>&1 > /dev/null`"
	done
	echo "scale=2;($result)/10" | bc
done

echo "benchmarking nfdump hard"

for trace in $NUMS; do
	echo -n "$trace\t"
	result="0"
	trace="traces-$trace.nfdump"
	cat $trace > /dev/null
	for i in `seq 1 10`; do
		result="$result+`time -f "%e" nfdump -r $trace -f filter-hard.nf 2>&1 > /dev/null`"
	done
	echo "scale=2;($result)/10" | bc
done

echo "benchmarking flow-tools easy"

for trace in $NUMS; do
	echo -n "$trace\t"
	result="0"
	trace="traces-$trace.flow"
	cat $trace > /dev/null
	for i in `seq 1 10`; do
		result="$result+`flow-cat $trace | time -f "%e" flow-nfilter -ffilter-easy.ft -Ffoo 2>&1 > /dev/null`"
	done
	echo "scale=2;($result)/10" | bc
done

echo "benchmarking flow-tools medium"

for trace in $NUMS; do
	echo -n "$trace\t"
	result="0"
	trace="traces-$trace.flow"
	cat $trace > /dev/null
	for i in `seq 1 10`; do
		result="$result+`flow-cat $trace | time -f "%e" flow-nfilter -ffilter-medium.ft -Ffoo 2>&1 > /dev/null`"
	done
	echo "scale=2;($result)/10" | bc
done

echo "benchmarking flow-tools hard"

for trace in $NUMS; do
	echo -n "$trace\t"
	result="0"
	trace="traces-$trace.flow"
	cat $trace > /dev/null
	for i in `seq 1 10`; do
		result="$result+`flow-cat $trace | time -f "%e" flow-nfilter -ffilter-hard.ft -Ffoo 2>&1 > /dev/null`"
	done
	echo "scale=2;($result)/10" | bc
done

echo "benchmarking flowy2 easy"

for trace in $NUMS; do
	echo -n "$trace\t"
	result="0"
	trace="traces-$trace.flow"
	cat $trace > /dev/null
	for i in `seq 1 10`; do
		result="$result+`time -f "%e" ./filter-easy 2>&1 > /dev/null < $trace`"
	done
	echo "scale=2;($result)/10" | bc
done

echo "benchmarking flowy2 medium"

for trace in $NUMS; do
	echo -n "$trace\t"
	result="0"
	trace="traces-$trace.flow"
	cat $trace > /dev/null
	for i in `seq 1 10`; do
		result="$result+`time -f "%e" ./filter-medium 2>&1 > /dev/null < $trace`"
	done
	echo "scale=2;($result)/10" | bc
done

echo "benchmarking flowy2 hard"

for trace in $NUMS; do
	echo -n "$trace\t"
	result="0"
	trace="traces-$trace.flow"
	cat $trace > /dev/null
	for i in `seq 1 10`; do
		result="$result+`time -f "%e" ./filter-hard 2>&1 > /dev/null < $trace`"
	done
	echo "scale=2;($result)/10" | bc
done
