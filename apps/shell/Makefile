APP_OUT = ./../../fs/test_app2

LINKER 	= linker.ld

SRC_DIR   = src
BUILD_DIR = build

SRC_ASM = $(shell find $(SRC_DIR) -name *.asm)
SRC_C   = $(shell find $(SRC_DIR) -name *.c)

HEADERS = $(shell find $(SRC_DIR) -name *.h)

OBJ_ASM = $(patsubst $(SRC_DIR)/%.asm, $(BUILD_DIR)/%.o, $(SRC_ASM))
OBJ_C   = $(patsubst $(SRC_DIR)/%.c,   $(BUILD_DIR)/%.o, $(SRC_C))

ASM = nasm
CC  = gcc

all: $(APP_OUT)

$(OBJ_ASM): $(BUILD_DIR)/%.o : $(SRC_DIR)/%.asm
	mkdir -p $(dir $@) && \
	$(ASM) -f elf32 $(patsubst $(BUILD_DIR)/%.o, $(SRC_DIR)/%.asm, $@) -o $@

$(OBJ_C): $(BUILD_DIR)/%.o : $(SRC_DIR)/%.c $(HEADERS)
	mkdir -p $(dir $@) && \
	$(CC) -I../../apolib/include -fno-pie -c -m32 -ffreestanding $(patsubst $(BUILD_DIR)/%.o, $(SRC_DIR)/%.c, $@) -o $@

$(APP_OUT): $(OBJ_ASM) $(OBJ_C) $(LINKER)
	mkdir -p $(dir $(APP_OUT)) && \
	ld -m elf_i386 -T linker.ld -n -o $(APP_OUT) ../../apolib/dist/apolib.o $(OBJ_C) $(OBJ_ASM)