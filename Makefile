TARGET  = emulator
BUILD   = $(EMUDIR)/build
EMUDIR  = emulator
INCLUDE = $(EMUDIR)/include
SOURCE  = $(EMUDIR)/source
CFILES  = $(notdir $(wildcard $(SOURCE)/*.c))
OBJ     = $(addprefix $(BUILD)/, $(CFILES:.c=.o))
DEPS    = $(OBJ:.o=.d)

CFLAGS += -W -Wall -Werror
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

clean:
	@rm -rvf $(BUILD)
