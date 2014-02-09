BUILD_DIR = build

.PHONY: all directory clean size

DEVICE = atmega256rfr2
TARGET = buttonTest

AVRDUDE = avrdude
AVRDUDE_PORT = usb
AVRDUDE_PROGRAMMER = avrisp2

CC = avr-gcc
OBJCOPY = avr-objcopy
SIZE = avr-size
MM_DIR = mkdir

CFLAGS += -W -Wall --std=gnu99 -Os
CFLAGS += -fdata-sections -ffunction-sections -fpack-struct -fshort-enums
CFLAGS += -funsigned-char -funsigned-bitfields -fno-strict-aliasing
CFLAGS += -mmcu=atmega256rfr2
CFLAGS += -MD -MP -MT $(BUILD_DIR)/$(*F).o -MF $(BUILD_DIR)/$(@F).d

ifeq ($(BUILD_DIR), Debug)
  CFLAGS += -g
endif

LDFLAGS += -Wl,--gc-sections
LDFLAGS += -mmcu=atmega256rfr2


INCLUDES += \
  -I../../hardware/firmware/megaDrivers \
  -I../../hardware/firmware/utils \
  -I.

SRCS += \
  main.c \
  ../../hardware/firmware/megaDrivers/lcd.c \

DEFINES += \
  -DF_CPU=16000000

DEFINES += -DUSE_STDIO

CFLAGS += $(INCLUDES) $(DEFINES)

OBJS = $(addprefix $(BUILD_DIR)/, $(notdir %/$(subst .c,.o, $(SRCS))))

all: directory $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin size

$(BUILD_DIR)/$(TARGET).elf: $(OBJS)
	@echo LD $@
	@$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).elf
	@echo OBJCOPY $@
	@$(OBJCOPY) -O ihex -R .eeprom $^ $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	@echo OBJCOPY $@
	@$(OBJCOPY) -O binary -R .eeprom $^ $@

%.o:
	@echo CC $@
	@$(CC) $(CFLAGS) $(filter %$(subst .o,.c,$(notdir $@)), $(SRCS)) -c -o $@

directory:
	@mkdir -p $(BUILD_DIR)

size: $(BUILD_DIR)/$(TARGET).elf
	@echo size:
	@$(SIZE) -t $^

clean:
	@echo clean
	@rm -rf $(BUILD_DIR)
	
program: $(BUILD_DIR)/$(TARGET).hex
	@echo programming
	#$(ATPROGRAM) -t $(PROG_INTERFACE) -i $(PROG_PROTOCOL) -d $(DEVICE) -s $(SERIAL_NUMBER) program -c -fl -f $(CURDIR)/$(BUILD_DIR)/$(TARGET).elf
	$(AVRDUDE) -p $(DEVICE) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER) -U flash:w:$(BUILD_DIR)/$(TARGET).hex

-include $(wildcard $(BUILD_DIR)/*.d)
