PROJECT = os-test

IMG_OUT = dist/os-test.img

LINKER 	= linker.ld

SRC_DIR   = src
BUILD_DIR = build

SRC_ASM = $(SRC_DIR)/boot/main.asm $(SRC_DIR)/kernel/int/isr_a.asm  $(SRC_DIR)/kernel/cpu/cpuid_a.asm #$(shell find $(SRC_DIR) -name *.asm)
SRC_C   = $(shell find $(SRC_DIR) -name *.c)

HEADERS = $(shell find $(SRC_DIR) -name *.h)

OBJ_ASM = $(patsubst $(SRC_DIR)/%.asm, $(BUILD_DIR)/%.o, $(SRC_ASM))
OBJ_C   = $(patsubst $(SRC_DIR)/%.c,   $(BUILD_DIR)/%.o, $(SRC_C))

BOOT_ASM = bootsector.asm

BOOT_BIN   = dist/bootsector.bin
KERNEL_BIN = dist/kernel.bin
ROM_BIN    = dist/rom_file.bin

FS_DATA = fs_data.bin

TEST_APP_ASM  = fs/test/app.asm
TEST_APP_OUT  = fs/test/app

TEST_TASK_ASM = fs/test/task.asm
TEST_TASK_OUT = fs/test/task

ASM = nasm
CC  = gcc

all: $(IMG_OUT)

$(BOOT_BIN): $(BOOT_ASM)
	mkdir -p $(dir $(BOOT_BIN)) && \
	$(ASM) $(BOOT_ASM) -o $(BOOT_BIN)

$(OBJ_ASM): $(BUILD_DIR)/%.o : $(SRC_DIR)/%.asm
	mkdir -p $(dir $@) && \
	$(ASM) -f elf32 $(patsubst $(BUILD_DIR)/%.o, $(SRC_DIR)/%.asm, $@) -o $@

$(OBJ_C): $(BUILD_DIR)/%.o : $(SRC_DIR)/%.c $(HEADERS)
	mkdir -p $(dir $@) && \
	$(CC) -fno-pie -c -m32 -ffreestanding $(patsubst $(BUILD_DIR)/%.o, $(SRC_DIR)/%.c, $@) -o $@

$(KERNEL_BIN): $(OBJ_ASM) $(OBJ_C) $(LINKER)
	mkdir -p $(dir $(KERNEL_BIN)) && \
	echo $(shell find src -name *.asm) && \
	ld -m elf_i386 -n -o $(KERNEL_BIN) -T $(LINKER) $(OBJ_ASM) $(OBJ_C)

$(TEST_APP_OUT): $(TEST_APP_ASM)
	$(ASM) -f bin $(TEST_APP_ASM) -o $(TEST_APP_OUT)

$(TEST_TASK_OUT): $(TEST_TASK_ASM)
	$(ASM) -f bin $(TEST_TASK_ASM) -o $(TEST_TASK_OUT)

$(IMG_OUT): $(BOOT_BIN) $(KERNEL_BIN) $(TEST_APP_OUT) $(TEST_TASK_OUT)
	cat $(BOOT_BIN) $(KERNEL_BIN) > $(ROM_BIN)
	cd apps/test_app && make && cd ../..
	node ./fs_converter.js



clean:
	rm -rf $(BUILD_DIR) $(KERNEL_BOOT) dist