PROJECT = os-test

ISO_OUT = dist/apostrof.iso

LINKER 	= linker.ld

SRC_DIR   = src
BUILD_DIR = build

SRC_ASM = $(SRC_DIR)/boot/header.asm $(SRC_DIR)/boot/main.asm $(SRC_DIR)/kernel/int/isr_a.asm  $(SRC_DIR)/kernel/cpu/cpuid_a.asm #$(shell find $(SRC_DIR) -name *.asm)
SRC_C   = $(shell find $(SRC_DIR) -name *.c)
SRC_CXX = $(shell find $(SRC_DIR) -name *.cpp)

HEADERS = $(shell find $(SRC_DIR) -name *.h -name *.hpp)

OBJ_ASM = $(patsubst $(SRC_DIR)/%.asm, $(BUILD_DIR)/%.o, $(SRC_ASM))
OBJ_C   = $(patsubst $(SRC_DIR)/%.c,   $(BUILD_DIR)/%.c.o, $(SRC_C))
OBJ_CXX = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.cpp.o, $(SRC_CXX))

APP_FILES = $(shell find apps)

BOOT_ASM = bootsector.asm

BOOT_BIN   = dist/bootsector.bin
ROM_BIN    = dist/rom_file.bin
KERNEL_BIN = iso/boot/kernel.bin

FS_DATA = fs_data.bin

TEST_APP_ASM  = fs/test/app.asm
TEST_APP_OUT  = fs/test/app

TEST_TASK_ASM = fs/test/task.asm
TEST_TASK_OUT = fs/test/task

ASM = nasm
CC  = gcc
CXX = g++

all: $(ISO_OUT)

$(BOOT_BIN): $(BOOT_ASM)
	mkdir -p $(dir $(BOOT_BIN)) && \
	$(ASM) $(BOOT_ASM) -o $(BOOT_BIN)

$(OBJ_ASM): $(BUILD_DIR)/%.o : $(SRC_DIR)/%.asm
	mkdir -p $(dir $@) && \
	$(ASM) -f elf32 -F dwarf -g $(patsubst $(BUILD_DIR)/%.o, $(SRC_DIR)/%.asm, $@) -o $@

$(OBJ_C): $(BUILD_DIR)/%.o : $(SRC_DIR)/% $(HEADERS)
	mkdir -p $(dir $@) && \
	$(CC) -g -fno-pie -c -m32 -ffreestanding $(patsubst $(BUILD_DIR)/%.c.o, $(SRC_DIR)/%.c, $@) -o $@

$(OBJ_CXX): $(BUILD_DIR)/%.o : $(SRC_DIR)/% $(HEADERS)
	mkdir -p $(dir $@) && \
	$(CXX) -g -fno-exceptions -fno-rtti -fno-pie -c -m32 -ffreestanding $(patsubst $(BUILD_DIR)/%.cpp.o, $(SRC_DIR)/%.cpp, $@) -o $@

$(KERNEL_BIN): $(OBJ_ASM) $(OBJ_C) $(OBJ_CXX) $(LINKER)
	mkdir -p $(dir $(KERNEL_BIN)) && \
	echo $(shell find src -name *.asm) && \
	ld -m elf_i386 -n -o $(KERNEL_BIN) -T $(LINKER) $(OBJ_ASM) $(OBJ_C) $(OBJ_CXX)

$(TEST_APP_OUT): $(TEST_APP_ASM)
	$(ASM) -f bin $(TEST_APP_ASM) -o $(TEST_APP_OUT)

$(TEST_TASK_OUT): $(TEST_TASK_ASM)
	$(ASM) -f bin $(TEST_TASK_ASM) -o $(TEST_TASK_OUT)

$(ISO_OUT): $(BOOT_BIN) $(KERNEL_BIN) $(TEST_APP_OUT) $(TEST_TASK_OUT) $(APP_FILES)
	cd apolib && make && cd ..
	cd apps/test_app && make && cd ../..
	cd apps/test_app2 && make && cd ../..
	cd apps/terminal && make && cd ../..
	grub-mkrescue /usr/lib/grub/i386-pc -o $(ISO_OUT) ./iso
#	cat $(BOOT_BIN) $(KERNEL_BIN) > $(ROM_BIN)
#	node ./fs_converter.js

clean:
	rm -rf $(BUILD_DIR) $(KERNEL_BOOT) dist