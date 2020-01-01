#!/usr/bin/env bash
FILE_DIR="../../../input_files"

INPUT_FILES="int64.in
int128.in
int256.in
int1024.in
float64.in
float128.in
float256.in
float1024.in"

OP_LIST="ad
mm
sm
tr
ts"

TEST_NUM=10

SEQ_DIR="results/sequential"

THRD4_DIR="results/thread4"

THRD8_DIR="results/thread8"

CMD="./../../../../build/smops"

#Build SMOPS first
cd ..
make

cd test_performance

#8 Thread Test
cd $THRD8_DIR
for op in $OP_LIST
do
	cd $op
	for f in $INPUT_FILES
	do
		for i in {1..10};
		do
			echo "Thread8: OP: $op TEST: $i INPUT: $f"
			$CMD --$op -t 8 -f $FILE_DIR/$f $FILE_DIR/$f | tail -2 >> $f.out
		done
	done
	cd ..
done

cd ../..

#4 Thread Test

cd $THRD4_DIR
for op in $OP_LIST
do
	cd $op
	for f in $INPUT_FILES
	do
		for i in {1..10};
		do
			echo "Thread4: OP: $op TEST: $i INPUT: $f"
			$CMD --$op -t 4 -f $FILE_DIR/$f $FILE_DIR/$f | tail -2 >> $f.out
		done
	done
	cd ..
done

cd ../..

#Sequential Test
cd $SEQ_DIR
for op in $OP_LIST
do
	cd $op
	for f in $INPUT_FILES
	do
		for i in {1..10};
		do
			echo "Sequential: OP: $op TEST: $i INPUT: $f"
			$CMD --$op -t 1 -f $FILE_DIR/$f $FILE_DIR/$f | tail -2 >> $f.out
		done
	done
	cd ..
done
cd ../..

python3 graph_result.py >> results/speed_up.txt
