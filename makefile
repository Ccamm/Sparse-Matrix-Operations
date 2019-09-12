GCC := gcc -std=c99 -Wall -pedantic -Werror

SRC_DIR := src
LIB_DIR := $(SRC_DIR)/lib
BIN_DIR := bin

LIB_SRCS := $(LIB_DIR)/smops_ctx.c
SRCS := $(SRC_DIR)/main.c
LIB_HDR := $(LIB_DIR)/smopslib.h

$(BIN_DIR)/smops: $(BIN_DIR)/. $(BIN_DIR)/smopslib.o $(SRCS) 
	$(GCC) -o $@ $(BIN_DIR)/smopslib.o $(SRCS) -lm -fopenmp

$(BIN_DIR)/smopslib.o : $(BIN_DIR)/. $(LIB_SRCS) $(LIB_HDR)
	$(GCC) -o $@ -c $(LIB_SRCS) -lm -fopenmp

$(BIN_DIR)/.:
	mkdir $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR)
