#!/bin/bash
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

THRD_DIR="results/thread4"

CMD="./../../../../build/smops"

#FIRST DO TESTS FOR THREADING (NUM OF THREADS 4)

cd $THRD_DIR
for op in $OP_LIST
do
	cd $op
	for f in $INPUT_FILES
	do
		for i in {1..$TEST_NUM};
		do
			echo "Thread: OP: $op TEST: $i INPUT: $f"
			$CMD --$op -t 4 -f $FILE_DIR/$f $FILE_DIR/$f | tail -2 >> $f.out
		done
	done
	cd ..
done

cd ../..

#DO TEST FOR SEQUENTIAL
cd $SEQ_DIR
for op in $OP_LIST
do
	cd $op
	for f in $INPUT_FILES
	do
		for i in {1..$TEST_NUM};
		do
			echo "Sequential: OP: $op TEST: $i INPUT: $f"
			$CMD --$op -t 1 -f $FILE_DIR/$f $FILE_DIR/$f | tail -2 >> $f.out
		done
	done
	cd ..
done
cd ../..
python3 graph_result.py
