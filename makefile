GCC := gcc -std=c99 -Wall -pedantic -Werror

SRC_DIR := src
LIB_DIR := $(SRC_DIR)/lib
OP_DIR := $(LIB_DIR)/ops

BUILD_DIR := build
BIN_DIR := $(BUILD_DIR)/bin
LIB_BIN_DIR := $(BIN_DIR)/lib
OP_BIN_DIR := $(LIB_BIN_DIR)/ops

LIB_HDR := $(LIB_DIR)/smopslib.h

LIB_SRCS := $(LIB_DIR)/smops_ctx.c $(LIB_DIR)/smops_matrix.c $(LIB_BIN_DIR)/smops_load.c\
$(LIB_DIR)/smops_data.c $(LIB_DIR)/smops_result.c
LIB_OBJS := $(LIB_BIN_DIR)/smops_ctx.o $(LIB_BIN_DIR)/smops_matrix.o $(LIB_BIN_DIR)/smops_load.o\
$(LIB_BIN_DIR)/smops_data.o $(LIB_BIN_DIR)/smops_result.o

OP_SRCS := $(OP_DIR)/smops_ops.c $(OP_DIR)/smops_tr.c $(OP_DIR)/smops_ts.c\
$(OP_DIR)/smops_sm.c $(OP_DIR)/smops_ad.c $(OP_DIR)/smops_mm.c
OP_OBJS := $(OP_BIN_DIR)/smops_ops.o $(OP_BIN_DIR)/smops_tr.o $(OP_BIN_DIR)/smops_ts.o\
$(OP_BIN_DIR)/smops_sm.o $(OP_BIN_DIR)/smops_ad.o $(OP_BIN_DIR)/smops_mm.o

SRCS := $(SRC_DIR)/main.c
SRC_OBJS := $(BIN_DIR)/main.o


$(BUILD_DIR)/smops: $(BUILD_DIR)/. $(BUILD_DIR)/libsmops.a $(SRC_OBJS)
	$(GCC) -o $@ $(SRC_OBJS) -L$(BUILD_DIR)/ -lsmops -fopenmp

$(BUILD_DIR)/libsmops.a : $(BUILD_DIR)/. $(LIB_OBJS) $(OP_OBJS)
	ar -cvq $@ $(LIB_OBJS) $(OP_OBJS)

$(LIB_BIN_DIR)/smops_ctx.o: $(LIB_BIN_DIR)/. $(LIB_DIR)/smops_ctx.c
	$(GCC) -o $@ -c $(LIB_DIR)/smops_ctx.c

$(LIB_BIN_DIR)/smops_load.o : $(LIB_BIN_DIR)/. $(LIB_DIR)/smops_load.c
	$(GCC) -o $@ -c $(LIB_DIR)/smops_load.c -fopenmp

$(LIB_BIN_DIR)/smops_data.o : $(LIB_BIN_DIR)/. $(LIB_DIR)/smops_data.c
	$(GCC) -o $@ -c $(LIB_DIR)/smops_data.c -fopenmp

$(LIB_BIN_DIR)/smops_matrix.o: $(LIB_BIN_DIR)/. $(LIB_DIR)/smops_matrix.c
	$(GCC) -o $@ -c $(LIB_DIR)/smops_matrix.c

$(LIB_BIN_DIR)/smops_result.o: $(LIB_BIN_DIR)/. $(LIB_DIR)/smops_result.c
	$(GCC) -o $@ -c $(LIB_DIR)/smops_result.c

$(OP_BIN_DIR)/smops_ops.o: $(OP_BIN_DIR)/. $(OP_DIR)/smops_ops.c
	$(GCC) -o $@ -c $(OP_DIR)/smops_ops.c

$(OP_BIN_DIR)/smops_tr.o: $(OP_BIN_DIR)/. $(OP_DIR)/smops_tr.c
	$(GCC) -o $@ -c $(OP_DIR)/smops_tr.c -fopenmp

$(OP_BIN_DIR)/smops_ts.o: $(OP_BIN_DIR)/. $(OP_DIR)/smops_ts.c
	$(GCC) -o $@ -c $(OP_DIR)/smops_ts.c -fopenmp

$(OP_BIN_DIR)/smops_sm.o: $(OP_BIN_DIR)/. $(OP_DIR)/smops_sm.c
	$(GCC) -o $@ -c $(OP_DIR)/smops_sm.c -fopenmp

$(OP_BIN_DIR)/smops_ad.o: $(OP_BIN_DIR)/. $(OP_DIR)/smops_ad.c
	$(GCC) -o $@ -c $(OP_DIR)/smops_ad.c -fopenmp

$(OP_BIN_DIR)/smops_mm.o: $(OP_BIN_DIR)/. $(OP_DIR)/smops_mm.c
	$(GCC) -o $@ -c $(OP_DIR)/smops_mm.c -fopenmp

$(BIN_DIR)/main.o: $(BIN_DIR)/. $(SRC_DIR)/main.c
	$(GCC) -o $@ -c $(SRC_DIR)/main.c

$(BUILD_DIR)/.:
	mkdir $(BUILD_DIR)

$(BIN_DIR)/.:
	mkdir -p $(BIN_DIR)

$(LIB_BIN_DIR)/.:
	mkdir -p $(LIB_BIN_DIR)
$(OP_BIN_DIR)/.:
	mkdir -p $(OP_BIN_DIR)

clean:
	rm -rf $(BUILD_DIR)
