# -------------------------------------------------------------------
#  Mini-RISC Embedded Software Makefile
#
#  Usage :
#      make TEST=1
#      make TEST=2 exec
# -------------------------------------------------------------------

# Vérification du paramètre TEST
ifndef TEST
$(error Usage: make TEST=1  |  make TEST=2)
endif

# -------------------------------------------------------------------
# Chemins
# -------------------------------------------------------------------
ROOT_DIR := ..
TEST_DIR := $(ROOT_DIR)/embedded_software/test_$(TEST)
BUILD    := $(TEST_DIR)/build

# Fichiers source
SRCS := $(wildcard $(TEST_DIR)/*.S)
TARGET = esw
OBJS   = $(addprefix $(BUILD)/, $(notdir $(SRCS:.S=.o)))
DEPS   = $(OBJS:.o=.d)

# -------------------------------------------------------------------
# Toolchain
# -------------------------------------------------------------------
TC      = riscv32-MINIRISC-elf
CC      = $(TC)-gcc
LD      = $(TC)-gcc
SIZE    = $(TC)-size
OBJCOPY = $(TC)-objcopy
OBJDUMP = $(TC)-objdump

# Flags
CFLAGS  += -march=rv32im_zicsr -W -Wall -O2
LDFLAGS += -nostartfiles -Wl,-Ttext=0x80000000

# -------------------------------------------------------------------
# Règles
# -------------------------------------------------------------------
.PHONY: all clean exec

all: $(BUILD)/$(TARGET).elf $(BUILD)/$(TARGET).bin $(BUILD)/$(TARGET).lss

-include $(DEPS)

# Compilation du .S
$(BUILD)/%.o: $(TEST_DIR)/%.S
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@ \
		-MMD -MP -MF"$(BUILD)/$*.d"

# Link
$(BUILD)/$(TARGET).elf: $(OBJS)
	$(LD) -o $@ $^ $(CFLAGS) $(LDFLAGS)
	@echo "────────────── SIZE ──────────────"
	@$(SIZE) $@
	@echo "──────────────────────────────────"

# Formats binaires
$(BUILD)/$(TARGET).bin: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(BUILD)/$(TARGET).lss: $(BUILD)/$(TARGET).elf
	$(OBJDUMP) -D $< > $@

# Nettoyage
clean:
	rm -rf $(BUILD)
