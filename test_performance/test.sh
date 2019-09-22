#!/bin/sh
FILE_DIR="../../../input_files"

INPUT_FILES="$FILE_DIR/int64.in
$FILE_DIR/int128.in
$FILE_DIR/int256.in
$FILE_DIR/int1024.in
$FILE_DIR/float64.in
$FILE_DIR/float128.in
$FILE_DIR/float256.in
$FILE_DIR/float1024.in"

OP_LIST="ad
mm
sm
tr
ts"

TEST_NUM=5

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
		for test_num in {1..$TEST_NUM}
		do
			$CMD --$op -t 4 -f $f $f | tail -2 >> $f_$test_num.out
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
		for test_num in {1..$TEST_NUM}
		do
			$CMD --$op -t 1 -f $f $f | tail -2 >> $f_$test_num.out
		done
	done
	cd ..
done
cd ../..
