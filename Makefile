#	$@		target file
#	$<		first dependency
#	$^		all dependencies

# --------------------------------------------------------------------------- #
# Configuration										  #
# --------------------------------------------------------------------------- #

.PHONY: all clean run-qemu run-bochs

# Build tools
CC			:= gcc
LD			:= ld
RM			:= rm
QEMUX86		:= qemu-system-i386
BOCHS		:= bochs
ASM			:= nasm
CINC		:= include
CFLAGS		:= -O0 -Wall -Werror -m32 -mno-sse -fno-pie -ffreestanding -fno-stack-protector -nostdinc -fno-asynchronous-unwind-tables -I $(CINC)

# Directories
BUILD_DIR			:= build
OUTPUT_DIR			:= $(BUILD_DIR)/output
ISO_UNPACKED_DIR	:= $(BUILD_DIR)/iso
KERNEL_DIR			:= kernel
ARCH_DIR			:= arch/x86
TEST_DIR			:= tests

DIRS := $(BUILD_DIR) $(OUTPUT_DIR) $(ISO_UNPACKED_DIR) $(ISO_UNPACKED_DIR)/boot/grub \
	$(OUTPUT_DIR)/$(TEST_DIR) $(OUTPUT_DIR)/$(ARCH_DIR) $(OUTPUT_DIR)/$(ARCH_DIR)/boot \
	$(OUTPUT_DIR)/$(ARCH_DIR)/kernel $(OUTPUT_DIR)/$(KERNEL_DIR)

# List of C and object files
KERNEL_SRC		= $(wildcard $(KERNEL_DIR)/*.c)
KERNEL_OBJ		= $(patsubst $(KERNEL_DIR)/%.c,$(OUTPUT_DIR)/kernel/%.o,$(KERNEL_SRC))
ARCH_SRC		= $(wildcard $(ARCH_DIR)/kernel/*.c)
ARCH_OBJ		= $(patsubst $(ARCH_DIR)/kernel/%.c,$(OUTPUT_DIR)/$(ARCH_DIR)/kernel/%.o,$(ARCH_SRC))
TEST_SRC		= $(wildcard $(TEST_DIR)/*.c)
TEST_OBJ		= $(patsubst $(TEST_DIR)/%.c,$(OUTPUT_DIR)/$(TEST_DIR)/%.o,$(TEST_SRC))

# Filenames
FINAL_ISO_FILENAME	:= final-image.iso

# --------------------------------------------------------------------------- #
# Final Output														       	  #
# --------------------------------------------------------------------------- #

# Create the final ISO image which will use GRUB as a bootloader
$(OUTPUT_DIR)/$(FINAL_ISO_FILENAME): $(OUTPUT_DIR)/kernel.bin | $(ISO_UNPACKED_DIR)/boot/grub
	cat $^ > $@
	cp $(OUTPUT_DIR)/kernel.bin $(ISO_UNPACKED_DIR)/boot/os.bin
	grub-mkrescue -o $(OUTPUT_DIR)/$(FINAL_ISO_FILENAME) $(ISO_UNPACKED_DIR)

# --------------------------------------------------------------------------- #
# Source Assembly & Compilation												  #
# --------------------------------------------------------------------------- #

$(DIRS):
	mkdir -p $@

# Assemble our entry code, which will set up some basic page tables before passing
# to the C kernel
$(OUTPUT_DIR)/boot.o: $(ARCH_DIR)/boot/boot.asm | $(DIRS)
	$(ASM) -f elf32 $< -o $@

$(OUTPUT_DIR)/reload-segments.o: $(ARCH_DIR)/kernel/reload-segments.asm
	$(ASM) -f elf32 $< -o $@

$(OUTPUT_DIR)/task-switch.o: $(ARCH_DIR)/kernel/task-switch.asm
	$(ASM) -f elf32 $< -o $@

# Assemble our Interrupt Descriptor Table, which contains assembly stubs to be called
# upon interrupts, which then pass to the C kernel
$(OUTPUT_DIR)/isr.o: $(ARCH_DIR)/kernel/isr.asm
	$(ASM) -f elf32 $< -o $@

# Link together our final kernel image
$(OUTPUT_DIR)/kernel.bin: $(OUTPUT_DIR)/boot.o $(OUTPUT_DIR)/isr.o $(OUTPUT_DIR)/reload-segments.o $(OUTPUT_DIR)/task-switch.o ${KERNEL_OBJ} ${ARCH_OBJ} ${TEST_OBJ}
	$(LD) -Map $(OUTPUT_DIR)/linker.map -m elf_i386 -o $@ -T linker.ld $^

# Compile our kernel C files
$(OUTPUT_DIR)/kernel/%.o: $(KERNEL_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile our architecture-specific kernel C files
$(OUTPUT_DIR)/$(ARCH_DIR)/kernel/%.o: $(ARCH_DIR)/kernel/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile our kernel test C files
$(OUTPUT_DIR)/tests/%.o: $(TEST_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

all: $(OUTPUT_DIR)/$(FINAL_ISO_FILENAME)

clean:
	rm -rf $(OUTPUT_DIR)

# --------------------------------------------------------------------------- #
# Execution									  #
# --------------------------------------------------------------------------- #

run-qemu:
	$(QEMUX86) -cdrom $(OUTPUT_DIR)/$(FINAL_ISO_FILENAME) -monitor stdio -d cpu_reset

run-bochs:
	$(BOCHS) -f bochs.config -q -rc bochs.rc

# --------------------------------------------------------------------------- #
