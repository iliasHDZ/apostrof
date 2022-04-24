SRC_DIR     = src
BUILD_DIR   = build
INCLUDE_DIR = include

OBJ_OUT = dist/apolib.o

SRC_ASM = $(shell find $(SRC_DIR) -name *.asm)
SRC_C   = $(shell find $(SRC_DIR) -name *.c)

HEADERS = $(shell find $(SRC_DIR) -name *.h)

OBJ_ASM = $(patsubst $(SRC_DIR)/%.asm, $(BUILD_DIR)/%.o, $(SRC_ASM))
OBJ_C   = $(patsubst $(SRC_DIR)/%.c,   $(BUILD_DIR)/%.o, $(SRC_C))

ASM = nasm
CC  = gcc

all: $(OBJ_OUT)

$(OBJ_ASM): $(BUILD_DIR)/%.o : $(SRC_DIR)/%.asm
	mkdir -p $(dir $@) && \
	$(ASM) -f elf32 $(patsubst $(BUILD_DIR)/%.o, $(SRC_DIR)/%.asm, $@) -o $@

$(OBJ_C): $(BUILD_DIR)/%.o : $(SRC_DIR)/%.c $(HEADERS)
	mkdir -p $(dir $@) && \
	$(CC) -I$(INCLUDE_DIR) -fno-pie -c -m32 -ffreestanding $(patsubst $(BUILD_DIR)/%.o, $(SRC_DIR)/%.c, $@) -o $@

$(OBJ_OUT): $(OBJ_ASM) $(OBJ_C) $(LINKER)
	mkdir -p $(dir $(OBJ_OUT)) && \
	ld -m elf_i386 -relocatable -o $(OBJ_OUT) $(OBJ_C) $(OBJ_ASM)