CC       = gcc

VPATH = $(SRC_DIR1) : $(SRC_DIR2) 

SRC_DIR1 = ../../../../code/encode
SRC_DIR2 = ../../../../code/common
INC_DIR1 = ../../../../code/encode
INC_DIR2 = ../../../../code/common
OBJ_DIR  = ./obj

SOURCES  = $(notdir $(wildcard $(SRC_DIR1)/*.c $(SRC_DIR2)/*.c))
OBJS     = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SOURCES))

CFLAGS   = -O3 -fPIC -m64 -msse4.2 -D_LINUX64 -D_M_X64 -I$(INC_DIR1) -I$(INC_DIR2)

EXECUTABLE = hlmc_linux64.a

all: $(EXECUTABLE)

$(OBJS): $(OBJ_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(EXECUTABLE) : $(OBJS)
	ar rc $(EXECUTABLE) $(OBJS)

.PHONY: clean
clean:
	rm -f $(OBJ_DIR)/*.o $(EXECUTABLE)

