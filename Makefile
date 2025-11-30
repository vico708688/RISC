TARGET  = emulator
BUILD   = build
INCLUDE = include
SOURCE  = source
HEADER  = include
CFILES  = $(notdir $(wildcard $(SOURCE)/*.c))
OBJ     = $(addprefix $(BUILD)/, $(CFILES:.c=.o))
DEPS    = $(OBJ:.o=.d)
# TODO: résoudre les warnings
CFLAGS += -W -Wall -Wno-unused-variable -Wno-implicit-function-declaration
CFLAGS += -O0 -g
CFLAGS += -I$(INCLUDE)
LDFLAGS = 

all: $(BUILD)/$(TARGET)

.PHONY: clean exec gdb

-include $(DEPS)

$(BUILD)/%.o: $(SOURCE)/%.c
	@mkdir -p $(@D)
	gcc $(CFLAGS) -c -o $@ $< -MMD -MP -MF"$(@:%.o=%.d)"

$(BUILD)/$(TARGET): $(OBJ)
	gcc $(CFLAGS) -o $@ $^ $(LDFLAGS)

exec: $(BUILD)/$(TARGET)
	./$<

# gdb: $(BUILD)/$(TARGET)
#     gdb --tui $<

clean:
	@rm -rvf $(BUILD)