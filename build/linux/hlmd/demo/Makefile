CC       = gcc

VPATH = $(SRC_DIR1) : $(SRC_DIR2) : $(SRC_DIR3)

INC_DIR1  = ../../../../apps/demo_dec
INC_DIR2  = ../../../../code/common
INC_DIR3  = ../../../../code/decode
SRC_DIR1  = ../../../../apps/demo_dec
SRC_DIR2  = ../../../../code/common
SRC_DIR3  = ../../../../code/decode
OBJ_DIR   = ./obj

SOURCES  = $(notdir $(wildcard $(SRC_DIR1)/*.c $(SRC_DIR2)/*.c $(SRC_DIR3)/*.c))

OBJS     = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SOURCES))

CFLAGS   = -O3 -m64 -mavx -mavx2 -msse4.2 -D_LINUX64 -D_M_X64 -I$(INC_DIR1) -I$(INC_DIR2) -I$(INC_DIR3) 

EXECUTABLE = ../demo_hlmd_linux64

all: $(EXECUTABLE)

$(OBJS): $(OBJ_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(EXECUTABLE) : $(OBJS) ../lib/hlmd_linux64.a
	$(CC) -o $(EXECUTABLE) $(OBJS) ../lib/hlmd_linux64.a -lm


.PHONY: clean
clean:
	rm -f $(OBJ_DIR)/*.o $(EXECUTABLE)
