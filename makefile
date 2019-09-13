GCC := gcc -std=c99 -Wall -pedantic -Werror

SRC_DIR := src
LIB_DIR := $(SRC_DIR)/lib

BUILD_DIR := build
BIN_DIR := $(BUILD_DIR)/bin
LIB_BIN_DIR := $(BIN_DIR)/lib

LIB_HDR := $(LIB_DIR)/smopslib.h

LIB_SRCS := $(LIB_DIR)/smops_ctx.c $(LIB_DIR)/smops_matrix.c
LIB_OBJS := $(LIB_BIN_DIR)/smops_ctx.o $(LIB_BIN_DIR)/smops_matrix.o

SRCS := $(SRC_DIR)/main.c
SRC_OBJS := $(BIN_DIR)/main.o


$(BUILD_DIR)/smops: $(BUILD_DIR)/. $(BUILD_DIR)/libsmops.a $(SRC_OBJS)
	$(GCC) -o $@ $(SRC_OBJS) -L$(BUILD_DIR)/ -lsmops

$(BUILD_DIR)/libsmops.a : $(BUILD_DIR)/. $(LIB_OBJS)
	ar -cvq $@ $(LIB_OBJS)

$(LIB_BIN_DIR)/smops_ctx.o: $(LIB_BIN_DIR)/. $(LIB_DIR)/smops_ctx.c
	$(GCC) -o $@ -c $(LIB_DIR)/smops_ctx.c

$(LIB_BIN_DIR)/smops_matrix.o: $(LIB_BIN_DIR)/. $(LIB_DIR)/smops_matrix.c
	$(GCC) -o $@ -c $(LIB_DIR)/smops_matrix.c

$(BIN_DIR)/main.o: $(BIN_DIR)/. $(SRC_DIR)/main.c
	$(GCC) -o $@ -c $(SRC_DIR)/main.c

$(BUILD_DIR)/.:
	mkdir $(BUILD_DIR)

$(BIN_DIR)/.:
	mkdir -p $(BIN_DIR)

$(LIB_BIN_DIR)/.:
	mkdir -p $(LIB_BIN_DIR)

clean:
	rm -rf $(BUILD_DIR)
