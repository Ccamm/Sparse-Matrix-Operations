#!/usr/bin/env python3
import matplotlib.pyplot as plt
import numpy as np

sv_fileload_dir = "results/graphs/fileload"
sv_operation_dir = "results/graphs/operation"

seq_results = "results/sequential"
thrd4_results =  "results/thread4"

ops_dict = {
    "ad": "Addition",
    "mm": "Matrix Multiplication",
    "sm": "Scalar Multiplication",
    "tr": "Transpose",
    "ts": "Trace"
}

float_str = "float"
int_str = "int"

types = [float_str, int_str]

suffix = ".in.out"

x_axis_dict = { 64: 0, 128: 1, 256: 2, 1024: 3}

def get_speed_up(results, row_seq, row_thrd):
    speed_up = results[row_seq][3]/results[row_thrd][3]
    return "{:1.4f}".format(speed_up)

def plot_graphs(results, op, typ):
    x = np.array([64, 128, 256, 1024], dtype=int)
    fig, ax = plt.subplots()
    seq_load, = ax.plot(x[:], results[0,:], 'r--', label='Sequential')
    thrd_load, = ax.plot(x[:], results[2,:], 'b-', label='Threaded')
    ax.legend((seq_load, thrd_load), ('Sequential', 'Threaded'))
    ax.set_title(ops_dict[op] + " Load Times Type: " + typ + "  Speed Up: " +
    get_speed_up(results, 0, 2))
    ax.set_xlabel("Matrix Dimension")
    ax.set_ylabel("Time (s)")
    ax.set_xscale("log")
    ax.set_yscale("log")
    fig.savefig(sv_fileload_dir+"/"+op+typ+".png")

    fig, ax = plt.subplots()

    seq_load, = ax.plot(x[:], results[1,:], 'r--', label='Sequential')
    thrd_load, = ax.plot(x[:], results[3,:], 'b-', label='Threaded')
    ax.legend((seq_load, thrd_load), ('Sequential', 'Threaded'))
    ax.set_title(ops_dict[op] + " Operation Times Type: " +typ + " Speed Up: " +
    get_speed_up(results, 1, 3))
    ax.set_xlabel("Matrix Dimension")
    ax.set_ylabel("Time (s)")
    ax.set_xscale("log")
    ax.set_yscale("log")
    fig.savefig(sv_operation_dir+"/"+op+typ+".png")


for op in ops_dict:
    for typ in types:
        results = np.zeros((4,4), dtype=float)
        for data_siz in x_axis_dict:
            f = open(seq_results+"/"+op+"/"+typ+str(data_siz)+suffix, "r")
            for i in range(0,10):
                row = 1
                if (i%2 == 0): row = 0
                results[row][x_axis_dict[data_siz]] += float(f.readline()[:-1])
            f.close()
            f = open(thrd4_results+"/"+op+"/"+typ+str(data_siz)+suffix, "r")
            for i in range(0,10):
                row = 3
                if (i%2 == 0): row = 2
                results[row][x_axis_dict[data_siz]] += float(f.readline()[:-1])
            f.close()

            for i in range(0,4):
                for j in range(0,4):
                    results[i][j] = results[i][j]/10;
        plot_graphs(results, op, typ)
